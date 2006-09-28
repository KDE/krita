/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include "KoLineBorder.h"
#include "KoViewConverter.h"
#include "KoShape.h"

#include <QPainterPath>

KoLineBorder::KoLineBorder()
: m_lineSize(-1)
, m_color(Qt::black)
{
}

KoLineBorder::KoLineBorder(double lineSize, QColor color)
: m_lineSize(lineSize)
, m_color(color)
{
}

KoInsets* KoLineBorder::borderInsets(const KoShape *shape, KoInsets &insets) {
    Q_UNUSED(shape);
    double lineSize = m_lineSize;
    if(lineSize < 0)
         lineSize = 1;
    lineSize /= 2; // since we draw a line half inside, and half outside the object.
    insets.top = lineSize;
    insets.bottom = lineSize;
    insets.left = lineSize;
    insets.right = lineSize;
    return &insets;
}

bool KoLineBorder::hasTransparency() {
    return m_color.alpha() > 0;
}

void KoLineBorder::paintBorder(KoShape *shape, QPainter &painter, const KoViewConverter &converter) {
    double zoomX, zoomY;
    converter.zoom(&zoomX, &zoomY);
    painter.scale(zoomX, zoomY);

    QPen pen;
    pen.setColor(m_color);
    pen.setWidthF(qMax(0.0, m_lineSize));
    pen.setJoinStyle(Qt::MiterJoin);
    painter.setPen(pen);
    painter.drawPath(shape->outline());
}
