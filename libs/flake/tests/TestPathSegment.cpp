/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008-2009 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "TestPathSegment.h"
#include <KoPathSegment.h>
#include <KoPathPoint.h>
#include <QPainterPath>
#include <simpletest.h>

void TestPathSegment::segmentAssign()
{
    KoPathSegment s1(QPointF(0, 0), QPointF(100, 100));
    KoPathSegment s1Copy = s1;
    QVERIFY(s1 == s1Copy);

    KoPathSegment s2(QPointF(0, 0), QPointF(100, 100), QPointF(200, 0));
    KoPathSegment s2Copy = s2;
    QVERIFY(s2 == s2Copy);

    KoPathSegment s3(QPointF(0, 0), QPointF(100, 100), QPointF(200, 100), QPointF(300, 0));
    KoPathSegment s3Copy = s3;
    QVERIFY(s3 == s3Copy);
}

void TestPathSegment::segmentCopy()
{
    KoPathSegment s1(QPointF(0, 0), QPointF(100, 100));
    KoPathSegment s1Copy(s1);
    QVERIFY(s1 == s1Copy);

    KoPathSegment s2(QPointF(0, 0), QPointF(100, 100), QPointF(200, 0));
    KoPathSegment s2Copy(s2);
    QVERIFY(s2 == s2Copy);

    KoPathSegment s3(QPointF(0, 0), QPointF(100, 100), QPointF(200, 100), QPointF(300, 0));
    KoPathSegment s3Copy(s3);
    QVERIFY(s3 == s3Copy);
}

void TestPathSegment::segmentDegree()
{
    KoPathSegment s0(0, 0);
    QCOMPARE(s0.degree(), -1);

    KoPathSegment s1(QPointF(0, 0), QPointF(100, 100));
    QCOMPARE(s1.degree(), 1);

    KoPathSegment s2(QPointF(0, 0), QPointF(100, 100), QPointF(200, 0));
    QCOMPARE(s2.degree(), 2);

    KoPathSegment s3(QPointF(0, 0), QPointF(100, 100), QPointF(200, 100), QPointF(300, 0));
    QCOMPARE(s3.degree(), 3);
}

void TestPathSegment::segmentConvexHull()
{
    KoPathSegment s1(QPointF(0, 0), QPointF(100, 100));
    QList<QPointF> hull1 = s1.convexHull();
    QCOMPARE(hull1.count(), 2);
    QCOMPARE(hull1[0], QPointF(0, 0));
    QCOMPARE(hull1[1], QPointF(100, 100));

    KoPathSegment s2(QPointF(0, 0), QPointF(100, 100), QPointF(200, 0));
    QList<QPointF> hull2 = s2.convexHull();
    QCOMPARE(hull2.count(), 3);
    QCOMPARE(hull2[0], QPointF(0, 0));
    QCOMPARE(hull2[1], QPointF(100, 100));
    QCOMPARE(hull2[2], QPointF(200, 0));

    KoPathSegment s3(QPointF(0, 0), QPointF(100, 100), QPointF(200, 100), QPointF(300, 0));
    QList<QPointF> hull3 = s3.convexHull();
    QCOMPARE(hull3.count(), 4);
    QCOMPARE(hull3[0], QPointF(0, 0));
    QCOMPARE(hull3[1], QPointF(100, 100));
    QCOMPARE(hull3[2], QPointF(200, 100));
    QCOMPARE(hull3[3], QPointF(300, 0));

    KoPathSegment s4(QPointF(0, 0), QPointF(150, 100), QPointF(150, 50), QPointF(300, 0));
    QList<QPointF> hull4 = s4.convexHull();
    QCOMPARE(hull4.count(), 3);
    QCOMPARE(hull4[0], QPointF(0, 0));
    QCOMPARE(hull4[1], QPointF(150, 100));
    QCOMPARE(hull4[2], QPointF(300, 0));
}

