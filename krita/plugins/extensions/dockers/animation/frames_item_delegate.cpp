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

#include "frames_item_delegate.h"

#include <QPen>
#include <QPainter>

#include "timeline_frames_model_base.h"


FramesItemDelegate::FramesItemDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

FramesItemDelegate::~FramesItemDelegate()
{
}

void FramesItemDelegate::paintActiveFrameSelector(QPainter *painter, const QRect &rc)
{
    QColor baseColor = QColor(200, 220, 150);
    QColor colorDark = baseColor.darker(130);
    const int lineWidth = rc.width() > 20 ? 4 : 2;

    const int x0 = rc.x();
    const int y0 = rc.y();
    const int x1 = rc.right();
    const int y1 = rc.bottom();

    QVector<QLine> linesDark;
    linesDark << QLine(x0 + lineWidth / 2, y0, x0  + lineWidth / 2, y1);
    linesDark << QLine(x1 -  lineWidth / 2 + 1, y0, x1 - lineWidth / 2 + 1, y1);

    QPen oldPen = painter->pen();
    painter->setPen(QPen(colorDark, lineWidth));
    painter->drawLines(linesDark);
    painter->setPen(oldPen);
}

void FramesItemDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    QItemDelegate::paint(painter, option, index);

    QVariant value = index.data(TimelineFramesModelBase::ActiveFrameRole);
    if (value.isValid() && value.toBool()) {
        paintActiveFrameSelector(painter, option.rect);
    }
}
