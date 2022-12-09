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
#include <QScrollBar>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QSplitter>
#include <QToolButton>
#include <QWheelEvent>
#include <QLineEdit>
#include <QComboBox>
#include <QStandardPaths>

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
#include <KisResourceUserOperations.h>

#include "KisStorageChooserWidget.h"


class Q_DECL_HIDDEN KisResourceItemChooser::Private
{
public:
    Private(QString _resourceType)
        : resourceType(_resourceType)
    {}

    QString resourceType;

    // Top/Bottom bar
    KisTagFilterResourceProxyModel *tagFilterProxyModel {0};
    KisResourceTaggingManager *tagManager {0};
    KisPopupButton *viewModeButton {0};
    KisStorageChooserWidget *storagePopupButton {0};

    // Resources view
    KisResourceItemListView *view {0};
    QSplitter *resourcesSplitter {0};

    QScrollArea *previewScroller {0};
    QLabel *previewLabel {0};

    // Import and Export buttons
    QFrame* importExportBtns {0};
    QToolButton *importButton {0};
    QToolButton *deleteButton {0};
    QButtonGroup *buttonGroup {0};  // does not seem to do much, maybe replace with std::array

    // Custom Buttons
    QList<QAbstractButton*> customButtons;

    bool usePreview {false};
    bool tiledPreview {false};
    bool grayscalePreview {false};
    bool synced {false};
    bool updatesBlocked {false};

    KoResourceSP currentResource;
    Layout layout = Layout::NotSet;
    ListViewMode requestedViewMode = ListViewMode::IconGrid;

    // Horizontal Layout Widgets
    QSplitter* horzSplitter {0};
    QFrame* left {0};
    QFrame* right {0};
    QToolButton* scroll_left {0};
    QToolButton* scroll_right {0};
};

KisResourceItemChooser::KisResourceItemChooser(const QString &resourceType, bool usePreview, QWidget *parent)
    : QWidget(parent)
    , d(new Private(resourceType))
{
    // Tag manager
    d->tagFilterProxyModel = new KisTagFilterResourceProxyModel(resourceType, this);
    d->tagFilterProxyModel->sort(Qt::DisplayRole);
    d->tagManager = new KisResourceTaggingManager(resourceType, d->tagFilterProxyModel, this);

    // Viewmode button
    d->viewModeButton = new KisPopupButton(this);
    d->viewModeButton->setVisible(false);
    d->viewModeButton->setArrowVisible(false);
    d->viewModeButton->setAutoRaise(true);

    // Storage button
    d->storagePopupButton = new KisStorageChooserWidget(resourceType, this);
    d->storagePopupButton->setToolTip(i18n("Storage Resources"));
    d->storagePopupButton->setAutoRaise(true);
    d->storagePopupButton->setArrowVisible(false);

    // Resource List View
    d->view = new KisResourceItemListView(this);
    d->view->setObjectName("ResourceItemview");
    d->view->setStrictSelectionMode(true);

    if (d->resourceType == ResourceType::Gradients) {
        d->view->setFixedToolTipThumbnailSize(QSize(256, 64));
        d->view->setToolTipShouldRenderCheckers(true);
    }
    else if (d->resourceType == ResourceType::PaintOpPresets) {
        d->view->setFixedToolTipThumbnailSize(QSize(128, 128));
    }
    else if (d->resourceType == ResourceType::Patterns || d->resourceType == ResourceType::Palettes) {
        d->view->setToolTipShouldRenderCheckers(false);
        d->view->setFixedToolTipThumbnailSize(QSize(256, 256));
    }

    d->view->setItemDelegate(new KisResourceItemDelegate(this));
    d->view->setSelectionMode(QAbstractItemView::SingleSelection);
    d->view->viewport()->installEventFilter(this);
    d->view->setModel(d->tagFilterProxyModel);

    connect(d->tagFilterProxyModel, SIGNAL(afterFilterChanged()), this, SLOT(afterFilterChanged()));

    connect(d->view, SIGNAL(currentResourceChanged(QModelIndex)), this, SLOT(activate(QModelIndex)));
    connect(d->view, SIGNAL(currentResourceClicked(QModelIndex)), this, SLOT(clicked(QModelIndex)));
    connect(d->view, SIGNAL(contextMenuRequested(QPoint)), this, SLOT(contextMenuRequested(QPoint)));
    connect(d->view, SIGNAL(sigSizeChanged()), this, SLOT(updateView()));

    // Splitter with resource views and preview scroller
    d->resourcesSplitter = new QSplitter(this);
    d->resourcesSplitter->addWidget(d->view);
    d->resourcesSplitter->setStretchFactor(0, 1);

    d->usePreview = usePreview;
    if (d->usePreview) {
        d->previewScroller = new QScrollArea(this);
        d->previewScroller->setWidgetResizable(true);
        d->previewScroller->setBackgroundRole(QPalette::Dark);
        d->previewScroller->setVisible(true);
        d->previewScroller->setAlignment(Qt::AlignCenter);
        d->previewLabel = new QLabel(this);
        d->previewScroller->setWidget(d->previewLabel);
        d->resourcesSplitter->addWidget(d->previewScroller);

        if (d->resourcesSplitter->count() == 2) {
            d->resourcesSplitter->setSizes(QList<int>() << 280 << 160);
        }

        QScroller* scroller = KisKineticScroller::createPreconfiguredScroller(d->previewScroller);
        if (scroller) {
            connect(scroller, SIGNAL(stateChanged(QScroller::State)), this, SLOT(slotScrollerStateChanged(QScroller::State)));
        }
    }

    // Import and Export buttons
    d->importButton = new QToolButton(this);
    d->importButton->setToolTip(i18nc("@info:tooltip", "Import resource"));
    d->importButton->setAutoRaise(true);
    d->importButton->setEnabled(true);

    d->deleteButton = new QToolButton(this);
    d->deleteButton->setToolTip(i18nc("@info:tooltip", "Delete resource"));
    d->deleteButton->setEnabled(false);
    d->deleteButton->setAutoRaise(true);

    d->buttonGroup = new QButtonGroup(this);
    d->buttonGroup->setExclusive(false);
    d->buttonGroup->addButton(d->importButton, Button_Import);
    d->buttonGroup->addButton(d->deleteButton, Button_Remove);
    connect(d->buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotButtonClicked(int)));

    d->importExportBtns = new QFrame(this);
    QHBoxLayout* importExportLayout = new QHBoxLayout(d->importExportBtns);
    importExportLayout->addWidget(d->importButton);
    importExportLayout->addWidget(d->deleteButton);

    // Layout
    QGridLayout* thisLayout = new QGridLayout(this);
    thisLayout->setObjectName("ResourceChooser this");
    thisLayout->setMargin(0);
    thisLayout->setSpacing(0);

    // Horizontal Layout
    {
        d->scroll_left = new QToolButton(this);
        d->scroll_left->setIcon(KisIconUtils::loadIcon("draw-arrow-back"));
        connect(d->scroll_left, &QToolButton::clicked, this, &KisResourceItemChooser::scrollBackwards);
        d->scroll_right = new QToolButton(this);
        d->scroll_right->setIcon(KisIconUtils::loadIcon("draw-arrow-forward"));
        connect(d->scroll_right, &QToolButton::clicked, this, &KisResourceItemChooser::scrollForwards);

        d->left = new QFrame(this);
        QHBoxLayout* leftLayout = new QHBoxLayout(d->left);
        leftLayout->setObjectName("ResourceChooser left");
        leftLayout->setMargin(0);
        leftLayout->setSpacing(0);

        d->right = new QFrame(this);
        QHBoxLayout* rightLayout = new QHBoxLayout(d->right);
        rightLayout->setObjectName("ResourceChooser right");
        rightLayout->setMargin(0);
        rightLayout->setSpacing(0);

        d->horzSplitter = new QSplitter(this);
        d->horzSplitter->setOrientation(Qt::Orientation::Horizontal);

        d->scroll_left->hide();
        d->scroll_right->hide();
        d->left->hide();
        d->right->hide();
        d->horzSplitter->hide();
    }

    // Other
    changeLayoutBasedOnSize();
    updateView();

    updateButtonState();
    showTaggingBar(false);
}