void TestPathSegment::segmentPointAt()
{
    KoPathSegment s1(QPointF(0, 0), QPointF(100, 0));
    QCOMPARE(s1.pointAt(0.0), QPointF(0, 0));
    QCOMPARE(s1.pointAt(0.5), QPointF(50, 0));
    QCOMPARE(s1.pointAt(1.0), QPointF(100, 0));

    KoPathSegment s2(QPointF(0, 0), QPointF(100, 100), QPointF(200, 0));
    QCOMPARE(s2.pointAt(0.0), QPointF(0, 0));
    QCOMPARE(s2.pointAt(0.5), QPointF(100, 50));
    QCOMPARE(s2.pointAt(1.0), QPointF(200, 0));

    KoPathSegment s3(QPointF(0, 0), QPointF(100, 100), QPointF(200, 100), QPointF(300, 0));
    QCOMPARE(s3.pointAt(0.0), QPointF(0, 0));
    QCOMPARE(s3.pointAt(1.0 / 3.0), QPointF(100, 100*2.0 / 3.0));
    QCOMPARE(s3.pointAt(2.0 / 3.0), QPointF(200, 100*2.0 / 3.0));
    QCOMPARE(s3.pointAt(1.0), QPointF(300, 0));
}

void TestPathSegment::segmentSplitAt()
{
    KoPathSegment s1(QPointF(0, 0), QPointF(100, 0));
    QPair<KoPathSegment, KoPathSegment> parts1 = s1.splitAt( 0.5 );
    QCOMPARE(parts1.first.first()->point(), QPointF(0, 0));
    QCOMPARE(parts1.first.second()->point(), QPointF(50, 0));
    QCOMPARE(parts1.first.degree(), 1);
    QCOMPARE(parts1.second.first()->point(), QPointF(50, 0));
    QCOMPARE(parts1.second.second()->point(), QPointF(100, 0));
    QCOMPARE(parts1.second.degree(), 1);

    QPainterPath p1;
    p1.moveTo( QPoint(0, 0) );
    p1.lineTo( QPointF(100, 0) );
    QCOMPARE( parts1.first.second()->point(), p1.pointAtPercent( 0.5 ) );

    KoPathSegment s2(QPointF(0, 0), QPointF(100, 100), QPointF(200, 0));
    QPair<KoPathSegment, KoPathSegment> parts2 = s2.splitAt( 0.5 );
    QCOMPARE(parts2.first.first()->point(), QPointF(0, 0));
    QCOMPARE(parts2.first.second()->point(), QPointF(100, 50));
    QCOMPARE(parts2.first.degree(), 2);
    QCOMPARE(parts2.second.first()->point(), QPointF(100, 50));
    QCOMPARE(parts2.second.second()->point(), QPointF(200, 0));
    QCOMPARE(parts2.second.degree(), 2);

    QPainterPath p2;
    p2.moveTo( QPoint(0, 0) );
    p2.quadTo( QPointF(100, 100), QPointF(200, 0) );
    QCOMPARE( parts2.first.second()->point(), p2.pointAtPercent( 0.5 ) );

    KoPathSegment s3(QPointF(0, 0), QPointF(100, 100), QPointF(200, 100), QPointF(300, 0));
    QPair<KoPathSegment, KoPathSegment> parts3 = s3.splitAt( 0.5 );
    QCOMPARE(parts3.first.first()->point(), QPointF(0, 0));
    QCOMPARE(parts3.first.second()->point(), QPointF(150, 75));
    QCOMPARE(parts3.first.degree(), 3);
    QCOMPARE(parts3.second.first()->point(), QPointF(150, 75));
    QCOMPARE(parts3.second.second()->point(), QPointF(300, 0));
    QCOMPARE(parts3.second.degree(), 3);

    QPainterPath p3;
    p3.moveTo( QPoint(0, 0) );
    p3.cubicTo( QPointF(100, 100), QPointF(200, 100), QPointF(300, 0) );
    QCOMPARE( parts3.first.second()->point(), p3.pointAtPercent( 0.5 ) );
}

