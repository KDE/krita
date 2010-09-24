/* This file is part of the KDE project
 * Copyright (C) 2006,2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoPathPointTypeCommand.h"
#include <klocale.h>
#include <math.h>

KoPathPointTypeCommand::KoPathPointTypeCommand(
    const QList<KoPathPointData> & pointDataList,
    PointType pointType,
    QUndoCommand *parent)
        : KoPathBaseCommand(parent)
        , m_pointType(pointType)
{
    QList<KoPathPointData>::const_iterator it(pointDataList.begin());
    for (; it != pointDataList.end(); ++it) {
        KoPathPoint *point = it->pathShape->pointByIndex(it->pointIndex);
        if (point) {
            PointData pointData(*it);
            pointData.m_oldControlPoint1 = it->pathShape->shapeToDocument(point->controlPoint1());
            pointData.m_oldControlPoint2 = it->pathShape->shapeToDocument(point->controlPoint2());
            pointData.m_oldProperties = point->properties();
            pointData.m_hadControlPoint1 = point->activeControlPoint1();
            pointData.m_hadControlPoint2 = point->activeControlPoint2();
            m_oldPointData.append(pointData);
            m_shapes.insert(it->pathShape);
        }
    }
    setText(i18n("Set point type"));
}

KoPathPointTypeCommand::~KoPathPointTypeCommand()
{
}

void KoPathPointTypeCommand::redo()
{
    QUndoCommand::redo();
    repaint(false);
    m_additionalPointData.clear();

    QList<PointData>::iterator it(m_oldPointData.begin());
    for (; it != m_oldPointData.end(); ++it) {
        KoPathPoint *point = it->m_pointData.pathShape->pointByIndex(it->m_pointData.pointIndex);
        KoPathPoint::PointProperties properties = point->properties();

        switch (m_pointType) {
        case Line: {
            point->removeControlPoint1();
            point->removeControlPoint2();
            break;
        }
        case Curve: {
            KoPathPointIndex pointIndex = it->m_pointData.pointIndex;
            KoPathPointIndex prevIndex;
            KoPathPointIndex nextIndex;
            KoPathShape * path = it->m_pointData.pathShape;
            // get previous path node
            if (pointIndex.second > 0)
                prevIndex = KoPathPointIndex(pointIndex.first, pointIndex.second - 1);
            else if (pointIndex.second == 0 && path->isClosedSubpath(pointIndex.first))
                prevIndex = KoPathPointIndex(pointIndex.first, path->subpathPointCount(pointIndex.first) - 1);
            // get next node
            if (pointIndex.second < path->subpathPointCount(pointIndex.first) - 1)
                nextIndex = KoPathPointIndex(pointIndex.first, pointIndex.second + 1);
            else if (pointIndex.second < path->subpathPointCount(pointIndex.first) - 1
                     && path->isClosedSubpath(pointIndex.first))
                nextIndex = KoPathPointIndex(pointIndex.first, 0);

            KoPathPoint * prevPoint = path->pointByIndex(prevIndex);
            KoPathPoint * nextPoint = path->pointByIndex(nextIndex);

            if (prevPoint && ! point->activeControlPoint1() && appendPointData(KoPathPointData(path, prevIndex))) {
                KoPathSegment cubic = KoPathSegment(prevPoint, point).toCubic();
                if (prevPoint->activeControlPoint2()) {
                    prevPoint->setControlPoint2(cubic.first()->controlPoint2());
                    point->setControlPoint1(cubic.second()->controlPoint1());
                } else
                    point->setControlPoint1(cubic.second()->controlPoint1());
            }
            if (nextPoint && ! point->activeControlPoint2() && appendPointData(KoPathPointData(path, nextIndex))) {
                KoPathSegment cubic = KoPathSegment(point, nextPoint).toCubic();
                if (nextPoint->activeControlPoint1()) {
                    point->setControlPoint2(cubic.first()->controlPoint2());
                    nextPoint->setControlPoint1(cubic.second()->controlPoint1());
                } else
                    point->setControlPoint2(cubic.first()->controlPoint2());
            }
            break;
        }
        case Symmetric: {
            properties &= ~KoPathPoint::IsSmooth;
            properties |= KoPathPoint::IsSymmetric;

            // calculate vector from node point to first control point and normalize it
            QPointF directionC1 = point->controlPoint1() - point->point();
            qreal dirLengthC1 = sqrt(directionC1.x() * directionC1.x() + directionC1.y() * directionC1.y());
            directionC1 /= dirLengthC1;
            // calculate vector from node point to second control point and normalize it
            QPointF directionC2 = point->controlPoint2() - point->point();
            qreal dirLengthC2 = sqrt(directionC2.x() * directionC2.x() + directionC2.y() * directionC2.y());
            directionC2 /= dirLengthC2;
            // calculate the average distance of the control points to the node point
            qreal averageLength = 0.5 * (dirLengthC1 + dirLengthC2);
            // compute position of the control points so that they lie on a line going through the node point
            // the new distance of the control points is the average distance to the node point
            point->setControlPoint1(point->point() + 0.5 * averageLength * (directionC1 - directionC2));
            point->setControlPoint2(point->point() + 0.5 * averageLength * (directionC2 - directionC1));
        }
        break;
        case Smooth: {
            properties &= ~KoPathPoint::IsSymmetric;
            properties |= KoPathPoint::IsSmooth;

            // calculate vector from node point to first control point and normalize it
            QPointF directionC1 = point->controlPoint1() - point->point();
            qreal dirLengthC1 = sqrt(directionC1.x() * directionC1.x() + directionC1.y() * directionC1.y());
            directionC1 /= dirLengthC1;
            // calculate vector from node point to second control point and normalize it
            QPointF directionC2 = point->controlPoint2() - point->point();
            qreal dirLengthC2 = sqrt(directionC2.x() * directionC2.x() + directionC2.y() * directionC2.y());
            directionC2 /= dirLengthC2;
            // compute position of the control points so that they lie on a line going through the node point
            // the new distance of the control points is the average distance to the node point
            point->setControlPoint1(point->point() + 0.5 * dirLengthC1 * (directionC1 - directionC2));
            point->setControlPoint2(point->point() + 0.5 * dirLengthC2 * (directionC2 - directionC1));
        }
        break;
        case Corner:
        default:
            properties &= ~KoPathPoint::IsSymmetric;
            properties &= ~KoPathPoint::IsSmooth;
            break;
        }
        point->setProperties(properties);
    }
    repaint(true);
}

void KoPathPointTypeCommand::undo()
{
    QUndoCommand::undo();
    repaint(false);

    /*
    QList<PointData>::iterator it(m_oldPointData.begin());
    for (; it != m_oldPointData.end(); ++it)
    {
        KoPathShape *pathShape = it->m_pointData.pathShape;
        KoPathPoint *point = pathShape->pointByIndex(it->m_pointData.pointIndex);

        point->setProperties(it->m_oldProperties);
        if (it->m_hadControlPoint1)
            point->setControlPoint1(pathShape->documentToShape(it->m_oldControlPoint1));
        else
            point->removeControlPoint1();
        if (it->m_hadControlPoint2)
            point->setControlPoint2(pathShape->documentToShape(it->m_oldControlPoint2));
        else
            point->removeControlPoint2();
    }
    */
    undoChanges(m_oldPointData);
    undoChanges(m_additionalPointData);

    repaint(true);
}

