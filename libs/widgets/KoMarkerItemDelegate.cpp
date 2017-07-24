/* This file is part of the KDE project
 * Copyright (C) 2011 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2011 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoMarkerItemDelegate.h"

#include <KoPathShape.h>
#include <KoMarker.h>

#include <QPainter>
#include <QPen>

#include "kis_global.h"

KoMarkerItemDelegate::KoMarkerItemDelegate(KoFlake::MarkerPosition position, QObject *parent)
: QAbstractItemDelegate(parent)
, m_position(position)
{
}

KoMarkerItemDelegate::~KoMarkerItemDelegate()
{
}

void KoMarkerItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    }

    QPen pen(option.palette.text(), 2);
    KoMarker *marker = index.data(Qt::DecorationRole).value<KoMarker*>();
    drawMarkerPreview(painter, option.rect.adjusted(1, 0, -1, 0), pen, marker, m_position);
}

QSize KoMarkerItemDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    return QSize(80,30);
}

void KoMarkerItemDelegate::drawMarkerPreview(QPainter *painter, const QRect &rect, const QPen &pen, KoMarker *marker, KoFlake::MarkerPosition position)
{
    if (marker) {
        marker->drawPreview(painter, rect, pen, position);
    } else {
        const qreal centerY = QRectF(rect).center().y();
        QPen oldPen = painter->pen();
        painter->setPen(pen);
        painter->drawLine(rect.left(), centerY, rect.right(), centerY);
        painter->setPen(oldPen);
    }
}