void TestPathSegment::segmentIntersections()
{
    // simple line intersections
    {
        KoPathSegment s1(QPointF(0, 0), QPointF(100, 0));
        KoPathSegment s2(QPointF(50, -50), QPointF(50, 50));
        QList<QPointF> isects = s1.intersections(s2);
        QCOMPARE(isects.count(), 1);
    }
    {
        KoPathSegment s1(QPointF(0, 0), QPointF(100, 100));
        KoPathSegment s2(QPointF(25, 100), QPointF(75, 50));
        QList<QPointF> isects = s1.intersections(s2);
        QCOMPARE(isects.count(), 1);
    }
    // curve intersections
    {
        KoPathSegment s1(QPointF(0, 0), QPointF(50, 50), QPointF(100, -50), QPointF(150, 0));
        KoPathSegment s2(QPointF(75, 75), QPointF(125, 25), QPointF(25, -25), QPointF(75, -75));
        QList<QPointF> isects = s1.intersections(s2);
        QCOMPARE(isects.count(), 1);
    }
    {
        KoPathSegment s1(QPointF(0, 0), QPointF(50, 50), QPointF(100, -50), QPointF(150, 0));
        KoPathSegment s2(QPointF(100, 75), QPointF(150, 25), QPointF(50, -25), QPointF(100, -75));
        QList<QPointF> isects = s1.intersections(s2);
        QCOMPARE(isects.count(), 1);
    }
    {
        KoPathSegment s1(QPointF(0, 0), QPointF(25, 50), QPointF(75, 50), QPointF(100, 0));
        KoPathSegment s2(QPointF(0, 30), QPointF(25, -20), QPointF(75, -20), QPointF(100, 30));
        QList<QPointF> isects = s1.intersections(s2);
        QCOMPARE(isects.count(), 2);
    }
}

void TestPathSegment::segmentLength()
{
    {
        // line segment
        KoPathSegment s(QPointF(0, 0), QPointF(100, 0));
        QCOMPARE(s.length(), 100.0);
    }
    {
        // quadric curve segment
        KoPathSegment s1(QPointF(0, 0), QPointF(50, 0), QPointF(100, 0));
        QCOMPARE(s1.length(), 100.0);
        KoPathSegment s2(QPointF(0, 0), QPointF(50, 50), QPointF(100, 0));
        QPainterPath p2;
        p2.moveTo(QPointF(0, 0));
        p2.quadTo(QPointF(50, 50), QPointF(100, 0));
        // verify that difference is less than 0.5 percent of the length
        QVERIFY(s2.length() - p2.length() < 0.005 * s2.length(0.01));
    }
    {
        // cubic curve segment
        KoPathSegment s1(QPointF(0, 0), QPointF(25, 0), QPointF(75, 0), QPointF(100, 0));
        QCOMPARE(s1.length(), 100.0);
        KoPathSegment s2(QPointF(0, 0), QPointF(25, 50), QPointF(75, 50), QPointF(100, 0));
        QPainterPath p2;
        p2.moveTo(QPointF(0, 0));
        p2.cubicTo(QPointF(25, 50), QPointF(75, 50), QPointF(100, 0));
        // verify that difference is less than 0.5 percent of the length
        QVERIFY(s2.length() - p2.length() < 0.005 * s2.length(0.01));
    }
}

