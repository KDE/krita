/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2006 Thorsten Zachmann <zachmann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "TestPathShape.h"

#include <QPainterPath>
#include "KoPathShape.h"
#include "KoPathPoint.h"
#include "KoPathPointData.h"
#include "KoPathSegment.h"

#include <QTest>

void TestPathShape::close()
{
    KoPathShape path;
    path.lineTo(QPointF(10, 0));
    path.lineTo(QPointF(10, 10));

    QPainterPath ppath(QPointF(0, 0));
    ppath.lineTo(QPointF(10, 0));
    ppath.lineTo(10, 10);

    QVERIFY(ppath == path.outline());

    path.close();
    ppath.closeSubpath();

    QVERIFY(ppath == path.outline());

    path.lineTo(QPointF(0, 10));
    ppath.lineTo(0, 10);

    QVERIFY(ppath == path.outline());
}

void TestPathShape::moveTo()
{
    KoPathShape path;
    path.moveTo(QPointF(10, 10));
    QPainterPath ppath(QPointF(10, 10));
    path.lineTo(QPointF(20, 20));
    ppath.lineTo(20, 20);
    QVERIFY(ppath == path.outline());
    path.moveTo(QPointF(30, 30));
    ppath.moveTo(30, 30);
    path.lineTo(QPointF(40, 40));
    ppath.lineTo(QPointF(40, 40));
    QVERIFY(ppath == path.outline());
}

void TestPathShape::normalize()
{
    KoPathShape path;
    path.moveTo(QPointF(10, 10));
    path.lineTo(QPointF(20, 20));
    path.normalize();
    QPainterPath ppath(QPointF(0, 0));
    ppath.lineTo(10, 10);
    QVERIFY(ppath == path.outline());
}

void TestPathShape::pathPointIndex()
{
    KoPathShape path;
    KoPathPoint * point1 = path.moveTo(QPointF(10, 10));
    KoPathPointIndex p1Index(0, 0);
    KoPathPoint * point2 = path.lineTo(QPointF(20, 20));
    KoPathPointIndex p2Index(0, 1);
    KoPathPoint * point3 = path.moveTo(QPointF(30, 30));
    KoPathPointIndex p3Index(1, 0);
    KoPathPoint * point4 = path.lineTo(QPointF(40, 40));
    KoPathPointIndex p4Index(1, 1);
    KoPathPoint * point5 = 0;
    KoPathPointIndex p5Index(-1, -1);

    QCOMPARE(p1Index, path.pathPointIndex(point1));
    QCOMPARE(p2Index, path.pathPointIndex(point2));
    QCOMPARE(p3Index, path.pathPointIndex(point3));
    QCOMPARE(p4Index, path.pathPointIndex(point4));
    QCOMPARE(p5Index, path.pathPointIndex(point5));

    QVERIFY(p1Index == path.pathPointIndex(point1));
    QVERIFY(p2Index == path.pathPointIndex(point2));
    QVERIFY(p3Index == path.pathPointIndex(point3));
    QVERIFY(p4Index == path.pathPointIndex(point4));
    QVERIFY(p5Index == path.pathPointIndex(point5));
}

void TestPathShape::pointByIndex()
{
    KoPathShape path;
    KoPathPoint * point1 = path.moveTo(QPointF(10, 10));
    KoPathPoint * point2 = path.lineTo(QPointF(20, 20));
    KoPathPoint * point3 = path.moveTo(QPointF(30, 30));
    KoPathPoint * point4 = path.lineTo(QPointF(40, 40));
    KoPathPoint * point5 = 0;

    QVERIFY(point1 == path.pointByIndex(path.pathPointIndex(point1)));
    QVERIFY(point2 == path.pointByIndex(path.pathPointIndex(point2)));
    QVERIFY(point3 == path.pointByIndex(path.pathPointIndex(point3)));
    QVERIFY(point4 == path.pointByIndex(path.pathPointIndex(point4)));
    QVERIFY(point5 == path.pointByIndex(path.pathPointIndex(point5)));
}

