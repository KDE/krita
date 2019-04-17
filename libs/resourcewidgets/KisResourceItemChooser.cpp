/* This file is part of the KDE project
   Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
   Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
   Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
   Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
   Copyright (c) 2013 Sascha Suelzer <s.suelzer@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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

#include <klocalizedstring.h>

#include <KoIcon.h>
#include <KoFileDialog.h>
#include <KisKineticScroller.h>
#include <KisMimeDatabase.h>

#include <KisResourceModelProvider.h>
#include <KisResourceModel.h>
#include <KisResourceGridProxyModel.h>
#include <KisTagFilterResourceProxyModel.h>
#include <KisResourceLoaderRegistry.h>

#include "KisResourceItemView.h"
#include "KisResourceItemDelegate.h"
#include "KisTagFilterWidget.h"
#include "KisTagChooserWidget.h"
#include "KisResourceItemChooserSync.h"
#include "KisResourceTaggingManager.h"


#include "kis_assert.h"


class Q_DECL_HIDDEN KisResourceItemChooser::Private
{
public:
    Private(QString _resourceType)
        : resourceType(_resourceType)
    {}

    QString resourceType;

    KisResourceModel *resourceModel {0};
    KisResourceGridProxyModel *resourceGridProxyModel {0};
    KisTagFilterResourceProxyModel *tagFilterProxyModel {0};
    QSortFilterProxyModel *extraFilterModel {0};

    KisResourceTaggingManager *tagManager {0};
    KisResourceItemView *view {0};
    QButtonGroup *buttonGroup {0};
    QToolButton *viewModeButton {0};

    QScrollArea *previewScroller {0};
    QLabel *previewLabel {0};
    QSplitter *splitter {0};
    QGridLayout *buttonLayout {0};

    QPushButton *importButton {0};
    QPushButton *deleteButton {0};

    bool usePreview {false};
    bool tiledPreview {false};
    bool grayscalePreview {false};
    bool synced {false};
    bool updatesBlocked {false};

    QModelIndex savedResourceWhileReset; // Indexes on the proxyModel, not the source resource model

    QList<QAbstractButton*> customButtons;

};

KisResourceItemChooser::KisResourceItemChooser(const QString &resourceType, bool usePreview, QWidget *parent, QSortFilterProxyModel *extraFilterProxy)
    : QWidget(parent)
    , d(new Private(resourceType))
{
    d->extraFilterModel = extraFilterProxy;
    if (d->extraFilterModel) {
        d->extraFilterModel->setParent(this);
    }

    d->splitter = new QSplitter(this);

    d->resourceModel = KisResourceModelProvider::resourceModel(resourceType);

    d->tagFilterProxyModel = new KisTagFilterResourceProxyModel(this);
    d->tagFilterProxyModel->setSourceModel(d->resourceModel);

    d->resourceGridProxyModel = new KisResourceGridProxyModel(this);

    if (d->extraFilterModel) {
        d->extraFilterModel->setSourceModel(d->resourceModel);
        d->resourceGridProxyModel->setSourceModel(d->extraFilterModel);
    }
    else {
        d->resourceGridProxyModel->setSourceModel(d->tagFilterProxyModel);
    }

    d->resourceGridProxyModel->setRowStride(10);

    connect(d->resourceModel, SIGNAL(beforeResourcesLayoutReset(QModelIndex)), SLOT(slotBeforeResourcesLayoutReset(QModelIndex)));
    connect(d->resourceModel, SIGNAL(afterResourcesLayoutReset()), SLOT(slotAfterResourcesLayoutReset()));

    d->view = new KisResourceItemView(this);
    d->view->setObjectName("ResourceItemview");
    d->view->setModel(d->resourceGridProxyModel);
    d->view->setItemDelegate(new KisResourceItemDelegate(this));
    d->view->setSelectionMode(QAbstractItemView::SingleSelection);
    d->view->viewport()->installEventFilter(this);

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

    d->importButton = new QPushButton(this);

    d->importButton->setToolTip(i18nc("@info:tooltip", "Import resource"));
    d->importButton->setEnabled(true);
    d->buttonGroup->addButton(d->importButton, Button_Import);
    d->buttonLayout->addWidget(d->importButton, 0, 0);

    d->deleteButton = new QPushButton(this);
    d->deleteButton->setToolTip(i18nc("@info:tooltip", "Delete resource"));
    d->deleteButton->setEnabled(false);
    d->buttonGroup->addButton(d->deleteButton, Button_Remove);
    d->buttonLayout->addWidget(d->deleteButton, 0, 1);

    connect(d->buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotButtonClicked(int)));

    d->buttonLayout->setColumnStretch(0, 1);
    d->buttonLayout->setColumnStretch(1, 1);
    d->buttonLayout->setColumnStretch(2, 2);
    d->buttonLayout->setSpacing(0);
    d->buttonLayout->setMargin(0);

    d->viewModeButton = new QToolButton(this);
    d->viewModeButton->setPopupMode(QToolButton::InstantPopup);
    d->viewModeButton->setVisible(false);

    d->tagManager = new KisResourceTaggingManager(d->tagFilterProxyModel, this);
    connect(d->tagManager, SIGNAL(updateView()), this, SLOT(updateView()));

    layout->addWidget(d->tagManager->tagChooserWidget(), 0, 0);
    layout->addWidget(d->viewModeButton, 0, 1);
    layout->addWidget(d->splitter, 1, 0, 1, 2);
    layout->addWidget(d->tagManager->tagFilterWidget(), 2, 0, 1, 2);
    layout->addLayout(d->buttonLayout, 3, 0, 1, 2);
    layout->setMargin(0);
    layout->setSpacing(0);

    updateView();

    updateButtonState();
    showTaggingBar(false);
    activated(d->resourceGridProxyModel->index(0, 0));
}

KisResourceItemChooser::~KisResourceItemChooser()
{
    disconnect();
    delete d;
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
                d->resourceGridProxyModel->importResourceFile(filename);
            }
        }
    }
    else if (button == Button_Remove) {
        QModelIndex index = d->view->currentIndex();
        if (index.isValid()) {
            d->resourceGridProxyModel->removeResource(index);
        }
        int row = index.row();
        int column = index.column();

        if (column == 0) {
            int rowMin = --row;
            row = qBound(0, rowMin, row);
        }
        int columnMin = --column;
        column = qBound(0, columnMin, column);
        setCurrentItem(row, column);
        activated(d->resourceGridProxyModel->index(row, column));
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

void KisResourceItemChooser::setRowCount(int rowCount)
{
    int resourceCount = d->resourceModel->rowCount();
    d->resourceGridProxyModel->setRowStride(static_cast<qreal>(resourceCount) / rowCount);
    //Force an update to get the right row height (in theory)
    QRect geometry = d->view->geometry();
    d->view->setViewMode(KisResourceItemView::FIXED_ROWS);
    d->view->setGeometry(geometry.adjusted(0, 0, 0, 1));
    d->view->setGeometry(geometry);
}

void KisResourceItemChooser::setColumnCount(int columnCount)
{
    d->resourceGridProxyModel->setRowStride(columnCount);
}

int KisResourceItemChooser::rowCount() const
{
    return d->resourceGridProxyModel->rowCount();
}

void KisResourceItemChooser::setRowHeight(int rowHeight)
{
    d->view->verticalHeader()->setDefaultSectionSize(rowHeight);
}

void KisResourceItemChooser::setColumnWidth(int columnWidth)
{
    d->view->horizontalHeader()->setDefaultSectionSize(columnWidth);
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
    QModelIndex index = d->resourceModel->indexFromResource(resource);
    d->view->setCurrentIndex(index);
    updatePreview(index);
}

void KisResourceItemChooser::slotBeforeResourcesLayoutReset(QModelIndex activateAfterReset)
{
    QModelIndex proxyIndex = d->resourceGridProxyModel->mapFromSource(d->tagFilterProxyModel->mapFromSource(activateAfterReset));
    d->savedResourceWhileReset = proxyIndex.isValid() ? proxyIndex : d->view->currentIndex();
}

void KisResourceItemChooser::slotAfterResourcesLayoutReset()
{
    if (d->savedResourceWhileReset.isValid()) {
        this->blockSignals(true);
        setCurrentItem(d->savedResourceWhileReset.row(), d->savedResourceWhileReset.column());
        this->blockSignals(false);
    }
    d->savedResourceWhileReset = QModelIndex();
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

void KisResourceItemChooser::setCurrentItem(int row, int column)
{
    QModelIndex index = d->resourceModel->index(row, column);
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

    qDebug() << "resource" << resource << (resource ? resource->name() : "") << (resource ? QString("%1").arg(resource->valid()) : "");

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

    QImage image = idx.data(Qt::UserRole + KisResourceModel::Image).value<QImage>();

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
    KoResourceSP r = d->resourceGridProxyModel->resourceForIndex(index);
    return r;
}

QSize KisResourceItemChooser::viewSize() const
{
    return d->view->size();
}

KisResourceItemView *KisResourceItemChooser::itemView() const
{
    return d->view;
}

void KisResourceItemChooser::contextMenuRequested(const QPoint &pos)
{
    d->tagManager->contextMenuRequested(currentResource(), pos);
}

void KisResourceItemChooser::setViewModeButtonVisible(bool visible)
{
    d->viewModeButton->setVisible(visible);
}

QToolButton *KisResourceItemChooser::viewModeButton() const
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
        int resourceCount = d->resourceModel->rowCount();
        int width = d->view->width();
        int maxColumns = width / length;
        int cols = width / (2 * length) + 1;
        while (cols <= maxColumns) {
            int size = width / cols;
            int rows = ceil(resourceCount / (double)cols);
            if (rows * size < (d->view->height() - 5)) {
                break;
            }
            cols++;
        }
        setColumnCount(cols);
    }
    d->view->updateView();
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
    d->viewModeButton->setIcon(koIcon("view-choose"));
    d->importButton->setIcon(koIcon("document-open"));
    d->deleteButton->setIcon(koIcon("trash-empty"));
}