void TestPathSegment::segmentFlatness()
{
    // line segments
    {
        KoPathSegment s1(QPointF(0, 0), QPointF(100, 0));
        QVERIFY(s1.isFlat());
        KoPathSegment s2(QPointF(0, 0), QPointF(0, 100));
        QVERIFY(s2.isFlat());
        KoPathSegment s3(QPointF(0, 0), QPointF(100, 100));
        QVERIFY(s3.isFlat());
    }
    // quadratic segments
    {
        KoPathSegment s1(QPointF(0, 0), QPointF(50, 0), QPointF(100, 0));
        QVERIFY(s1.isFlat());
        KoPathSegment s2(QPointF(0, 0), QPointF(0, 50), QPointF(0, 100));
        QVERIFY(s2.isFlat());
        KoPathSegment s3(QPointF(0, 0), QPointF(50, 50), QPointF(100, 100));
        QVERIFY(s3.isFlat());
        KoPathSegment s4(QPointF(0, 0), QPointF(50, 50), QPointF(100, 0));
        QVERIFY(! s4.isFlat());
        KoPathSegment s5(QPointF(0, 0), QPointF(50, -50), QPointF(100, 0));
        QVERIFY(! s5.isFlat());
        KoPathSegment s6(QPointF(0, 0), QPointF(0, 100), QPointF(100, 100));
        QVERIFY(! s6.isFlat());
    }
    // cubic segments
    {
        KoPathSegment s1(QPointF(0, 0), QPointF(25, 0), QPointF(75, 0), QPointF(100, 0));
        QVERIFY(s1.isFlat());
        KoPathSegment s2(QPointF(0, 0), QPointF(0, 25), QPointF(0, 75), QPointF(0, 100));
        QVERIFY(s2.isFlat());
        KoPathSegment s3(QPointF(0, 0), QPointF(25, 25), QPointF(75, 75), QPointF(100, 100));
        QVERIFY(s3.isFlat());
        KoPathSegment s4(QPointF(0, 0), QPointF(25, 50), QPointF(75, 50), QPointF(100, 0));
        QVERIFY(! s4.isFlat());
        KoPathSegment s5(QPointF(0, 0), QPointF(25, -50), QPointF(75, -50), QPointF(100, 0));
        QVERIFY(! s5.isFlat());
        KoPathSegment s6(QPointF(0, 0), QPointF(-25, 75), QPointF(25, 125), QPointF(100, 100));
        QVERIFY(! s6.isFlat());
    }
}

