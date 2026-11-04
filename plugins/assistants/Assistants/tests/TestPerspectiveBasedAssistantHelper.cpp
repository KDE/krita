/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "TestPerspectiveBasedAssistantHelper.h"

#include <testui.h>
#include <testutil.h>

#include <cmath>

#include <kis_painting_assistant.h>
#include <CurvilinearPerspectiveAssistant.h>
#include <PerspectiveBasedAssistantHelper.h>
#include <TwoPointAssistant.h>
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
    if (first.intersects(second, &inters) != QLineF::NoIntersection) {
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

void TestPerspectiveBasedAssistantHelper::testTwoPointAssistantPerspectiveGrid_data()
{
    QTest::addColumn<QTransform>("imageTransform");
    QTest::addColumn<bool>("reverseHandles");

    QTest::newRow("identity") << QTransform() << false;
    QTest::newRow("translated") << QTransform::fromTranslate(42.0, -17.0) << false;

    QTransform rotated;
    rotated.translate(42.0, -17.0);
    rotated.rotate(37.0);
    QTest::newRow("rotated") << rotated << false;

    QTransform enlarged = rotated;
    enlarged.scale(2.5, 2.5);
    QTest::newRow("enlarged") << enlarged << false;
    QTest::newRow("reversed-vanishing-points") << enlarged << true;

    QTransform reduced = rotated;
    reduced.scale(0.25, 0.25);
    QTest::newRow("reduced") << reduced << false;
}

void TestPerspectiveBasedAssistantHelper::testTwoPointAssistantLocalTransform_data()
{
    testTwoPointAssistantPerspectiveGrid_data();
}

void TestPerspectiveBasedAssistantHelper::testTwoPointAssistantLocalTransform()
{
    QFETCH(QTransform, imageTransform);
    QFETCH(bool, reverseHandles);

    QPointF vpA = imageTransform.map(QPointF(-10.0, 0.0));
    QPointF vpB = imageTransform.map(QPointF(10.0, 0.0));
    if (reverseHandles) {
        qSwap(vpA, vpB);
    }
    const qreal scale = QLineF(vpA, vpB).length() / 20.0;
    const auto closePoint = [](const QPointF &a, const QPointF &b) {
        return QLineF(a, b).length() < 1e-7;
    };

    TwoPointAssistant assistant;
    for (qreal centerX : {-3.0, 0.0, 3.0}) {
        for (qreal centerY : {-5.0, 0.0, 5.0}) {
            const QPointF center = imageTransform.map(QPointF(centerX, centerY));
            const QPointF origin = imageTransform.map(QPointF(centerX, 0.0));
            qreal size = 0.0;
            const QTransform transform = assistant.localTransform(vpA, vpB, center, &size);
            const qreal direction = reverseHandles ? -1.0 : 1.0;

            QVERIFY(closePoint(transform.map(origin), QPointF()));
            QVERIFY(closePoint(transform.map(center), QPointF(0.0, direction * centerY * scale)));
            const qreal firstVpX = reverseHandles ? 10.0 : -10.0;
            const qreal secondVpX = -firstVpX;
            QVERIFY(closePoint(transform.map(vpA), QPointF(direction * (firstVpX - centerX) * scale, 0.0)));
            QVERIFY(closePoint(transform.map(vpB), QPointF(direction * (secondVpX - centerX) * scale, 0.0)));
            QVERIFY(qAbs(transform.map(vpA).y()) < 1e-7);
            QVERIFY(qAbs(transform.map(vpB).y()) < 1e-7);
            QVERIFY(qAbs(size - std::sqrt(100.0 - centerX * centerX) * scale) < 1e-7);
            QVERIFY(closePoint(transform.inverted().map(QPointF()), origin));
            QVERIFY(qAbs(QLineF(transform.map(center), transform.map(vpA)).length()
                         - QLineF(center, vpA).length()) < 1e-7);

            // A valid opposite vanishing point is on y = 0, not a null point.
            QVERIFY(!qFuzzyIsNull(transform.map(vpB).x()));
        }
    }
}

void TestPerspectiveBasedAssistantHelper::testTwoPointAssistantPerspectiveGrid()
{
    QFETCH(QTransform, imageTransform);
    QFETCH(bool, reverseHandles);

    // In image coordinates the horizon is y = 0 and the cone radius is 10.
    // The third handle is deliberately off the horizon. localTransform() puts
    // its projection onto the horizon at the local origin. Rotation and
    // translation preserve lengths, so size has the same units as the image.
    QPointF vpA = imageTransform.map(QPointF(-10.0, 0.0));
    QPointF vpB = imageTransform.map(QPointF(10.0, 0.0));
    if (reverseHandles) {
        qSwap(vpA, vpB);
    }

    TwoPointAssistant assistant;
    assistant.addHandle(new KisPaintingAssistantHandle(vpA), HandleType::NORMAL);
    assistant.addHandle(new KisPaintingAssistantHandle(vpB), HandleType::NORMAL);
    assistant.addHandle(new KisPaintingAssistantHandle(imageTransform.map(QPointF(0.0, -5.0))), HandleType::NORMAL);

    QVERIFY(assistant.isAssistantComplete());
    QVERIFY(assistant.isActive());

    // Build expectations independently in image space, not using the transform
    // under test. Cover both sides of the horizon and the inclusive boundary.
    for (qreal x : {-3.0, 0.0, 3.0}) {
        for (qreal y : {0.0, -5.0, 5.0, -10.0, 10.0, -11.0, 11.0}) {
            const QPointF point = imageTransform.map(QPointF(x, y));
            QCOMPARE(assistant.contains(point), qAbs(y) <= 10.0);
            const qreal expectedDistance = qMin(qAbs(y) / 10.0, 1.0);
            QVERIFY(qAbs(assistant.distance(point) - expectedDistance) < 0.0000001);
        }
    }
}

void TestPerspectiveBasedAssistantHelper::testCurvilinearPerspectiveGrid()
{
    CurvilinearPerspectiveAssistant assistant;
    assistant.addHandle(new KisPaintingAssistantHandle(QPointF(-10.0, 0.0)), HandleType::NORMAL);
    assistant.addHandle(new KisPaintingAssistantHandle(QPointF(10.0, 0.0)), HandleType::NORMAL);

    QVERIFY(assistant.isAssistantComplete());
    QVERIFY(assistant.isActive());

    const QPointF horizonPoint(0.0, 0.0);
    const QPointF nearPoint(0.0, 10.0);
    const QPointF farPoint(0.0, 30.0);

    auto fuzzyCompare = [] (double a, double b) {
        return qAbs(a - b) < 0.0000001;
    };

    QVERIFY(assistant.contains(horizonPoint));
    QVERIFY(assistant.contains(nearPoint));
    QVERIFY(assistant.contains(farPoint));

    QVERIFY(fuzzyCompare(assistant.distance(horizonPoint), 0.0));
    QVERIFY(fuzzyCompare(assistant.distance(nearPoint), 0.5));
    QVERIFY(fuzzyCompare(assistant.distance(farPoint), 0.75));
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
