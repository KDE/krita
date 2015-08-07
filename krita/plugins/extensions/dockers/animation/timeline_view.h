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

#ifndef _TIMELINE_VIEW_H_
#define _TIMELINE_VIEW_H_

#include <QTreeView>

#include "keyframe_channel_delegate.h"

class KisTimelineModel;
class TimelineWidget;

class TimelineView : public QTreeView
{
    Q_OBJECT

public:
    TimelineView(QWidget *parent, TimelineWidget *timelineWidget);

    QModelIndex indexAt(const QPoint &point) const;
    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command);

    bool isDragging();
    float dragOffset();

    int timeToPosition(int time) const;
    int positionToTime(int x) const;
    bool isWithingView(int time) const;

    void setModel(QAbstractItemModel *model);

protected:
    bool viewportEvent(QEvent *e);

private slots:
    void rowsChanged();

private:
    TimelineWidget *m_timelineWidget;
    KeyframeChannelDelegate *m_channelDelegate;

    bool m_canStartDrag;
    bool m_isDragging;
    float m_dragStart;
    float m_dragOffset;

    int getTime(const QModelIndex &channelIndex, int index) const;
    int findKeyframe(const QModelIndex &channelIndex, int x, bool exact) const;
    void selectKeyframesBetween(const QModelIndex &channelIndex, int fromX, int toX, QItemSelectionModel::SelectionFlags command);
    int getKeyframeAt(const QModelIndex &channelIndex, int x) const;
};

#endif