KisResourceItemChooser::~KisResourceItemChooser()
{
    disconnect();
    delete d;
}

void KisResourceItemChooser::setListViewMode(ListViewMode viewMode)
{
    if (d->layout != Layout::Horizontal) {
        d->view->setListViewMode(viewMode);
    }
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
        dialog.setDefaultDir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
        dialog.setCaption(i18nc("@title:window", "Choose File to Add"));
        Q_FOREACH(const QString &filename, dialog.filenames()) {
            if (QFileInfo(filename).exists() && QFileInfo(filename).isReadable()) {

                KoResourceSP previousResource = this->currentResource();
                KoResourceSP newResource = KisResourceUserOperations::importResourceFileWithUserInput(this, "", d->resourceType, filename);

                if (previousResource && newResource && !currentResource()) {
                    /// We have overridden the currently selected resource and
                    /// nothing is selected now
                    setCurrentResource(newResource);
                } else if (currentResource() == newResource) {
                    /// We have overridden the currently selected resource and
                    /// the model has managed to track the selection under it
                    /// (that is not possible right now, but can theoretically
                    /// happen under some circumstances)
                    const QModelIndex index = d->tagFilterProxyModel->indexForResource(newResource);
                    updatePreview(index);
                }
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
        activate(d->tagFilterProxyModel->index(row, index.column()));
    }
    updateButtonState();
}

void KisResourceItemChooser::showButtons(bool show)
{
    // assert(show == false);
    if (show) {
        d->importExportBtns->show();
    }
    else {
        d->importExportBtns->hide();
    }
}

void KisResourceItemChooser::showTaggingBar(bool show)
{
    d->tagManager->showTaggingBar(show);
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

KoResourceSP KisResourceItemChooser::currentResource(bool includeHidden) const
{
    if (includeHidden || d->view->selectionModel()->isSelected(d->view->currentIndex())) {
        return d->currentResource;
    }

    return nullptr;
}

void KisResourceItemChooser::setCurrentResource(KoResourceSP resource)
{
    // don't update if the change came from the same chooser
    if (d->updatesBlocked) {
        return;
    }
    QModelIndex index = d->tagFilterProxyModel->indexForResource(resource);
    d->view->setCurrentIndex(index);

    // The resource may currently be filtered out, but we want to be able
    // to select it if the filter changes and includes the resource.
    // Otherwise, activate() already took care of setting the current resource.
    if (!index.isValid()) {
        d->currentResource = resource;
    }
    updatePreview(index);
}

void KisResourceItemChooser::setPreviewOrientation(Qt::Orientation orientation)
{
    d->resourcesSplitter->setOrientation(orientation);
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

void KisResourceItemChooser::scrollBackwards()
{
    QScrollBar* bar = d->view->horizontalScrollBar();
    bar->setValue(bar->value() - d->view->gridSize().width());
}

void KisResourceItemChooser::scrollForwards()
{
    QScrollBar* bar = d->view->horizontalScrollBar();
    bar->setValue(bar->value() + d->view->gridSize().width());
}

void KisResourceItemChooser::activate(const QModelIndex &index)
{
    if (!index.isValid())
    {
        updateButtonState();
        return;
    }

    KoResourceSP resource = resourceFromModelIndex(index);

    if (resource && resource->valid()) {
        if (resource != d->currentResource) {
            d->currentResource = resource;
            d->updatesBlocked = true;
            emit resourceSelected(resource);
            d->updatesBlocked = false;
        }
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
    d->tagManager->contextMenuRequested(currentResource(), pos);
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

void KisResourceItemChooser::afterFilterChanged()
{
    // Note: Item model reset events silently reset the view's selection model too.
    // This currently only covers models resets as part of filter changes.
    QModelIndex idx = d->tagFilterProxyModel->indexForResource(d->currentResource);

    if (idx.isValid()) {
        d->view->setCurrentIndex(idx);
    }

    updateButtonState();
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
    changeLayoutBasedOnSize();
    updateView();
}

void KisResourceItemChooser::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    changeLayoutBasedOnSize();
    updateView();
}

void KisResourceItemChooser::changeLayoutBasedOnSize()
{
    // Vertical
    if (height() > 80) {

        if (d->layout == Layout::Vertical) {
            return;
        }

        d->view->setListViewMode(d->requestedViewMode);

        QGridLayout* thisLayout = dynamic_cast<QGridLayout*>(layout());
        thisLayout->addWidget(d->tagManager->tagChooserWidget(), 0, 0);
        thisLayout->addWidget(d->viewModeButton, 0, 1);
        thisLayout->addWidget(d->storagePopupButton, 0, 2);
        thisLayout->addWidget(d->resourcesSplitter, 1, 0, 1, 3);
        thisLayout->setRowStretch(1, 1);
        thisLayout->addWidget(d->tagManager->tagFilterWidget(), 2, 0, 1, 3);
        thisLayout->addWidget(d->importExportBtns, 3, 0, 1, 3);

        d->horzSplitter->hide();
        d->left->hide();
        d->right->hide();

        d->layout = Layout::Vertical;
    }
    // Horizontal
    else {
        if (d->layout == Layout::Horizontal) {
            return;
        }

        d->view->setListViewMode(ListViewMode::IconStripHorizontal);

        QLayout* leftLayout = d->left->layout();
        leftLayout->addWidget(d->resourcesSplitter);
        leftLayout->addWidget(d->scroll_left);
        leftLayout->addWidget(d->scroll_right);

        QLayout* rightLayout = d->right->layout();
        rightLayout->addWidget(d->tagManager->tagChooserWidget());
        rightLayout->addWidget(d->viewModeButton);
        rightLayout->addWidget(d->storagePopupButton);
        rightLayout->addWidget(d->tagManager->tagFilterWidget());
        rightLayout->addWidget(d->importExportBtns);        

        d->horzSplitter->addWidget(d->left);
        d->horzSplitter->setStretchFactor(0, 2);
        d->horzSplitter->addWidget(d->right);
        d->horzSplitter->setStretchFactor(1, 0);

        QGridLayout* thisLayout = dynamic_cast<QGridLayout*>(layout());
        thisLayout->setRowStretch(1, 0);
        thisLayout->addWidget(d->horzSplitter);

        d->horzSplitter->show();
        d->left->show();
        d->scroll_left->show();
        d->scroll_right->show();
        d->right->show();

        d->layout = Layout::Horizontal;
    }
}

void KisResourceItemChooser::updateView()
{
    /// helps to set icons here in case the theme is changed
    d->viewModeButton->setIcon(KisIconUtils::loadIcon("view-choose"));
    d->importButton->setIcon(koIcon("document-import-16"));
    d->deleteButton->setIcon(koIcon("edit-delete"));
    d->storagePopupButton->setIcon(koIcon("bundle_archive"));
    d->tagManager->tagChooserWidget()->updateIcons();
}