void TestPathShape::segmentByIndex()
{
    KoPathShape path;
    KoPathPoint * point1 = path.moveTo(QPointF(20, 20));
    KoPathPoint * point2 = path.lineTo(QPointF(15, 25));
    path.lineTo(QPointF(10, 20));
    path.close();
    path.moveTo(QPointF(20, 30));
    KoPathPoint * point3 = path.lineTo(QPointF(20, 30));
    path.moveTo(QPointF(30, 30));
    path.lineTo(QPointF(40, 30));
    path.lineTo(QPointF(40, 40));
    path.curveTo(QPointF(40, 45), QPointF(30, 45), QPointF(30, 40));
    KoPathPoint * point4 = path.moveTo(QPointF(50, 50));
    path.lineTo(QPointF(60, 50));
    path.lineTo(QPointF(60, 60));
    KoPathPoint * point5 = path.curveTo(QPointF(60, 65), QPointF(50, 65), QPointF(50, 60));
    path.close();

    QVERIFY(KoPathSegment(point1, point2) == path.segmentByIndex(path.pathPointIndex(point1)));
    // test last point in a open path
    QVERIFY(KoPathSegment(0, 0) == path.segmentByIndex(path.pathPointIndex(point3)));
    // test last point in a closed path
    QVERIFY(KoPathSegment(point5, point4) == path.segmentByIndex(path.pathPointIndex(point5)));
    // test out of bounds
    QVERIFY(KoPathSegment(0, 0) == path.segmentByIndex(KoPathPointIndex(3, 4)));
    QVERIFY(KoPathSegment(0, 0) == path.segmentByIndex(KoPathPointIndex(4, 0)));
}

void TestPathShape::pointCount()
{
    KoPathShape path;
    path.moveTo(QPointF(20, 20));
    path.lineTo(QPointF(15, 25));
    path.lineTo(QPointF(10, 20));
    path.close();

    QVERIFY(path.pointCount() == 3);

    path.moveTo(QPointF(20, 30));
    path.lineTo(QPointF(20, 30));
    path.moveTo(QPointF(30, 30));
    path.lineTo(QPointF(40, 30));
    path.lineTo(QPointF(40, 40));
    path.curveTo(QPointF(40, 45), QPointF(30, 45), QPointF(30, 40));

    QVERIFY(path.pointCount() == 9);

    path.moveTo(QPointF(50, 50));
    path.lineTo(QPointF(60, 50));
    path.lineTo(QPointF(60, 60));
    path.curveTo(QPointF(60, 65), QPointF(50, 65), QPointF(50, 60));
    path.close();

    QVERIFY(path.pointCount() == 13);
}

void TestPathShape::subpathPointCount()
{
    KoPathShape path;
    path.moveTo(QPointF(20, 20));
    path.lineTo(QPointF(15, 25));
    path.lineTo(QPointF(10, 20));
    path.close();
    path.moveTo(QPointF(20, 30));
    path.lineTo(QPointF(20, 30));
    path.moveTo(QPointF(30, 30));
    path.lineTo(QPointF(40, 30));
    path.lineTo(QPointF(40, 40));
    path.curveTo(QPointF(40, 45), QPointF(30, 45), QPointF(30, 40));
    path.moveTo(QPointF(50, 50));
    path.lineTo(QPointF(60, 50));
    path.lineTo(QPointF(60, 60));
    path.curveTo(QPointF(60, 65), QPointF(50, 65), QPointF(50, 60));
    path.close();

    QVERIFY(path.subpathPointCount(0) == 3);
    QVERIFY(path.subpathPointCount(1) == 2);
    QVERIFY(path.subpathPointCount(2) == 4);
    QVERIFY(path.subpathPointCount(3) == 4);
    QVERIFY(path.subpathPointCount(4) == -1);
}

void TestPathShape::isClosedSubpath()
{
    KoPathShape path;
    path.moveTo(QPointF(20, 20));
    path.lineTo(QPointF(15, 25));
    path.lineTo(QPointF(10, 20));
    path.close();
    path.moveTo(QPointF(20, 30));
    path.lineTo(QPointF(20, 30));
    path.moveTo(QPointF(30, 30));
    path.lineTo(QPointF(40, 30));
    path.lineTo(QPointF(40, 40));
    path.curveTo(QPointF(40, 45), QPointF(30, 45), QPointF(30, 40));
    path.close();
    path.moveTo(QPointF(50, 50));
    path.lineTo(QPointF(60, 50));
    path.lineTo(QPointF(60, 60));
    path.curveTo(QPointF(60, 65), QPointF(50, 65), QPointF(50, 60));
    path.close();

    QVERIFY(path.isClosedSubpath(0) == true);
    QVERIFY(path.isClosedSubpath(1) == false);
    QVERIFY(path.isClosedSubpath(2) == true);
    QVERIFY(path.isClosedSubpath(3) == true);
}

