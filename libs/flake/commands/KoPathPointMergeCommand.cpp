/* This file is part of the KDE project
 * Copyright (C) 2009 Jan Hambrecht <jaham@gmx.net>
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

#include "KoPathPointMergeCommand.h"
#include "KoPathPoint.h"
#include "KoPathPointData.h"
#include "KoPathShape.h"
#include <KLocale>
#include <QPointF>

class KoPathPointMergeCommand::Private
{
public:
    Private(const KoPathPointData &pointData1, const KoPathPointData &pointData2)
    : pathShape(pointData1.pathShape)
    , endPoint(pointData1.pointIndex)
    , startPoint(pointData2.pointIndex)
    , splitIndex(KoPathPointIndex(-1, -1))
    , removedPoint(0)
    , reverse(ReverseNone)
    {
    }

    ~Private()
    {
        delete removedPoint;
    }

    KoPathPoint * mergePoints(KoPathPoint * p1, KoPathPoint * p2)
    {
        QPointF mergePosition = 0.5 * (p1->point() + p2->point());
        QPointF mergeControlPoint1 = mergePosition + (p1->controlPoint1() - p1->point());
        QPointF mergeControlPoint2 = mergePosition + (p2->controlPoint2() - p2->point());

        // change position and control points of first merged point
        p1->setPoint(mergePosition);
        if (p1->activeControlPoint1()) {
            p1->setControlPoint1(mergeControlPoint1);
        }
        if (p2->activeControlPoint2()) {
            p1->setControlPoint2(mergeControlPoint2);
        }

        // remove the second merged point
        KoPathPointIndex removeIndex = pathShape->pathPointIndex(p2);
        return pathShape->removePoint(removeIndex);
    }

    void resetPoints(KoPathPointIndex index1, KoPathPointIndex index2)
    {
        KoPathPoint * p1 = pathShape->pointByIndex(index1);
        KoPathPoint * p2 = pathShape->pointByIndex(index2);
        p1->setPoint(pathShape->documentToShape(oldNodePoint1));
        p2->setPoint(pathShape->documentToShape(oldNodePoint2));
        if (p1->activeControlPoint1()) {
            p1->setControlPoint1(pathShape->documentToShape(oldControlPoint1));
        }
        if (p2->activeControlPoint2()) {
            p2->setControlPoint2(pathShape->documentToShape(oldControlPoint2));
        }
    }

    KoPathShape * pathShape;
    KoPathPointIndex endPoint;
    KoPathPointIndex startPoint;
    KoPathPointIndex splitIndex;

    // the control points have to be stored in document positions
    QPointF oldNodePoint1;
    QPointF oldControlPoint1;
    QPointF oldNodePoint2;
    QPointF oldControlPoint2;

    KoPathPoint * removedPoint;

    enum Reverse {
        ReverseNone = 0,
        ReverseFirst = 1,
        ReverseSecond = 2
    };
    int reverse;
};

/**
 * How does is work:
 *
 * The goal is to merge the point that is ending an open subpath with the one
 * starting the same or another open subpath.
 */
KoPathPointMergeCommand::KoPathPointMergeCommand(const KoPathPointData &pointData1, const KoPathPointData &pointData2, QUndoCommand *parent)
    : QUndoCommand(parent), d(new Private(pointData1, pointData2))
{
    Q_ASSERT(pointData1.pathShape == pointData2.pathShape);
    Q_ASSERT(d->pathShape);
    Q_ASSERT(!d->pathShape->isClosedSubpath(d->endPoint.first));
    Q_ASSERT(d->endPoint.second == 0 ||
             d->endPoint.second == d->pathShape->subpathPointCount(d->endPoint.first) - 1);
    Q_ASSERT(!d->pathShape->isClosedSubpath(d->startPoint.first));
    Q_ASSERT(d->startPoint.second == 0 ||
             d->startPoint.second == d->pathShape->subpathPointCount(d->startPoint.first) - 1);

    // if we have two different subpaths we might need to reverse them
    if (d->endPoint.first != d->startPoint.first) {
        // sort by point index
        if (d->startPoint < d->endPoint)
            qSwap(d->endPoint, d->startPoint);
        // mark first subpath to be reversed if first point starts a subpath with more than one point
        if (d->endPoint.second == 0 && d->pathShape->subpathPointCount(d->endPoint.first) > 1)
            d->reverse |= Private::ReverseFirst;
        // mark second subpath to be reversed if second point does not start a subpath with more than one point
        if (d->startPoint.second != 0 && d->pathShape->subpathPointCount(d->startPoint.first) > 1)
            d->reverse |= Private::ReverseSecond;
    } else {
        Q_ASSERT(d->endPoint.second != d->startPoint.second);
        if (d->endPoint < d->startPoint)
            qSwap(d->endPoint, d->startPoint);
    }

    KoPathPoint * p1 = d->pathShape->pointByIndex(d->endPoint);
    KoPathPoint * p2 = d->pathShape->pointByIndex(d->startPoint);

    d->oldNodePoint1 = d->pathShape->shapeToDocument(p1->point());
    if (d->reverse & Private::ReverseFirst) {
        d->oldControlPoint1 = d->pathShape->shapeToDocument(p1->controlPoint2());
    } else {
        d->oldControlPoint1 = d->pathShape->shapeToDocument(p1->controlPoint1());
    }
    d->oldNodePoint2 = d->pathShape->shapeToDocument(p2->point());
    if (d->reverse & Private::ReverseSecond) {
        d->oldControlPoint2 = d->pathShape->shapeToDocument(p2->controlPoint1());
    } else {
        d->oldControlPoint2 = d->pathShape->shapeToDocument(p2->controlPoint2());
    }

    setText(i18n("Merge points"));
}

