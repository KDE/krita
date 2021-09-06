/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_algebra_2d_test.h"

#include <simpletest.h>

#include "kis_algebra_2d.h"
#include "kis_debug.h"

namespace KisAlgebra2D {

}

void KisAlgebra2DTest::testHalfPlane()
{
    {
        QPointF a(10,10);
        QPointF b(5,5);

        KisAlgebra2D::RightHalfPlane p(a, b);

        QVERIFY(p.value(QPointF(7, 5)) > 0);
        QVERIFY(p.value(QPointF(3, 5)) < 0);
        QVERIFY(p.value(QPointF(3, 3)) == 0);
    }

    {
        QPointF a(10,10);
        QPointF b(15,10);

        KisAlgebra2D::RightHalfPlane p(a, b);
        QCOMPARE(p.value(QPointF(3, 5)), -5.0);
        QCOMPARE(p.value(QPointF(500, 15)), 5.0);
        QCOMPARE(p.value(QPointF(1000, 10)), 0.0);

        QCOMPARE(p.valueSq(QPointF(3, 5)), -25.0);
        QCOMPARE(p.valueSq(QPointF(500, 15)), 25.0);
        QCOMPARE(p.valueSq(QPointF(1000, 10)), 0.0);

        QCOMPARE(p.pos(QPointF(3, 5)), -1);
        QCOMPARE(p.pos(QPointF(500, 15)), 1);
        QCOMPARE(p.pos(QPointF(1000, 10)), 0);
    }
}

void KisAlgebra2DTest::testOuterCircle()
{
    QPointF a(10,10);
    KisAlgebra2D::OuterCircle p(a, 5);

    QVERIFY(p.value(QPointF(3, 5)) > 0);
    QVERIFY(p.value(QPointF(7, 7)) < 0);
    QVERIFY(p.value(QPointF(10, 5)) == 0);

    QCOMPARE(p.value(QPointF(10, 12)), -3.0);
    QCOMPARE(p.value(QPointF(10, 15)), 0.0);
    QCOMPARE(p.value(QPointF(10, 17)), 2.0);
}

void KisAlgebra2DTest::testQuadraticEquation()
{
    int result = 0;
    qreal x1 = 0;
    qreal x2 = 0;

    result = KisAlgebra2D::quadraticEquation(3, -11, 6, &x1, &x2);

    QCOMPARE(result, 2);
    QCOMPARE(x2, 2.0 / 3.0);
    QCOMPARE(x1, 3.0);

    result = KisAlgebra2D::quadraticEquation(9, -12, 4, &x1, &x2);

    QCOMPARE(result, 1);
    QCOMPARE(x1, 2.0 / 3.0);

    result = KisAlgebra2D::quadraticEquation(9, -1, 4, &x1, &x2);
    QCOMPARE(result, 0);
}

void KisAlgebra2DTest::testIntersections()
{
    QVector<QPointF> points;

    points = KisAlgebra2D::intersectTwoCircles(QPointF(10,10), 5.0,
                                               QPointF(20,10), 5.0);

    QCOMPARE(points.size(), 1);
    QCOMPARE(points[0], QPointF(15, 10));

    points = KisAlgebra2D::intersectTwoCircles(QPointF(10,10), 5.0,
                                               QPointF(18,10), 5.0);

    QCOMPARE(points.size(), 2);
    QCOMPARE(points[0], QPointF(14, 13));
    QCOMPARE(points[1], QPointF(14, 7));

    points = KisAlgebra2D::intersectTwoCircles(QPointF(10,10), 5.0,
                                               QPointF(10,20), 5.0);

    QCOMPARE(points.size(), 1);
    QCOMPARE(points[0], QPointF(10, 15));

    points = KisAlgebra2D::intersectTwoCircles(QPointF(10,10), 5.0,
                                               QPointF(10,18), 5.0);

    QCOMPARE(points.size(), 2);
    QCOMPARE(points[0], QPointF(7, 14));
    QCOMPARE(points[1], QPointF(13, 14));

    points = KisAlgebra2D::intersectTwoCircles(QPointF(10,10), 5.0,
                                               QPointF(17,17), 5.0);

    QCOMPARE(points.size(), 2);
    QCOMPARE(points[0], QPointF(13, 14));
    QCOMPARE(points[1], QPointF(14, 13));

    points = KisAlgebra2D::intersectTwoCircles(QPointF(10,10), 5.0,
                                               QPointF(10,100), 5.0);

    QCOMPARE(points.size(), 0);
}

void KisAlgebra2DTest::testWeirdIntersections()
{
    QVector<QPointF> points;

    QPointF c1 = QPointF(5369.14,3537.98);
    QPointF c2 = QPointF(5370.24,3536.71);
    qreal r1 = 8.5;
    qreal r2 = 10;

    points = KisAlgebra2D::intersectTwoCircles(c1, r1, c2, r2);

    QCOMPARE(points.size(), 2);
    //QCOMPARE(points[0], QPointF(15, 10));
}

void KisAlgebra2DTest::testMatrixDecomposition1()
{
    QTransform Sh;
    Sh.shear(0.2, 0);
    QTransform R;
    R.rotate(30);
    const QTransform t0 =
        QTransform::fromScale(0.5, -0.6) *
        R * Sh *
        QTransform::fromTranslate(100, 200);

    KisAlgebra2D::DecomposedMatix matrix(t0);

    QCOMPARE(matrix.isValid(), true);
    QVERIFY(KisAlgebra2D::fuzzyMatrixCompare(matrix.transform(), t0, 1e-4));
}

void KisAlgebra2DTest::testMatrixDecomposition2()
{
    QPolygonF poly;
    poly << QPointF(-0.3,-0.3);
    poly << QPointF(1,0);
    poly << QPointF(0.8,0.8);
    poly << QPointF(0,1);

    QTransform t0;
    bool valid = QTransform::squareToQuad(poly, t0);
    QVERIFY(valid);

    KisAlgebra2D::DecomposedMatix matrix(t0);

    QCOMPARE(matrix.isValid(), true);
    QVERIFY(KisAlgebra2D::fuzzyMatrixCompare(matrix.transform(), t0, 1e-4));
}



