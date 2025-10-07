/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2025 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisFourPointInterpolatorTest.h"

#include <simpletest.h>


#include "kis_four_point_interpolator_backward.h"
#include "kis_four_point_interpolator_forward.h"
#include "kis_debug.h"


//#define TEST_POINTS_OUTSIDE_POLYGON

std::pair<QPolygonF, QPolygonF> getFoldedTetragon() {
    QPolygonF src;

    src << QPointF(48, 32);
    src << QPointF(56, 32);
    src << QPointF(56, 40);
    src << QPointF(48, 40);

    QPolygonF dst;

    dst << QPointF(49, 32);
    dst << QPointF(56, 32);
    dst << QPointF(59, 40);
    dst << QPointF(69, 40);

    return std::make_pair(src, dst);

}

void KisFourPointInterpolatorTest::testForwardInterpolator()
{
    QPolygonF src;

    src << QPointF(0, 0);
    src << QPointF(100, 0);
    src << QPointF(100, 100);
    src << QPointF(0, 100);

    QPolygonF dst;

    dst << QPointF(0, 0);
    dst << QPointF(100, 10);
    dst << QPointF(100, 120);
    dst << QPointF(0, 100);

    KisFourPointInterpolatorForward interp(src, dst);

    QCOMPARE(interp.map(QPointF(0,50)), QPointF(0,50));
    QCOMPARE(interp.map(QPointF(50,0)), QPointF(50,5));
    QCOMPARE(interp.map(QPointF(100,0)), QPointF(100,10));
    QCOMPARE(interp.map(QPointF(100,50)), QPointF(100,65));

    QCOMPARE(interp.map(QPointF(100,100)), QPointF(100,120));
    QCOMPARE(interp.map(QPointF(50,100)), QPointF(50,110));
    QCOMPARE(interp.map(QPointF(50,50)), QPointF(50,57.5));
}



void KisFourPointInterpolatorTest::testBackwardInterpolatorXShear()
{
    QPolygonF src;

    src << QPointF(0, 0);
    src << QPointF(100, 0);
    src << QPointF(100, 100);
    src << QPointF(0, 100);

    QPolygonF dst;

    dst << QPointF(0, 0);
    dst << QPointF(100, 0);
    dst << QPointF(120, 100);
    dst << QPointF(10, 100);

    KisFourPointInterpolatorBackward interp(src, dst);

    QCOMPARE(interp.map(QPointF(10,100)), QPointF(0,100));
    QCOMPARE(interp.map(QPointF(5,50)), QPointF(0,50));
    QCOMPARE(interp.map(QPointF(110,50)), QPointF(100,50));
    QCOMPARE(interp.map(QPointF(57.5,50)), QPointF(50,50));
}

void KisFourPointInterpolatorTest::testBackwardInterpolatorYShear()
{
    QPolygonF src;

    src << QPointF(0, 0);
    src << QPointF(100, 0);
    src << QPointF(100, 100);
    src << QPointF(0, 100);

    QPolygonF dst;

    dst << QPointF(0, 0);
    dst << QPointF(100, 10);
    dst << QPointF(100, 120);
    dst << QPointF(0, 100);

    KisFourPointInterpolatorBackward interp(src, dst);

    QCOMPARE(interp.map(QPointF(100,10)), QPointF(100,0));
    QCOMPARE(interp.map(QPointF(50,5)), QPointF(50,0));
    QCOMPARE(interp.map(QPointF(50,110)), QPointF(50,100));
    QCOMPARE(interp.map(QPointF(50,57.5)), QPointF(50,50));
}

void KisFourPointInterpolatorTest::testBackwardInterpolatorXYShear()
{
    QPolygonF src;

    src << QPointF(0, 0);
    src << QPointF(100, 0);
    src << QPointF(100, 100);
    src << QPointF(0, 100);

    QPolygonF dst;

    dst << QPointF(0, 0);
    dst << QPointF(100, 10);
    dst << QPointF(140, 120);
    dst << QPointF(20, 100);


    KisFourPointInterpolatorBackward interp(src, dst);

    QCOMPARE(interp.map(QPointF(100,10)), QPointF(100,0));
    QCOMPARE(interp.map(QPointF(50,5)), QPointF(50,0));
    QCOMPARE(interp.map(QPointF(80,110)), QPointF(50,100));
    QCOMPARE(interp.map(QPointF(120,65)), QPointF(100,50));
    QCOMPARE(interp.map(QPointF(10,50)), QPointF(0,50));
}

