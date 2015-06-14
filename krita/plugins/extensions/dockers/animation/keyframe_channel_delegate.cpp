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

#include <QPainter>
#include <QItemSelectionModel>

#include "KoIcon.h"
#include "keyframe_channel_delegate.h"
#include "kis_timeline_model.h"
#include "timeline_view.h"

KeyframeChannelDelegate::KeyframeChannelDelegate(QObject *parent, TimelineView *view)
    : QAbstractItemDelegate(parent)
    , view(view)
{
    keyframeIcons = koIcon("light_timeline_keyframe").pixmap(16, 16, QIcon::Normal);
}

void KeyframeChannelDelegate::paint(QPainter *painter, const QStyleOptionViewItem &styleOpt, const QModelIndex &index) const
{
    QItemSelectionModel *selection = view->selectionModel();

    painter->save();

    int count = index.model()->rowCount(index);

    for (int i=0; i<count; i++) {
        QModelIndex keyframe = index.child(i, 0);
        paintKeyframe(painter, styleOpt, keyframe, selection->isSelected(keyframe));
    }

    painter->restore();
}

void KeyframeChannelDelegate::paintKeyframe(QPainter *painter, const QStyleOptionViewItem &styleOpt, QModelIndex keyframe, bool isSelected) const
{
    int time = keyframe.data(KisTimelineModel::TimeRole).toInt();

    if (isSelected && view->isDragging()) {
        int oldPos = view->timeToPosition(time);
        time = view->positionToTime(oldPos + view->dragOffset());
    }

    int x = view->timeToPosition(time);
    int y = styleOpt.rect.y();

    if (isSelected) {
        painter->drawPixmap(QRect(x, y, 7, 12), keyframeIcons, QRect(8, 0, 7, 12));
    } else {
        painter->drawPixmap(QRect(x, y, 7, 12), keyframeIcons, QRect(0, 0, 7, 12));
    }
}

QSize KeyframeChannelDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(option.rect.width(), 12);
}
