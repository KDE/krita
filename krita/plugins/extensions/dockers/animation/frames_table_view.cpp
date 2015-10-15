/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "frames_table_view.h"

#include "timeline_frames_model.h"

#include "timeline_ruler_header.h"
#include "timeline_layers_header.h"

#include <cmath>

#include <QHeaderView>
#include <QDropEvent>
#include <QToolButton>
#include <QMenu>
#include <QScrollBar>


#include "kis_debug.h"
#include "frames_item_delegate.h"

#include "kis_draggable_tool_button.h"

#include "kicon.h"
#include "kis_icon_utils.h"

struct FramesTableView::Private
{
    Private()
        : fps(1),
          zoom(1.0),
          initialDragZoomValue(1.0),
          dragInProgress(false) {}

    TimelineFramesModel *model;
    TimelineRulerHeader *horizontalRuler;
    TimelineLayersHeader *layersHeader;
    int fps;
    qreal zoom;
    qreal initialDragZoomValue;

    QToolButton *addLayersButton;
    QMenu *layerEditingMenu;
    QMenu *existingLayersMenu;

    QMenu *frameCreationMenu;
    QMenu *frameEditingMenu;

    QToolButton *zoomDragButton;

    bool dragInProgress;
};

FramesTableView::FramesTableView(QWidget *parent)
    : QTableView(parent),
      m_d(new Private)
{
    setCornerButtonEnabled(false);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::SingleSelection);

    setItemDelegate(new FramesItemDelegate(this));

    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    setAcceptDrops(true);
    setDropIndicatorShown(true);

    m_d->horizontalRuler = new TimelineRulerHeader(this);

#if QT_VERSION < 0x050000
    m_d->horizontalRuler->setResizeMode(QHeaderView::Fixed);
#else
    m_d->horizontalRuler->setSectionsResizeMode(QHeaderView::Fixed);
#endif

    m_d->horizontalRuler->setDefaultSectionSize(18);
    this->setHorizontalHeader(m_d->horizontalRuler);

    m_d->layersHeader = new TimelineLayersHeader(this);

#if QT_VERSION < 0x050000
    m_d->layersHeader->setResizeMode(QHeaderView::Fixed);
#else
    m_d->layersHeader->setSectionsResizeMode(QHeaderView::Fixed);
#endif

    m_d->layersHeader->setDefaultSectionSize(24);
    m_d->layersHeader->setHighlightSections(true);

#if QT_VERSION < 0x050000
    m_d->layersHeader->setClickable(true);
#else
    m_d->layersHeader->setSectionsClickable(true);
#endif

    this->setVerticalHeader(m_d->layersHeader);

    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), SLOT(slotUpdateInfiniteFramesCount()));
    connect(horizontalScrollBar(), SIGNAL(sliderReleased()), SLOT(slotUpdateInfiniteFramesCount()));

    m_d->addLayersButton = new QToolButton(this);
    m_d->addLayersButton->setAutoRaise(true);
    m_d->addLayersButton->setIcon(KisIconUtils::loadIcon("addlayer"));
    m_d->addLayersButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_d->addLayersButton->setPopupMode(QToolButton::InstantPopup);

    m_d->layerEditingMenu = new QMenu(this);
    m_d->layerEditingMenu->addAction("New Layer", this, SLOT(slotAddNewLayer()));
    m_d->existingLayersMenu = m_d->layerEditingMenu->addMenu("Add Existing Layer");
    m_d->layerEditingMenu->addSeparator();
    m_d->layerEditingMenu->addAction("Hide from Timeline", this, SLOT(slotHideLayerFromTimeline()));
    m_d->layerEditingMenu->addAction("Remove Layer", this, SLOT(slotRemoveLayer()));

    connect(m_d->existingLayersMenu, SIGNAL(aboutToShow()), SLOT(slotUpdateLayersMenu()));
    connect(m_d->existingLayersMenu, SIGNAL(triggered(QAction*)), SLOT(slotAddExistingLayer(QAction*)));

    connect(m_d->layersHeader, SIGNAL(sigRequestContextMenu(const QPoint&)), SLOT(slotLayerContextMenuRequested(const QPoint&)));

    m_d->addLayersButton->setMenu(m_d->layerEditingMenu);

    m_d->frameCreationMenu = new QMenu(this);
    m_d->frameCreationMenu->addAction("New Frame", this, SLOT(slotNewFrame()));
    m_d->frameCreationMenu->addAction("Copy Frame", this, SLOT(slotCopyFrame()));

    m_d->frameEditingMenu = new QMenu(this);
    m_d->frameEditingMenu->addAction("Remove Frame", this, SLOT(slotRemoveFrame()));


    m_d->zoomDragButton = new KisDraggableToolButton(this);
    m_d->zoomDragButton->setAutoRaise(true);
    m_d->zoomDragButton->setIcon(KisIconUtils::loadIcon("zoom-in"));
    m_d->zoomDragButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_d->zoomDragButton->setPopupMode(QToolButton::InstantPopup);
    connect(m_d->zoomDragButton, SIGNAL(valueChanged(int)), SLOT(slotZoomButtonChanged(int)));
    connect(m_d->zoomDragButton, SIGNAL(pressed()), SLOT(slotZoomButtonPressed()));

    setFramesPerSecond(12);
}