void KisFourPointInterpolatorTest::testBackwardInterpolatorRoundTrip()
{
    QPolygonF src;

    src << QPointF(0, 0);
    src << QPointF(100, 0);
    src << QPointF(100, 100);
    src << QPointF(0, 100);

    QPolygonF dst;

    dst << QPointF(100, 100);
    dst << QPointF(20, 140);
    dst << QPointF(10, 80);
    dst << QPointF(15, 5);

    KisFourPointInterpolatorForward f(src, dst);
    KisFourPointInterpolatorBackward b(src, dst);

    for (int y = 0; y <= 100; y += 1) {
        for (int x = 0; x <= 100; x += 1) {
            QPointF pt(x, y);

            QPointF fwdPt = f.map(pt);
            QPointF bwdPt = b.map(fwdPt);

            //dbgKrita << "R:" << ppVar(pt) << ppVar(fwdPt) << ppVar(bwdPt) << (bwdPt - pt);
            QVERIFY((bwdPt - pt).manhattanLength() < 1e-3);
        }
    }
}

void KisFourPointInterpolatorTest::testBackwardInterpolatorUnevenlyShearedTetragon()
{
    QPolygonF src;

    src << QPointF(48, 32);
    src << QPointF(56, 32);
    src << QPointF(56, 40);
    src << QPointF(48, 40);

    QPolygonF dst;

    dst << QPointF(49, 32);
    dst << QPointF(56, 32);
    dst << QPointF(69, 40);
    dst << QPointF(59, 40);

    KisFourPointInterpolatorForward f(src, dst);
    KisFourPointInterpolatorBackward b(src, dst);

    // polygon points
    for (int i = 0; i < src.length(); i++) {
        QPointF pt = src[i];
        QPointF fwdPt = f.map(pt);
        QPointF bwdPt = b.map(fwdPt);

        //dbgKrita << "R:" << ppVar(pt) << ppVar(fwdPt) << ppVar(bwdPt) << (bwdPt - pt);
        QVERIFY((bwdPt - pt).manhattanLength() < 1e-3);
    }

    // points from within polygons
    QRectF srcBounds = src.boundingRect();
    for (int x = srcBounds.left(); x <= srcBounds.right(); x++) {
        for (int y = srcBounds.top(); y <= srcBounds.bottom(); y++) {

            QPointF pt = QPointF(x, y);


#ifndef TEST_POINTS_OUTSIDE_POLYGON
            if (srcBounds.contains(pt)) {
#endif
                QPointF fwdPt = f.map(pt);
                QPointF bwdPt = b.map(fwdPt);

                //dbgKrita << "R:" << ppVar(pt) << ppVar(fwdPt) << ppVar(bwdPt) << (bwdPt - pt);
                if ((bwdPt - pt).manhattanLength() >= 1e-3) {
                    ENTER_FUNCTION() << "R:" << ppVar(pt) << ppVar(fwdPt) << ppVar(bwdPt) << (bwdPt - pt);
                }
                QVERIFY((bwdPt - pt).manhattanLength() < 1e-3);
#ifndef TEST_POINTS_OUTSIDE_POLYGON
            }
#endif
        }
    }

#ifdef TEST_POINTS_OUTSIDE_POLYGON
    // points from outside of the polygons
    for (int y = 0; y <= 100; y += 1) {
        for (int x = 0; x <= 100; x += 1) {
            QPointF pt(x, y);
            QPointF fwdPt = f.map(pt);
            QPointF bwdPt = b.map(fwdPt);

            //dbgKrita << "R:" << ppVar(pt) << ppVar(fwdPt) << ppVar(bwdPt) << (bwdPt - pt);
            //DBGVERIFY(((bwdPt - pt).manhattanLength() < 1e-3), (QString(pt).append(fwdPt).append(bwdPt)));
            QVERIFY((bwdPt - pt).manhattanLength() < 1e-3);
        }
    }
#endif
}



