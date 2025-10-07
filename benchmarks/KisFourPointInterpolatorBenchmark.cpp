/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2025 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisFourPointInterpolatorBenchmark.h"

#include <simpletest.h>


#include "kis_four_point_interpolator_backward.h"
#include "kis_four_point_interpolator_forward.h"
#include "kis_debug.h"


//#define _CALCULATE_POINTS_OUTSIDE_POLYGON



void KisFourPointInterpolatorBenchmark::testForwardInterpolator()
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
    int count = 100;
    QBENCHMARK {
        for (int i = 0; i < count; i++) {
            interp.map(QPointF(0,50));
            interp.map(QPointF(50,0));
            interp.map(QPointF(100,0));
            interp.map(QPointF(100,50));

            interp.map(QPointF(100,100));
            interp.map(QPointF(50,100));
            interp.map(QPointF(50,50));
        }
    }
}



void KisFourPointInterpolatorBenchmark::testBackwardInterpolatorXShear()
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

    QBENCHMARK {
        interp.map(QPointF(10,100));
        interp.map(QPointF(5,50));
        interp.map(QPointF(110,50));
        interp.map(QPointF(57.5,50));
    }
}

void KisFourPointInterpolatorBenchmark::testBackwardInterpolatorYShear()
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

    QBENCHMARK {
        interp.map(QPointF(100,10)), QPointF(100,0);
        interp.map(QPointF(50,5)), QPointF(50,0);
        interp.map(QPointF(50,110)), QPointF(50,100);
        interp.map(QPointF(50,57.5)), QPointF(50,50);
    }
}

void KisFourPointInterpolatorBenchmark::testBackwardInterpolatorXYShear()
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

    QBENCHMARK {
        interp.map(QPointF(100,10)), QPointF(100,0);
        interp.map(QPointF(50,5)), QPointF(50,0);
        interp.map(QPointF(80,110)), QPointF(50,100);
        interp.map(QPointF(120,65)), QPointF(100,50);
        interp.map(QPointF(10,50)), QPointF(0,50);
    }
}

void KisFourPointInterpolatorBenchmark::testBackwardInterpolatorRoundTrip()
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

    QBENCHMARK {
        for (int y = 0; y <= 100; y += 1) {
            for (int x = 0; x <= 100; x += 1) {
                QPointF pt(x, y);

                QPointF fwdPt = f.map(pt);
                QPointF bwdPt = b.map(fwdPt);

                //dbgKrita << "R:" << ppVar(pt) << ppVar(fwdPt) << ppVar(bwdPt) << (bwdPt - pt);
                qreal length = (bwdPt - pt).manhattanLength() < 1e-3;
            }
        }
    }
}

void KisFourPointInterpolatorBenchmark::testBackwardInterpolatorUnevenlyShearedTetragon()
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

    QBENCHMARK {

        // polygon points
        for (int i = 0; i < src.length(); i++) {
            QPointF pt = src[i];
            QPointF fwdPt = f.map(pt);
            QPointF bwdPt = b.map(fwdPt);

            //dbgKrita << "R:" << ppVar(pt) << ppVar(fwdPt) << ppVar(bwdPt) << (bwdPt - pt);
            qreal length = (bwdPt - pt).manhattanLength() < 1e-3;
        }

        // points from within polygons
        QRectF srcBounds = src.boundingRect();
        for (int x = srcBounds.left(); x <= srcBounds.right(); x++) {
            for (int y = srcBounds.top(); y <= srcBounds.bottom(); y++) {

                QPointF pt = QPointF(x, y);
                QPointF fwdPt = f.map(pt);
                QPointF bwdPt = b.map(fwdPt);

                //dbgKrita << "R:" << ppVar(pt) << ppVar(fwdPt) << ppVar(bwdPt) << (bwdPt - pt);
                qreal length = (bwdPt - pt).manhattanLength() < 1e-3;
            }
        }

#ifdef _CALCULATE_POINTS_OUTSIDE_POLYGON
        // points from outside of the polygons
        for (int y = 0; y <= 100; y += 1) {
            for (int x = 0; x <= 100; x += 1) {
                QPointF pt(x, y);
                QPointF fwdPt = f.map(pt);
                QPointF bwdPt = b.map(fwdPt);

                //dbgKrita << "R:" << ppVar(pt) << ppVar(fwdPt) << ppVar(bwdPt) << (bwdPt - pt);
                //DBGVERIFY(((bwdPt - pt).manhattanLength() < 1e-3), (QString(pt).append(fwdPt).append(bwdPt)));
                qreal length = (bwdPt - pt).manhattanLength() < 1e-3;
            }
        }
#endif

    }

}



void KisFourPointInterpolatorBenchmark::testBackwardInterpolatorFoldedTetragon()
{
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

    KisFourPointInterpolatorForward f(src, dst);
    KisFourPointInterpolatorBackward b(src, dst);

    QBENCHMARK {

        // polygon points
        for (int i = 0; i < src.length(); i++) {
            QPointF pt = src[i];
            QPointF fwdPt = f.map(pt);
            QPointF bwdPt = b.map(fwdPt);

            //dbgKrita << "R:" << ppVar(pt) << ppVar(fwdPt) << ppVar(bwdPt) << (bwdPt - pt);
            qreal length = (bwdPt - pt).manhattanLength() < 1e-3;

        }

        // points from within polygons
        QRectF srcBounds = src.boundingRect();
        for (int x = srcBounds.left(); x <= srcBounds.right(); x++) {
            for (int y = srcBounds.top(); y <= srcBounds.bottom(); y++) {

                QPointF pt = QPointF(x, y);
                QPointF fwdPt = f.map(pt);
                QPointF bwdPt = b.map(fwdPt);

                //dbgKrita << "R:" << ppVar(pt) << ppVar(fwdPt) << ppVar(bwdPt) << (bwdPt - pt);
                qreal length = (bwdPt - pt).manhattanLength() < 1e-3;
            }
        }

#ifdef _CALCULATE_POINTS_OUTSIDE_POLYGON
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
                qreal length = bwdPt - pt).manhattanLength() < 1e-3;
            }
        }
#endif

    }

}

void KisFourPointInterpolatorBenchmark::testBackwardInterpolatorSpecialCase()
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

#ifdef _CALCULATE_POINTS_OUTSIDE_POLYGON
    QBENCHMARK {
        QPointF result = b.getValue();
    }

    QPointF result = b.getValue();
    qCritical() << result;
#endif

}

SIMPLE_TEST_MAIN(KisFourPointInterpolatorBenchmark)
