/*
 *  Copyright (c) 2015 Jouni Pentikäinen <mctyyppi42@gmail.com>
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

#ifndef _KEYFRAME_CHANNEL_DELEGATE_H_
#define _KEYFRAME_CHANNEL_DELEGATE_H_

#include <QAbstractItemDelegate>
#include <QItemSelectionModel>

class TimelineView;

class KeyframeChannelDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    KeyframeChannelDelegate(QObject *parent, TimelineView *view);

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &styleOpt, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    int getKeyframeAt(const QModelIndex &channelIndex, int x);
    void selectKeyframesBetween(const QModelIndex &channelIndex, int fromX, int toX, QItemSelectionModel::SelectionFlags command);

private:
    TimelineView *view;
    QPixmap keyframeIcons;

    void paintKeyframe(QPainter *painter, const QStyleOptionViewItem &styleOpt, QModelIndex keyframe, bool isSelected) const;
};

#endif
