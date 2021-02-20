/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2009 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2006, 2007 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoPathPointMergeCommand.h"
#include "KoPathPoint.h"
#include "KoPathPointData.h"
#include "KoPathShape.h"
#include <klocalizedstring.h>
#include <QPointF>
#include "kis_assert.h"
#include "KoPathMergeUtils.h"


class Q_DECL_HIDDEN KoPathPointMergeCommand::Private
{
public:
    Private(const KoPathPointData &pointData1, const KoPathPointData &pointData2)
    : pathShape(pointData1.pathShape)
    , pointIndex1(pointData1.pointIndex)
    , pointIndex2(pointData2.pointIndex)
    , removedPoint(0)
    , mergedPointIndex(-1, -1)
    , reverse(ReverseNone)
    {
    }

    ~Private()
    {
        delete removedPoint;
    }

    void savePointState(KoPathPoint *point) {
        savedNodePoint1 = point->point();
        savedControlPoint11 = KritaUtils::fetchControlPoint(point, true);
        savedControlPoint12 = KritaUtils::fetchControlPoint(point, false);
    }

    void restorePointState(KoPathPoint *point) {
        point->setPoint(savedNodePoint1);
        KritaUtils::restoreControlPoint(point, true, savedControlPoint11);
        KritaUtils::restoreControlPoint(point, false, savedControlPoint12);
    }

    void mergePoints(KoPathPoint * p1, KoPathPoint * p2, KoPathPoint *dstPoint)
    {
        QPointF mergePosition = 0.5 * (p1->point() + p2->point());

        boost::optional<QPointF> mergedControlPoint1;
        boost::optional<QPointF> mergedControlPoint2;

        if (p1->activeControlPoint1()) {
            mergedControlPoint1 = mergePosition + (p1->controlPoint1() - p1->point());
        }

        if (p2->activeControlPoint2()) {
            mergedControlPoint2 = mergePosition + (p2->controlPoint2() - p2->point());
        }

        dstPoint->setPoint(mergePosition);
        KritaUtils::restoreControlPoint(dstPoint, true, mergedControlPoint1);
        KritaUtils::restoreControlPoint(dstPoint, false, mergedControlPoint2);
    }

    bool closeSubpathMode() const {
        return pointIndex2.first == pointIndex1.first;
    }

    KoPathShape * pathShape;
    KoPathPointIndex pointIndex1;
    KoPathPointIndex pointIndex2;

    KoPathPoint * removedPoint;
    KoPathPointIndex mergedPointIndex;

    enum Reverse {
        ReverseNone = 0,
        ReverseFirst = 1,
        ReverseSecond = 2
    };

    int reverse;

    QPointF savedNodePoint1;

    boost::optional<QPointF> savedControlPoint11;
    boost::optional<QPointF> savedControlPoint12;
};

/**
 * How does is work:
 *
 * The goal is to merge the point that is ending an open subpath with the one
 * starting the same or another open subpath.
 */
KoPathPointMergeCommand::KoPathPointMergeCommand(const KoPathPointData &pointData1, const KoPathPointData &pointData2, KUndo2Command *parent)
    : KUndo2Command(parent), d(new Private(pointData1, pointData2))
{
    KIS_ASSERT(pointData1.pathShape == pointData2.pathShape);
    KIS_ASSERT(d->pathShape);

    KIS_ASSERT(!d->pathShape->isClosedSubpath(d->pointIndex1.first));
    KIS_ASSERT(!d->pathShape->isClosedSubpath(d->pointIndex2.first));

    KIS_ASSERT(d->pointIndex1.second == 0 ||
             d->pointIndex1.second == d->pathShape->subpathPointCount(d->pointIndex1.first) - 1);

    KIS_ASSERT(d->pointIndex2.second == 0 ||
             d->pointIndex2.second == d->pathShape->subpathPointCount(d->pointIndex2.first) - 1);

    KIS_ASSERT(d->pointIndex2 != d->pointIndex1);

    if (d->pointIndex2 < d->pointIndex1) {
        std::swap(d->pointIndex2, d->pointIndex1);
    }

    // if we have two different subpaths we might need to reverse them
    if (!d->closeSubpathMode()) {
        if (d->pointIndex1.second == 0 &&
            d->pathShape->subpathPointCount(d->pointIndex1.first) > 1) {

            d->reverse |= Private::ReverseFirst;
        }

        if (d->pointIndex2.second != 0 &&
            d->pathShape->subpathPointCount(d->pointIndex2.first) > 1) {

            d->reverse |= Private::ReverseSecond;
        }
    }

    setText(kundo2_i18n("Merge points"));
}

