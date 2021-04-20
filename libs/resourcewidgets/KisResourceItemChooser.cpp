/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
   SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
   SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
   SPDX-FileCopyrightText: 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
   SPDX-FileCopyrightText: 2011 José Luis Vergara <pentalis@gmail.com>
   SPDX-FileCopyrightText: 2013 Sascha Suelzer <s.suelzer@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "KisResourceItemChooser.h"

#include <math.h>

#include <QGridLayout>
#include <QButtonGroup>
#include <QHeaderView>
#include <QAbstractProxyModel>
#include <QLabel>
#include <QScrollArea>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QSplitter>
#include <QToolButton>
#include <QWheelEvent>
#include <QLineEdit>
#include <QComboBox>

#include <klocalizedstring.h>

#include <KoIcon.h>
#include <KoFileDialog.h>
#include <KisKineticScroller.h>
#include <KisMimeDatabase.h>
#include "KisPopupButton.h"

#include <KisResourceModel.h>
#include <KisTagFilterResourceProxyModel.h>
#include <KisResourceLoaderRegistry.h>

#include "KisResourceItemListView.h"
#include "KisResourceItemDelegate.h"
#include "KisTagFilterWidget.h"
#include "KisTagChooserWidget.h"
#include "KisResourceItemChooserSync.h"
#include "KisResourceTaggingManager.h"



#include "KisStorageChooserWidget.h"
#include "kis_assert.h"


class Q_DECL_HIDDEN KisResourceItemChooser::Private
{
public:
    Private(QString _resourceType)
        : resourceType(_resourceType)
    {}

    QString resourceType;

    KisTagFilterResourceProxyModel *tagFilterProxyModel {0};

    KisResourceTaggingManager *tagManager {0};
    KisResourceItemListView *view {0};
    QButtonGroup *buttonGroup {0};
    KisPopupButton *viewModeButton {0};
    KisStorageChooserWidget *storagePopupButton {0};

    QScrollArea *previewScroller {0};
    QLabel *previewLabel {0};
    QSplitter *splitter {0};
    QGridLayout *buttonLayout {0};

    QToolButton *importButton {0};
    QToolButton *deleteButton {0};

    bool usePreview {false};
    bool tiledPreview {false};
    bool grayscalePreview {false};
    bool synced {false};
    bool updatesBlocked {false};

    QModelIndex savedResourceWhileReset; // Indexes on the proxyModel, not the source resource model

    QList<QAbstractButton*> customButtons;

    KoResourceSP currentResource;
};

