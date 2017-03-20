/* This file is part of the KDE project
 * Copyright (C) 2009 Jan Hambrecht <jaham@gmx.net>
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

#include "TestPointMergeCommand.h"
#include "KoPathPointMergeCommand.h"
#include "KoPathShape.h"
#include "KoPathPoint.h"
#include "KoPathPointData.h"

#include <QTest>
#include <FlakeDebug.h>

void TestPointMergeCommand::closeSingleLinePath()
{
    KoPathShape path1;
    path1.moveTo(QPointF(40, 0));
    path1.lineTo(QPointF(60, 0));
    path1.lineTo(QPointF(60, 30));
    path1.lineTo(QPointF(0, 30));
    path1.lineTo(QPointF(0, 0));
    path1.lineTo(QPointF(20, 0));

    KoPathPointIndex index1(0,0);
    KoPathPointIndex index2(0,5);

    KoPathPointData pd1(&path1, index1);
    KoPathPointData pd2(&path1, index2);

    KoPathPoint * p1 = path1.pointByIndex(index1);
    KoPathPoint * p2 = path1.pointByIndex(index2);

    QVERIFY(!path1.isClosedSubpath(0));
    QCOMPARE(path1.subpathPointCount(0), 6);
    QCOMPARE(p1->point(), QPointF(40,0));
    QCOMPARE(p2->point(), QPointF(20,0));

    KoPathPointMergeCommand cmd1(pd1,pd2);
    cmd1.redo();

    QVERIFY(path1.isClosedSubpath(0));
    QCOMPARE(path1.subpathPointCount(0), 5);
    QCOMPARE(p2->point(), QPointF(30,0));

    cmd1.undo();

    QVERIFY(!path1.isClosedSubpath(0));
    QCOMPARE(path1.subpathPointCount(0), 6);
    QCOMPARE(p1->point(), QPointF(40,0));
    QCOMPARE(p2->point(), QPointF(20,0));

    KoPathPointMergeCommand cmd2(pd2,pd1);
    cmd2.redo();

    QVERIFY(path1.isClosedSubpath(0));
    QCOMPARE(path1.subpathPointCount(0), 5);
    QCOMPARE(p2->point(), QPointF(30,0));

    cmd2.undo();

    QVERIFY(!path1.isClosedSubpath(0));
    QCOMPARE(path1.subpathPointCount(0), 6);
    QCOMPARE(p1->point(), QPointF(40,0));
    QCOMPARE(p2->point(), QPointF(20,0));
}

void TestPointMergeCommand::closeSingleCurvePath()
{
    KoPathShape path1;
    path1.moveTo(QPointF(40, 0));
    path1.curveTo(QPointF(60, 0), QPointF(60,0), QPointF(60,60));
    path1.lineTo(QPointF(0, 60));
    path1.curveTo(QPointF(0, 0), QPointF(0,0), QPointF(20,0));

    KoPathPointIndex index1(0,0);
    KoPathPointIndex index2(0,3);

    KoPathPointData pd1(&path1, index1);
    KoPathPointData pd2(&path1, index2);

    KoPathPoint * p1 = path1.pointByIndex(index1);
    KoPathPoint * p2 = path1.pointByIndex(index2);

    QVERIFY(!path1.isClosedSubpath(0));
    QCOMPARE(path1.subpathPointCount(0), 4);
    QCOMPARE(p1->point(), QPointF(40,0));
    QVERIFY(!p1->activeControlPoint1());
    QCOMPARE(p2->point(), QPointF(20,0));
    QVERIFY(!p2->activeControlPoint2());

    KoPathPointMergeCommand cmd1(pd1,pd2);
    cmd1.redo();

    QVERIFY(path1.isClosedSubpath(0));
    QCOMPARE(path1.subpathPointCount(0), 3);
    QCOMPARE(p2->point(), QPointF(30,0));
    QVERIFY(p2->activeControlPoint1());
    QVERIFY(p2->activeControlPoint2());

    cmd1.undo();

    QVERIFY(!path1.isClosedSubpath(0));
    QCOMPARE(path1.subpathPointCount(0), 4);
    QCOMPARE(p1->point(), QPointF(40,0));
    QVERIFY(!p1->activeControlPoint1());
    QCOMPARE(p2->point(), QPointF(20,0));
    QVERIFY(!p2->activeControlPoint2());

    KoPathPointMergeCommand cmd2(pd2,pd1);
    cmd2.redo();

    QVERIFY(path1.isClosedSubpath(0));
    QCOMPARE(path1.subpathPointCount(0), 3);
    QCOMPARE(p2->point(), QPointF(30,0));
    QVERIFY(p2->activeControlPoint1());
    QVERIFY(p2->activeControlPoint2());

    cmd2.undo();

    QVERIFY(!path1.isClosedSubpath(0));
    QCOMPARE(path1.subpathPointCount(0), 4);
    QCOMPARE(p1->point(), QPointF(40,0));
    QVERIFY(!p1->activeControlPoint1());
    QCOMPARE(p2->point(), QPointF(20,0));
    QVERIFY(!p2->activeControlPoint2());
}

void TestPointMergeCommand::connectLineSubpaths()
{
    KoPathShape path1;
    path1.moveTo(QPointF(0,0));
    path1.lineTo(QPointF(10,0));
    path1.moveTo(QPointF(20,0));
    path1.lineTo(QPointF(30,0));

    KoPathPointIndex index1(0,1);
    KoPathPointIndex index2(1,0);

    KoPathPointData pd1(&path1, index1);
    KoPathPointData pd2(&path1, index2);

    QCOMPARE(path1.subpathCount(), 2);
    QCOMPARE(path1.pointByIndex(index1)->point(), QPointF(10,0));
    QCOMPARE(path1.pointByIndex(index2)->point(), QPointF(20,0));

    KoPathPointMergeCommand cmd1(pd1, pd2);
    cmd1.redo();

    QCOMPARE(path1.subpathCount(), 1);
    QCOMPARE(path1.pointByIndex(index1)->point(), QPointF(15,0));

    cmd1.undo();

    QCOMPARE(path1.subpathCount(), 2);
    QCOMPARE(path1.pointByIndex(index1)->point(), QPointF(10,0));
    QCOMPARE(path1.pointByIndex(index2)->point(), QPointF(20,0));

    KoPathPointMergeCommand cmd2(pd2, pd1);
    cmd2.redo();

    QCOMPARE(path1.subpathCount(), 1);
    QCOMPARE(path1.pointByIndex(index1)->point(), QPointF(15,0));

    cmd2.undo();

    QCOMPARE(path1.subpathCount(), 2);
    QCOMPARE(path1.pointByIndex(index1)->point(), QPointF(10,0));
    QCOMPARE(path1.pointByIndex(index2)->point(), QPointF(20,0));
}

void TestPointMergeCommand::connectCurveSubpaths()
{
    KoPathShape path1;
    path1.moveTo(QPointF(0,0));
    path1.curveTo(QPointF(20,0),QPointF(0,20),QPointF(20,20));
    path1.moveTo(QPointF(50,0));
    path1.curveTo(QPointF(30,0), QPointF(50,20), QPointF(30,20));

    KoPathPointIndex index1(0,1);
    KoPathPointIndex index2(1,1);

    KoPathPointData pd1(&path1, index1);
    KoPathPointData pd2(&path1, index2);

    QCOMPARE(path1.subpathCount(), 2);
    QCOMPARE(path1.pointByIndex(index1)->point(), QPointF(20,20));
    QCOMPARE(path1.pointByIndex(index1)->controlPoint1(), QPointF(0,20));
    QCOMPARE(path1.pointByIndex(index2)->point(), QPointF(30,20));
    QCOMPARE(path1.pointByIndex(index2)->controlPoint1(), QPointF(50,20));
    QVERIFY(path1.pointByIndex(index1)->activeControlPoint1());
    QVERIFY(!path1.pointByIndex(index1)->activeControlPoint2());

    KoPathPointMergeCommand cmd1(pd1, pd2);
    cmd1.redo();

    QCOMPARE(path1.subpathCount(), 1);
    QCOMPARE(path1.pointByIndex(index1)->point(), QPointF(25,20));
    QCOMPARE(path1.pointByIndex(index1)->controlPoint1(), QPointF(5,20));
    QCOMPARE(path1.pointByIndex(index1)->controlPoint2(), QPointF(45,20));
    QVERIFY(path1.pointByIndex(index1)->activeControlPoint1());
    QVERIFY(path1.pointByIndex(index1)->activeControlPoint2());

    cmd1.undo();

    QCOMPARE(path1.subpathCount(), 2);
    QCOMPARE(path1.pointByIndex(index1)->point(), QPointF(20,20));
    QCOMPARE(path1.pointByIndex(index1)->controlPoint1(), QPointF(0,20));
    QCOMPARE(path1.pointByIndex(index2)->point(), QPointF(30,20));
    QCOMPARE(path1.pointByIndex(index2)->controlPoint1(), QPointF(50,20));
    QVERIFY(path1.pointByIndex(index1)->activeControlPoint1());
    QVERIFY(!path1.pointByIndex(index1)->activeControlPoint2());

    KoPathPointMergeCommand cmd2(pd2, pd1);
    cmd2.redo();

    QCOMPARE(path1.subpathCount(), 1);
    QCOMPARE(path1.pointByIndex(index1)->point(), QPointF(25,20));
    QCOMPARE(path1.pointByIndex(index1)->controlPoint1(), QPointF(5,20));
    QCOMPARE(path1.pointByIndex(index1)->controlPoint2(), QPointF(45,20));
    QVERIFY(path1.pointByIndex(index1)->activeControlPoint1());
    QVERIFY(path1.pointByIndex(index1)->activeControlPoint2());

    cmd2.undo();

    QCOMPARE(path1.subpathCount(), 2);
    QCOMPARE(path1.pointByIndex(index1)->point(), QPointF(20,20));
    QCOMPARE(path1.pointByIndex(index1)->controlPoint1(), QPointF(0,20));
    QCOMPARE(path1.pointByIndex(index2)->point(), QPointF(30,20));
    QCOMPARE(path1.pointByIndex(index2)->controlPoint1(), QPointF(50,20));
    QVERIFY(path1.pointByIndex(index1)->activeControlPoint1());
    QVERIFY(!path1.pointByIndex(index1)->activeControlPoint2());
}

#include <MockShapes.h>
#include <commands/KoPathCombineCommand.h>
#include "kis_debug.h"
#include <KoPathPointData.h>

void TestPointMergeCommand::testCombineShapes()
{
    MockShapeController mockController;
    MockCanvas canvas(&mockController);

    QList<KoPathShape*> shapes;

    for (int i = 0; i < 3; i++) {
        const QPointF step(15,15);
        const QRectF rect = QRectF(5,5,10,10).translated(step * i);

        QPainterPath p;
        p.addRect(rect);

        KoPathShape *shape = KoPathShape::createShapeFromPainterPath(p);
        QCOMPARE(shape->absoluteOutlineRect(), rect);

        shapes << shape;
        mockController.addShape(shape);
    }

    KoPathCombineCommand cmd(&mockController, shapes);
    cmd.redo();

    QCOMPARE(canvas.shapeManager()->shapes().size(), 1);

    KoPathShape *combinedShape = dynamic_cast<KoPathShape*>(canvas.shapeManager()->shapes().first());
    QCOMPARE(combinedShape, cmd.combinedPath());
    QCOMPARE(combinedShape->subpathCount(), 3);
    QCOMPARE(combinedShape->absoluteOutlineRect(), QRectF(5,5,40,40));

    QList<KoPathPointData> tstPoints;
    QList<KoPathPointData> expPoints;

    tstPoints << KoPathPointData(shapes[0], KoPathPointIndex(0,1));
    expPoints << KoPathPointData(combinedShape, KoPathPointIndex(0,1));

    tstPoints << KoPathPointData(shapes[1], KoPathPointIndex(0,2));
    expPoints << KoPathPointData(combinedShape, KoPathPointIndex(1,2));

    tstPoints << KoPathPointData(shapes[2], KoPathPointIndex(0,3));
    expPoints << KoPathPointData(combinedShape, KoPathPointIndex(2,3));

    for (int i = 0; i < tstPoints.size(); i++) {
        KoPathPointData convertedPoint = cmd.originalToCombined(tstPoints[i]);
        QCOMPARE(convertedPoint, expPoints[i]);
    }

    qDeleteAll(canvas.shapeManager()->shapes());
}

#include <commands/KoMultiPathPointMergeCommand.h>
#include <KoSelection.h>

inline QPointF fetchPoint(KoPathShape *shape, int subpath, int pointIndex) {
    return shape->absoluteTransformation(0).map(
        shape->pointByIndex(KoPathPointIndex(subpath, pointIndex))->point());
}

void dumpShape(KoPathShape *shape, const QString &fileName)
{
    QImage tmp(50,50, QImage::Format_ARGB32);
    tmp.fill(0);
    QPainter p(&tmp);
    p.drawPath(shape->absoluteTransformation(0).map(shape->outline()));
    tmp.save(fileName);
}

void testMultipathMergeShapesImpl(const int srcPointIndex1,
                                  const int srcPointIndex2,
                                  const QRectF &expectedOutlineRect,
                                  const QList<QPointF> &expectedResultPoints,
                                  bool singleShape = false)
{
    MockShapeController mockController;
    MockCanvas canvas(&mockController);

    QList<KoPathShape*> shapes;

    for (int i = 0; i < 3; i++) {
        const QPointF step(15,15);
        const QRectF rect = QRectF(5,5,10,10).translated(step * i);

        QPainterPath p;
        p.moveTo(rect.topLeft());
        p.lineTo(rect.bottomRight());
        p.lineTo(rect.topRight());

        KoPathShape *shape = KoPathShape::createShapeFromPainterPath(p);
        QCOMPARE(shape->absoluteOutlineRect(), rect);

        shapes << shape;
        mockController.addShape(shape);
    }


    {
        KoPathPointData pd1(shapes[0], KoPathPointIndex(0,srcPointIndex1));
        KoPathPointData pd2(shapes[singleShape ? 0 : 1], KoPathPointIndex(0,srcPointIndex2));

        KoMultiPathPointMergeCommand cmd(pd1, pd2, &mockController, canvas.shapeManager()->selection());

        cmd.redo();

        const int expectedShapesCount = singleShape ? 3 : 2;
        QCOMPARE(canvas.shapeManager()->shapes().size(), expectedShapesCount);

        KoPathShape *combinedShape = 0;

        if (!singleShape) {
            combinedShape = dynamic_cast<KoPathShape*>(canvas.shapeManager()->shapes()[1]);
            QCOMPARE(combinedShape, cmd.testingCombinedPath());
        } else {
            combinedShape = dynamic_cast<KoPathShape*>(canvas.shapeManager()->shapes()[0]);
            QCOMPARE(combinedShape, shapes[0]);
        }

        QCOMPARE(combinedShape->subpathCount(), 1);
        QCOMPARE(combinedShape->absoluteOutlineRect(), expectedOutlineRect);

        if (singleShape) {
            QCOMPARE(combinedShape->isClosedSubpath(0), true);
        }


        QCOMPARE(combinedShape->subpathPointCount(0), expectedResultPoints.size());
        for (int i = 0; i < expectedResultPoints.size(); i++) {
            if (fetchPoint(combinedShape, 0, i) != expectedResultPoints[i]) {
                qDebug() << ppVar(i);
                qDebug() << ppVar(fetchPoint(combinedShape, 0, i));
                qDebug() << ppVar(expectedResultPoints[i]);

                QFAIL("Resulting shape points are different!");
            }
        }

        QList<KoShape*> shapes = canvas.shapeManager()->selection()->selectedEditableShapes();
        QCOMPARE(shapes.size(), 1);
        QCOMPARE(shapes.first(), combinedShape);

        //dumpShape(combinedShape, "tmp_0_seq.png");
        cmd.undo();

        QCOMPARE(canvas.shapeManager()->shapes().size(), 3);
    }
}


void TestPointMergeCommand::testMultipathMergeShapesBothSequential()
{
    // both sequential
    testMultipathMergeShapesImpl(2, 0,
                                 QRectF(5,5,25,25),
                                 {
                                     QPointF(5,5),
                                     QPointF(15,15),
                                     QPointF(17.5,12.5), // merged by melding the points!
                                     QPointF(30,30),
                                     QPointF(30,20)
                                 });
}

void TestPointMergeCommand::testMultipathMergeShapesFirstReversed()
{
    // first reversed
    testMultipathMergeShapesImpl(0, 0,
                                 QRectF(12.5,5,17.5,25),
                                 {
                                     QPointF(15,5),
                                     QPointF(15,15),
                                     QPointF(12.5,12.5), // merged by melding the points!
                                     QPointF(30,30),
                                     QPointF(30,20)
                                 });
}

void TestPointMergeCommand::testMultipathMergeShapesSecondReversed()
{
    // second reversed
    testMultipathMergeShapesImpl(2, 2,
                                 QRectF(5,5,25,25),
                                 {
                                     QPointF(5,5),
                                     QPointF(15,15),
                                     QPointF(22.5,12.5), // merged by melding the points!
                                     QPointF(30,30),
                                     QPointF(20,20)
                                 });
}

void TestPointMergeCommand::testMultipathMergeShapesBothReversed()
{
    // both reversed
    testMultipathMergeShapesImpl(0, 2,
                                 QRectF(15,5,15,25),
                                 {
                                     QPointF(15,5),
                                     QPointF(15,15),
                                     QPointF(17.5,12.5), // merged by melding the points!
                                     QPointF(30,30),
                                     QPointF(20,20)
                                 });
}

void TestPointMergeCommand::testMultipathMergeShapesSingleShapeEndToStart()
{
    // close end->start
    testMultipathMergeShapesImpl(2, 0,
                                 QRectF(10,5,5,10),
                                 {
                                     QPointF(15,15),
                                     QPointF(10,5)
                                 }, true);
}

void TestPointMergeCommand::testMultipathMergeShapesSingleShapeStartToEnd()
{
    // close start->end
    testMultipathMergeShapesImpl(0, 2,
                                 QRectF(10,5,5,10),
                                 {
                                     QPointF(15,15),
                                     QPointF(10,5)
                                 }, true);
}

QTEST_MAIN(TestPointMergeCommand)
