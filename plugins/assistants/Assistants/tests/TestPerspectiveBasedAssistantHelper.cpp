/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "TestPerspectiveBasedAssistantHelper.h"


#include <sdk/tests/testutil.h>
#include <sdk/tests/testui.h>

#include <kis_painting_assistant.h>
#include <PerspectiveBasedAssistantHelper.h>


void TestPerspectiveBasedAssistantHelper::testDistanceInGrid()
{
    QList<KisPaintingAssistantHandleSP> handles = getHandles({QPointF(-4, 4), QPointF(4, 4), QPointF(8, 8), QPointF(-8, 8)});
    QList<QPointF> pointsToCheck;
    for (int i = 0; i <= 8; i++) {
        pointsToCheck << QPointF(0, i);
    }

    for (int i = 0; i < pointsToCheck.size(); i++) {
        qreal response = PerspectiveBasedAssistantHelper::distanceInGrid(handles, true, pointsToCheck[i]);
        ENTER_FUNCTION() << ppVar(pointsToCheck[i]) << ppVar(response) << "should be: " << i/8.0;
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
