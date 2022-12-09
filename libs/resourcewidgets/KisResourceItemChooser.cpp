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
    QSplitter *resources_splitter {0};

    QScrollArea *previewScroller {0};
    QLabel *previewLabel {0};

    // Import and Export buttons
    // QGridLayout *buttonLayout {0};
    QFrame* import_export_btns = nullptr;
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

    // Horizontal Layout Widgets
    QSplitter* horz_splitter = nullptr;
    QFrame* left = nullptr;
    QFrame* right = nullptr;
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
    d->resources_splitter = new QSplitter(this);
    d->resources_splitter->addWidget(d->view);
    d->resources_splitter->setStretchFactor(0, 2);

    d->usePreview = usePreview;
    if (d->usePreview) {
        d->previewScroller = new QScrollArea(this);
        d->previewScroller->setWidgetResizable(true);
        d->previewScroller->setBackgroundRole(QPalette::Dark);
        d->previewScroller->setVisible(true);
        d->previewScroller->setAlignment(Qt::AlignCenter);
        d->previewLabel = new QLabel(this);
        d->previewScroller->setWidget(d->previewLabel);
        d->resources_splitter->addWidget(d->previewScroller);

        if (d->resources_splitter->count() == 2) {
            d->resources_splitter->setSizes(QList<int>() << 280 << 160);
        }

        QScroller* scroller = KisKineticScroller::createPreconfiguredScroller(d->previewScroller);
        if (scroller) {
            connect(scroller, SIGNAL(stateChanged(QScroller::State)), this, SLOT(slotScrollerStateChanged(QScroller::State)));
        }
    }

    d->resources_splitter->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    connect(d->resources_splitter, SIGNAL(splitterMoved(int,int)), SIGNAL(splitterMoved()));

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

    d->import_export_btns = new QFrame(this);
    QHBoxLayout* import_export_layout = new QHBoxLayout(d->import_export_btns);
    import_export_layout->addWidget(d->importButton);
    import_export_layout->addWidget(d->deleteButton);

    // Layout
    QGridLayout* this_layout = new QGridLayout(this);
    this_layout->setObjectName("ResourceChooser this");
    this_layout->setMargin(0);
    this_layout->setSpacing(0);

    // Horizontal Layout
    {
        d->left = new QFrame(this);
        QHBoxLayout* left_layout = new QHBoxLayout(d->left);
        left_layout->setObjectName("ResourceChooser left");
        left_layout->setMargin(0);
        left_layout->setSpacing(0);

        d->right = new QFrame(this);
        QHBoxLayout* right_layout = new QHBoxLayout(d->right);
        right_layout->setObjectName("ResourceChooser right");
        right_layout->setMargin(0);
        right_layout->setSpacing(0);

        d->horz_splitter = new QSplitter(this);
        d->horz_splitter->setOrientation(Qt::Orientation::Horizontal);

        d->left->hide();
        d->right->hide();
        d->horz_splitter->hide();
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
    if (show) {
        d->import_export_btns->show();
    }
    else {
        d->import_export_btns->hide();
    }
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
    d->resources_splitter->setOrientation(orientation);
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
    if (height() > 100) {

        if (d->layout == Layout::Vertical) {
            return;
        }

        QGridLayout* this_layout = dynamic_cast<QGridLayout*>(layout());
        this_layout->addWidget(d->tagManager->tagChooserWidget(), 0, 0);
        this_layout->addWidget(d->viewModeButton, 0, 1);
        this_layout->addWidget(d->storagePopupButton, 0, 2);
        this_layout->addWidget(d->resources_splitter, 1, 0, 1, 3);
        this_layout->addWidget(d->tagManager->tagFilterWidget(), 2, 0, 1, 3);
        this_layout->addWidget(d->import_export_btns, 3, 0, 1, 3);

        d->horz_splitter->hide();
        d->left->hide();
        d->right->hide();

        d->layout = Layout::Vertical;
    }
    // Horizontal
    else {
        if (d->layout == Layout::Horizontal) {
            return;
        }

        QLayout* left_layout = d->left->layout();
        left_layout->addWidget(d->tagManager->tagChooserWidget());
        left_layout->addWidget(d->viewModeButton);
        left_layout->addWidget(d->storagePopupButton);

        QLayout* right_layout = d->right->layout();
        right_layout->addWidget(d->tagManager->tagFilterWidget());
        right_layout->addWidget(d->import_export_btns);

        d->horz_splitter->addWidget(d->left);
        d->horz_splitter->addWidget(d->resources_splitter);
        d->horz_splitter->setStretchFactor(1, 1);
        d->horz_splitter->addWidget(d->right);
        layout()->addWidget(d->horz_splitter);

        d->horz_splitter->show();
        d->left->show();
        d->right->show();

        d->layout = Layout::Horizontal;
    }
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