KoPathPointMergeCommand::~KoPathPointMergeCommand()
{
    delete d;
}

void KoPathPointMergeCommand::redo()
{
    KUndo2Command::redo();

    KIS_SAFE_ASSERT_RECOVER_RETURN(!d->removedPoint);

    KoPathPoint * point1 = d->pathShape->pointByIndex(d->pointIndex1);
    KoPathPoint * point2 = d->pathShape->pointByIndex(d->pointIndex2);

    d->pathShape->update();

    if (d->closeSubpathMode()) {
        d->savePointState(point1);
        d->mergePoints(point2, point1, point1);
        d->removedPoint = d->pathShape->removePoint(d->pointIndex2);

        KoPathPointIndex newStartIndex(d->pointIndex1.first, 0);

        d->pathShape->closeSubpath(newStartIndex);
        d->mergedPointIndex = newStartIndex;

    } else {
        if (d->reverse & Private::ReverseFirst) {
            d->pathShape->reverseSubpath(d->pointIndex1.first);
        }

        if (d->reverse & Private::ReverseSecond) {
            d->pathShape->reverseSubpath(d->pointIndex2.first);
        }

        d->pathShape->moveSubpath(d->pointIndex2.first, d->pointIndex1.first + 1);
        d->mergedPointIndex = d->pathShape->pathPointIndex(point1);
        d->pathShape->join(d->pointIndex1.first);

        d->savePointState(point1);
        d->mergePoints(point1, point2, point1);

        KoPathPointIndex removeIndex = d->pathShape->pathPointIndex(point2);
        d->removedPoint = d->pathShape->removePoint(removeIndex);
    }

    d->pathShape->recommendPointSelectionChange({d->mergedPointIndex});
    d->pathShape->update();
}

void KoPathPointMergeCommand::undo()
{
    KUndo2Command::undo();

    d->pathShape->update();

    KIS_SAFE_ASSERT_RECOVER_RETURN(d->removedPoint);

    if (d->closeSubpathMode()) {
        d->pathShape->openSubpath(d->mergedPointIndex);
        d->pathShape->insertPoint(d->removedPoint, d->pointIndex2);
        d->restorePointState(d->pathShape->pointByIndex(d->pointIndex1));
    } else {
        d->pathShape->breakAfter(d->mergedPointIndex);
        d->pathShape->insertPoint(d->removedPoint, KoPathPointIndex(d->mergedPointIndex.first+1,0));
        d->restorePointState(d->pathShape->pointByIndex(d->mergedPointIndex));
        d->pathShape->moveSubpath(d->mergedPointIndex.first+1, d->pointIndex2.first);

        // undo the reversion of the subpaths
        if (d->reverse & Private::ReverseFirst) {
            d->pathShape->reverseSubpath(d->pointIndex1.first);
        }

        if (d->reverse & Private::ReverseSecond) {
            d->pathShape->reverseSubpath(d->pointIndex2.first);
        }
    }

    // reset the removed point
    d->removedPoint = 0;
    d->mergedPointIndex = KoPathPointIndex(-1,-1);

    d->pathShape->recommendPointSelectionChange({d->pointIndex1, d->pointIndex2});
    d->pathShape->update();
}

KoPathPointData KoPathPointMergeCommand::mergedPointData() const
{
    return KoPathPointData(d->pathShape, d->mergedPointIndex);
}
