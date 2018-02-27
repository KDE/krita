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
#include "KoResourceItemChooser.h"

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

#include <klocalizedstring.h>

#include <KoIcon.h>
#include <KoFileDialog.h>
#include <KisMimeDatabase.h>
#include "KoResourceServerAdapter.h"
#include "KoResourceItemView.h"
#include "KoResourceItemDelegate.h"
#include "KoResourceModel.h"
#include <resources/KoResource.h>
#include "KoResourceTaggingManager.h"
#include "KoTagFilterWidget.h"
#include "KoTagChooserWidget.h"
#include "KoResourceItemChooserSync.h"

class Q_DECL_HIDDEN KoResourceItemChooser::Private
{
public:
    Private()
        : model(0)
        , view(0)
        , buttonGroup(0)
        , viewModeButton(0)
        , usePreview(false)
        , previewScroller(0)
        , previewLabel(0)
        , splitter(0)
        , tiledPreview(false)
        , grayscalePreview(false)
        , synced(false)
        , updatesBlocked(false)
        , savedResourceWhileReset(0)
    {}
    KoResourceModel *model;
    KoResourceTaggingManager *tagManager;
    KoResourceItemView *view;
    QButtonGroup *buttonGroup;
    QToolButton  *viewModeButton;

    bool usePreview;
    QScrollArea *previewScroller;
    QLabel *previewLabel;
    QSplitter *splitter;
    QGridLayout *buttonLayout;
    bool tiledPreview;
    bool grayscalePreview;
    bool synced;
    bool updatesBlocked;

    KoResource *savedResourceWhileReset;

    QList<QAbstractButton*> customButtons;
};

KoResourceItemChooser::KoResourceItemChooser(QSharedPointer<KoAbstractResourceServerAdapter> resourceAdapter, QWidget *parent, bool usePreview)
    : QWidget(parent)
    , d(new Private())
{
    Q_ASSERT(resourceAdapter);

    d->splitter = new QSplitter(this);

    d->model = new KoResourceModel(resourceAdapter, this);
    connect(d->model, SIGNAL(beforeResourcesLayoutReset(KoResource *)), SLOT(slotBeforeResourcesLayoutReset(KoResource *)));
    connect(d->model, SIGNAL(afterResourcesLayoutReset()), SLOT(slotAfterResourcesLayoutReset()));

    d->view = new KoResourceItemView(this);
    d->view->setObjectName("ResourceItemview");
    d->view->setModel(d->model);
    d->view->setItemDelegate(new KoResourceItemDelegate(this));
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
    }

    d->splitter->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    connect(d->splitter, SIGNAL(splitterMoved(int, int)), SIGNAL(splitterMoved()));

    d->buttonGroup = new QButtonGroup(this);
    d->buttonGroup->setExclusive(false);

    QGridLayout *layout = new QGridLayout(this);

    d->buttonLayout = new QGridLayout();

    importButton = new QPushButton(this);

    importButton->setToolTip(i18nc("@info:tooltip", "Import resource"));
    importButton->setEnabled(true);
    d->buttonGroup->addButton(importButton, Button_Import);
    d->buttonLayout->addWidget(importButton, 0, 0);

    deleteButton = new QPushButton(this);
    deleteButton->setToolTip(i18nc("@info:tooltip", "Delete resource"));
    deleteButton->setEnabled(false);
    d->buttonGroup->addButton(deleteButton, Button_Remove);
    d->buttonLayout->addWidget(deleteButton, 0, 1);

    connect(d->buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotButtonClicked(int)));

    d->buttonLayout->setColumnStretch(0, 1);
    d->buttonLayout->setColumnStretch(1, 1);
    d->buttonLayout->setColumnStretch(2, 2);
    d->buttonLayout->setSpacing(0);
    d->buttonLayout->setMargin(0);

    d->viewModeButton = new QToolButton(this);
    d->viewModeButton->setPopupMode(QToolButton::InstantPopup);
    d->viewModeButton->setVisible(false);

    d->tagManager = new KoResourceTaggingManager(d->model, this);
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
    activated(d->model->index(0, 0));
}

KoResourceItemChooser::~KoResourceItemChooser()
{
    disconnect();
    delete d;
}

void KoResourceItemChooser::slotButtonClicked(int button)
{
    if (button == Button_Import) {
        QString extensions = d->model->extensions();
        QStringList mimeTypes;
        Q_FOREACH(const QString &suffix, extensions.split(":")) {
            mimeTypes << KisMimeDatabase::mimeTypeForSuffix(suffix);
        }

        KoFileDialog dialog(0, KoFileDialog::OpenFile, "OpenDocument");
        dialog.setMimeTypeFilters(mimeTypes);
        dialog.setCaption(i18nc("@title:window", "Choose File to Add"));
        QString filename = dialog.filename();

        d->model->importResourceFile(filename);
    } else if (button == Button_Remove) {
        QModelIndex index = d->view->currentIndex();
        int row = index.row();
        int column = index.column();
        if (index.isValid()) {

            KoResource *resource = resourceFromModelIndex(index);
            if (resource) {
                d->model->removeResource(resource);
            }
        }
        if (column == 0) {
            int rowMin = --row;
            row = qBound(0, rowMin, row);
        }
        int columnMin = --column;
        column = qBound(0, columnMin, column);
        setCurrentItem(row, column);
        activated(d->model->index(row, column));
    }
    updateButtonState();
}