void KisAlgebra2DTest::testDivisionWithFloor()
{
    QBENCHMARK {

    for (int a = -1000; a < 1000; a++) {
        for (int b = -1000; b < 1000; b++) {
            if (b == 0) continue;

            int refValue = qFloor(qreal(a) / b);
            int value = KisAlgebra2D::divideFloor(a, b);

            if (refValue != value) {
                qDebug() << ppVar(a) << ppVar(b);

                QCOMPARE(value, refValue);
            }
        }
    }
    }
}

#include <QPainter>
#include <QPainterPath>

void KisAlgebra2DTest::testDrawEllipse()
{
    QImage image(QSize(300,300), QImage::Format_ARGB32);
    image.fill(255);

    QPainter gc(&image);

    QTransform rot;
    rot.rotate(-30);

    QTransform shear;
    shear.shear(0.5, 0.3);

    const QTransform transform =
        rot * QTransform::fromTranslate(10, 30) * shear * QTransform::fromTranslate(150, 150);

    const qreal a = 100;
    const qreal b = 50;

    gc.setTransform(transform);
    gc.setPen(Qt::black);
    gc.drawEllipse(QPointF(0,0), a, b);

    gc.setPen(Qt::blue);
    gc.drawEllipse(QPointF(a, 0), 3, 3);

    gc.setPen(Qt::red);
    gc.drawEllipse(QPointF(0, b), 3, 3);

    QPointF newAxes;
    QTransform newTransform;

    std::tie(newAxes, newTransform) = KisAlgebra2D::transformEllipse(QPointF(a, b), transform);

    gc.setOpacity(50);
    gc.resetTransform();
    gc.setTransform(newTransform);
    gc.setPen(QPen(Qt::blue, 2));
    gc.drawEllipse(QPointF(0,0), newAxes.x(), newAxes.y());

    gc.setPen(QPen(Qt::green, 2));
    gc.drawEllipse(QPointF(newAxes.x(), 0), 5, 5);

    gc.setPen(Qt::yellow);
    gc.drawEllipse(QPointF(0, newAxes.y()), 5, 5);

    image.save("ellipse_result.png");
}

void KisAlgebra2DTest::testNullRectProcessing()
{
    QPainterPath line;
    line.moveTo(10,10);
    line.lineTo(110,10);

    const QRectF lineRect(line.boundingRect());
    qDebug() << ppVar(lineRect) << ppVar(lineRect.isValid()) << ppVar(lineRect.isNull());



    // test relative operations
    const QPointF relPoint = KisAlgebra2D::absoluteToRelative(QPointF(30, 10), lineRect);
    QCOMPARE(relPoint, QPointF(0.2, 0.0));

    const QPointF absPoint = KisAlgebra2D::relativeToAbsolute(relPoint, lineRect);
    QCOMPARE(absPoint, QPointF(30.0, 10.0));

    const QTransform relTransform = KisAlgebra2D::mapToRect(lineRect);
    QCOMPARE(relTransform.map(relPoint), absPoint);

    // test relative isotropic operations
    QCOMPARE(KisAlgebra2D::absoluteToRelative(
                 KisAlgebra2D::relativeToAbsolute(0.2, lineRect),
                 lineRect),
             0.2);

    /// test transformations

    // translate
    QCOMPARE(QTransform::fromTranslate(10, 20).mapRect(lineRect), QRect(20, 30, 100, 0));

    // scale
    QCOMPARE(QTransform::fromScale(2.0, 2.0).mapRect(lineRect), QRect(20, 20, 200, 0));

    // rotate
    QTransform rot;
    rot.rotate(90);
    QCOMPARE(rot.mapRect(lineRect), QRectF(-10, 10, 0, 100));

    // shear-x
    QTransform shearX;
    shearX.shear(2.0, 0.0);
    QCOMPARE(shearX.mapRect(lineRect), QRectF(30, 10, 100, 0));

    // shear-y
    QTransform shearY;
    shearY.shear(0.0, 2.0);
    QCOMPARE(shearY.mapRect(lineRect), QRectF(10, 30, 100, 200));

    /// binary operations

    QCOMPARE(QRectF() | lineRect, lineRect);
    QCOMPARE(QRectF(10, 10, 40, 40) | lineRect, QRectF(10, 10, 100, 40));

    QCOMPARE(QRectF(20, 20, 40, 40) & lineRect, QRectF());

    QEXPECT_FAIL("", "Qt's konjunstion operator doesn't work with line-rects", Continue);
    QCOMPARE(QRectF(10, 10, 40, 40) & lineRect, QRectF(10, 10, 40, 0));

    /// QPolygon's bounding rect

    QCOMPARE(QPolygonF(lineRect).boundingRect(), lineRect);
}

void KisAlgebra2DTest::testLineIntersections()
{
    using KisAlgebra2D::intersectLines;
    double epsilon = 1e-06;

    {
        boost::optional<QPointF> p =
                intersectLines(QLineF(QPointF(50,50), QPointF(100,50)),
                               QLineF(QPointF(75,0), QPointF(75,1)));
        QVERIFY(p);
        QCOMPARE(*p, QPointF(75, 50));
    }

    {
        boost::optional<QPointF> p =
                intersectLines(QLineF(QPointF(50,50), QPointF(100,50)),
                               QLineF(QPointF(75,0), QPointF(76,1)));
        QVERIFY(!p);
    }

    {
        boost::optional<QPointF> p =
                intersectLines(QLineF(QPointF(50,50), QPointF(100,50)),
                               QLineF(QPointF(50,51), QPointF(100,51)));
        QVERIFY(!p);
    }

    {
        boost::optional<QPointF> p =
                intersectLines(QLineF(QPointF(51,50), QPointF(51,100)),
                               QLineF(QPointF(50,50), QPointF(50,100)));
        QVERIFY(!p);
    }

    {
        boost::optional<QPointF> p =
                intersectLines(QLineF(QPointF(50,50), QPointF(51,51)),
                               QLineF(QPointF(51,50), QPointF(52,51)));
        QVERIFY(!p);
    }

    {
        // first bounded, then unbounded
        boost::optional<QPointF> p =
                intersectLines(QLineF(QPointF(4,5),QPointF(-1,5)),
                               QLineF(QPointF(-1,0),QPointF(-1,3)));
        QVERIFY(p);
        qCritical() << "What I've got is" << *p << qAbs(p.value().x() + 1) << qAbs(p.value().y() - 5);
        QVERIFY(qAbs(p.value().x() + 1) < epsilon);
        QVERIFY(qAbs(p.value().y() - 5) < epsilon);
    }
}