void TestPathShape::insertPoint()
{
    KoPathShape path;
    path.moveTo(QPointF(10, 10));
    path.lineTo(QPointF(20, 20));
    path.moveTo(QPointF(30, 30));
    path.lineTo(QPointF(40, 40));
    path.close();

    // add before the first point of a open subpath
    KoPathPoint *point1 = new KoPathPoint(&path, QPointF(5, 5), KoPathPoint::Normal);
    KoPathPointIndex p1Index(0, 0);
    QVERIFY(path.insertPoint(point1, p1Index) == true);
    QVERIFY(point1->parent() == &path);

    KoPathPoint *point2 = new KoPathPoint(&path, QPointF(15, 15), KoPathPoint::Normal);
    KoPathPointIndex p2Index(0, 2);
    QVERIFY(path.insertPoint(point2, p2Index) == true);
    QVERIFY(point2->parent() == &path);

    // add after last point of a open subpath
    KoPathPoint *point3 = new KoPathPoint(&path, QPointF(25, 25), KoPathPoint::Normal);
    KoPathPointIndex p3Index(0, 4);
    QVERIFY(path.insertPoint(point3, p3Index) == true);
    QVERIFY(point3->parent() == &path);

    KoPathPoint *point4 = new KoPathPoint(&path, QPointF(40, 30), KoPathPoint::Normal);
    KoPathPointIndex p4Index(1, 1);
    QVERIFY(path.insertPoint(point4, p4Index) == true);
    QVERIFY(point4->parent() == &path);

    // add before the first point of a closed subpath
    KoPathPoint *point5 = new KoPathPoint(&path, QPointF(30, 35), KoPathPoint::Normal);
    KoPathPointIndex p5Index(1, 0);
    QVERIFY(path.insertPoint(point5, p5Index) == true);
    QVERIFY(point5->parent() == &path);

    // add after last point of a closed subpath
    KoPathPoint *point6 = new KoPathPoint(&path, QPointF(35, 40), KoPathPoint::Normal);
    KoPathPointIndex p6Index(1, 4);
    QVERIFY(path.insertPoint(point6, p6Index) == true);
    QVERIFY(point6->parent() == &path);

    // test out of bounds
    KoPathPoint *point7 = new KoPathPoint(&path, QPointF(0, 0), KoPathPoint::Normal);
    // subpath index out of bounds
    KoPathPointIndex p7Index(2, 0);
    QVERIFY(path.insertPoint(point7, p7Index) == false);
    // position in subpath out of bounds
    p7Index.second = 6;
    QVERIFY(path.insertPoint(point7, p7Index) == false);

    QPainterPath ppath(QPointF(5, 5));
    ppath.lineTo(10, 10);
    ppath.lineTo(15, 15);
    ppath.lineTo(20, 20);
    ppath.lineTo(25, 25);
    ppath.moveTo(30, 35);
    ppath.lineTo(30, 30);
    ppath.lineTo(40, 30);
    ppath.lineTo(40, 40);
    ppath.lineTo(35, 40);
    ppath.closeSubpath();

    QVERIFY(ppath == path.outline());

    KoPathShape path2;
    path2.moveTo(QPointF(0, 0));
    KoPathPoint * p = new KoPathPoint(0, QPointF(100, 100));
    QVERIFY(path2.insertPoint(p, KoPathPointIndex(0, 1)) == true);
    QVERIFY(p->parent() == &path2);
}

