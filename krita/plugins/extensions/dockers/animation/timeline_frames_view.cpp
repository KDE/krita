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

#include "timeline_frames_view.h"

#include "timeline_frames_model.h"

#include "timeline_ruler_header.h"
#include "timeline_layers_header.h"

#include <cmath>
#include <limits>

#include <QPainter>

#include <QHeaderView>
#include <QDropEvent>
#include <QToolButton>
#include <QMenu>
#include <QScrollBar>
#include <QIcon>
#include <QDrag>

#include "kis_debug.h"
#include "timeline_frames_item_delegate.h"

#include "kis_draggable_tool_button.h"

#include "kis_icon_utils.h"

#include "kis_animation_utils.h"
#include "kis_custom_modifiers_catcher.h"
#include "kis_action.h"

typedef QPair<QRect, QModelIndex> QItemViewPaintPair;
typedef QList<QItemViewPaintPair> QItemViewPaintPairs;

struct TimelineFramesView::Private
{
    Private(TimelineFramesView *_q)
        : q(_q),
          fps(1),
          zoom(1.0),
          initialDragZoomValue(1.0),
          zoomStillPointIndex(-1),
          zoomStillPointOriginalOffset(0),
          dragInProgress(false),
          modifiersCatcher(0)
    {}

    TimelineFramesView *q;

    TimelineFramesModel *model;
    TimelineRulerHeader *horizontalRuler;
    TimelineLayersHeader *layersHeader;
    int fps;
    qreal zoom;
    qreal initialDragZoomValue;
    int zoomStillPointIndex;
    int zoomStillPointOriginalOffset;
    QPoint initialDragPanValue;
    QPoint startZoomPanDragPos;

    QToolButton *addLayersButton;
    KisAction *showHideLayerAction;
    QMenu *layerEditingMenu;
    QMenu *existingLayersMenu;

    QMenu *frameCreationMenu;
    QMenu *frameEditingMenu;

    QMenu *multipleFrameEditingMenu;
    QMap<QString, KisAction*> globalActions;


    KisDraggableToolButton *zoomDragButton;

    bool dragInProgress;

    KisCustomModifiersCatcher *modifiersCatcher;
    QPoint lastPressedPosition;


    QStyleOptionViewItemV4 viewOptionsV4() const;
    QItemViewPaintPairs draggablePaintPairs(const QModelIndexList &indexes, QRect *r) const;
    QPixmap renderToPixmap(const QModelIndexList &indexes, QRect *r) const;

};