void testLineRectIntersection(QRect rect, QLineF line, QLineF expected, bool extendFirst, bool extendSecond, bool intersectsExpected)
{
    QLineF result = line;
    bool intersects = KisAlgebra2D::intersectLineRect(result, rect, extendFirst, extendSecond);

    float epsilon = 1e-4; // good enough for most usecases // I don't use qFuzzyCompare because it has too small epsilon

    auto fuzzyCompare = [epsilon] (float a, float b) -> bool {
            return (qAbs(a - b) < epsilon); };
    auto fuzzyComparePoint = [&] (QPointF a, QPointF b) -> bool {
            return fuzzyCompare(a.x(), b.x()) && fuzzyCompare(a.y(), b.y()); };
    auto fuzzyCompareLine = [&] (QLineF a, QLineF b) -> bool {
            return fuzzyComparePoint(a.p1(), b.p1()) && fuzzyComparePoint(a.p2(), b.p2()); };

    bool success = fuzzyCompareLine(result, expected) && intersectsExpected == intersects;

    if (!success) {
        qCritical() << "FAILURE";
        qCritical() << "intersects: " << intersects;
        qCritical() << "intersects expected: " << intersectsExpected;
        qCritical() << "Extend: " << extendFirst << extendSecond;
        qCritical();

        qCritical() << "original: " << line;
        qCritical() << "rectangle was: " << rect;
        qCritical() << "expected: " << expected;
        qCritical() << "result: " << result;
    }

    QVERIFY(success);

}


