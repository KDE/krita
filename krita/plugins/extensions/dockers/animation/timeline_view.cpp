/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
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

#include "timeline_view.h"

#include <QHeaderView>
#include <QEvent>
#include <QMouseEvent>
#include <QDebug>

#include "kis_timeline_model.h"
#include "timeline_widget.h"
#include "kundo2magicstring.h"

TimelineView::TimelineView(QWidget *parent, TimelineWidget *timelineWidget)
    : QTreeView(parent)
    , m_timelineWidget(timelineWidget)
    , m_channelDelegate(new KeyframeChannelDelegate(this, this))
{
    setItemDelegateForColumn(1, m_channelDelegate);

    setVerticalScrollMode(ScrollPerPixel);
    //setSelectionMode(ExtendedSelection); // TODO
    setSelectionMode(SingleSelection);
    setSelectionBehavior(SelectItems);
    header()->hide();
    setItemsExpandable(false);
}

QModelIndex TimelineView::indexAt(const QPoint &point) const
{
    QModelIndex cell = QTreeView::indexAt(point);
    if (cell.column() == 0) return cell;
    if (!cell.isValid()) return QModelIndex();

    int row = getKeyframeAt(cell, point.x());
    if (row < 0) return QModelIndex();

    return cell.child(row, 0);
}

void TimelineView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    QRect rectN = rect.normalized();
    QModelIndex cell = QTreeView::indexAt(rectN.topLeft());

    if (cell.column() == 1) {
        selectKeyframesBetween(cell, rectN.left(), rectN.right(), command);
    }

    viewport()->update();
}

bool TimelineView::viewportEvent(QEvent *e)
{
    if (model()) {
        switch(e->type()) {
        case QEvent::MouseButtonPress: {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(e);

            if (mouseEvent->button() == Qt::LeftButton) {
                QModelIndex index = indexAt(mouseEvent->pos());

                if (index.isValid()) {
                    QTreeView::viewportEvent(e);

                    if (selectionModel()->isSelected(index)) {
                        m_dragStart = mouseEvent->pos().x();
                        m_canStartDrag = true;
                    }
                    return true;
                }
            }
            return false;
        }

        case QEvent::MouseMove: {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(e);

            if (m_canStartDrag && abs(mouseEvent->pos().x() - m_dragStart) > 5) {
                m_isDragging = true;
                m_canStartDrag = false;
            }

            if (m_isDragging) {
                m_dragOffset = mouseEvent->pos().x() - m_dragStart;

                int oldTime = selectionModel()->selectedIndexes().first().data(KisTimelineModel::TimeRole).toInt();
                int oldPos = timeToPosition(oldTime);
                int newTime = positionToTime(oldPos + m_dragOffset);
                m_timelineWidget->scrubTo(newTime, true);

                viewport()->update();
                return true;
            }
            return false;
        }

        case QEvent::MouseButtonRelease: {
            m_canStartDrag = false;

            if (m_isDragging) {
                QModelIndexList selected = selectionModel()->selectedIndexes();

                KisTimelineModel *timelineModel = qobject_cast<KisTimelineModel*>(model());

                timelineModel->beginMacro(kundo2_i18n("Move keyframes"));

                for (int i=0; i<selected.size(); i++) {
                    QModelIndex item = selected[i];

                    int oldPos = timeToPosition(item.data(KisTimelineModel::TimeRole).toInt());
                    int newTime = positionToTime(oldPos + m_dragOffset);

                    if (newTime >= 0) {
                        model()->setData(item, newTime, KisTimelineModel::TimeRole);
                    }
                }

                int time = selected.first().data(KisTimelineModel::TimeRole).toInt();
                m_timelineWidget->scrubTo(time, false);
                timelineModel->endMacro();

                m_isDragging = false;
                return true;
            }

            return false;
        }

        default:
            return QTreeView::viewportEvent(e);
        }
    }

    return false;
}

int TimelineView::getKeyframeAt(const QModelIndex &channelIndex, int x) const
{
    return findKeyframe(channelIndex, x, true);
}

void TimelineView::selectKeyframesBetween(const QModelIndex &channelIndex, int fromX, int toX, QItemSelectionModel::SelectionFlags command)
{
    QItemSelectionModel *selection = selectionModel();

    int first = findKeyframe(channelIndex, fromX, false);
    int last = findKeyframe(channelIndex, toX, false);

    if (getTime(channelIndex, first) < positionToTime(fromX)) first++;

    for (int i=first; i <= last; i++) {
        selection->select(channelIndex.child(i, 0), command);
    }
}

int TimelineView::findKeyframe(const QModelIndex &channelIndex, int x, bool exact) const
{
    int min = 0;
    int max = channelIndex.model()->rowCount(channelIndex) - 1;

    while (max >= min) {
        int i = (max + min) / 2;
        int time = getTime(channelIndex, i);
        int x_here = timeToPosition(time);
        int x_next = timeToPosition(time + 1);

        if (x_here <= x && x < x_next) {
            return i;
        } else if (x_here < x) {
            min = i + 1;
        } else {
            max = i - 1;
        }
    }

    return (exact) ? -1 : max;
}

int TimelineView::getTime(const QModelIndex &channelIndex, int index) const
{
    return channelIndex.child(index, 0).data(KisTimelineModel::TimeRole).toInt();
}

int TimelineView::timeToPosition(int time) const
{
    return time * 8 + columnViewportPosition(1);
}

int TimelineView::positionToTime(int x) const
{
    return (x - columnViewportPosition(1)) / 8;
}

bool TimelineView::isWithingView(int time) const
{
    int x = timeToPosition(time);
    return x >= columnViewportPosition(1) && x < viewport()->width();
}

void TimelineView::setModel(QAbstractItemModel *newModel)
{
    disconnect(model(), 0, this, 0);

    QTreeView::setModel(newModel);

    connect(newModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(rowsChanged()));
    // Note: QTreeView already handles deletions, so we don't need to connect those

    setColumnWidth(0, 1);
}

void TimelineView::rowsChanged()
{
    viewport()->update();
}

bool TimelineView::isDragging()
{
    return m_isDragging;
}

float TimelineView::dragOffset()
{
    return m_dragOffset;
}

#include "timeline_view.moc"