TimelineFramesView::TimelineFramesView(QWidget *parent)
    : QTableView(parent),
      m_d(new Private(this))
{
    m_d->modifiersCatcher = new KisCustomModifiersCatcher(this);
    m_d->modifiersCatcher->addModifier("pan-zoom", Qt::Key_Space);
    m_d->modifiersCatcher->addModifier("offset-frame", Qt::Key_Alt);

    setCornerButtonEnabled(false);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    setItemDelegate(new TimelineFramesItemDelegate(this));

    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDefaultDropAction(Qt::MoveAction);

    m_d->horizontalRuler = new TimelineRulerHeader(this);

#if QT_VERSION < 0x050000
    m_d->horizontalRuler->setResizeMode(QHeaderView::Fixed);
#else
    m_d->horizontalRuler->setSectionResizeMode(QHeaderView::Fixed);
#endif

    m_d->horizontalRuler->setDefaultSectionSize(18);
    this->setHorizontalHeader(m_d->horizontalRuler);

    m_d->layersHeader = new TimelineLayersHeader(this);

#if QT_VERSION < 0x050000
    m_d->layersHeader->setResizeMode(QHeaderView::Fixed);
#else
    m_d->layersHeader->setSectionResizeMode(QHeaderView::Fixed);
#endif

    m_d->layersHeader->setDefaultSectionSize(24);
    m_d->layersHeader->setMinimumWidth(60);
    m_d->layersHeader->setHighlightSections(true);

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

    m_d->showHideLayerAction = new KisAction(KisAnimationUtils::showLayerActionName, this);
    m_d->showHideLayerAction->setActivationFlags(KisAction::ACTIVE_LAYER);
    connect(m_d->showHideLayerAction, SIGNAL(triggered()), SLOT(slotHideLayerFromTimeline()));
    m_d->showHideLayerAction->setCheckable(true);
    m_d->globalActions.insert("show_in_timeline", m_d->showHideLayerAction);
    m_d->layerEditingMenu->addAction(m_d->showHideLayerAction);

    m_d->layerEditingMenu->addAction("Remove Layer", this, SLOT(slotRemoveLayer()));

    connect(m_d->existingLayersMenu, SIGNAL(aboutToShow()), SLOT(slotUpdateLayersMenu()));
    connect(m_d->existingLayersMenu, SIGNAL(triggered(QAction*)), SLOT(slotAddExistingLayer(QAction*)));

    connect(m_d->layersHeader, SIGNAL(sigRequestContextMenu(const QPoint&)), SLOT(slotLayerContextMenuRequested(const QPoint&)));

    m_d->addLayersButton->setMenu(m_d->layerEditingMenu);

    m_d->frameCreationMenu = new QMenu(this);
    m_d->frameCreationMenu->addAction(KisAnimationUtils::addFrameActionName, this, SLOT(slotNewFrame()));
    m_d->frameCreationMenu->addAction(KisAnimationUtils::duplicateFrameActionName, this, SLOT(slotCopyFrame()));

    m_d->frameEditingMenu = new QMenu(this);
    m_d->frameEditingMenu->addAction(KisAnimationUtils::removeFrameActionName, this, SLOT(slotRemoveFrame()));

    m_d->multipleFrameEditingMenu = new QMenu(this);
    m_d->multipleFrameEditingMenu->addAction(KisAnimationUtils::removeFramesActionName, this, SLOT(slotRemoveFrame()));

    m_d->zoomDragButton = new KisDraggableToolButton(this);
    m_d->zoomDragButton->setAutoRaise(true);
    m_d->zoomDragButton->setIcon(KisIconUtils::loadIcon("zoom-in"));
    m_d->zoomDragButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_d->zoomDragButton->setToolTip("Zoom Timeline. Hold down and drag left or right.");
    m_d->zoomDragButton->setPopupMode(QToolButton::InstantPopup);
    connect(m_d->zoomDragButton, SIGNAL(valueChanged(int)), SLOT(slotZoomButtonChanged(int)));
    connect(m_d->zoomDragButton, SIGNAL(pressed()), SLOT(slotZoomButtonPressed()));

    setFramesPerSecond(12);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
}

TimelineFramesView::~TimelineFramesView()
{
}

QMap<QString, KisAction*> TimelineFramesView::globalActions() const
{
    return m_d->globalActions;
}

void resizeToMinimalSize(QAbstractButton *w, int minimalSize) {
    QSize buttonSize = w->sizeHint();
    if (buttonSize.height() > minimalSize) {
        buttonSize = QSize(minimalSize, minimalSize);
    }
    w->resize(buttonSize);
}

void TimelineFramesView::updateGeometries()
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

void TimelineFramesView::setModel(QAbstractItemModel *model)
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

    connect(m_d->model, SIGNAL(sigInfiniteTimelineUpdateNeeded()),
            this, SLOT(slotUpdateInfiniteFramesCount()));
}

void TimelineFramesView::setFramesPerSecond(int fps)
{
    m_d->fps = fps;
    m_d->horizontalRuler->setFramePerSecond(fps);

    // For some reason simple update sometimes doesn't work here, so
    // reset the whole header
    //
    // m_d->horizontalRuler->reset();
}

qreal TimelineFramesView::zoom() const
{
    return m_d->zoom;
}

void TimelineFramesView::setZoom(qreal zoom)
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
        QPersistentModelIndex index = currentIndex();
        m_d->horizontalRuler->reset();
        setCurrentIndex(index);

        slotUpdateInfiniteFramesCount();
    }
}

void TimelineFramesView::setZoomDouble(double zoom)
{
    setZoom(zoom);
}

void TimelineFramesView::slotZoomButtonPressed()
{
    m_d->zoomStillPointIndex = currentIndex().column();
    slotZoomButtonPressedImpl();
}

void TimelineFramesView::slotZoomButtonPressedImpl()
{
    const int w = m_d->horizontalRuler->defaultSectionSize();

    m_d->zoomStillPointOriginalOffset =
        w * m_d->zoomStillPointIndex -
        horizontalScrollBar()->value();

    m_d->initialDragZoomValue = zoom();
}