KisResourceItemChooser::KisResourceItemChooser(const QString &resourceType, bool usePreview, QWidget *parent)
    : QWidget(parent)
    , d(new Private(resourceType))
{
    d->splitter = new QSplitter(this);

    d->view = new KisResourceItemListView(this);
    d->view->setObjectName("ResourceItemview");

    if (d->resourceType == ResourceType::Gradients) {
        d->view->setFixedToolTipThumbnailSize(QSize(256, 64));
        d->view->setToolTipShouldRenderCheckers(true);
    }
    else if (d->resourceType == ResourceType::PaintOpPresets) {
        d->view->setFixedToolTipThumbnailSize(QSize(128, 128));
    }

    d->view->setItemDelegate(new KisResourceItemDelegate(this));
    d->view->setSelectionMode(QAbstractItemView::SingleSelection);
    d->view->viewport()->installEventFilter(this);

    d->tagFilterProxyModel = new KisTagFilterResourceProxyModel(resourceType, this);
    d->view->setModel(d->tagFilterProxyModel);
    d->tagFilterProxyModel->sort(Qt::DisplayRole);

    connect(d->tagFilterProxyModel, SIGNAL(beforeFilterChanges()), this, SLOT(beforeFilterChanges()));
    connect(d->tagFilterProxyModel, SIGNAL(afterFilterChanged()), this, SLOT(afterFilterChanged()));

    connect(d->view, SIGNAL(currentResourceChanged(QModelIndex)), this, SLOT(activated(QModelIndex)));
    connect(d->view, SIGNAL(currentResourceClicked(QModelIndex)), this, SLOT(clicked(QModelIndex)));
    connect(d->view, SIGNAL(contextMenuRequested(QPoint)), this, SLOT(contextMenuRequested(QPoint)));
    connect(d->view, SIGNAL(sigSizeChanged()), this, SLOT(updateView()));

    d->splitter->addWidget(d->view);
    d->splitter->setStretchFactor(0, 2);

    d->usePreview = usePreview;
    if (d->usePreview) {
        d->previewScroller = new QScrollArea(this);
        d->previewScroller->setWidgetResizable(true);
        d->previewScroller->setBackgroundRole(QPalette::Dark);
        d->previewScroller->setVisible(true);
        d->previewScroller->setAlignment(Qt::AlignCenter);
        d->previewLabel = new QLabel(this);
        d->previewScroller->setWidget(d->previewLabel);
        d->splitter->addWidget(d->previewScroller);

        if (d->splitter->count() == 2) {
            d->splitter->setSizes(QList<int>() << 280 << 160);
        }

        QScroller* scroller = KisKineticScroller::createPreconfiguredScroller(d->previewScroller);
        if (scroller) {
            connect(scroller, SIGNAL(stateChanged(QScroller::State)), this, SLOT(slotScrollerStateChanged(QScroller::State)));
        }
    }

    d->splitter->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    connect(d->splitter, SIGNAL(splitterMoved(int,int)), SIGNAL(splitterMoved()));

    d->buttonGroup = new QButtonGroup(this);
    d->buttonGroup->setExclusive(false);

    QGridLayout *layout = new QGridLayout(this);

    d->buttonLayout = new QGridLayout();

    d->importButton = new QToolButton(this);
    d->importButton->setToolTip(i18nc("@info:tooltip", "Import resource"));
    d->importButton->setAutoRaise(true);
    d->importButton->setEnabled(true);
    d->buttonGroup->addButton(d->importButton, Button_Import);
    d->buttonLayout->addWidget(d->importButton, 0, 0);

    d->deleteButton = new QToolButton(this);
    d->deleteButton->setToolTip(i18nc("@info:tooltip", "Delete resource"));
    d->deleteButton->setEnabled(false);
    d->deleteButton->setAutoRaise(true);
    d->buttonGroup->addButton(d->deleteButton, Button_Remove);
    d->buttonLayout->addWidget(d->deleteButton, 0, 1);

    connect(d->buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotButtonClicked(int)));

    d->buttonLayout->setColumnStretch(0, 1);
    d->buttonLayout->setColumnStretch(1, 1);
    d->buttonLayout->setColumnStretch(2, 2);
    d->buttonLayout->setSpacing(0);
    d->buttonLayout->setMargin(0);

    d->viewModeButton = new KisPopupButton(this);
    d->viewModeButton->setVisible(false);
    d->viewModeButton->setArrowVisible(false);
    d->tagManager = new KisResourceTaggingManager(resourceType, d->tagFilterProxyModel, this);

    d->storagePopupButton = new KisStorageChooserWidget(this);
    d->storagePopupButton->setToolTip(i18n("Storage Resources"));
    d->storagePopupButton->setFlat(true);
    d->storagePopupButton->setArrowVisible(false);

    layout->addWidget(d->tagManager->tagChooserWidget(), 0, 0);
    layout->addWidget(d->viewModeButton, 0, 1);
    layout->addWidget(d->storagePopupButton, 0, 2);
    layout->addWidget(d->splitter, 1, 0, 1, 3);
    layout->addWidget(d->tagManager->tagFilterWidget(), 2, 0, 1, 3);
    layout->addLayout(d->buttonLayout, 3, 0, 1, 3);
    layout->setMargin(0);
    layout->setSpacing(0);

    updateView();

    updateButtonState();
    showTaggingBar(false);
    activated(d->view->model()->index(0, 0));
}

KisResourceItemChooser::~KisResourceItemChooser()
{
    disconnect();
    delete d;
}

KisTagFilterResourceProxyModel *KisResourceItemChooser::tagFilterModel() const
{
    return d->tagFilterProxyModel;
}

void KisResourceItemChooser::slotButtonClicked(int button)
{
    if (button == Button_Import) {
        QStringList mimeTypes = KisResourceLoaderRegistry::instance()->mimeTypes(d->resourceType);
        KoFileDialog dialog(0, KoFileDialog::OpenFiles, "OpenDocument");
        dialog.setMimeTypeFilters(mimeTypes);
        dialog.setCaption(i18nc("@title:window", "Choose File to Add"));
        Q_FOREACH(const QString &filename, dialog.filenames()) {
            if (QFileInfo(filename).exists() && QFileInfo(filename).isReadable()) {
                tagFilterModel()->importResourceFile(filename);
            }
        }
        tagFilterModel()->sort(Qt::DisplayRole);
    }
    else if (button == Button_Remove) {
        QModelIndex index = d->view->currentIndex();
        if (index.isValid()) {
            d->tagFilterProxyModel->setResourceInactive(index);
        }
        int row = index.row();
        int rowMin = --row;
        row = qBound(0, rowMin, row);
        setCurrentItem(row);
        activated(d->tagFilterProxyModel->index(row, index.column()));
    }
    updateButtonState();
}