void KisFourPointInterpolatorTest::testBackwardInterpolatorFoldedTetragon()
{
    auto pair = getFoldedTetragon();
    QPolygonF src = pair.first;
    QPolygonF dst = pair.second;

    KisFourPointInterpolatorForward f(src, dst);
    KisFourPointInterpolatorBackward b(src, dst);

    // polygon points
    for (int i = 0; i < src.length(); i++) {
        QPointF pt = src[i];
        QPointF fwdPt = f.map(pt);
        QPointF bwdPt = b.map(fwdPt);

        //dbgKrita << "R:" << ppVar(pt) << ppVar(fwdPt) << ppVar(bwdPt) << (bwdPt - pt);
        if ((bwdPt - pt).manhattanLength() >= 1e-3) {
            ENTER_FUNCTION() << "R:" << ppVar(pt) << ppVar(fwdPt) << ppVar(bwdPt) << (bwdPt - pt);
        }
        QVERIFY((bwdPt - pt).manhattanLength() < 1e-3);
    }

    // points from within polygons
    QRectF srcBounds = src.boundingRect();
    for (int x = srcBounds.left(); x <= srcBounds.right(); x++) {
        for (int y = srcBounds.top(); y <= srcBounds.bottom(); y++) {

            QPointF pt(x, y);
#ifndef TEST_POINTS_OUTSIDE_POLYGON
            if (srcBounds.contains(pt)) {
#endif
                QPointF fwdPt = f.map(pt);
                QPointF bwdPt = b.map(fwdPt);

                //dbgKrita << "R:" << ppVar(pt) << ppVar(fwdPt) << ppVar(bwdPt) << (bwdPt - pt);
                QVERIFY((bwdPt - pt).manhattanLength() < 1e-3);
#ifndef TEST_POINTS_OUTSIDE_POLYGON
            }
#endif
        }
    }
#ifdef TEST_POINTS_OUTSIDE_POLYGON
    // points from outside of the polygons
    for (int y = 0; y <= 100; y += 1) {
        for (int x = 0; x <= 100; x += 1) {
            QPointF pt(x, y);
            QPointF fwdPt = f.map(pt);
            QPointF bwdPt = b.map(fwdPt);

            //dbgKrita << "R:" << ppVar(pt) << ppVar(fwdPt) << ppVar(bwdPt) << (bwdPt - pt);
            if ((bwdPt - pt).manhattanLength() >= 1e-3) {
                qCritical() << ppVar(pt) << ppVar(fwdPt) << ppVar(bwdPt) << (bwdPt - pt);
            }
            QVERIFY((bwdPt - pt).manhattanLength() < 1e-3);
        }
    }
#endif
}

void KisFourPointInterpolatorTest::testBackwardInterpolatorSpecialCase()
{
    QPolygonF src;

    src << QPointF(144, 184);
    src << QPointF(152, 184);
    src << QPointF(152, 192);
    src << QPointF(144, 192);

    QPolygonF dst;

    dst << QPointF(162.206, 132.418);
    dst << QPointF(155.284, 132.652);
    dst << QPointF(155.427, 139.67);
    dst << QPointF(162.532, 139.43);

    KisFourPointInterpolatorBackward b(src, dst);

    b.setY(133);
    b.setX(156);

    QPointF result = b.getValue();
    qCritical() << result;

}

void KisFourPointInterpolatorTest::testBackwardInterpolatorSpecialCaseSecond()
{
    auto pair = getFoldedTetragon();
    QPolygonF src = pair.first;
    QPolygonF dst = pair.second;


    KisFourPointInterpolatorForward f(src, dst);
    KisFourPointInterpolatorBackward b(src, dst);

    QPointF pt = QPointF(56,40);
    QPointF claimedDstPt = QPointF(59, 40);
    QPointF srcPtJustBwd = b.map(claimedDstPt);

    ENTER_FUNCTION() << ppVar(srcPtJustBwd);
    QVERIFY(qIsFinite(srcPtJustBwd.x()));
    QVERIFY(qIsFinite(srcPtJustBwd.y()));

    QVERIFY((srcPtJustBwd - pt).manhattanLength() < 1e-3);

    QPointF dstPt = f.map(pt);
    QPointF srcPt = b.map(dstPt);

    ENTER_FUNCTION() << ppVar(srcPt);
    QVERIFY((srcPt - pt).manhattanLength() < 1e-3);

}

SIMPLE_TEST_MAIN(KisFourPointInterpolatorTest)