FramesTableView::~FramesTableView()
{
}

void resizeToMinimalSize(QAbstractButton *w, int minimalSize) {
    QSize buttonSize = w->sizeHint();
    if (buttonSize.height() > minimalSize) {
        buttonSize = QSize(minimalSize, minimalSize);
    }
    w->resize(buttonSize);
}

void FramesTableView::updateGeometries()
{
    QTableView::updateGeometries();

    const int availableHeight = m_d->horizontalRuler->height();
    const int margin = 2;
    const int minimalSize = availableHeight - 2 * margin;

    resizeToMinimalSize(m_d->addLayersButton, minimalSize);
    resizeToMinimalSize(m_d->zoomDragButton, minimalSize);

    int x = 2 * margin;
    int y = (availableHeight - minimalSize) / 2;
    m_d->addLayersButton->move(x, 2 * y);

    const int availableWidth = m_d->layersHeader->width();

    x = availableWidth - margin - minimalSize;
    m_d->zoomDragButton->move(x, 2 * y);
}

void FramesTableView::setModel(QAbstractItemModel *model)
{
    TimelineFramesModel *framesModel = qobject_cast<TimelineFramesModel*>(model);
    m_d->model = framesModel;

    QTableView::setModel(model);

    connect(m_d->model, SIGNAL(headerDataChanged(Qt::Orientation, int, int)),
            this, SLOT(slotHeaderDataChanged(Qt::Orientation, int, int)));

    connect(m_d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(slotDataChanged(QModelIndex,QModelIndex)));

    connect(m_d->model, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
            this, SLOT(slotReselectCurrentIndex()));
}

void FramesTableView::setFramesPerSecond(int fps)
{
    m_d->fps = fps;
    m_d->horizontalRuler->setFramePerSecond(fps);

    // For some reason simple update sometimes doesn't work here, so
    // reset the whole header
    //
    // m_d->horizontalRuler->reset();
}

qreal FramesTableView::zoom() const
{
    return m_d->zoom;
}

void FramesTableView::setZoom(qreal zoom)
{
    const int minSectionSize = 4;
    const int unitSectionSize = 18;

    int newSectionSize = zoom * unitSectionSize;

    if (newSectionSize < minSectionSize) {
        newSectionSize = minSectionSize;
        zoom = qreal(newSectionSize) / unitSectionSize;
    }

    if (!qFuzzyCompare(m_d->zoom, zoom)) {
        m_d->zoom = zoom;
        m_d->horizontalRuler->setDefaultSectionSize(newSectionSize);

        // For some reason simple update doesn't work here,
        // so reset the whole header
        m_d->horizontalRuler->reset();
        slotUpdateInfiniteFramesCount();
    }
}

void FramesTableView::setZoomDouble(double zoom)
{
    setZoom(zoom);
}

void FramesTableView::slotZoomButtonPressed()
{
    m_d->initialDragZoomValue = zoom();
}

void FramesTableView::slotZoomButtonChanged(int value)
{
    qreal zoomCoeff = std::pow(2.0, qreal(value) / KisDraggableToolButton::unitRadius());
    setZoom(m_d->initialDragZoomValue * zoomCoeff);
}

void FramesTableView::slotUpdateInfiniteFramesCount()
{
    if (horizontalScrollBar()->isSliderDown()) return;

    const int sectionWidth = m_d->horizontalRuler->defaultSectionSize();
    const int calculatedIndex = (horizontalScrollBar()->value() * sectionWidth + m_d->horizontalRuler->width() - 1) / sectionWidth;

    m_d->model->setLastVisibleFrame(calculatedIndex);
}

void FramesTableView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QTableView::currentChanged(current, previous);

    m_d->model->setData(previous, false, TimelineFramesModel::ActiveLayerRole);
    m_d->model->setData(current, true, TimelineFramesModel::ActiveLayerRole);

    m_d->model->setData(previous, false, TimelineFramesModel::ActiveFrameRole);
    m_d->model->setData(current, true, TimelineFramesModel::ActiveFrameRole);
}

void FramesTableView::slotReselectCurrentIndex()
{
    QModelIndex index = currentIndex();
    currentChanged(index, index);
}

void FramesTableView::slotDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    int selectedRow = -1;
    int selectedColumn = -1;

    for (int i = topLeft.row(); i <= bottomRight.row(); i++) {
        QVariant value = m_d->model->data(
            m_d->model->index(i, topLeft.column()),
            TimelineFramesModel::ActiveLayerRole);

        if (value.isValid() && value.toBool()) {
            selectedRow = i;
            break;
        }
    }

    for (int j = topLeft.column(); j <= bottomRight.column(); j++) {
        QVariant value = m_d->model->data(
            m_d->model->index(topLeft.row(), j),
            TimelineFramesModel::ActiveFrameRole);

        if (value.isValid() && value.toBool()) {
            selectedColumn = j;
            break;
        }
    }

    QModelIndex index = currentIndex();

    if (!index.isValid()) return;

    if (selectedRow == -1) {
        selectedRow = index.row();
    }

    if (selectedColumn == -1) {
        selectedColumn = index.column();
    }

    if (selectedRow != index.row() ||
        selectedColumn != index.column()) {

        if (!m_d->dragInProgress) {
            setCurrentIndex(m_d->model->index(selectedRow, selectedColumn));
        }
    }
}