void TimelineFramesView::slotZoomButtonChanged(int value)
{
    qreal zoomCoeff = std::pow(2.0, qreal(value) / KisDraggableToolButton::unitRadius());
    setZoom(m_d->initialDragZoomValue * zoomCoeff);

    const int w = m_d->horizontalRuler->defaultSectionSize();
    horizontalScrollBar()->setValue(w * m_d->zoomStillPointIndex - m_d->zoomStillPointOriginalOffset);
}

void TimelineFramesView::slotUpdateInfiniteFramesCount()
{
    if (horizontalScrollBar()->isSliderDown()) return;

    const int sectionWidth = m_d->horizontalRuler->defaultSectionSize();
    const int calculatedIndex =
        (horizontalScrollBar()->value() +
         m_d->horizontalRuler->width() - 1) / sectionWidth;

    m_d->model->setLastVisibleFrame(calculatedIndex);
}

void TimelineFramesView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QTableView::currentChanged(current, previous);

    if (previous.column() != current.column()) {
        m_d->model->setData(previous, false, TimelineFramesModel::ActiveFrameRole);
        m_d->model->setData(current, true, TimelineFramesModel::ActiveFrameRole);
    }
}

void TimelineFramesView::slotReselectCurrentIndex()
{
    QModelIndex index = currentIndex();
    currentChanged(index, index);
}

void TimelineFramesView::slotDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    int selectedColumn = -1;

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

    if (!index.isValid() && selectedColumn < 0) {
        return;
    }


    if (selectedColumn == -1) {
        selectedColumn = index.column();
    }

    if (selectedColumn != index.column() && !m_d->dragInProgress) {
        setCurrentIndex(m_d->model->index(index.row(), selectedColumn));
    }
}

void TimelineFramesView::slotHeaderDataChanged(Qt::Orientation orientation, int first, int last)
{
    Q_UNUSED(first);
    Q_UNUSED(last);

    if (orientation == Qt::Horizontal) {
        const int newFps = m_d->model->headerData(0, Qt::Horizontal, TimelineFramesModel::FramesPerSecondRole).toInt();

        if (newFps != m_d->fps) {
            setFramesPerSecond(newFps);
        }
    } else /* if (orientation == Qt::Vertical) */ {
        updateShowInTimeline();
    }
}

void TimelineFramesView::rowsInserted(const QModelIndex& parent, int start, int end)
{
    QTableView::rowsInserted(parent, start, end);
    updateShowInTimeline();
}


inline bool isIndexDragEnabled(QAbstractItemModel *model, const QModelIndex &index) {
    return (model->flags(index) & Qt::ItemIsDragEnabled);
}

QStyleOptionViewItemV4 TimelineFramesView::Private::viewOptionsV4() const
{
    QStyleOptionViewItemV4 option = q->viewOptions();
    option.locale = q->locale();
    option.locale.setNumberOptions(QLocale::OmitGroupSeparator);
    option.widget = q;
    return option;
}

QItemViewPaintPairs TimelineFramesView::Private::draggablePaintPairs(const QModelIndexList &indexes, QRect *r) const
{
    Q_ASSERT(r);
    QRect &rect = *r;
    const QRect viewportRect = q->viewport()->rect();
    QItemViewPaintPairs ret;
    for (int i = 0; i < indexes.count(); ++i) {
        const QModelIndex &index = indexes.at(i);
        const QRect current = q->visualRect(index);
        if (current.intersects(viewportRect)) {
            ret += qMakePair(current, index);
            rect |= current;
        }
    }
    rect &= viewportRect;
    return ret;
}

QPixmap TimelineFramesView::Private::renderToPixmap(const QModelIndexList &indexes, QRect *r) const
{
    Q_ASSERT(r);
    QItemViewPaintPairs paintPairs = draggablePaintPairs(indexes, r);
    if (paintPairs.isEmpty())
        return QPixmap();
    QPixmap pixmap(r->size());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QStyleOptionViewItemV4 option = viewOptionsV4();
    option.state |= QStyle::State_Selected;
    for (int j = 0; j < paintPairs.count(); ++j) {
        option.rect = paintPairs.at(j).first.translated(-r->topLeft());
        const QModelIndex &current = paintPairs.at(j).second;
        //adjustViewOptionsForIndex(&option, current);
        q->itemDelegate(current)->paint(&painter, option, current);
    }
    return pixmap;
}

