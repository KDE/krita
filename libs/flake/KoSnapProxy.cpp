/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008-2009 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoSnapProxy.h"
#include "KoSnapGuide.h"
#include "KoCanvasBase.h"
#include "KoShapeManager.h"
#include "KoPathShape.h"
#include "KoPathPoint.h"
#include <KoSnapData.h>
#include <KoShapeLayer.h>

KoSnapProxy::KoSnapProxy(KoSnapGuide * snapGuide)
        : m_snapGuide(snapGuide)
{
}

QList<QPointF> KoSnapProxy::pointsInRect(const QRectF &rect, bool omitEditedShape)
{
    QList<QPointF> points;
    QList<KoShape*> shapes = shapesInRect(rect, omitEditedShape);
    Q_FOREACH (KoShape * shape, shapes) {
        Q_FOREACH (const QPointF & point, pointsFromShape(shape)) {
            if (rect.contains(point))
                points.append(point);
        }
    }

    return points;
}

QList<KoShape*> KoSnapProxy::shapesInRect(const QRectF &rect, bool omitEditedShape)
{
    QList<KoShape*> shapes = m_snapGuide->canvas()->shapeManager()->shapesAt(rect);
    Q_FOREACH (KoShape * shape, m_snapGuide->ignoredShapes()) {
        const int index = shapes.indexOf(shape);
        if (index >= 0) {
            shapes.removeAt(index);
        }
    }


    if (omitEditedShape) {
        Q_FOREACH (KoPathPoint *point, m_snapGuide->ignoredPathPoints()) {
            const int index = shapes.indexOf(point->parent());
            if (index >= 0) {
                shapes.removeAt(index);
            }
        }
    }

    if (!omitEditedShape && m_snapGuide->additionalEditedShape()) {
        QRectF bound = m_snapGuide->additionalEditedShape()->boundingRect();
        if (rect.intersects(bound) || rect.contains(bound))
            shapes.append(m_snapGuide->additionalEditedShape());
    }
    return shapes;
}

QList<QPointF> KoSnapProxy::pointsFromShape(KoShape * shape)
{
    QList<QPointF> snapPoints;
    // no snapping to hidden shapes
    if (! shape->isVisible())
        return snapPoints;

    // return the special snap points of the shape
    snapPoints += shape->snapData().snapPoints();

    KoPathShape * path = dynamic_cast<KoPathShape*>(shape);
    if (path) {
        QTransform m = path->absoluteTransformation();

        QList<KoPathPoint*> ignoredPoints = m_snapGuide->ignoredPathPoints();

        int subpathCount = path->subpathCount();
        for (int subpathIndex = 0; subpathIndex < subpathCount; ++subpathIndex) {
            int pointCount = path->subpathPointCount(subpathIndex);
            for (int pointIndex = 0; pointIndex < pointCount; ++pointIndex) {
                KoPathPoint * p = path->pointByIndex(KoPathPointIndex(subpathIndex, pointIndex));
                if (! p || ignoredPoints.contains(p))
                    continue;

                snapPoints.append(m.map(p->point()));
            }
        }
    }
    else
    {
        // add the bounding box corners as default snap points
        QRectF bbox = shape->boundingRect();
        snapPoints.append(bbox.topLeft());
        snapPoints.append(bbox.topRight());
        snapPoints.append(bbox.bottomRight());
        snapPoints.append(bbox.bottomLeft());
    }

    return snapPoints;
}

QList<KoPathSegment> KoSnapProxy::segmentsInRect(const QRectF &rect, bool omitEditedShape)
{
    
    QList<KoShape*> shapes = shapesInRect(rect, omitEditedShape);
    QList<KoPathPoint*> ignoredPoints = m_snapGuide->ignoredPathPoints();

    QList<KoPathSegment> segments;
    Q_FOREACH (KoShape * shape, shapes) {
        QList<KoPathSegment> shapeSegments;
        QRectF rectOnShape = shape->documentToShape(rect);
        KoPathShape * path = dynamic_cast<KoPathShape*>(shape);
        if (path) {
            shapeSegments = path->segmentsAt(rectOnShape);
        } else {
            Q_FOREACH (const KoPathSegment & s, shape->snapData().snapSegments()) {
                QRectF controlRect = s.controlPointRect();
                if (! rect.intersects(controlRect) && ! controlRect.contains(rect))
                    continue;
                QRectF bound = s.boundingRect();
                if (! rect.intersects(bound) && ! bound.contains(rect))
                    continue;
                shapeSegments.append(s);
            }
        }

        QTransform m = shape->absoluteTransformation();
        // transform segments to document coordinates
        Q_FOREACH (const KoPathSegment & s, shapeSegments) {
            if (ignoredPoints.contains(s.first()) || ignoredPoints.contains(s.second()))
                continue;
            segments.append(s.mapped(m));
        }
    }
    return segments;
}

QList<KoShape*> KoSnapProxy::shapes(bool omitEditedShape)
{
    QList<KoShape*> allShapes = m_snapGuide->canvas()->shapeManager()->shapes();
    QList<KoShape*> filteredShapes;
    QList<KoShape*> ignoredShapes = m_snapGuide->ignoredShapes();

    // filter all hidden and ignored shapes
    Q_FOREACH (KoShape * shape, allShapes) {
        if (shape->isVisible() &&
            !ignoredShapes.contains(shape) &&
            !dynamic_cast<KoShapeLayer*>(shape)) {

            filteredShapes.append(shape);
        }
    }

    if (omitEditedShape) {
        Q_FOREACH (KoPathPoint *point, m_snapGuide->ignoredPathPoints()) {
            const int index = filteredShapes.indexOf(point->parent());
            if (index >= 0) {
                filteredShapes.removeAt(index);
            }
        }
    }

    if (!omitEditedShape && m_snapGuide->additionalEditedShape()) {
        filteredShapes.append(m_snapGuide->additionalEditedShape());
    }

    return filteredShapes;
}

KoCanvasBase * KoSnapProxy::canvas()
{
    return m_snapGuide->canvas();
}