void KoPathPointTypeCommand::undoChanges(const QList<PointData> &data)
{
    QList<PointData>::const_iterator it(data.begin());
    for (; it != data.end(); ++it) {
        KoPathShape *pathShape = it->m_pointData.pathShape;
        KoPathPoint *point = pathShape->pointByIndex(it->m_pointData.pointIndex);

        point->setProperties(it->m_oldProperties);
        if (it->m_hadControlPoint1)
            point->setControlPoint1(pathShape->documentToShape(it->m_oldControlPoint1));
        else
            point->removeControlPoint1();
        if (it->m_hadControlPoint2)
            point->setControlPoint2(pathShape->documentToShape(it->m_oldControlPoint2));
        else
            point->removeControlPoint2();
    }
}

bool KoPathPointTypeCommand::appendPointData(KoPathPointData data)
{
    KoPathPoint *point = data.pathShape->pointByIndex(data.pointIndex);
    if (! point)
        return false;

    PointData pointData(data);
    pointData.m_oldControlPoint1 = data.pathShape->shapeToDocument(point->controlPoint1());
    pointData.m_oldControlPoint2 = data.pathShape->shapeToDocument(point->controlPoint2());
    pointData.m_oldProperties = point->properties();
    pointData.m_hadControlPoint1 = point->activeControlPoint1();
    pointData.m_hadControlPoint2 = point->activeControlPoint2();

    m_additionalPointData.append(pointData);

    return true;
}
