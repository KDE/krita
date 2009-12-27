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
#include <KDebug>

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

QTEST_MAIN(TestPointMergeCommand)
#include <TestPointMergeCommand.moc>