void TestPathShape::removePoint()
{
    KoPathShape path;
    KoPathPoint *point1 = path.moveTo(QPointF(10, 10));
    path.lineTo(QPointF(20, 10));
    KoPathPoint *point3 = path.lineTo(QPointF(20, 20));
    path.lineTo(QPointF(15, 25));
    KoPathPoint *point5 = path.lineTo(QPointF(10, 20));
    KoPathPoint *point6 = path.moveTo(QPointF(30, 30));
    path.lineTo(QPointF(40, 30));
    KoPathPoint *point8 = path.lineTo(QPointF(40, 40));
    path.curveTo(QPointF(40, 45), QPointF(30, 45), QPointF(30, 40));
    KoPathPoint *point10 = path.lineTo(QPointF(30, 35));
    path.close();

    // remove from beginning of a open subpath
    QVERIFY(path.removePoint(path.pathPointIndex(point1)) == point1);
    // remove from middle of a open subpath
    QVERIFY(path.removePoint(path.pathPointIndex(point3)) == point3);
    // remove from end of a open subpath
    QVERIFY(path.removePoint(path.pathPointIndex(point5)) == point5);

    // remove from beginning of a closed subpath
    QVERIFY(path.removePoint(path.pathPointIndex(point6)) == point6);
    // remove from middle of a closed subpath
    QVERIFY(path.removePoint(path.pathPointIndex(point8)) == point8);
    // remove from end of a closed subpath
    QVERIFY(path.removePoint(path.pathPointIndex(point10)) == point10);

    QPainterPath ppath(QPointF(20, 10));
    ppath.lineTo(15, 25);
    ppath.moveTo(40, 30);
    ppath.quadTo(30, 45, 30, 40);
    ppath.closeSubpath();

    QVERIFY(ppath == path.outline());
}

void TestPathShape::splitAfter()
{
    KoPathShape path;
    path.moveTo(QPointF(10, 10));
    path.lineTo(QPointF(20, 10));
    path.lineTo(QPointF(20, 20));
    path.lineTo(QPointF(15, 25));
    path.lineTo(QPointF(10, 20));
    path.moveTo(QPointF(30, 30));
    path.lineTo(QPointF(40, 30));
    path.lineTo(QPointF(40, 40));
    path.curveTo(QPointF(40, 45), QPointF(30, 45), QPointF(30, 40));
    path.close();

    QVERIFY(path.breakAfter(KoPathPointIndex(0, 1)) == true);
    // try to break at the last point in the subpath
    QVERIFY(path.breakAfter(KoPathPointIndex(1, 2)) == false);
    // try to break a closed subpath
    QVERIFY(path.breakAfter(KoPathPointIndex(2, 1)) == false);

    QPainterPath ppath(QPointF(10, 10));
    ppath.lineTo(20, 10);
    ppath.moveTo(20, 20);
    ppath.lineTo(15, 25);
    ppath.lineTo(10, 20);
    ppath.moveTo(30, 30);
    ppath.lineTo(40, 30);
    ppath.lineTo(40, 40);
    ppath.cubicTo(40, 45, 30, 45, 30, 40);
    ppath.closeSubpath();

    QVERIFY(ppath == path.outline());
}

void TestPathShape::join()
{
    KoPathShape path;
    path.moveTo(QPointF(10, 10));
    path.lineTo(QPointF(20, 10));
    path.moveTo(QPointF(20, 20));
    path.lineTo(QPointF(15, 25));
    path.lineTo(QPointF(10, 20));
    path.moveTo(QPointF(30, 30));
    path.lineTo(QPointF(40, 30));
    path.lineTo(QPointF(40, 40));
    path.curveTo(QPointF(40, 45), QPointF(30, 45), QPointF(30, 40));
    path.close();
    path.moveTo(QPointF(50, 50));
    path.lineTo(QPointF(60, 60));

    QVERIFY(path.join(0) == true);
    // try to join to a closed subpath
    QVERIFY(path.join(0) == false);
    // try to join from a closed subpath
    QVERIFY(path.join(1) == false);
    // try to join last subpath
    QVERIFY(path.join(2) == false);

    QPainterPath ppath(QPointF(10, 10));
    ppath.lineTo(20, 10);
    ppath.lineTo(20, 20);
    ppath.lineTo(15, 25);
    ppath.lineTo(10, 20);
    ppath.moveTo(30, 30);
    ppath.lineTo(40, 30);
    ppath.lineTo(40, 40);
    ppath.cubicTo(40, 45, 30, 45, 30, 40);
    ppath.closeSubpath();
    ppath.moveTo(50, 50);
    ppath.lineTo(60, 60);

    QVERIFY(ppath == path.outline());
}

