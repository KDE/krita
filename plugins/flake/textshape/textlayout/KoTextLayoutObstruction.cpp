/* This file is part of the KDE project
 * Copyright (C) 2006-2007, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 Ko Gmbh <cbo@kogmbh.com>
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

#include <KoShapeStrokeModel.h>
#include <KoShapeShadow.h>
#include <KoShapeGroup.h>
#include <KoClipPath.h>
#include <KoInsets.h>

#include <QTransform>
#include <QPainterPath>

KoTextLayoutObstruction::KoTextLayoutObstruction(KoShape *shape, const QTransform &matrix)
    : m_side(None),
    m_polygon(QPolygonF()),
    m_line(QRectF()),
    m_shape(shape),
    m_runAroundThreshold(0)
{
    qreal borderHalfWidth;
    QPainterPath path = decoratedOutline(m_shape, borderHalfWidth);

    //TODO check if path is convex. otherwise do triangulation and create more convex obstructions
    init(matrix, path, shape->textRunAroundDistanceLeft(), shape->textRunAroundDistanceTop(), shape->textRunAroundDistanceRight(), shape->textRunAroundDistanceBottom(), borderHalfWidth);

    if (shape->textRunAroundSide() == KoShape::NoRunAround) {
        // make the shape take the full width of the text area
        m_side = Empty;
    } else if (shape->textRunAroundSide() == KoShape::RunThrough) {
        m_distanceLeft = 0;
        m_distanceTop = 0;
        m_distanceRight = 0;
        m_distanceBottom = 0;
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

KoTextLayoutObstruction::KoTextLayoutObstruction(const QRectF &rect, bool rtl)
    : m_side(None),
    m_polygon(QPolygonF()),
    m_line(QRectF()),
    m_shape(0),
    m_runAroundThreshold(0)
{
    qreal borderHalfWidth = 0;
    qreal textRunAroundDistance = 1;

    QPainterPath path;
    path.addRect(rect);

    init(QTransform(), path, textRunAroundDistance, 0.0, textRunAroundDistance, 0.0,  borderHalfWidth);
    if (rtl) {
        m_side = Right;
    } else {
        m_side = Left;
    }
}

QPainterPath KoTextLayoutObstruction::decoratedOutline(const KoShape *shape, qreal &borderHalfWidth) const
{
    const KoShapeGroup *shapeGroup = dynamic_cast<const KoShapeGroup *>(shape);
    if (shapeGroup) {
        QPainterPath groupPath;

        foreach (const KoShape *child, shapeGroup->shapes()) {
            groupPath += decoratedOutline(child, borderHalfWidth);
        }
        return groupPath;
    }

    QPainterPath path;
    if (shape->textRunAroundContour() != KoShape::ContourBox) {
        KoClipPath *clipPath = shape->clipPath();
        if (clipPath) {
            path = clipPath->pathForSize(shape->size());
        } else {
            path = shape->outline();
        }
    } else {
        path.addRect(shape->outlineRect());
    }

    QRectF bb = shape->outlineRect();
    borderHalfWidth = 0;
 
    if (shape->stroke()) {
        KoInsets insets;
        shape->stroke()->strokeInsets(shape, insets);
        /*
        bb.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
        path = QPainterPath();
        path.addRect(bb);
        */
        borderHalfWidth = qMax(qMax(insets.left, insets.top),qMax(insets.right, insets.bottom));
    }

    if (shape->shadow() && shape->shadow()->isVisible()) {
        QTransform transform = shape->absoluteTransformation();
        bb = transform.mapRect(bb);
        KoInsets insets;
        shape->shadow()->insets(insets);
        bb.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
        path = QPainterPath();
        path.addRect(bb);
        path = transform.inverted().map(path);
    }

    return path;
}

void KoTextLayoutObstruction::init(const QTransform &matrix, const QPainterPath &obstruction, qreal distanceLeft, qreal distanceTop, qreal distanceRight, qreal distanceBottom, qreal borderHalfWidth)
{
    m_distanceLeft = distanceLeft;
    m_distanceTop = distanceTop;
    m_distanceRight = distanceRight;
    m_distanceBottom = distanceBottom;
    QPainterPath path =  matrix.map(obstruction);
    distanceLeft += borderHalfWidth;
    distanceTop += borderHalfWidth;
    distanceRight += borderHalfWidth;
    distanceBottom += borderHalfWidth;

    qreal extraWidth = distanceLeft + distanceRight;
    qreal extraHeight = distanceTop + distanceBottom;
    if (extraWidth != 0.0 || extraHeight != 0.0) {
        // Let's extend the outline with at least the border half width in all directions.
        // However since the distance can be express in 4 directions and QPainterPathStroker only
        // handles a penWidth we do some tricks to get it working.
        //
        // Explanation in one dimension only: we sum the distances top and below and use that as the
        // penWidth. Afterwards we translate the result so it is distributed correctly by top and bottom.
        // Now by doing that we would also implicitly set the left+right size of the pen which is no good,
        // so in order to set that to a minimal value (we choose 1, as 0 would give division by 0) we do
        // the following:. We scale the original path by sumX, stroke the path with penwidth=sumY, then
        // scale it back. Effectively we have now stroked with a pen sized 1 x sumY.
        //
        // The math to do both x an y in one go becomes a little more complex, but only a little.
        extraWidth = qMax(qreal(0.1), extraWidth);
        extraHeight = qMax(qreal(0.1), extraHeight);

        QPainterPathStroker stroker;
        stroker.setWidth(extraWidth);
        stroker.setJoinStyle(Qt::MiterJoin);
        stroker.setCapStyle(Qt::SquareCap);
        QPainterPath bigPath = stroker.createStroke(QTransform().scale(1.0, extraWidth / extraHeight).map(path));
        bigPath = QTransform().scale(1.0, extraHeight / extraWidth). map(bigPath);
        path += bigPath.translated(extraWidth / 2 - distanceLeft, extraHeight / 2 - distanceTop);
    }

    m_bounds = path.boundingRect();

    // Now we need to change the path into a polygon for easier handling later on
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

    qreal borderHalfWidth;
    QPainterPath path = decoratedOutline(m_shape, borderHalfWidth);

    init(matrix, path, m_distanceLeft, m_distanceTop, m_distanceRight, m_distanceBottom, borderHalfWidth);
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