void KoResourceItemChooser::showButtons(bool show)
{
    foreach (QAbstractButton * button, d->buttonGroup->buttons()) {
        show ? button->show() : button->hide();
    }

    Q_FOREACH (QAbstractButton *button, d->customButtons) {
        show ? button->show() : button->hide();
    }
}

void KoResourceItemChooser::addCustomButton(QAbstractButton *button, int cell)
{
    d->buttonLayout->addWidget(button, 0, cell);
    d->buttonLayout->setColumnStretch(2, 1);
    d->buttonLayout->setColumnStretch(3, 1);
}

void KoResourceItemChooser::showTaggingBar(bool show)
{
    d->tagManager->showTaggingBar(show);

}

void KoResourceItemChooser::setRowCount(int rowCount)
{
    int resourceCount = d->model->resourcesCount();
    d->model->setColumnCount(static_cast<qreal>(resourceCount) / rowCount);
    //Force an update to get the right row height (in theory)
    QRect geometry = d->view->geometry();
    d->view->setViewMode(KoResourceItemView::FIXED_ROWS);
    d->view->setGeometry(geometry.adjusted(0, 0, 0, 1));
    d->view->setGeometry(geometry);
}

void KoResourceItemChooser::setColumnCount(int columnCount)
{
    d->model->setColumnCount(columnCount);
}

void KoResourceItemChooser::setRowHeight(int rowHeight)
{
    d->view->verticalHeader()->setDefaultSectionSize(rowHeight);
}

void KoResourceItemChooser::setColumnWidth(int columnWidth)
{
    d->view->horizontalHeader()->setDefaultSectionSize(columnWidth);
}

void KoResourceItemChooser::setItemDelegate(QAbstractItemDelegate *delegate)
{
    d->view->setItemDelegate(delegate);
}

KoResource *KoResourceItemChooser::currentResource() const
{
    QModelIndex index = d->view->currentIndex();
    if (index.isValid()) {
        return resourceFromModelIndex(index);
    }
    return 0;
}

void KoResourceItemChooser::setCurrentResource(KoResource *resource)
{
    // don't update if the change came from the same chooser
    if (d->updatesBlocked) {
        return;
    }

    QModelIndex index = d->model->indexFromResource(resource);

    d->view->setCurrentIndex(index);
    updatePreview(index.isValid() ? resource : 0);
}

void KoResourceItemChooser::slotBeforeResourcesLayoutReset(KoResource *activateAfterReset)
{
    d->savedResourceWhileReset = activateAfterReset ? activateAfterReset : currentResource();
}

void KoResourceItemChooser::slotAfterResourcesLayoutReset()
{
    if (d->savedResourceWhileReset) {
        this->blockSignals(true);
        setCurrentResource(d->savedResourceWhileReset);
        this->blockSignals(false);
    }
}

void KoResourceItemChooser::setPreviewOrientation(Qt::Orientation orientation)
{
    d->splitter->setOrientation(orientation);
}

void KoResourceItemChooser::setPreviewTiled(bool tiled)
{
    d->tiledPreview = tiled;
}

void KoResourceItemChooser::setGrayscalePreview(bool grayscale)
{
    d->grayscalePreview = grayscale;
}

void KoResourceItemChooser::setCurrentItem(int row, int column)
{
    QModelIndex index = d->model->index(row, column);
    if (!index.isValid())
        return;

    d->view->setCurrentIndex(index);
    if (index.isValid()) {
        updatePreview(resourceFromModelIndex(index));
    }
}

void KoResourceItemChooser::setProxyModel(QAbstractProxyModel *proxyModel)
{
    proxyModel->setSourceModel(d->model);
    d->view->setModel(proxyModel);
}

void KoResourceItemChooser::activated(const QModelIndex &/*index*/)
{
    KoResource *resource = currentResource();
    if (resource) {
        d->updatesBlocked = true;
        emit resourceSelected(resource);
        d->updatesBlocked = false;
        updatePreview(resource);
        updateButtonState();
    }
}

void KoResourceItemChooser::clicked(const QModelIndex &index)
{
    Q_UNUSED(index);

    KoResource *resource = currentResource();
    if (resource) {
        emit resourceClicked(resource);
    }
}

void KoResourceItemChooser::updateButtonState()
{
    QAbstractButton *removeButton = d->buttonGroup->button(Button_Remove);
    if (! removeButton)
        return;

    KoResource *resource = currentResource();
    if (resource) {
        removeButton->setEnabled(!resource->permanent());
        return;
    }
    removeButton->setEnabled(false);
}