void TestPathShape::moveSubpath()
{
    KoPathShape path;
    path.moveTo(QPointF(10, 10));
    path.lineTo(QPointF(20, 10));
    path.moveTo(QPointF(20, 20));
    path.lineTo(QPointF(15, 25));
    path.lineTo(QPointF(10, 20));
    path.moveTo(QPointF(30, 30));
    path.lineTo(QPointF(40, 30));
    path.lineTo(QPointF(40, 40));
    path.curveTo(QPointF(40, 45), QPointF(30, 45), QPointF(30, 40));
    path.close();

    QVERIFY(path.moveSubpath(0, 1) == true);
    QVERIFY(path.moveSubpath(1, 0) == true);
    QVERIFY(path.moveSubpath(2, 1) == true);
    QVERIFY(path.moveSubpath(0, 2) == true);
    QVERIFY(path.moveSubpath(3, 1) == false);
    QVERIFY(path.moveSubpath(1, 3) == false);

    QPainterPath ppath(QPointF(30, 30));
    ppath.lineTo(40, 30);
    ppath.lineTo(40, 40);
    ppath.cubicTo(40, 45, 30, 45, 30, 40);
    ppath.closeSubpath();
    ppath.moveTo(20, 20);
    ppath.lineTo(15, 25);
    ppath.lineTo(10, 20);
    ppath.moveTo(10, 10);
    ppath.lineTo(20, 10);

    QVERIFY(ppath == path.outline());
}

void TestPathShape::openSubpath()
{
    KoPathShape path;
    path.moveTo(QPointF(20, 20));
    KoPathPoint *point1 = path.lineTo(QPointF(15, 25));
    path.lineTo(QPointF(10, 20));
    path.close();
    KoPathPoint *point2 = path.moveTo(QPointF(30, 30));
    path.lineTo(QPointF(40, 30));
    path.lineTo(QPointF(40, 40));
    path.curveTo(QPointF(40, 45), QPointF(30, 45), QPointF(30, 40));
    path.close();
    path.moveTo(QPointF(50, 50));
    path.lineTo(QPointF(60, 50));
    path.lineTo(QPointF(60, 60));
    KoPathPoint *point3 = path.curveTo(QPointF(60, 65), QPointF(50, 65), QPointF(50, 60));
    path.close();
    KoPathPoint *point4 = path.moveTo(QPointF(100, 100));
    point4->setControlPoint2(QPointF(120, 120));
    path.lineTo(QPointF(140, 140));
    KoPathPoint *point5 = path.lineTo(QPointF(140, 100));
    path.close();

    // open at middle point in subpath
    QVERIFY(path.openSubpath(path.pathPointIndex(point1)) == KoPathPointIndex(0, 2));
    QVERIFY(path.pointByIndex(KoPathPointIndex(0,0))->properties() & KoPathPoint::StartSubpath);
    QVERIFY(path.pointByIndex(KoPathPointIndex(0,2))->properties() & KoPathPoint::StopSubpath);
    // open at first point in subpath
    QVERIFY(path.openSubpath(path.pathPointIndex(point2)) == KoPathPointIndex(1, 0));
    QVERIFY(path.pointByIndex(KoPathPointIndex(1,0))->properties() & KoPathPoint::StartSubpath);
    QVERIFY(path.pointByIndex(KoPathPointIndex(1,3))->properties() & KoPathPoint::StopSubpath);
    // open at last point in subpath
    QVERIFY(path.openSubpath(path.pathPointIndex(point3)) == KoPathPointIndex(2, 1));
    QVERIFY(path.pointByIndex(KoPathPointIndex(2,0))->properties() & KoPathPoint::StartSubpath);
    QVERIFY(path.pointByIndex(KoPathPointIndex(2,3))->properties() & KoPathPoint::StopSubpath);
    // try to open open subpath
    QVERIFY(path.openSubpath(path.pathPointIndex(point3)) == KoPathPointIndex(-1, -1));
    // open if the first path is a curve
    QVERIFY(path.openSubpath(path.pathPointIndex(point5)) == KoPathPointIndex(3, 1));
    QVERIFY(path.pointByIndex(KoPathPointIndex(3,0))->properties() & KoPathPoint::StartSubpath);
    QVERIFY(path.pointByIndex(KoPathPointIndex(3,2))->properties() & KoPathPoint::StopSubpath);
    // try to open none existing subpath
    QVERIFY(path.openSubpath(KoPathPointIndex(4, 1)) == KoPathPointIndex(-1, -1));

    QPainterPath ppath(QPointF(15, 25));
    ppath.lineTo(10, 20);
    ppath.lineTo(20, 20);
    ppath.moveTo(30, 30);
    ppath.lineTo(40, 30);
    ppath.lineTo(40, 40);
    ppath.cubicTo(40, 45, 30, 45, 30, 40);
    ppath.moveTo(50, 60);
    ppath.lineTo(50, 50);
    ppath.lineTo(60, 50);
    ppath.lineTo(60, 60);
    ppath.moveTo(140, 100);
    ppath.lineTo(100, 100);
    ppath.quadTo(120, 120, 140, 140);

    QVERIFY(ppath == path.outline());
}