void KisAlgebra2DTest::testLineRectIntersectionsManual()
{
    int x1 = 100, x2 = 400; // for x values
    int y1 = 130, y2 = 430; // for y values


    QRect rect1(x1, y1, x2 - x1, y2 - y1);
    QLineF line1original(x1, y1, x2, y2);
    QLineF line1expected = line1original;

    testLineRectIntersection(rect1, line1original, line1expected, false, false, true);


    QRect rect2(x1, y1, x2, y2);
    QLineF line2original(x1, y1, x2, y2);
    QLineF line2expected = line2original;

    testLineRectIntersection(rect2, line2original, line2expected, false, false, true);

    QRect rect3 = QRect(0,0, 2616, 1748);
    QLineF line3original = QLineF(QPointF(1110.74,398.93), QPointF(0,394.068));
    QLineF line3expected = line3original;

    testLineRectIntersection(rect3, line3original, line3expected, false, false, true);

    QRect rect4 = rect1;
    QLineF line4 = QLineF(x2, y1, x1, y2);
    testLineRectIntersection(rect4, line4, line4, false, false, true);


    QRect rect5 = rect1;
    QLineF line5 = QLineF(x2, y2, x1, y1);
    testLineRectIntersection(rect5, line5, line5, false, false, true);

    QRect rect6 = rect1;
    QLineF line6 = QLineF(x2, y1, x2, y2);
    testLineRectIntersection(rect6, line6, line6, false, false, true);

    QRect rect7 = rect3;
    QLineF line7 = QLineF(QPointF(338.69,421.014),QPointF(983.456,421.014));
    testLineRectIntersection(rect7, line7, line7, false, false, true);

    QRect rect8 = rect3;
    QLineF line8 = QLineF(QPointF(-3545.25,618.966),QPointF(102.104,618.966));
    QLineF line8expected = QLineF(QPointF(0,618.966),QPointF(102.104,618.966));
    testLineRectIntersection(rect8, line8, line8expected, false, false, true);


    // vertical
    //   inside the line
    testLineRectIntersection(QRect(0, 0, 20, 20), QLineF(0, 5, 0, 15), QLineF(0, 5, 0, 15), false, false, true);
    //   exactly the line
    testLineRectIntersection(QRect(0, 0, 20, 20), QLineF(0, 0, 0, 20), QLineF(0, 0, 0, 20), false, false, true);
    //   extending the line
    testLineRectIntersection(QRect(0, 0, 20, 20), QLineF(0, -5, 0, 25), QLineF(0, 0, 0, 20), false, false, true);

    // diagonal
    //   exactly
    testLineRectIntersection(QRect(0, 0, 20, 20), QLineF(0, 0, 20, 20), QLineF(0, 0, 20, 20), false, false, true);
    //   inside
    testLineRectIntersection(QRect(0, 0, 20, 20), QLineF(5, 5, 15, 15), QLineF(5, 5, 15, 15), false, false, true);
    //   extending
    testLineRectIntersection(QRect(0, 0, 20, 20), QLineF(-5, -5, 30, 30), QLineF(0, 0, 20, 20), false, false, true);


    // start: (-1, -2)
    // end: (4, 5);
    // width, height: 5, 7
    QRect testRect = QRect(-1, -2, 5, 7);

    // first item: line, second item: result after cropping
    QList<QLineF> linesListIntersecting = QList<QLineF>({
                                                            // horizontal
                                                            QLineF(-1, -2, 4, -2), QLineF(-1, -2, 4, -2), // on the edge, exact
                                                            QLineF(-1, 0, 4, 0), QLineF(-1, 0, 4, 0), // below the edge, exact
                                                            QLineF(-10, -2, 10, -2), QLineF(-1, -2, 4, -2), // on the edge, longer
                                                            QLineF(-10, 0, 10, 0), QLineF(-1, 0, 4, 0), // below the edge, longer
                                                            QLineF(0, -2, 3, -2), QLineF(0, -2, 3, -2), // on the edge, shorter
                                                            QLineF(0, 0, 3, -2), QLineF(0, 0, 3, -2), // below the edge, shorter

                                                            // vertical
                                                            QLineF(-1, -2, -1, 5), QLineF(-1, -2, -1, 5), // on the edge, exact
                                                            QLineF(0, -2, 0, 5), QLineF(0, -2, 0, 5), // below the edge, exact
                                                            QLineF(-1, -10, -1, 10), QLineF(-1, -2, -1, 5), // on the edge, longer
                                                            QLineF(0, -10, 0, 10), QLineF(0, -2, 0, 5), // below the edge, longer
                                                            QLineF(-1, 0, -1, 3), QLineF(-1, 0, -1, 3), // on the edge, shorter
                                                            QLineF(0, 0, 0, 3), QLineF(0, 0, 0, 3), // below the edge, shorter


                                                            // skewed
                                                            QLineF(-6, 0, 9, 3), QLineF(-1, 1, 4, 2), // very horizontal-like
                                                            QLineF(-1, 1, 4, 2), QLineF(-1, 1, 4, 2), // very horizontal-like, exact
                                                            QLineF(-6, 0, 4, 2), QLineF(-1, 1, 4, 2), // very horizontal-like, half-exact

                                                            QLineF(-6, -9, 9, 12), QLineF(-1, -2, 4, 5), // perfectly diagonal
                                                            QLineF(-1, -2, 4, 5), QLineF(-1, -2, 4, 5), // perfectly diagonal, exact
                                                            QLineF(-1, -2, 9, 12), QLineF(-1, -2, 4, 5), // perfectly diagonal, half-exact

                                                            QLineF(9, 12, -6, -9), QLineF(4, 5, -1, -2), // perfectly diagonal, p1 and p2 reversed

                                                            QLineF(1, 3, 3, 4), QLineF(1, 3, 3, 4), // skewed, fully inside
                                                            QLineF(2, 2, 6, 4), QLineF(2, 2, 4, 3), // skewed, partially inside


                                                        });

    for (int i = 0; i < linesListIntersecting.size()/2; i++)
    {
        testLineRectIntersection(testRect, linesListIntersecting[2*i], linesListIntersecting[2*i + 1], false, false, true);
    }

    // now with extend = true

    // first item: line, second item: result after cropping
    QList<QLineF> linesListIntersectingExtend = QList<QLineF>({
                                                            // horizontal
                                                            QLineF(-1, -2, 4, -2), QLineF(-1, -2, 4, -2), // on the edge, exact
                                                            QLineF(-1, 0, 4, 0), QLineF(-1, 0, 4, 0), // below the edge, exact
                                                            QLineF(-10, -2, 10, -2), QLineF(-1, -2, 4, -2), // on the edge, longer
                                                            QLineF(-10, 0, 10, 0), QLineF(-1, 0, 4, 0), // below the edge, longer
                                                            QLineF(0, -2, 3, -2), QLineF(-1, -2, 4, -2), // on the edge, shorter
                                                            QLineF(0, 0, 3, 0), QLineF(-1, 0, 4, 0), // below the edge, shorter

                                                            // vertical
                                                            QLineF(-1, -2, -1, 5), QLineF(-1, -2, -1, 5), // on the edge, exact
                                                            QLineF(0, -2, 0, 5), QLineF(0, -2, 0, 5), // below the edge, exact
                                                            QLineF(-1, -10, -1, 10), QLineF(-1, -2, -1, 5), // on the edge, longer
                                                            QLineF(0, -10, 0, 10), QLineF(0, -2, 0, 5), // below the edge, longer
                                                            QLineF(-1, 0, -1, 3), QLineF(-1, -2, -1, 5), // on the edge, shorter
                                                            QLineF(0, 0, 0, 3), QLineF(0, -2, 0, 5), // below the edge, shorter


                                                            // skewed
                                                            QLineF(-6, 0, 9, 3), QLineF(-1, 1, 4, 2), // very horizontal-like
                                                            QLineF(-1, 1, 4, 2), QLineF(-1, 1, 4, 2), // very horizontal-like, exact
                                                            QLineF(-6, 0, 4, 2), QLineF(-1, 1, 4, 2), // very horizontal-like, half-exact

                                                            QLineF(-6, -9, 9, 12), QLineF(-1, -2, 4, 5), // perfectly diagonal
                                                            QLineF(-1, -2, 4, 5), QLineF(-1, -2, 4, 5), // perfectly diagonal, exact
                                                            QLineF(-1, -2, 9, 12), QLineF(-1, -2, 4, 5), // perfectly diagonal, half-exact

                                                            QLineF(9, 12, -6, -9), QLineF(4, 5, -1, -2), // perfectly diagonal, p1 and p2 reversed

                                                            QLineF(1, 3, 3, 4), QLineF(-1, 2, 4, 4.5), // skewed, fully inside
                                                            QLineF(2, 2, 6, 4), QLineF(-1, 0.5, 4, 3), // skewed, partially inside

                                                        });

    for (int i = 0; i < linesListIntersectingExtend.size()/2; i++)
    {
        testLineRectIntersection(testRect, linesListIntersectingExtend[2*i], linesListIntersectingExtend[2*i + 1], true, true, true);
    }

    QList<QLineF> linesListIntersectingExtendOneSide = QList<QLineF>({
                                                            // horizontal

                                                            QLineF(-1, -2, 4, -2), QLineF(-1, -2, 4, -2), // on the edge, exact
                                                            QLineF(-1, 0, 4, 0), QLineF(-1, 0, 4, 0), // below the edge, exact
                                                            QLineF(-10, -2, 10, -2), QLineF(-1, -2, 4, -2), // on the edge, longer
                                                            QLineF(-10, 0, 10, 0), QLineF(-1, 0, 4, 0), // below the edge, longer
                                                            QLineF(0, -2, 3, -2), QLineF(-1, -2, 3, -2), // on the edge, shorter
                                                            QLineF(0, 0, 3, 0), QLineF(-1, 0, 3, 0), // below the edge, shorter

                                                            // vertical
                                                            QLineF(-1, -2, -1, 5), QLineF(-1, -2, -1, 5), // on the edge, exact
                                                            QLineF(0, -2, 0, 5), QLineF(0, -2, 0, 5), // below the edge, exact
                                                            QLineF(-1, -10, -1, 10), QLineF(-1, -2, -1, 5), // on the edge, longer
                                                            QLineF(0, -10, 0, 10), QLineF(0, -2, 0, 5), // below the edge, longer
                                                            QLineF(-1, 0, -1, 3), QLineF(-1, -2, -1, 3), // on the edge, shorter
                                                            QLineF(0, 0, 0, 3), QLineF(0, -2, 0, 3), // below the edge, shorter


                                                            // skewed
                                                            QLineF(-6, 0, 9, 3), QLineF(-1, 1, 4, 2), // very horizontal-like
                                                            QLineF(-1, 1, 4, 2), QLineF(-1, 1, 4, 2), // very horizontal-like, exact
                                                            QLineF(-6, 0, 4, 2), QLineF(-1, 1, 4, 2), // very horizontal-like, half-exact

                                                            QLineF(-6, -9, 9, 12), QLineF(-1, -2, 4, 5), // perfectly diagonal
                                                            QLineF(-1, -2, 4, 5), QLineF(-1, -2, 4, 5), // perfectly diagonal, exact
                                                            QLineF(-1, -2, 9, 12), QLineF(-1, -2, 4, 5), // perfectly diagonal, half-exact

                                                            QLineF(9, 12, -6, -9), QLineF(4, 5, -1, -2), // perfectly diagonal, p1 and p2 reversed

                                                            QLineF(1, 3, 3, 4), QLineF(-1, 2, 3, 4), // skewed, fully inside
                                                            QLineF(2, 2, 6, 4), QLineF(-1, 0.5, 4, 3), // skewed, partially inside


                                                        });

    for (int i = 0; i < linesListIntersectingExtend.size()/2; i++)
    {
        // first from one side
        testLineRectIntersection(testRect, linesListIntersectingExtendOneSide[2*i], linesListIntersectingExtendOneSide[2*i + 1], true, false, true);

        // and then reversed
        QLineF l = QLineF(linesListIntersectingExtendOneSide[2*i].p2(), linesListIntersectingExtendOneSide[2*i].p1());
        QLineF lexp = QLineF(linesListIntersectingExtendOneSide[2*i + 1].p2(), linesListIntersectingExtendOneSide[2*i + 1].p1());
        testLineRectIntersection(testRect, l, lexp, false, true, true);
    }


}

