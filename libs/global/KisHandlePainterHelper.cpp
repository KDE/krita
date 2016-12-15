/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KisHandlePainterHelper.h"

#include <QPainter>


KisHandlePainterHelper::KisHandlePainterHelper(QPainter *_painter, qreal handleRadius)
    : m_painter(_painter),
      m_painterTransform(m_painter->transform()),
      m_decomposedMatrix(m_painterTransform)
{
    m_painter->setTransform(QTransform());
    m_handleTransform = m_decomposedMatrix.shearTransform() * m_decomposedMatrix.rotateTransform();

    if (handleRadius > 0.0) {
        const QRectF handleRect(-handleRadius, -handleRadius, 2 * handleRadius, 2 * handleRadius);
        m_handlePolygon = m_handleTransform.map(QPolygonF(handleRect));
    }
}

KisHandlePainterHelper::~KisHandlePainterHelper() {
    m_painter->setTransform(m_painterTransform);
}

void KisHandlePainterHelper::drawHandleRect(const QPointF &center, qreal radius) {
    QRectF handleRect(-radius, -radius, 2 * radius, 2 * radius);
    QPolygonF handlePolygon = m_handleTransform.map(QPolygonF(handleRect));
    handlePolygon.translate(m_painterTransform.map(center));
    m_painter->drawPolygon(handlePolygon);
}

void KisHandlePainterHelper::drawHandleRect(const QPointF &center) {
    m_painter->drawPolygon(m_handlePolygon.translated(m_painterTransform.map(center)));
}

void KisHandlePainterHelper::drawRubberLine(const QPolygonF &poly) {
    m_painter->drawPolygon(m_painterTransform.map(poly));
}