void KisResourceItemChooser::showButtons(bool show)
{
    foreach (QAbstractButton * button, d->buttonGroup->buttons()) {
        show ? button->show() : button->hide();
    }

    Q_FOREACH (QAbstractButton *button, d->customButtons) {
        show ? button->show() : button->hide();
    }
}

void KisResourceItemChooser::addCustomButton(QAbstractButton *button, int cell)
{
    d->buttonLayout->addWidget(button, 0, cell);
    d->buttonLayout->setColumnStretch(2, 1);
    d->buttonLayout->setColumnStretch(3, 1);
}

void KisResourceItemChooser::showTaggingBar(bool show)
{
    d->tagManager->showTaggingBar(show);

}

int KisResourceItemChooser::rowCount() const
{
    return d->view->model()->rowCount();
}

void KisResourceItemChooser::setRowHeight(int rowHeight)
{
    d->view->setItemSize(QSize(d->view->gridSize().width(), rowHeight));
}

void KisResourceItemChooser::setColumnWidth(int columnWidth)
{
    d->view->setItemSize(QSize(columnWidth, d->view->gridSize().height()));
}

void KisResourceItemChooser::setItemDelegate(QAbstractItemDelegate *delegate)
{
    d->view->setItemDelegate(delegate);
}

KoResourceSP KisResourceItemChooser::currentResource() const
{
    QModelIndex index = d->view->currentIndex();
    if (index.isValid()) {
        return resourceFromModelIndex(index);
    }
    return 0;
}

void KisResourceItemChooser::setCurrentResource(KoResourceSP resource)
{
    // don't update if the change came from the same chooser
    if (d->updatesBlocked) {
        return;
    }
    QModelIndex index = d->tagFilterProxyModel->indexForResource(resource);
    d->view->setCurrentIndex(index);
    updatePreview(index);
}

void KisResourceItemChooser::setPreviewOrientation(Qt::Orientation orientation)
{
    d->splitter->setOrientation(orientation);
}

void KisResourceItemChooser::setPreviewTiled(bool tiled)
{
    d->tiledPreview = tiled;
}

void KisResourceItemChooser::setGrayscalePreview(bool grayscale)
{
    d->grayscalePreview = grayscale;
}

void KisResourceItemChooser::setCurrentItem(int row)
{
    QModelIndex index = d->view->model()->index(row, 0);
    if (!index.isValid())
        return;

    d->view->setCurrentIndex(index);
    if (index.isValid()) {
        updatePreview(index);
    }
}

void KisResourceItemChooser::activated(const QModelIndex &index)
{
    if (!index.isValid()) return;

    KoResourceSP resource = 0;

    if (index.isValid()) {
        resource = resourceFromModelIndex(index);
    }

    if (resource && resource->valid()) {
        d->updatesBlocked = true;
        emit resourceSelected(resource);
        d->updatesBlocked = false;
        updatePreview(index);
        updateButtonState();
    }
}

void KisResourceItemChooser::clicked(const QModelIndex &index)
{
    Q_UNUSED(index);

    KoResourceSP resource = currentResource();
    if (resource) {
        emit resourceClicked(resource);
    }
}

void KisResourceItemChooser::updateButtonState()
{
    QAbstractButton *removeButton = d->buttonGroup->button(Button_Remove);
    if (! removeButton)
        return;

    KoResourceSP resource = currentResource();
    if (resource) {
        removeButton->setEnabled(!resource->permanent());
        return;
    }
    removeButton->setEnabled(false);
}