void TestPathSegment::nearestPoint()
{
    // line segments
    {
        KoPathSegment s1(QPointF(0, 0), QPointF(100, 0));
        QCOMPARE( s1.nearestPoint( QPointF(0,0) ),    0.0 );
        QCOMPARE( s1.nearestPoint( QPointF(-20,0) ),  0.0 );
        QCOMPARE( s1.nearestPoint( QPointF(100,0) ),  1.0 );
        QCOMPARE( s1.nearestPoint( QPointF(120,0) ),  1.0 );
        QCOMPARE( s1.nearestPoint( QPointF(50,0) ),   0.5 );
        QCOMPARE( s1.nearestPoint( QPointF(50,-10) ), 0.5 );
        QCOMPARE( s1.nearestPoint( QPointF(50,10) ),  0.5 );
        QCOMPARE( s1.nearestPoint( QPointF(63,50) ),  0.63 );
        QCOMPARE( s1.nearestPoint( QPointF(63,50) ),  0.63 );
        QCOMPARE( s1.nearestPoint( QPointF(63,50) ),  0.63 );
        QCOMPARE( s1.nearestPoint( s1.pointAt( 0.25 ) ),  0.25 );
        QCOMPARE( s1.nearestPoint( s1.pointAt( 0.75 ) ),  0.75 );
    }
    // quadratic segments
    {
        KoPathSegment s1(QPointF(0, 0), QPointF(50, 0), QPointF(100, 0));
        QCOMPARE( s1.nearestPoint( QPointF(0,0) ),    0.0 );
        QCOMPARE( s1.nearestPoint( QPointF(-20,0) ),  0.0 );
        QCOMPARE( s1.nearestPoint( QPointF(100,0) ),  1.0 );
        QCOMPARE( s1.nearestPoint( QPointF(120,0) ),  1.0 );
        QCOMPARE( s1.nearestPoint( QPointF(50,0) ),   0.5 );
        QCOMPARE( s1.nearestPoint( QPointF(50,-10) ), 0.5 );
        QCOMPARE( s1.nearestPoint( QPointF(50,10) ),  0.5 );
        KoPathSegment s2(QPointF(0, 0), QPointF(50, 50), QPointF(100, 0));
        QCOMPARE( s2.nearestPoint( QPointF(0,0) ),    0.0 );
        QCOMPARE( s2.nearestPoint( QPointF(-20,0) ),  0.0 );
        QCOMPARE( s2.nearestPoint( QPointF(100,0) ),  1.0 );
        QCOMPARE( s2.nearestPoint( QPointF(120,0) ),  1.0 );
        QCOMPARE( s2.nearestPoint( QPointF(50,50) ),   0.5 );

        QCOMPARE( s2.nearestPoint( s2.pointAt( 0.0 ) ), 0.0 );
        QCOMPARE( s2.nearestPoint( s2.pointAt( 0.25 ) ), 0.25 );
        QCOMPARE( s2.nearestPoint( s2.pointAt( 0.5 ) ), 0.5 );
        QCOMPARE( s2.nearestPoint( s2.pointAt( 0.75 ) ), 0.75 );
        QCOMPARE( s2.nearestPoint( s2.pointAt( 1.0 ) ), 1.0 );
    }
    // cubic segments
    {
        // a flat cubic bezier
        KoPathSegment s1(QPointF(0, 0), QPointF(25, 0), QPointF(75, 0), QPointF(100, 0));
        QCOMPARE( s1.nearestPoint( QPointF(0,0) ),    0.0 );
        QCOMPARE( s1.nearestPoint( QPointF(-20,0) ),  0.0 );
        QCOMPARE( s1.nearestPoint( QPointF(100,0) ),  1.0 );
        QCOMPARE( s1.nearestPoint( QPointF(120,0) ),  1.0 );
        QCOMPARE( s1.nearestPoint( QPointF(50,0) ),   0.5 );
        QCOMPARE( s1.nearestPoint( QPointF(50,-10) ), 0.5 );
        QCOMPARE( s1.nearestPoint( QPointF(50,10) ),  0.5 );
        KoPathSegment s2(QPointF(0, 0), QPointF(25, 50), QPointF(75, 50), QPointF(100, 0));
        QCOMPARE( s2.nearestPoint( QPointF(0,0) ),    0.0 );
        QCOMPARE( s2.nearestPoint( QPointF(-20,0) ),  0.0 );
        QCOMPARE( s2.nearestPoint( QPointF(100,0) ),  1.0 );
        QCOMPARE( s2.nearestPoint( QPointF(120,0) ),  1.0 );
        QCOMPARE( s2.nearestPoint( QPointF(50,50) ),   0.5 );

        QCOMPARE( s2.nearestPoint( s2.pointAt( 0.0 ) ), 0.0 );
        QCOMPARE( s2.nearestPoint( s2.pointAt( 0.25 ) ), 0.25 );
        QCOMPARE( s2.nearestPoint( s2.pointAt( 0.5 ) ), 0.5 );
        QCOMPARE( s2.nearestPoint( s2.pointAt( 0.75 ) ), 0.75 );
        QCOMPARE( s2.nearestPoint( s2.pointAt( 1.0 ) ), 1.0 );
    }
}

void TestPathSegment::paramAtLength()
{
    // line segment
    {
        KoPathSegment s1(QPointF(0,0), QPointF(100,0));
        QCOMPARE(s1.paramAtLength(0), 0.0);
        QCOMPARE(s1.paramAtLength(100.0), 1.0);
        QCOMPARE(s1.paramAtLength(50.0), 0.5);
        QCOMPARE(s1.paramAtLength(120.0), 1.0);
    }
    // quadratic segments
    {
        // a flat quadratic bezier
        KoPathSegment s1(QPointF(0, 0), QPointF(50, 0), QPointF(100, 0));
        QCOMPARE(s1.paramAtLength(0), 0.0);
        QCOMPARE(s1.paramAtLength(100.0), 1.0);
        QCOMPARE(s1.paramAtLength(120.0), 1.0);
    }
    // cubic segments
    {
        // a flat cubic bezier
        KoPathSegment s1(QPointF(0, 0), QPointF(25, 0), QPointF(75, 0), QPointF(100, 0));
        QCOMPARE(s1.paramAtLength(0), 0.0);
        QCOMPARE(s1.paramAtLength(100.0), 1.0);
        QCOMPARE(s1.paramAtLength(120.0), 1.0);
    }
}

SIMPLE_TEST_MAIN(TestPathSegment)