void TestPathShape::closeSubpath()
{
    KoPathShape path;
    path.moveTo(QPointF(20, 20));
    KoPathPoint *point1 = path.lineTo(QPointF(15, 25));
    path.lineTo(QPointF(10, 20));
    KoPathPoint *point2 = path.moveTo(QPointF(30, 30));
    path.lineTo(QPointF(40, 30));
    path.lineTo(QPointF(40, 40));
    path.curveTo(QPointF(40, 45), QPointF(30, 45), QPointF(30, 40));
    path.moveTo(QPointF(50, 50));
    path.lineTo(QPointF(60, 50));
    path.lineTo(QPointF(60, 60));
    KoPathPoint *point3 = path.curveTo(QPointF(60, 65), QPointF(50, 65), QPointF(50, 60));

    // open at middle point in subpath
    QVERIFY(path.closeSubpath(path.pathPointIndex(point1)) == KoPathPointIndex(0, 2));
    // open at first point in subpath
    QVERIFY(path.closeSubpath(path.pathPointIndex(point2)) == KoPathPointIndex(1, 0));
    // open at last point in subpath
    QVERIFY(path.closeSubpath(path.pathPointIndex(point3)) == KoPathPointIndex(2, 1));
    // try to close a closed subpath
    QVERIFY(path.closeSubpath(path.pathPointIndex(point3)) == KoPathPointIndex(-1, -1));
    // try to close a none existing subpath
    QVERIFY(path.closeSubpath(KoPathPointIndex(3, 1)) == KoPathPointIndex(-1, -1));
    // try to close at a none existing position in a subpath
    QVERIFY(path.closeSubpath(KoPathPointIndex(2, 4)) == KoPathPointIndex(-1, -1));

    QPainterPath ppath(QPointF(15, 25));
    ppath.lineTo(10, 20);
    ppath.lineTo(20, 20);
    ppath.closeSubpath();
    ppath.moveTo(30, 30);
    ppath.lineTo(40, 30);
    ppath.lineTo(40, 40);
    ppath.cubicTo(40, 45, 30, 45, 30, 40);
    ppath.closeSubpath();
    ppath.moveTo(50, 60);
    ppath.lineTo(50, 50);
    ppath.lineTo(60, 50);
    ppath.lineTo(60, 60);
    ppath.cubicTo(60, 65, 50, 65, 50, 60);
    ppath.closeSubpath();

    QVERIFY(ppath == path.outline());
}

void TestPathShape::openCloseSubpath()
{
    KoPathShape path;
    path.moveTo(QPointF(20, 20));
    KoPathPoint *point1 = path.lineTo(QPointF(15, 25));
    path.lineTo(QPointF(10, 20));
    path.close();
    KoPathPoint *point2 = path.moveTo(QPointF(30, 30));
    path.lineTo(QPointF(40, 30));
    path.lineTo(QPointF(40, 40));
    path.curveTo(QPointF(40, 45), QPointF(30, 45), QPointF(30, 40));

    KoPathPointIndex p1Index = path.pathPointIndex(point1);
    KoPathPointIndex p1OldIndex = path.openSubpath(p1Index);
    QVERIFY(path.closeSubpath(p1OldIndex) == p1Index);

    KoPathPointIndex p2Index = path.pathPointIndex(point2);
    KoPathPointIndex p2OldIndex = path.closeSubpath(p2Index);
    QVERIFY(path.openSubpath(p2OldIndex) == p2Index);

    QPainterPath ppath(QPointF(20, 20));
    ppath.lineTo(15, 25);
    ppath.lineTo(10, 20);
    ppath.closeSubpath();
    ppath.moveTo(30, 30);
    ppath.lineTo(40, 30);
    ppath.lineTo(40, 40);
    ppath.cubicTo(40, 45, 30, 45, 30, 40);

    QVERIFY(ppath == path.outline());
}