void TimelineFramesView::startDrag(Qt::DropActions supportedActions)
{
    QModelIndexList indexes = selectionModel()->selectedIndexes();

    if (!indexes.isEmpty() && m_d->modifiersCatcher->modifierPressed("offset-frame")) {
        QVector<int> rows;
        int leftmostColumn = std::numeric_limits<int>::max();

        Q_FOREACH (const QModelIndex &index, indexes) {
            leftmostColumn = qMin(leftmostColumn, index.column());
            if (!rows.contains(index.row())) {
                rows.append(index.row());
            }
        }

        const int lastColumn = m_d->model->columnCount() - 1;

        selectionModel()->clear();
        Q_FOREACH (const int row, rows) {
            QItemSelection sel(m_d->model->index(row, leftmostColumn), m_d->model->index(row, lastColumn));
            selectionModel()->select(sel, QItemSelectionModel::Select);
        }

        supportedActions = Qt::MoveAction;

        {
            QModelIndexList indexes = selectedIndexes();
            for(int i = indexes.count() - 1 ; i >= 0; --i) {
                if (!isIndexDragEnabled(m_d->model, indexes.at(i)))
                    indexes.removeAt(i);
            }

            selectionModel()->clear();

            if (indexes.count() > 0) {
                QMimeData *data = m_d->model->mimeData(indexes);
                if (!data)
                    return;
                QRect rect;
                QPixmap pixmap = m_d->renderToPixmap(indexes, &rect);
                rect.adjust(horizontalOffset(), verticalOffset(), 0, 0);
                QDrag *drag = new QDrag(this);
                drag->setPixmap(pixmap);
                drag->setMimeData(data);
                drag->setHotSpot(m_d->lastPressedPosition - rect.topLeft());
                drag->exec(supportedActions, Qt::MoveAction);
            }
        }
    } else {
        QAbstractItemView::startDrag(supportedActions);
    }
}

void TimelineFramesView::dragEnterEvent(QDragEnterEvent *event)
{
    m_d->dragInProgress = true;
    m_d->model->setScrubState(true);

    QTableView::dragEnterEvent(event);
}

void TimelineFramesView::dragMoveEvent(QDragMoveEvent *event)
{
    m_d->dragInProgress = true;
    m_d->model->setScrubState(true);

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

void TimelineFramesView::dropEvent(QDropEvent *event)
{
    m_d->dragInProgress = false;
    m_d->model->setScrubState(false);

    QAbstractItemView::dropEvent(event);
    setCurrentIndex(currentIndex());
}

void TimelineFramesView::dragLeaveEvent(QDragLeaveEvent *event)
{
    m_d->dragInProgress = false;
    m_d->model->setScrubState(false);

    QAbstractItemView::dragLeaveEvent(event);
    setCurrentIndex(currentIndex());
}

void TimelineFramesView::mousePressEvent(QMouseEvent *event)
{
    QPersistentModelIndex index = indexAt(event->pos());

    if (m_d->modifiersCatcher->modifierPressed("pan-zoom")) {
        m_d->startZoomPanDragPos = event->pos();

        if (event->button() == Qt::RightButton) {
            // TODO: try calculate index under mouse cursor even when
            //       it is outside any visible row
            m_d->zoomStillPointIndex =
                index.isValid() ? index.column() : currentIndex().column();

            slotZoomButtonPressedImpl();
        } else if (event->button() == Qt::LeftButton) {
            m_d->initialDragPanValue =
                QPoint(horizontalScrollBar()->value(),
                       verticalScrollBar()->value());
        }
        event->accept();

    } else if (event->button() == Qt::RightButton) {

        int numSelectedItems = selectionModel()->selectedIndexes().size();

        if (index.isValid() &&
            numSelectedItems <= 1 &&
            m_d->model->data(index, TimelineFramesModel::FrameEditableRole).toBool()) {

            model()->setData(index, true, TimelineFramesModel::ActiveLayerRole);
            model()->setData(index, true, TimelineFramesModel::ActiveFrameRole);
            setCurrentIndex(index);

            if (model()->data(index, TimelineFramesModel::FrameExistsRole).toBool()) {
                m_d->frameEditingMenu->exec(event->globalPos());
            } else {
                m_d->frameCreationMenu->exec(event->globalPos());
            }
        } else if (numSelectedItems > 1) {
            m_d->multipleFrameEditingMenu->exec(event->globalPos());
        }

    } else {
        if (index.isValid()) {
            m_d->model->setLastClickedIndex(index);
        }

        m_d->lastPressedPosition =
            QPoint(horizontalOffset(), verticalOffset()) + event->pos();

        QAbstractItemView::mousePressEvent(event);
    }
}

void TimelineFramesView::mouseMoveEvent(QMouseEvent *e)
{
    if (m_d->modifiersCatcher->modifierPressed("pan-zoom")) {
        QPoint diff = e->pos() - m_d->startZoomPanDragPos;

        if (e->buttons() & Qt::RightButton) {
            slotZoomButtonChanged(m_d->zoomDragButton->calculateValue(diff));
        } else if (e->buttons() & Qt::LeftButton) {

            QPoint offset = QPoint(m_d->initialDragPanValue.x() - diff.x(),
                                   m_d->initialDragPanValue.y() - diff.y());

            const int height = m_d->layersHeader->defaultSectionSize();

            horizontalScrollBar()->setValue(offset.x());
            verticalScrollBar()->setValue(offset.y() / height);
        }
        e->accept();
    } else {
        m_d->model->setScrubState(true);
        QTableView::mouseMoveEvent(e);
    }
}

void TimelineFramesView::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_d->modifiersCatcher->modifierPressed("pan-zoom")) {
        e->accept();
    } else {
        m_d->model->setScrubState(false);
        QTableView::mouseReleaseEvent(e);
    }
}