KoPathPointMergeCommand::~KoPathPointMergeCommand()
{
    delete d;
}

void KoPathPointMergeCommand::redo()
{
    QUndoCommand::redo();

    if (d->removedPoint)
        return;

    d->pathShape->update();

    KoPathPoint * endPoint = d->pathShape->pointByIndex(d->endPoint);
    KoPathPoint * startPoint = d->pathShape->pointByIndex(d->startPoint);

    // are we just closing a single subpath ?
    if (d->endPoint.first == d->startPoint.first) {
        // change the endpoint of the subpath
        d->removedPoint = d->mergePoints(endPoint, startPoint);
        // set endpoint of subpath to close the subpath
        endPoint->setProperty(KoPathPoint::CloseSubpath);
        // set new startpoint of subpath to close the subpath
        KoPathPointIndex newStartIndex(d->startPoint.first,0);
        d->pathShape->pointByIndex(newStartIndex)->setProperty(KoPathPoint::CloseSubpath);
    } else {
        // first revert subpaths if needed
        if (d->reverse & Private::ReverseFirst) {
            d->pathShape->reverseSubpath(d->endPoint.first);
        }
        if (d->reverse & Private::ReverseSecond) {
            d->pathShape->reverseSubpath(d->startPoint.first);
        }
        // move the subpaths so the second is directly after the first
        d->pathShape->moveSubpath(d->startPoint.first, d->endPoint.first + 1);
        d->splitIndex = d->pathShape->pathPointIndex(endPoint);
        // join both subpaths
        d->pathShape->join(d->endPoint.first);
        // change the first point of the points to merge
        d->removedPoint = d->mergePoints(endPoint, startPoint);
    }

    d->pathShape->normalize();
    d->pathShape->update();
}

void KoPathPointMergeCommand::undo()
{
    QUndoCommand::undo();

    if (!d->removedPoint)
        return;

    d->pathShape->update();

    // check if we just have closed a single subpath
    if (d->endPoint.first == d->startPoint.first) {
        // open the subpath at the old/new first point
        d->pathShape->openSubpath(d->startPoint);
        // reinsert the old first point
        d->pathShape->insertPoint(d->removedPoint, d->startPoint);
        // reposition the points
        d->resetPoints(d->endPoint, d->startPoint);
    } else {
        // break merged subpaths apart
        d->pathShape->breakAfter(d->splitIndex);
        // reinsert the old second point
        d->pathShape->insertPoint(d->removedPoint, KoPathPointIndex(d->splitIndex.first+1,0));
        // reposition the first point
        d->resetPoints(d->splitIndex, KoPathPointIndex(d->splitIndex.first+1,0));
        // move second subpath to its old position
        d->pathShape->moveSubpath(d->splitIndex.first+1, d->startPoint.first);
        // undo the reversion of the subpaths
        if (d->reverse & Private::ReverseFirst) {
            d->pathShape->reverseSubpath(d->endPoint.first);
        }
        if (d->reverse & Private::ReverseSecond) {
            d->pathShape->reverseSubpath(d->startPoint.first);
        }
    }

    d->pathShape->normalize();
    d->pathShape->update();

    // reset the removed point
    d->removedPoint = 0;
}
