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
#include "kis_algebra_2d.h"


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

void KisHandlePainterHelper::drawGradientHandle(const QPointF &center, qreal radius) {
    QPolygonF handlePolygon;

    handlePolygon << QPointF(-radius, 0);
    handlePolygon << QPointF(0, radius);
    handlePolygon << QPointF(radius, 0);
    handlePolygon << QPointF(0, -radius);

    handlePolygon = m_handleTransform.map(handlePolygon);
    m_painter->drawPolygon(handlePolygon.translated(m_painterTransform.map(center)));
}

void KisHandlePainterHelper::drawGradientCrossHandle(const QPointF &center, qreal radius) {

    { // Draw a cross
        QPainterPath p;
        p.moveTo(-radius, -radius);
        p.lineTo(radius, radius);
        p.moveTo(radius, -radius);
        p.lineTo(-radius, radius);

        p = m_handleTransform.map(p);
        m_painter->drawPath(p.translated(m_painterTransform.map(center)));
    }

    { // Draw a square
        const qreal halfRadius = 0.5 * radius;

        QPolygonF handlePolygon;
        handlePolygon << QPointF(-halfRadius, 0);
        handlePolygon << QPointF(0, halfRadius);
        handlePolygon << QPointF(halfRadius, 0);
        handlePolygon << QPointF(0, -halfRadius);

        handlePolygon = m_handleTransform.map(handlePolygon);
        m_painter->drawPolygon(handlePolygon.translated(m_painterTransform.map(center)));
    }
}

void KisHandlePainterHelper::drawArrow(const QPointF &pos, const QPointF &from, qreal radius)
{
    QPainterPath p;

    QLineF line(pos, from);
    line.setLength(radius);

    QPointF norm = KisAlgebra2D::leftUnitNormal(pos - from);
    norm *= 0.34 * radius;

    p.moveTo(line.p2() + norm);
    p.lineTo(line.p1());
    p.lineTo(line.p2() - norm);

    p.translate(-pos);

    m_painter->drawPath(m_handleTransform.map(p).translated(m_painterTransform.map(pos)));
}

void KisHandlePainterHelper::drawGradientArrow(const QPointF &start, const QPointF &end, qreal radius)
{
    QPainterPath p;
    p.moveTo(start);
    p.lineTo(end);
    m_painter->drawPath(m_painterTransform.map(p));

    const qreal length = kisDistance(start, end);
    const QPointF diff = end - start;

    if (length > 5 * radius) {
        drawArrow(start + 0.33 * diff, start, radius);
        drawArrow(start + 0.66 * diff, start, radius);
    } else if (length > 3 * radius) {
        drawArrow(start + 0.5 * diff, start, radius);
    }
}

void KisHandlePainterHelper::drawRubberLine(const QPolygonF &poly) {
    m_painter->drawPolygon(m_painterTransform.map(poly));
}