void TimelineFramesView::slotUpdateLayersMenu()
{
    QAction *action = 0;

    m_d->existingLayersMenu->clear();

    QVariant value = model()->headerData(0, Qt::Vertical, TimelineFramesModel::OtherLayersRole);
    if (value.isValid()) {
        TimelineFramesModel::OtherLayersList list = value.value<TimelineFramesModel::OtherLayersList>();

        int i = 0;
        Q_FOREACH (const TimelineFramesModel::OtherLayer &l, list) {
            action = m_d->existingLayersMenu->addAction(l.name);
            action->setData(i++);
        }
    }
}

void TimelineFramesView::slotLayerContextMenuRequested(const QPoint &globalPos)
{
    m_d->layerEditingMenu->exec(globalPos);
}

void TimelineFramesView::updateShowInTimeline()
{
    const int row = m_d->model->activeLayerRow();
    const bool status = m_d->model->headerData(row, Qt::Vertical, TimelineFramesModel::LayerUsedInTimelineRole).toBool();
    m_d->showHideLayerAction->setChecked(status);
}

void TimelineFramesView::slotAddNewLayer()
{
    QModelIndex index = currentIndex();
    const int newRow = index.isValid() ? index.row() : 0;
    model()->insertRow(newRow);
}

void TimelineFramesView::slotAddExistingLayer(QAction *action)
{
    QVariant value = action->data();

    if (value.isValid()) {
        QModelIndex index = currentIndex();
        const int newRow = index.isValid() ? index.row() + 1 : 0;

        m_d->model->insertOtherLayer(value.toInt(), newRow);
    }
}

void TimelineFramesView::slotRemoveLayer()
{
    QModelIndex index = currentIndex();
    if (!index.isValid()) return;

    model()->removeRow(index.row());
}

void TimelineFramesView::slotHideLayerFromTimeline()
{
    const int row = m_d->model->activeLayerRow();
    const bool status = m_d->model->headerData(row, Qt::Vertical, TimelineFramesModel::LayerUsedInTimelineRole).toBool();
    m_d->model->setHeaderData(row, Qt::Vertical, !status, TimelineFramesModel::LayerUsedInTimelineRole);
}

void TimelineFramesView::slotNewFrame()
{
    QModelIndex index = currentIndex();
    if (!index.isValid() ||
        !m_d->model->data(index, TimelineFramesModel::FrameEditableRole).toBool()) {

        return;
    }

    m_d->model->createFrame(index);
}

void TimelineFramesView::slotCopyFrame()
{
    QModelIndex index = currentIndex();
    if (!index.isValid() ||
        !m_d->model->data(index, TimelineFramesModel::FrameEditableRole).toBool()) {

        return;
    }

    m_d->model->copyFrame(index);
}

void TimelineFramesView::slotRemoveFrame()
{
    QModelIndexList indexes = selectionModel()->selectedIndexes();

    QModelIndexList::iterator it = indexes.begin();
    QModelIndexList::iterator end = indexes.end();

    while (it != end) {
        if (!m_d->model->data(*it, TimelineFramesModel::FrameEditableRole).toBool()) {
            it = indexes.erase(it);
        } else {
            ++it;
        }
    }

    if (!indexes.isEmpty()) {
        m_d->model->removeFrames(indexes);
    }
}
