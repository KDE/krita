/* This file is part of the KDE project
 * Copyright (C) 2006 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#include "DivineProportionShape.h"

#include <QPen>
#include <QPainter>
#include <KDebug>

#include <math.h>

DivineProportionShape::DivineProportionShape()
    : m_divineProportion( (1.0 + sqrt(5.0)) / 2.0),
    m_orientation(TopRight),
    m_printable(false)
{
    setShapeId(DivineProportionShape_SHAPEID);
}

DivineProportionShape::~DivineProportionShape()
{
}

void DivineProportionShape::paint(QPainter &painter, const KoViewConverter &converter)
{
    if (m_printable) {
        applyConversion(painter, converter);
        draw(painter);
    }
}

void DivineProportionShape::paintDecorations(QPainter &painter, const KoViewConverter &converter, const KoCanvasBase *canvas)
{
    Q_UNUSED(canvas);
    if (!m_printable) {
        applyConversion(painter, converter);
        painter.setRenderHint(QPainter::Antialiasing);
        draw(painter);
    }
}

void DivineProportionShape::draw(QPainter &painter)
{
    painter.setPen(QPen(QColor(172, 196, 206)));
    QRectF rect(QPointF(0,0), size());
    bool top = (m_orientation == TopRight || m_orientation == TopLeft);
    bool left = (m_orientation == BottomLeft || m_orientation == TopLeft);
    divideVertical(painter, rect, top, left);

    painter.setPen(QPen(QColor(173, 123, 134)));
    const qreal x1 = rect.width() / m_divineProportion;
    const qreal x2 = rect.width() - x1;
    if ((top && !left) || (!top && left)) {
        painter.drawLine(rect.bottomLeft(), rect.topRight());
        painter.drawLine(QPointF(x1, 0), rect.bottomRight());
        painter.drawLine(QPointF(0,0), QPointF(x2, rect.bottom()));
    }
    else {
        painter.drawLine(rect.topLeft(), rect.bottomRight());
        painter.drawLine(QPointF(x2, 0), rect.bottomLeft());
        painter.drawLine(QPointF(x1, rect.bottom()), rect.topRight());
    }
}

void DivineProportionShape::saveOdf(KoShapeSavingContext & /*context*/) const
{
    // TODO
}

bool DivineProportionShape::loadOdf( const KoXmlElement & /*element*/, KoShapeLoadingContext & /*context*/ )
{
    return false; // TODO
}

void DivineProportionShape::divideHorizontal(QPainter &painter, const QRectF &rect, bool top, bool left)
{
    if (rect.height() < 2)
        return;
    const qreal y = rect.height() / m_divineProportion;
    const qreal offset = top ? rect.bottom() - y : rect.top() + y;

    // draw horizontal line.
    painter.drawLine(QPointF(rect.left(), offset), QPointF(rect.right(), offset));
    divideVertical(painter, QRectF( QPointF(rect.left(), top ? rect.top() : offset),
                QSizeF(rect.width(), rect.height() - y)), !top, left);
}

void DivineProportionShape::divideVertical(QPainter &painter, const QRectF &rect, bool top, bool left)
{
    if (rect.width() < 2)
        return;
    const qreal x = rect.width() / m_divineProportion;
    const qreal offset = left ? rect.right() - x : rect.left() + x;
    // draw vertical line
    painter.drawLine(QPointF(offset, rect.top()), QPointF(offset, rect.bottom()));
    divideHorizontal(painter, QRectF(QPointF( left ? rect.left() : offset, rect.top()),
                QSizeF(rect.width() - x, rect.height())), top, !left);
}

void DivineProportionShape::setOrientation(Orientation orientation)
{
    if (m_orientation == orientation)
        return;
    m_orientation = orientation;
    update();
}

void DivineProportionShape::setPrintable(bool on)
{
    if (m_printable == on)
        return;
    m_printable = on;
    update();
}