void KisResourceItemChooser::updatePreview(const QModelIndex &idx)
{
    if (!d->usePreview) return;

    if (!idx.isValid()) {
        d->previewLabel->setPixmap(QPixmap());
        return;
    }

    QImage image = idx.data(Qt::UserRole + KisAbstractResourceModel::Thumbnail).value<QImage>();

    if (image.format() != QImage::Format_RGB32 &&
        image.format() != QImage::Format_ARGB32 &&
        image.format() != QImage::Format_ARGB32_Premultiplied) {

        image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    }

    if (d->tiledPreview) {
        int width = d->previewScroller->width() * 4;
        int height = d->previewScroller->height() * 4;
        QImage img(width, height, image.format());
        QPainter gc(&img);
        gc.fillRect(img.rect(), Qt::white);
        gc.setPen(Qt::NoPen);
        gc.setBrush(QBrush(image));
        gc.drawRect(img.rect());
        image = img;
    }

    // Only convert to grayscale if it is rgb. Otherwise, it's gray already.
    if (d->grayscalePreview && !image.isGrayscale()) {
        QRgb *pixel = reinterpret_cast<QRgb *>(image.bits());
        for (int row = 0; row < image.height(); ++row) {
            for (int col = 0; col < image.width(); ++col) {
                const QRgb currentPixel = pixel[row * image.width() + col];
                const int red = qRed(currentPixel);
                const int green = qGreen(currentPixel);
                const int blue = qBlue(currentPixel);
                const int grayValue = (red * 11 + green * 16 + blue * 5) / 32;
                pixel[row * image.width() + col] = qRgb(grayValue, grayValue, grayValue);
            }
        }
    }
    d->previewLabel->setPixmap(QPixmap::fromImage(image));
}

KoResourceSP KisResourceItemChooser::resourceFromModelIndex(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return 0;
    }
    KoResourceSP r = d->tagFilterProxyModel->resourceForIndex(index);
    return r;
}

QSize KisResourceItemChooser::viewSize() const
{
    return d->view->size();
}

KisResourceItemListView *KisResourceItemChooser::itemView() const
{
    return d->view;
}

void KisResourceItemChooser::contextMenuRequested(const QPoint &pos)
{
    KoResourceSP current = currentResource();
    d->tagManager->contextMenuRequested(currentResource(), pos);
    this->setCurrentResource(current);
}

void KisResourceItemChooser::setStoragePopupButtonVisible(bool visible)
{
    d->storagePopupButton->setVisible(visible);
}

void KisResourceItemChooser::setViewModeButtonVisible(bool visible)
{
    d->viewModeButton->setVisible(visible);
}

KisPopupButton *KisResourceItemChooser::viewModeButton() const
{
    return d->viewModeButton;
}

void KisResourceItemChooser::setSynced(bool sync)
{
    if (d->synced == sync)
        return;

    d->synced = sync;
    KisResourceItemChooserSync *chooserSync = KisResourceItemChooserSync::instance();
    if (sync) {
        connect(chooserSync, SIGNAL(baseLengthChanged(int)), SLOT(baseLengthChanged(int)));
        baseLengthChanged(chooserSync->baseLength());
    } else {
        chooserSync->disconnect(this);
    }
}

void KisResourceItemChooser::baseLengthChanged(int length)
{
    if (d->synced) {
        d->view->setItemSize(QSize(length, length));
    }
}

void KisResourceItemChooser::beforeFilterChanges()
{
    d->currentResource = d->tagFilterProxyModel->resourceForIndex(d->view->currentIndex());
}

void KisResourceItemChooser::afterFilterChanged()
{
    QModelIndex idx = d->tagFilterProxyModel->indexForResource(d->currentResource);
    if (idx.isValid()) {
        d->view->setCurrentIndex(idx);
    }
}

bool KisResourceItemChooser::eventFilter(QObject *object, QEvent *event)
{
    if (d->synced && event->type() == QEvent::Wheel) {
        KisResourceItemChooserSync *chooserSync = KisResourceItemChooserSync::instance();
        QWheelEvent *qwheel = static_cast<QWheelEvent *>(event);
        if (qwheel->modifiers() & Qt::ControlModifier) {

            int degrees = qwheel->delta() / 8;
            int newBaseLength = chooserSync->baseLength() + degrees / 15 * 10;
            chooserSync->setBaseLength(newBaseLength);
            return true;
        }
    }
    return QObject::eventFilter(object, event);
}


void KisResourceItemChooser::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateView();
}

void KisResourceItemChooser::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    updateView();
}

void KisResourceItemChooser::updateView()
{
    if (d->synced) {
        KisResourceItemChooserSync *chooserSync = KisResourceItemChooserSync::instance();
        baseLengthChanged(chooserSync->baseLength());
    }

    /// helps to set icons here in case the theme is changed
    d->viewModeButton->setIcon(KisIconUtils::loadIcon("view-choose"));
    d->importButton->setIcon(koIcon("document-import-16"));
    d->deleteButton->setIcon(koIcon("edit-delete"));
    d->storagePopupButton->setIcon(koIcon("bundle_archive"));
    d->tagManager->tagChooserWidget()->updateIcons();
}