void KisAlgebra2DTest::testLineRectIntersectionsRandom()
{
    // start: (-1, -2)
    // end: (4, 5);
    // width, height: 5, 7
    QRect testRect = QRect(-1, -2, 5, 7);

    int numberOfTests = 100;
    int skipped = 0;
    int negative = 0;
    QRandomGenerator random(1000);

    for (int i = 0; i < numberOfTests; i++) {

        QList<QPointF> points;
        for (int j = 0; j < 2; j++)
        {
            // find two random points on the edges of the rectangle
            int direction = random.generate()%2;
            int boundary = random.generate()%2;
            points.append(QPointF());
            if (direction == 0) {
                points[j].setX(testRect.x() + random.generateDouble()*testRect.width());
                points[j].setY(boundary == 0 ? testRect.y() : (testRect.y() + testRect.height()));
            } else {
                points[j].setY(testRect.y() + random.generateDouble()*testRect.height());
                points[j].setX(boundary == 0 ? testRect.x() : (testRect.x() + testRect.width()));
            }
        }

        bool onEdge = false;
        if (points[0].x() == points[1].x() || points[0].y() == points[1].y()) {
            onEdge = true;
            // 25% chance that it happens
        }

        // line equation: x = x1 + t*(x2 - x1)
        float t1 = random.generateDouble()*3 - 1; // range: [-2, 3] - which means 2x higher chance it would end up outside of the rectangle than inside
        float t2 = random.generateDouble()*3 - 1; // range: [-2, 3] - which means 2x higher chance it would end up outside of the rectangle than inside

        if (t1 > t2) {
            float tmp = t1;
            t1 = t2;
            t2 = tmp;
        }

        QLineF testLine(points[0] + t1*(points[1] - points[0]), points[0] + t2*(points[1] - points[0]));

        QLineF expectedLine = testLine;
        bool expectedResult = true;
        if (onEdge) {
            if (points[0].x() == points[1].x()) /*vertical*/ {
                if (points[0].y() < points[1].y()) {
                    if (points[1].y() < testRect.y() || points[0].y() > testRect.y() + testRect.height()) {
                        expectedResult = false;
                    } else {
                        expectedLine.setP1(QPointF(points[1].x(), qMax(testLine.p1().y(), (double)testRect.y())));
                        expectedLine.setP2(QPointF(points[1].x(), qMin(testLine.p2().y(), (double)testRect.y() + testRect.height())));
                    }
                } else {
                    if (points[0].y() < testRect.y() || points[1].y() > testRect.y() + testRect.height()) {
                        expectedResult = false;
                    } else {
                        expectedLine.setP2(QPointF(points[1].x(), qMax(testLine.p2().y(), (double)testRect.y())));
                        expectedLine.setP1(QPointF(points[1].x(), qMin(testLine.p1().y(), (double)testRect.y() + testRect.height())));
                    }
                }
            } else /* horizontal */ {
                if (points[0].x() < points[1].x()) {
                    if (points[1].x() < testRect.x() || points[0].x() > testRect.x() + testRect.width()) {
                        expectedResult = false;
                    } else {
                        expectedLine.setP1(QPointF(qMax(testLine.p1().x(), (double)testRect.x()), points[1].y()));
                        expectedLine.setP2(QPointF(qMin(testLine.p2().x(), (double)testRect.x() + testRect.width()), points[1].y()));
                    }
                } else {
                    if (points[0].x() < testRect.x() || points[1].x() > testRect.x() + testRect.width()) {
                        expectedResult = false;
                    } else {
                        expectedLine.setP2(QPointF(qMax(testLine.p2().x(), (double)testRect.x()), points[1].y()));
                        expectedLine.setP1(QPointF(qMin(testLine.p1().x(), (double)testRect.x() + testRect.width()), points[1].y()));
                    }
                }
            }
            testLineRectIntersection(testRect, testLine, expectedLine, false, false, expectedResult);
        } else if ((t2 < 0) || (t1 > 1)) {
            if (!onEdge) {
                testLineRectIntersection(testRect, testLine, testLine, false, false, false);
                negative++;
            } else {
                skipped++;
            }
        } else {
            if (t1 < 0) {
                expectedLine.setP1(points[0]);
            } else {
                expectedLine.setP1(testLine.p1());
            }
            if (t2 > 1) {
                expectedLine.setP2(points[1]);
            } else {
                expectedLine.setP2(testLine.p2());
            }
            testLineRectIntersection(testRect, testLine, expectedLine, false, false, true);
            testLineRectIntersection(testRect, testLine, QLineF(points[0], expectedLine.p2()), true, false, true);
            testLineRectIntersection(testRect, testLine, QLineF(expectedLine.p1(), points[1]), false, true, true);
            testLineRectIntersection(testRect, testLine, QLineF(points[0], points[1]), true, true, true);
        }
    }

}