void TestPathShape::reverseSubpath()
{
    KoPathShape path;
    path.moveTo(QPointF(10, 10));
    path.lineTo(QPointF(20, 10));
    path.moveTo(QPointF(20, 20));
    path.lineTo(QPointF(15, 25));
    path.lineTo(QPointF(10, 20));
    path.moveTo(QPointF(30, 30));
    path.lineTo(QPointF(40, 30));
    path.lineTo(QPointF(40, 40));
    path.curveTo(QPointF(40, 45), QPointF(30, 45), QPointF(30, 40));
    path.close();

    QVERIFY(path.reverseSubpath(0) == true);
    QVERIFY(path.reverseSubpath(1) == true);
    QVERIFY(path.reverseSubpath(1) == true);
    QVERIFY(path.reverseSubpath(2) == true);
    QVERIFY(path.reverseSubpath(3) == false);

    QPainterPath ppath(QPointF(20, 10));
    ppath.lineTo(10, 10);
    ppath.moveTo(20, 20);
    ppath.lineTo(15, 25);
    ppath.lineTo(10, 20);
    ppath.moveTo(30, 40);
    ppath.cubicTo(30, 45, 40, 45, 40, 40);
    ppath.lineTo(40, 30);
    ppath.lineTo(30, 30);
    ppath.closeSubpath();

    QVERIFY(ppath == path.outline());

    QVERIFY(path.reverseSubpath(2) == true);
    QVERIFY(path.reverseSubpath(2) == true);

    QVERIFY(ppath == path.outline());
}

void TestPathShape::removeSubpath()
{
#if 0
    // enable again when point groups work
    KoPathShape path;
    path.moveTo(QPointF(10, 10));
    path.lineTo(QPointF(20, 10));
    path.lineTo(QPointF(20, 20));
    path.close();
    path.lineTo(QPointF(15, 25));
    path.lineTo(QPointF(10, 20));
    path.moveTo(QPointF(30, 30));
    path.lineTo(QPointF(40, 30));
    path.lineTo(QPointF(40, 40));
    path.curveTo(QPointF(40, 45), QPointF(30, 45), QPointF(30, 40));
    path.close();

    QVERIFY(path.removeSubpath(0) != 0);
    QVERIFY(path.removeSubpath(1) != 0);
    QVERIFY(path.removeSubpath(1) == 0);

    QPainterPath ppath(QPointF(10, 20));
    ppath.lineTo(15, 25);
    ppath.lineTo(10, 20);

    path.debugPath();

    QVERIFY(ppath == path.outline());
#endif

    KoPathShape path;
    path.moveTo(QPointF(10, 10));
    path.lineTo(QPointF(20, 10));
    path.lineTo(QPointF(20, 20));
    path.close();
    path.moveTo(QPointF(15, 25));
    path.lineTo(QPointF(10, 20));
    path.moveTo(QPointF(30, 30));
    path.lineTo(QPointF(40, 30));
    path.lineTo(QPointF(40, 40));
    path.curveTo(QPointF(40, 45), QPointF(30, 45), QPointF(30, 40));
    path.close();

    QVERIFY(path.removeSubpath(0) != 0);
    QVERIFY(path.removeSubpath(1) != 0);
    QVERIFY(path.removeSubpath(1) == 0);

    QPainterPath ppath(QPointF(15, 25));
    ppath.lineTo(10, 20);

    QVERIFY(ppath == path.outline());
}

