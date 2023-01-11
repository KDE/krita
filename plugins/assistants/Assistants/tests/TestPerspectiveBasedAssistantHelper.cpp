/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "TestPerspectiveBasedAssistantHelper.h"

#include <testui.h>
#include <testutil.h>


#include <kis_painting_assistant.h>
#include <PerspectiveBasedAssistantHelper.h>
#include <kis_algebra_2d.h>
#include <kis_global.h>



void TestPerspectiveBasedAssistantHelper::testDistanceInGrid()
{
    QList<KisPaintingAssistantHandleSP> handles = getHandles({QPointF(-4, 4), QPointF(4, 4), QPointF(8, 8), QPointF(-8, 8)});
    QList<QPointF> pointsToCheck;
    for (int i = 0; i <= 8; i++) {
        pointsToCheck << QPointF(0, i);
    }

    QPolygonF poly;
    bool correct = PerspectiveBasedAssistantHelper::getTetragon(handles, true, poly);
    ENTER_FUNCTION() << correct;

    PerspectiveBasedAssistantHelper::CacheData cache;
    PerspectiveBasedAssistantHelper::updateCacheData(cache, poly);

    auto fuzzyCompare = [] (double a, double b) {
        return qAbs(a - b) < 0.0000001;
    };


    for (int i = 0; i < pointsToCheck.size(); i++) {
        qreal response = PerspectiveBasedAssistantHelper::distanceInGrid(cache, pointsToCheck[i]);
        QVERIFY(fuzzyCompare(response, i/8.0));
    }

    // let's say: vps in (-10, 0) and (10, 0)
    // small point:
    ENTER_FUNCTION() << ppVar(6.0/19) << ppVar(4.0/19);
    QLineF first = QLineF(QPointF(10, 0), QPointF(-10, 8));
    QLineF second = QLineF(QPointF(-10, 0), QPointF(0, 13));
    QPointF inters;
    if (first.intersect(second, &inters) != QLineF::NoIntersection) {
        ENTER_FUNCTION() << "Intersection is: " << inters << "and was supposed to be: " << QPointF(-5 - 15.0/19, 6 + 6.0/19);
    }


    handles = getHandles({QPointF(0, 4), inters, QPointF(0, 13), QPointF(-inters.x(), inters.y())});
    pointsToCheck.clear();
    for (int i = 0; i <= 13; i++) {
        pointsToCheck << QPointF(0, i);
    }

    correct = PerspectiveBasedAssistantHelper::getTetragon(handles, true, poly);
    ENTER_FUNCTION() << correct;

    PerspectiveBasedAssistantHelper::updateCacheData(cache, poly);

    for (int i = 0; i < pointsToCheck.size(); i++) {
        qreal response = PerspectiveBasedAssistantHelper::distanceInGrid(cache, pointsToCheck[i]);
        QVERIFY(fuzzyCompare(response, i/13.0));

    }

}

QList<KisPaintingAssistantHandleSP> TestPerspectiveBasedAssistantHelper::getHandles(QList<QPointF> points)
{
    QList<KisPaintingAssistantHandleSP> handles;
    for(int i = 0; i < points.size(); i++) {
        handles << KisPaintingAssistantHandleSP(new KisPaintingAssistantHandle(points[i]));
    }
    return handles;
}

KISTEST_MAIN(TestPerspectiveBasedAssistantHelper)