void testLinePolygonIntersectionImpl(QPolygonF poly, QLineF line, QLineF expected, bool extendFirst, bool extendSecond, bool intersectsExpected)
{
    QLineF result = line;
    bool intersects = KisAlgebra2D::intersectLineConvexPolygon(result, poly, extendFirst, extendSecond);

    float epsilon = 1e-4; // good enough for most usecases // I don't use qFuzzyCompare because it has too small epsilon

    auto fuzzyCompare = [epsilon] (float a, float b) -> bool {
            return (qAbs(a - b) < epsilon); };
    auto fuzzyComparePoint = [&] (QPointF a, QPointF b) -> bool {
            return fuzzyCompare(a.x(), b.x()) && fuzzyCompare(a.y(), b.y()); };
    auto fuzzyCompareLine = [&] (QLineF a, QLineF b) -> bool {
            return fuzzyComparePoint(a.p1(), b.p1()) && fuzzyComparePoint(a.p2(), b.p2()); };


    bool success;
    if (!intersectsExpected) {
        qCritical() << "Checking the first case";
        success = intersectsExpected == intersects;
    } else {
        qCritical() << "Checking the second case, with comparing the line";
        success = fuzzyCompareLine(result, expected) && intersectsExpected == intersects;
    }


    if (!success) {
        qCritical() << "FAILURE";
        qCritical() << "intersects: " << intersects;
        qCritical() << "intersects expected: " << intersectsExpected;
        qCritical() << "Extend: " << extendFirst << extendSecond;
        qCritical();

        qCritical() << "original: " << line;
        //qCritical() << "rectangle was: " << ;
        qCritical() << "expected: " << expected;
        qCritical() << "result: " << result;
    }

    QVERIFY(success);

}

void testLinePolygonIntersection(QRect poly, QLineF line, QLineF expected, bool extendFirst, bool extendSecond, bool intersectsExpected)
{
    QPolygonF polygon = QRectF(poly);
    testLinePolygonIntersectionImpl(polygon, line, expected, extendFirst, extendSecond, intersectsExpected);
}