void TestPathShape::addSubpath()
{
    KoPathShape path;
    path.moveTo(QPointF(10, 10));
    path.lineTo(QPointF(20, 10));
    path.lineTo(QPointF(20, 20));
    path.close();
    path.moveTo(QPointF(15, 25));
    path.lineTo(QPointF(10, 20));
    path.moveTo(QPointF(30, 30));
    path.lineTo(QPointF(40, 30));
    path.lineTo(QPointF(40, 40));
    path.curveTo(QPointF(40, 45), QPointF(30, 45), QPointF(30, 40));
    path.close();

    KoSubpath * sp1 = path.removeSubpath(0);
    QVERIFY(path.addSubpath(sp1, 0) == true);

    KoSubpath * sp2 = path.removeSubpath(1);
    QVERIFY(path.addSubpath(sp2, 1) == true);

    QVERIFY(path.addSubpath(sp2, 4) == false);

    QPainterPath ppath(QPointF(10, 10));
    ppath.lineTo(20, 10);
    ppath.lineTo(20, 20);
    ppath.closeSubpath();
    ppath.moveTo(15, 25);
    ppath.lineTo(10, 20);
    ppath.moveTo(30, 30);
    ppath.lineTo(40, 30);
    ppath.lineTo(40, 40);
    ppath.cubicTo(40, 45, 30, 45, 30, 40);
    ppath.closeSubpath();

    QVERIFY(ppath == path.outline());
}

void TestPathShape::koPathPointDataLess()
{
    QList<KoPathPointData> v;
    v.push_back(KoPathPointData((KoPathShape*)1, KoPathPointIndex(1, 1)));
    v.push_back(KoPathPointData((KoPathShape*)1, KoPathPointIndex(1, 2)));
    v.push_back(KoPathPointData((KoPathShape*)1, KoPathPointIndex(1, 3)));
    v.push_back(KoPathPointData((KoPathShape*)1, KoPathPointIndex(1, 6)));
    v.push_back(KoPathPointData((KoPathShape*)2, KoPathPointIndex(2, 1)));
    v.push_back(KoPathPointData((KoPathShape*)2, KoPathPointIndex(2, 3)));
    v.push_back(KoPathPointData((KoPathShape*)2, KoPathPointIndex(3, 3)));
    v.push_back(KoPathPointData((KoPathShape*)3, KoPathPointIndex(1, 1)));
    v.push_back(KoPathPointData((KoPathShape*)3, KoPathPointIndex(1, 2)));

    QList<KoPathPointData> l;
    l.push_back(v[8]);
    l.push_back(v[0]);
    l.push_back(v[1]);
    l.push_back(v[7]);
    l.push_back(v[6]);
    l.push_back(v[2]);
    l.push_back(v[5]);
    l.push_back(v[3]);
    l.push_back(v[4]);

    std::sort(l.begin(), l.end());
    for (int i = 0; i < v.size(); ++i) {
        KoPathPointData ld = l.at(i);
        KoPathPointData vd = v[i];
        QVERIFY(ld.pathShape == vd.pathShape);
        QVERIFY(ld.pointIndex.first == vd.pointIndex.first);
        QVERIFY(ld.pointIndex.second == vd.pointIndex.second);
    }
}

void TestPathShape::closeMerge()
{
    KoPathShape path;
    KoPathPoint *p1 = path.moveTo(QPointF(0, 0));
    KoPathPoint *p2 = path.curveTo(QPointF(50, 0), QPointF(100, 50), QPointF(100, 100));
    KoPathPoint *p3 = path.curveTo(QPointF(50, 100), QPointF(0, 50), QPointF(0, 0));
    QVERIFY(p1->properties() & KoPathPoint::StartSubpath);
    QVERIFY((p1->properties() & KoPathPoint::CloseSubpath) == 0);
    QVERIFY(p1->activeControlPoint1() == false);
    QVERIFY(p1->activeControlPoint2());
    QVERIFY(p2->activeControlPoint1());
    QVERIFY(p2->activeControlPoint2());
    QVERIFY((p3->properties() & KoPathPoint::CloseSubpath) == 0);
    QVERIFY(p3->activeControlPoint1());
    QCOMPARE(path.pointCount(), 3);

    path.closeMerge();
    QCOMPARE(path.pointCount(), 2);
    QVERIFY(p1->properties() & KoPathPoint::CloseSubpath);
    QVERIFY(p1->activeControlPoint1());
    QVERIFY(p2->properties() & KoPathPoint::CloseSubpath);
    QVERIFY(p2->activeControlPoint2());

    QPainterPath ppath(QPointF(0, 0));
    ppath.cubicTo(50, 0, 100, 50, 100, 100);
    ppath.cubicTo(50, 100, 0, 50, 0, 0);

    QVERIFY(path.outline() == ppath);
}

QTEST_MAIN(TestPathShape)