void KoResourceItemChooser::updatePreview(KoResource *resource)
{
    if (!d->usePreview) return;

    if (!resource) {
        d->previewLabel->setPixmap(QPixmap());
        return;
    }

    QImage image = resource->image();

    if (image.format() != QImage::Format_RGB32 ||
        image.format() != QImage::Format_ARGB32 ||
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

KoResource *KoResourceItemChooser::resourceFromModelIndex(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    const QAbstractProxyModel *proxyModel = dynamic_cast<const QAbstractProxyModel *>(index.model());
    if (proxyModel) {
        //Get original model index, because proxy models destroy the internalPointer
        QModelIndex originalIndex = proxyModel->mapToSource(index);
        return static_cast<KoResource *>(originalIndex.internalPointer());
    }

    return static_cast<KoResource *>(index.internalPointer());
}

QSize KoResourceItemChooser::viewSize() const
{
    return d->view->size();
}

KoResourceItemView *KoResourceItemChooser::itemView() const
{
    return d->view;
}

void KoResourceItemChooser::contextMenuRequested(const QPoint &pos)
{
    d->tagManager->contextMenuRequested(currentResource(), pos);
}

void KoResourceItemChooser::setViewModeButtonVisible(bool visible)
{
    d->viewModeButton->setVisible(visible);
}

QToolButton *KoResourceItemChooser::viewModeButton() const
{
    return d->viewModeButton;
}

void KoResourceItemChooser::setSynced(bool sync)
{
    if (d->synced == sync)
        return;

    d->synced = sync;
    KoResourceItemChooserSync *chooserSync = KoResourceItemChooserSync::instance();
    if (sync) {
        connect(chooserSync, SIGNAL(baseLengthChanged(int)), SLOT(baseLengthChanged(int)));
        baseLengthChanged(chooserSync->baseLength());
    } else {
        chooserSync->disconnect(this);
    }
}

void KoResourceItemChooser::baseLengthChanged(int length)
{
    if (d->synced) {
        int resourceCount = d->model->resourcesCount();
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

bool KoResourceItemChooser::eventFilter(QObject *object, QEvent *event)
{
    if (d->synced && event->type() == QEvent::Wheel) {
        KoResourceItemChooserSync *chooserSync = KoResourceItemChooserSync::instance();
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

void KoResourceItemChooser::configureKineticScrolling(int gesture, int sensitivity, bool scrollbar)
{
    QScroller::ScrollerGestureType gestureType;

    switch (gesture) {
    case 1: {
        gestureType = QScroller::TouchGesture;
        break;
    }
    case 2: {
        gestureType = QScroller::LeftMouseButtonGesture;
        break;
    }
    default:
        return;
    }

    KoResourceItemView *view = itemView();

    view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    if (!scrollbar) {
        view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    QScroller *scroller = QScroller::scroller(view);
    scroller->grabGesture(view, gestureType);

    QScrollerProperties sp;

    // DragStartDistance seems to be based on meter per second; though it's
    // not explicitly documented, other QScroller values are in that metric.

    // To start kinetic scrolling, with minimal sensitity, we expect a drag
    // of 10 mm, with minimum sensitity any > 0 mm.

    const float mm = 0.001f; // 1 millimeter
    const float resistance = 1.0f - (sensitivity / 100.0f);

    sp.setScrollMetric(QScrollerProperties::DragStartDistance, resistance * 10.0f * mm);
    sp.setScrollMetric(QScrollerProperties::DragVelocitySmoothingFactor, 1.0f);
    sp.setScrollMetric(QScrollerProperties::MinimumVelocity, 0.0f);
    sp.setScrollMetric(QScrollerProperties::AxisLockThreshold, 1.0f);
    sp.setScrollMetric(QScrollerProperties::MaximumClickThroughVelocity, 0.0f);
    sp.setScrollMetric(QScrollerProperties::MousePressEventDelay, 1.0f - 0.75f * resistance);
    sp.setScrollMetric(QScrollerProperties::AcceleratingFlickSpeedupFactor, 1.5f);

    sp.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, QScrollerProperties::OvershootAlwaysOn);
    sp.setScrollMetric(QScrollerProperties::OvershootDragResistanceFactor, 0.1);
    sp.setScrollMetric(QScrollerProperties::OvershootDragDistanceFactor, 0.3);
    sp.setScrollMetric(QScrollerProperties::OvershootScrollDistanceFactor, 0.1);
    sp.setScrollMetric(QScrollerProperties::OvershootScrollTime, 0.4);

    scroller->setScrollerProperties(sp);
}

void KoResourceItemChooser::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateView();
}

void KoResourceItemChooser::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    updateView();
}

void KoResourceItemChooser::updateView()
{
    if (d->synced) {
        KoResourceItemChooserSync *chooserSync = KoResourceItemChooserSync::instance();
        baseLengthChanged(chooserSync->baseLength());
    }

    /// helps to set icons here in case the theme is changed
    d->viewModeButton->setIcon(koIcon("view-choose"));
    importButton->setIcon(koIcon("document-open"));
    deleteButton->setIcon(koIcon("trash-empty"));
}