void KisAlgebra2DTest::testLinePolygonIntersectionsManual()
{
    int x1 = 100, x2 = 400; // for x values
    int y1 = 130, y2 = 430; // for y values


    QRect rect1(x1, y1, x2 - x1, y2 - y1);
    QLineF line1original(x1, y1, x2, y2);
    QLineF line1expected = line1original;

    testLinePolygonIntersection(rect1, line1original, line1expected, false, false, true);

    //return;

    QRect rect2(x1, y1, x2, y2);
    QLineF line2original(x1, y1, x2, y2);
    QLineF line2expected = line2original;

    testLinePolygonIntersection(rect2, line2original, line2expected, false, false, true);

    QRect rect3 = QRect(0,0, 2616, 1748);
    QLineF line3original = QLineF(QPointF(1110.74,398.93), QPointF(0,394.068));
    QLineF line3expected = line3original;

    testLinePolygonIntersection(rect3, line3original, line3expected, false, false, true);

    QRect rect4 = rect1;
    QLineF line4 = QLineF(x2, y1, x1, y2);
    testLinePolygonIntersection(rect4, line4, line4, false, false, true);


    QRect rect5 = rect1;
    QLineF line5 = QLineF(x2, y2, x1, y1);
    testLinePolygonIntersection(rect5, line5, line5, false, false, true);

    QRect rect6 = rect1;
    QLineF line6 = QLineF(x2, y1, x2, y2);
    testLinePolygonIntersection(rect6, line6, line6, false, false, true);

    QRect rect7 = rect3;
    QLineF line7 = QLineF(QPointF(338.69,421.014),QPointF(983.456,421.014));
    testLinePolygonIntersection(rect7, line7, line7, false, false, true);

    QRect rect8 = rect3;
    QLineF line8 = QLineF(QPointF(-3545.25,618.966),QPointF(102.104,618.966));
    QLineF line8expected = QLineF(QPointF(0,618.966),QPointF(102.104,618.966));
    testLinePolygonIntersection(rect8, line8, line8expected, false, false, true);



    // vertical
    //   inside the line
    testLinePolygonIntersection(QRect(0, 0, 20, 20), QLineF(0, 5, 0, 15), QLineF(0, 5, 0, 15), false, false, true);
    //   exactly the line
    testLinePolygonIntersection(QRect(0, 0, 20, 20), QLineF(0, 0, 0, 20), QLineF(0, 0, 0, 20), false, false, true);
    //   extending the line
    testLinePolygonIntersection(QRect(0, 0, 20, 20), QLineF(0, -5, 0, 25), QLineF(0, 0, 0, 20), false, false, true);

    // diagonal
    //   exactly
    testLinePolygonIntersection(QRect(0, 0, 20, 20), QLineF(0, 0, 20, 20), QLineF(0, 0, 20, 20), false, false, true);
    //   inside
    testLinePolygonIntersection(QRect(0, 0, 20, 20), QLineF(5, 5, 15, 15), QLineF(5, 5, 15, 15), false, false, true);
    //   extending
    testLinePolygonIntersection(QRect(0, 0, 20, 20), QLineF(-5, -5, 30, 30), QLineF(0, 0, 20, 20), false, false, true);



    testLinePolygonIntersection(QRect(0, 0, 20, 20), QLineF(-5, -100, -5, 100), QLineF(), false, false, false);

    testLinePolygonIntersection(QRect(0, 0, 20, 20), QLineF(-5, -100, -3, 100), QLineF(), false, false, false);



    // start: (-1, -2)
    // end: (4, 5);
    // width, height: 5, 7
    QRect testRect = QRect(-1, -2, 5, 7);

    // first item: line, second item: result after cropping
    QList<QLineF> linesListIntersecting = QList<QLineF>({
                                                            // horizontal
                                                            QLineF(-1, -2, 4, -2), QLineF(-1, -2, 4, -2), // on the edge, exact
                                                            QLineF(-1, 0, 4, 0), QLineF(-1, 0, 4, 0), // below the edge, exact
                                                            QLineF(-10, -2, 10, -2), QLineF(-1, -2, 4, -2), // on the edge, longer
                                                            QLineF(-10, 0, 10, 0), QLineF(-1, 0, 4, 0), // below the edge, longer
                                                            QLineF(0, -2, 3, -2), QLineF(0, -2, 3, -2), // on the edge, shorter
                                                            QLineF(0, 0, 3, -2), QLineF(0, 0, 3, -2), // below the edge, shorter

                                                            // vertical
                                                            QLineF(-1, -2, -1, 5), QLineF(-1, -2, -1, 5), // on the edge, exact
                                                            QLineF(0, -2, 0, 5), QLineF(0, -2, 0, 5), // below the edge, exact
                                                            QLineF(-1, -10, -1, 10), QLineF(-1, -2, -1, 5), // on the edge, longer
                                                            QLineF(0, -10, 0, 10), QLineF(0, -2, 0, 5), // below the edge, longer
                                                            QLineF(-1, 0, -1, 3), QLineF(-1, 0, -1, 3), // on the edge, shorter
                                                            QLineF(0, 0, 0, 3), QLineF(0, 0, 0, 3), // below the edge, shorter


                                                            // skewed
                                                            QLineF(-6, 0, 9, 3), QLineF(-1, 1, 4, 2), // very horizontal-like
                                                            QLineF(-1, 1, 4, 2), QLineF(-1, 1, 4, 2), // very horizontal-like, exact
                                                            QLineF(-6, 0, 4, 2), QLineF(-1, 1, 4, 2), // very horizontal-like, half-exact

                                                            QLineF(-6, -9, 9, 12), QLineF(-1, -2, 4, 5), // perfectly diagonal
                                                            QLineF(-1, -2, 4, 5), QLineF(-1, -2, 4, 5), // perfectly diagonal, exact
                                                            QLineF(-1, -2, 9, 12), QLineF(-1, -2, 4, 5), // perfectly diagonal, half-exact

                                                            QLineF(9, 12, -6, -9), QLineF(4, 5, -1, -2), // perfectly diagonal, p1 and p2 reversed

                                                            QLineF(1, 3, 3, 4), QLineF(1, 3, 3, 4), // skewed, fully inside
                                                            QLineF(2, 2, 6, 4), QLineF(2, 2, 4, 3), // skewed, partially inside


                                                        });

    for (int i = 0; i < linesListIntersecting.size()/2; i++)
    {
        testLinePolygonIntersection(testRect, linesListIntersecting[2*i], linesListIntersecting[2*i + 1], false, false, true);
    }

    // now with extend = true

    // first item: line, second item: result after cropping
    QList<QLineF> linesListIntersectingExtend = QList<QLineF>({
                                                            // horizontal
                                                            QLineF(-1, -2, 4, -2), QLineF(-1, -2, 4, -2), // on the edge, exact
                                                            QLineF(-1, 0, 4, 0), QLineF(-1, 0, 4, 0), // below the edge, exact
                                                            QLineF(-10, -2, 10, -2), QLineF(-1, -2, 4, -2), // on the edge, longer
                                                            QLineF(-10, 0, 10, 0), QLineF(-1, 0, 4, 0), // below the edge, longer
                                                            QLineF(0, -2, 3, -2), QLineF(-1, -2, 4, -2), // on the edge, shorter
                                                            QLineF(0, 0, 3, 0), QLineF(-1, 0, 4, 0), // below the edge, shorter

                                                            // vertical
                                                            QLineF(-1, -2, -1, 5), QLineF(-1, -2, -1, 5), // on the edge, exact
                                                            QLineF(0, -2, 0, 5), QLineF(0, -2, 0, 5), // below the edge, exact
                                                            QLineF(-1, -10, -1, 10), QLineF(-1, -2, -1, 5), // on the edge, longer
                                                            QLineF(0, -10, 0, 10), QLineF(0, -2, 0, 5), // below the edge, longer
                                                            QLineF(-1, 0, -1, 3), QLineF(-1, -2, -1, 5), // on the edge, shorter
                                                            QLineF(0, 0, 0, 3), QLineF(0, -2, 0, 5), // below the edge, shorter


                                                            // skewed
                                                            QLineF(-6, 0, 9, 3), QLineF(-1, 1, 4, 2), // very horizontal-like
                                                            QLineF(-1, 1, 4, 2), QLineF(-1, 1, 4, 2), // very horizontal-like, exact
                                                            QLineF(-6, 0, 4, 2), QLineF(-1, 1, 4, 2), // very horizontal-like, half-exact

                                                            QLineF(-6, -9, 9, 12), QLineF(-1, -2, 4, 5), // perfectly diagonal
                                                            QLineF(-1, -2, 4, 5), QLineF(-1, -2, 4, 5), // perfectly diagonal, exact
                                                            QLineF(-1, -2, 9, 12), QLineF(-1, -2, 4, 5), // perfectly diagonal, half-exact

                                                            QLineF(9, 12, -6, -9), QLineF(4, 5, -1, -2), // perfectly diagonal, p1 and p2 reversed

                                                            QLineF(1, 3, 3, 4), QLineF(-1, 2, 4, 4.5), // skewed, fully inside
                                                            QLineF(2, 2, 6, 4), QLineF(-1, 0.5, 4, 3), // skewed, partially inside

                                                        });

    for (int i = 0; i < linesListIntersectingExtend.size()/2; i++)
    {
        testLinePolygonIntersection(testRect, linesListIntersectingExtend[2*i], linesListIntersectingExtend[2*i + 1], true, true, true);
    }

    QList<QLineF> linesListIntersectingExtendOneSide = QList<QLineF>({
                                                            // horizontal

                                                            QLineF(-1, -2, 4, -2), QLineF(-1, -2, 4, -2), // on the edge, exact
                                                            QLineF(-1, 0, 4, 0), QLineF(-1, 0, 4, 0), // below the edge, exact
                                                            QLineF(-10, -2, 10, -2), QLineF(-1, -2, 4, -2), // on the edge, longer
                                                            QLineF(-10, 0, 10, 0), QLineF(-1, 0, 4, 0), // below the edge, longer
                                                            QLineF(0, -2, 3, -2), QLineF(-1, -2, 3, -2), // on the edge, shorter
                                                            QLineF(0, 0, 3, 0), QLineF(-1, 0, 3, 0), // below the edge, shorter

                                                            // vertical
                                                            QLineF(-1, -2, -1, 5), QLineF(-1, -2, -1, 5), // on the edge, exact
                                                            QLineF(0, -2, 0, 5), QLineF(0, -2, 0, 5), // below the edge, exact
                                                            QLineF(-1, -10, -1, 10), QLineF(-1, -2, -1, 5), // on the edge, longer
                                                            QLineF(0, -10, 0, 10), QLineF(0, -2, 0, 5), // below the edge, longer
                                                            QLineF(-1, 0, -1, 3), QLineF(-1, -2, -1, 3), // on the edge, shorter
                                                            QLineF(0, 0, 0, 3), QLineF(0, -2, 0, 3), // below the edge, shorter


                                                            // skewed
                                                            QLineF(-6, 0, 9, 3), QLineF(-1, 1, 4, 2), // very horizontal-like
                                                            QLineF(-1, 1, 4, 2), QLineF(-1, 1, 4, 2), // very horizontal-like, exact
                                                            QLineF(-6, 0, 4, 2), QLineF(-1, 1, 4, 2), // very horizontal-like, half-exact

                                                            QLineF(-6, -9, 9, 12), QLineF(-1, -2, 4, 5), // perfectly diagonal
                                                            QLineF(-1, -2, 4, 5), QLineF(-1, -2, 4, 5), // perfectly diagonal, exact
                                                            QLineF(-1, -2, 9, 12), QLineF(-1, -2, 4, 5), // perfectly diagonal, half-exact

                                                            QLineF(9, 12, -6, -9), QLineF(4, 5, -1, -2), // perfectly diagonal, p1 and p2 reversed

                                                            QLineF(1, 3, 3, 4), QLineF(-1, 2, 3, 4), // skewed, fully inside
                                                            QLineF(2, 2, 6, 4), QLineF(-1, 0.5, 4, 3), // skewed, partially inside


                                                        });

    for (int i = 0; i < linesListIntersectingExtend.size()/2; i++)
    {
        // first from one side
        testLinePolygonIntersection(testRect, linesListIntersectingExtendOneSide[2*i], linesListIntersectingExtendOneSide[2*i + 1], true, false, true);

        // and then reversed
        QLineF l = QLineF(linesListIntersectingExtendOneSide[2*i].p2(), linesListIntersectingExtendOneSide[2*i].p1());
        QLineF lexp = QLineF(linesListIntersectingExtendOneSide[2*i + 1].p2(), linesListIntersectingExtendOneSide[2*i + 1].p1());
        testLinePolygonIntersection(testRect, l, lexp, false, true, true);
    }

}