void FramesTableView::slotHeaderDataChanged(Qt::Orientation orientation, int first, int last)
{
    Q_UNUSED(first);
    Q_UNUSED(last);

    if (orientation == Qt::Horizontal) {
        const int newFps = m_d->model->headerData(0, Qt::Horizontal, TimelineFramesModel::FramesPerSecondRole).toInt();

        if (newFps != m_d->fps) {
            setFramesPerSecond(newFps);
        }
    }
}

void FramesTableView::dragEnterEvent(QDragEnterEvent *event)
{
    m_d->dragInProgress = true;
    QTableView::dragEnterEvent(event);
}

void FramesTableView::dragMoveEvent(QDragMoveEvent *event)
{
    m_d->dragInProgress = true;

    QTableView::dragMoveEvent(event);

    if (event->isAccepted()) {
        QModelIndex index = indexAt(event->pos());
        if (!m_d->model->canDropFrameData(event->mimeData(), index)) {
            event->ignore();
        } else {
            selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
        }
    }
}

void FramesTableView::dropEvent(QDropEvent *event)
{
    m_d->dragInProgress = false;

    QAbstractItemView::dropEvent(event);
    setCurrentIndex(currentIndex());
}

void FramesTableView::dragLeaveEvent(QDragLeaveEvent *event)
{
    m_d->dragInProgress = false;

    QAbstractItemView::dragLeaveEvent(event);
    setCurrentIndex(currentIndex());
}

void FramesTableView::mousePressEvent(QMouseEvent *event)
{
    QPersistentModelIndex index = indexAt(event->pos());

    if (index.isValid() &&
        event->button() == Qt::RightButton &&
        m_d->model->data(index, TimelineFramesModel::FrameEditableRole).toBool()) {

        model()->setData(index, true, TimelineFramesModel::ActiveLayerRole);
        model()->setData(index, true, TimelineFramesModel::ActiveFrameRole);

        if (model()->data(index, TimelineFramesModel::FrameExistsRole).toBool()) {
            m_d->frameEditingMenu->exec(event->globalPos());
        } else {
            m_d->frameCreationMenu->exec(event->globalPos());
        }

    } else {
        QAbstractItemView::mousePressEvent(event);
    }
}

void FramesTableView::slotUpdateLayersMenu()
{
    QAction *action = 0;

    m_d->existingLayersMenu->clear();

    QVariant value = model()->headerData(0, Qt::Vertical, TimelineFramesModel::OtherLayersRole);
    if (value.isValid()) {
        TimelineFramesModel::OtherLayersList list = value.value<TimelineFramesModel::OtherLayersList>();

        int i = 0;
        foreach (const TimelineFramesModel::OtherLayer &l, list) {
            action = m_d->existingLayersMenu->addAction(l.name);
            action->setData(i++);
        }
    }
}

void FramesTableView::slotLayerContextMenuRequested(const QPoint &globalPos)
{
    m_d->layerEditingMenu->exec(globalPos);
}

void FramesTableView::slotAddNewLayer()
{
    QModelIndex index = currentIndex();
    if (!index.isValid()) return;

    const int newRow = index.row();
    model()->insertRow(newRow);
}

void FramesTableView::slotAddExistingLayer(QAction *action)
{
    QModelIndex index = currentIndex();
    if (!index.isValid()) return;

    QVariant value = action->data();

    if (value.isValid()) {
        const int newRow = index.row() + 1;
        m_d->model->insertOtherLayer(value.toInt(), newRow);
    }
}

void FramesTableView::slotRemoveLayer()
{
    QModelIndex index = currentIndex();
    if (!index.isValid()) return;

    model()->removeRow(index.row());
}

void FramesTableView::slotHideLayerFromTimeline()
{
    QModelIndex index = currentIndex();
    if (!index.isValid()) return;

    m_d->model->hideLayer(index.row());
}

void FramesTableView::slotNewFrame()
{
    QModelIndex index = currentIndex();
    if (!index.isValid() ||
        !m_d->model->data(index, TimelineFramesModel::FrameEditableRole).toBool()) {

        return;
    }

    m_d->model->createFrame(index);
}

void FramesTableView::slotCopyFrame()
{
    QModelIndex index = currentIndex();
    if (!index.isValid() ||
        !m_d->model->data(index, TimelineFramesModel::FrameEditableRole).toBool()) {

        return;
    }

    m_d->model->copyFrame(index);
}

void FramesTableView::slotRemoveFrame()
{
    QModelIndex index = currentIndex();
    if (!index.isValid() ||
        !m_d->model->data(index, TimelineFramesModel::FrameEditableRole).toBool()) {

        return;
    }

    m_d->model->removeFrame(index);
}
