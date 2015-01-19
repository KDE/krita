/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License, or (at your option)
 *  any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "TimelineItemDelegate.h"

#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QGradient>

#include <kis_config.h>
#include <kis_animation_frame.h>

TimelineItemDelegate::TimelineItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

void TimelineItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    KisAnimationFrame *frame = qobject_cast<KisAnimationFrame*>(index.data().value<QObject*>());

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    }
    else {
        painter->fillRect(option.rect, option.palette.base());
    }

    if (frame) {

        QRect rc = option.rect.adjusted(4, 4, -4, -4);
        painter->setBrush(option.palette.buttonText());
        painter->setPen(QPen(option.palette.button(), 2));

        painter->drawRect(rc);
    }
    else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}
