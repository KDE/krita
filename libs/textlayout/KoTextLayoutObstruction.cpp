/* This file is part of the KDE project
 * Copyright (C) 2006-2007, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 Ko Gmbh <casper.boemann@kogmbh.com>
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

#include "KoTextLayoutObstruction.h"
#include <KoShapeContainer.h>
#include <KoShapeBorderModel.h>
#include <KoShapeShadow.h>

#include <qnumeric.h>

KoTextLayoutObstruction::KoTextLayoutObstruction(KoShape *shape, const QTransform &matrix)
    : m_side(None),
    m_polygon(QPolygonF()),
    m_line(QRectF()),
    m_shape(shape),
    m_runAroundThreshold(0)
{
    QPainterPath path = shape->outline();

    QRectF bb = shape->outlineRect();

    if (m_shape->border()) {
        KoInsets insets;
        m_shape->border()->borderInsets(m_shape, insets);
        QRectF bb = m_shape->outlineRect();
        bb.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
        path = QPainterPath();
        path.addRect(bb);
    }

    if (m_shape->shadow()) {
        QTransform transform = m_shape->absoluteTransformation(0);
        bb = transform.mapRect(bb);
        KoInsets insets;
        m_shape->shadow()->insets(insets);
        bb.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
        path = QPainterPath();
        path.addRect(bb);
        path = transform.inverted().map(path);
    }

    //TODO check if path is convex. otherwise do triangulation and create more convex obstructions
    init(matrix, path, shape->textRunAroundDistance());

    if (shape->textRunAroundSide() == KoShape::NoRunAround) {
        // make the shape take the full width of the text area
        m_side = Empty;
    } else if (shape->textRunAroundSide() == KoShape::RunThrough) {
        m_distance = 0;
        // We don't exist.
        return;
    } else if (shape->textRunAroundSide() == KoShape::LeftRunAroundSide) {
        m_side = Left;
    } else if (shape->textRunAroundSide() == KoShape::RightRunAroundSide) {
        m_side = Right;
    } else if (shape->textRunAroundSide() == KoShape::BothRunAroundSide) {
        m_side = Both;
    } else if (shape->textRunAroundSide() == KoShape::BiggestRunAroundSide) {
        m_side = Bigger;
    } else if (shape->textRunAroundSide() == KoShape::EnoughRunAroundSide) {
        m_side = Enough;
        m_runAroundThreshold = shape->textRunAroundThreshold();
    }
}

void KoTextLayoutObstruction::init(const QTransform &matrix, const QPainterPath &obstruction, qreal distance)
{
    m_distance = distance;
    QPainterPath path =  matrix.map(obstruction);
    m_bounds = path.boundingRect();
    if (distance >= 0.0) {
        QTransform grow = matrix;
        grow.translate(m_bounds.width() / 2.0, m_bounds.height() / 2.0);
        qreal scaleX = distance;
        if (m_bounds.width() > 0)
            scaleX = (m_bounds.width() + distance) / m_bounds.width();
        qreal scaleY = distance;
        if (m_bounds.height() > 0)
            scaleY = (m_bounds.height() + distance) / m_bounds.height();
        Q_ASSERT(!qIsNaN(scaleY));
        Q_ASSERT(!qIsNaN(scaleX));
        grow.scale(scaleX, scaleY);
        grow.translate(-m_bounds.width() / 2.0, -m_bounds.height() / 2.0);

        path =  grow.map(obstruction);
        // kDebug() <<"Grow" << distance <<", Before:" << m_bounds <<", after:" << path.boundingRect();
        m_bounds = path.boundingRect();
    }

    m_polygon = path.toFillPolygon();
    QPointF prev = *(m_polygon.begin());
    foreach (const QPointF &vtx, m_polygon) { //initialized edges
        if (vtx.x() == prev.x() && vtx.y() == prev.y())
            continue;
        QLineF line;
        if (prev.y() < vtx.y()) // Make sure the vector lines all point downwards.
            line = QLineF(prev, vtx);
        else
            line = QLineF(vtx, prev);
        m_edges.insert(line.y1(), line);
        prev = vtx;
    }

}

qreal KoTextLayoutObstruction::xAtY(const QLineF &line, qreal y)
{
    if (line.dx() == 0)
        return line.x1();
    return line.x1() + (y - line.y1()) / line.dy() * line.dx();
}

void KoTextLayoutObstruction::changeMatrix(const QTransform &matrix)
{
    m_edges.clear();

    QPainterPath path = m_shape->outline();

    QRectF bb = m_shape->outlineRect();

    if (m_shape->border()) {
        KoInsets insets;
        m_shape->border()->borderInsets(m_shape, insets);
        QRectF bb = m_shape->outlineRect();
        bb.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
        path = QPainterPath();
        path.addRect(bb);
    }

    if (m_shape->shadow()) {
        QTransform transform = m_shape->absoluteTransformation(0);
        bb = transform.mapRect(bb);
        KoInsets insets;
        m_shape->shadow()->insets(insets);
        bb.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
        path = QPainterPath();
        path.addRect(bb);
        path = transform.inverted().map(path);
    }

    init(matrix, path, m_distance);
}

QRectF KoTextLayoutObstruction::cropToLine(const QRectF &lineRect)
{
    if (m_bounds.intersects(lineRect)) {
        m_line = lineRect;
        bool untilFirst = true;
        //check inner points
        foreach (const QPointF &point, m_polygon) {
            if (lineRect.contains(point)) {
                if (untilFirst) {
                    m_line.setLeft(point.x());
                    m_line.setRight(point.x());
                    untilFirst = false;
                } else {
                    if (point.x() < m_line.left()) {
                        m_line.setLeft(point.x());
                    } else if (point.x() > m_line.right()) {
                        m_line.setRight(point.x());
                    }
                }
            }
        }
        //check edges
        qreal points[2] = { lineRect.top(), lineRect.bottom() };
        for (int i = 0; i < 2; i++) {
            const qreal y = points[i];
            QMap<qreal, QLineF>::const_iterator iter = m_edges.constBegin();
            for (;iter != m_edges.constEnd(); ++iter) {
                QLineF line = iter.value();
                if (line.y2() < y) // not a section that will intersect with ou Y yet
                    continue;
                if (line.y1() > y) // section is below our Y, so abort loop
                    //break;
                    continue;
                if (qAbs(line.dy()) < 1E-10)  // horizontal lines don't concern us.
                    continue;

                qreal intersect = xAtY(iter.value(), y);
                if (untilFirst) {
                    m_line.setLeft(intersect);
                    m_line.setRight(intersect);
                    untilFirst = false;
                } else {
                    if (intersect < m_line.left()) {
                        m_line.setLeft(intersect);
                    } else if (intersect > m_line.right()) {
                        m_line.setRight(intersect);
                    }
                }
            }
        }
    } else {
        m_line = QRectF();
    }
    return m_line;
}

QRectF KoTextLayoutObstruction::getLeftLinePart(const QRectF &lineRect) const
{
    QRectF leftLinePart = lineRect;
    leftLinePart.setRight(m_line.left());
    return leftLinePart;
}

QRectF KoTextLayoutObstruction::getRightLinePart(const QRectF &lineRect) const
{
    QRectF rightLinePart = lineRect;
    if (m_line.right() > rightLinePart.left()) {
        rightLinePart.setLeft(m_line.right());
    }
    return rightLinePart;
}

bool KoTextLayoutObstruction::textOnLeft() const
{    
    return  m_side == Left;
}

bool KoTextLayoutObstruction::textOnRight() const
{
    return m_side == Right;
}

bool KoTextLayoutObstruction::textOnBiggerSide() const
{
    return m_side == Bigger;
}

bool KoTextLayoutObstruction::textOnEnoughSides() const
{
    return m_side == Enough;
}

qreal KoTextLayoutObstruction::runAroundThreshold() const
{
    return m_runAroundThreshold;
}

bool KoTextLayoutObstruction::noTextAround() const
{
    return m_side == Empty;
}

bool KoTextLayoutObstruction::compareRectLeft(KoTextLayoutObstruction *o1, KoTextLayoutObstruction *o2)
{
    return o1->m_line.left() < o2->m_line.left();
}