void KisAlgebra2DTest::testFindTrianglePoint()
{
    using KisAlgebra2D::findTrianglePoint;
    using KisAlgebra2D::findTrianglePointNearest;

    findTrianglePoint(QPointF(100, 900), QPointF(900, 100), 810, 800);

    findTrianglePoint(QPointF(), QPointF(10, 0), 5, 5);
    findTrianglePoint(QPointF(), QPointF(10, 0), 4, 6);
    findTrianglePoint(QPointF(), QPointF(10, 0), 4, 4);
    findTrianglePoint(QPointF(), QPointF(10, 0), 4, 6 + 1e-3);
    findTrianglePoint(QPointF(), QPointF(10, 0), 4, 6 - 1e-3);
    findTrianglePoint(QPointF(), QPointF(10, 0), 6, 6);
    findTrianglePoint(QPointF(), QPointF(10, 0), 7, 6);

    findTrianglePoint(QPointF(), QPointF(0, 10), 5, 5);
    findTrianglePoint(QPointF(), QPointF(0, 10), 4, 6);
    findTrianglePoint(QPointF(), QPointF(0, 10), 4, 4);
    findTrianglePoint(QPointF(), QPointF(0, 10), 4, 6 + 1e-3);
    findTrianglePoint(QPointF(), QPointF(0, 10), 4, 6 - 1e-3);
    findTrianglePoint(QPointF(), QPointF(0, 10), 6, 6);
    findTrianglePoint(QPointF(), QPointF(0, 10), 7, 6);
}

void KisAlgebra2DTest::testTriangularMotion()
{
    using KisAlgebra2D::moveElasticPoint;

    moveElasticPoint(QPointF(0,10),
                     QPointF(0,0), QPointF(0, 0.1),
                     QPointF(-5, 10), QPointF(5, 11));
}

void KisAlgebra2DTest::testElasticMotion()
{
    using KisAlgebra2D::norm;
    using KisAlgebra2D::dotProduct;
    using KisAlgebra2D::crossProduct;

    const QPointF oldBasePos(70,70);
    QPointF oldResultPoint(100,100);
    const QPointF offset(-2,4);

    QVector<QPointF> anchorPoints;
    anchorPoints << QPointF(0,0);
    anchorPoints << QPointF(100,0);
    anchorPoints << QPointF(0,100);
    anchorPoints << QPointF(50,0);
    anchorPoints << QPointF(0,50);

    const QPointF newBasePos = oldBasePos + offset;
    QPointF newResultPoint = KisAlgebra2D::moveElasticPoint(oldResultPoint, oldBasePos, newBasePos, anchorPoints);

    ENTER_FUNCTION() << ppVar(newResultPoint);
}

void KisAlgebra2DTest::testHaltonSequence()
{
    {

        KisAlgebra2D::HaltonSequenceGenerator g(2);

        for (int i = 0; i < 10; i++) {
            qDebug() << g.generate();
        }
    }

    {
        KisAlgebra2D::HaltonSequenceGenerator g(2);
        for (int i = 0; i < 10; i++) {
            qDebug() << g.generate(10);
        }
    }
}

SIMPLE_TEST_MAIN(KisAlgebra2DTest)
