/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_scanline_fill_test.h"

#include "testutil.h"

#include <qtest_kde.h>
#include <floodfill/kis_scanline_fill.h>
#include <floodfill/kis_fill_interval.h>
#include <floodfill/kis_fill_interval_map.h>

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include "kis_types.h"
#include "kis_paint_device.h"


void KisScanlineFillTest::testFillGeneral(const QVector<KisFillInterval> &initialBackwardIntervals,
                                          const QVector<QColor> &expectedResult,
                                          const QVector<KisFillInterval> &expectedForwardIntervals,
                                          const QVector<KisFillInterval> &expectedBackwardIntervals)
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    dev->setPixel(1, 0, Qt::white);
    dev->setPixel(2, 0, Qt::white);
    dev->setPixel(5, 0, Qt::white);
    dev->setPixel(8, 0, Qt::white);
    dev->setPixel(17, 0, Qt::white);

    QRect boundingRect(-10, -10, 30, 30);

    KisScanlineFill gc(dev, QPoint(), boundingRect);

    KisFillIntervalMap *backwardMap = gc.testingGetBackwardIntervals();
    foreach(const KisFillInterval &i, initialBackwardIntervals) {
        backwardMap->insertInterval(i);
    }

    KisFillInterval processInterval(0,10,0);
    gc.testingProcessLine(processInterval);

    Q_ASSERT(expectedResult.size() == processInterval.width());

    for (int i = 0; i < 11; i++) {
        QColor c;
        dev->pixel(i, 0, &c);
        dbgKrita << i << ":" << c.red();

        QCOMPARE(c, expectedResult[i]);
    }

    QVector<KisFillInterval> forwardIntervals =
        gc.testingGetForwardIntervals();

    for (int i = 0; i < forwardIntervals.size(); i++) {
        dbgKrita << "FW:" << forwardIntervals[i];
        QCOMPARE(forwardIntervals[i], expectedForwardIntervals[i]);
    }
    QCOMPARE(forwardIntervals.size(), expectedForwardIntervals.size());


    QStack<KisFillInterval> backwardIntervals =
        gc.testingGetBackwardIntervals()->fetchAllIntervals();

    for (int i = 0; i < backwardIntervals.size(); i++) {
        dbgKrita << "BW:" << backwardIntervals[i];
        QCOMPARE(backwardIntervals[i], expectedBackwardIntervals[i]);
    }
    QCOMPARE(backwardIntervals.size(), expectedBackwardIntervals.size());
}

inline QColor testingColor(quint8 c) {
    return QColor(c, c, c, c);
}

void KisScanlineFillTest::testSimpleFill()
{
    QVector<KisFillInterval> initialBackwardIntervals;
    QVector<QColor> expectedResult;
    expectedResult << testingColor(200); //  0
    expectedResult << testingColor(255); //  1
    expectedResult << testingColor(255); //  2
    expectedResult << testingColor(200); //  3
    expectedResult << testingColor(200); //  4
    expectedResult << testingColor(255); //  5
    expectedResult << testingColor(200); //  6
    expectedResult << testingColor(200); //  7
    expectedResult << testingColor(255); //  8
    expectedResult << testingColor(200); //  9
    expectedResult << testingColor(200); // 10

    QVector<KisFillInterval> expectedForwardIntervals;
    expectedForwardIntervals << KisFillInterval(-10,  0, 1);
    expectedForwardIntervals << KisFillInterval(3,  4, 1);
    expectedForwardIntervals << KisFillInterval(6,  7, 1);
    expectedForwardIntervals << KisFillInterval(9, 16, 1);

    QVector<KisFillInterval> expectedBackwardIntervals;
    expectedBackwardIntervals << KisFillInterval(-10,  -1, 0);
    expectedBackwardIntervals << KisFillInterval(11, 16, 0);

    testFillGeneral(initialBackwardIntervals,
                    expectedResult,
                    expectedForwardIntervals,
                    expectedBackwardIntervals);
}

void KisScanlineFillTest::testFillBackwardCollisionOnTheLeft()
{
    QVector<KisFillInterval> initialBackwardIntervals;
    initialBackwardIntervals << KisFillInterval(-10,  0, 0);

    QVector<QColor> expectedResult;
    expectedResult << testingColor(  0); //  0
    expectedResult << testingColor(255); //  1
    expectedResult << testingColor(255); //  2
    expectedResult << testingColor(200); //  3
    expectedResult << testingColor(200); //  4
    expectedResult << testingColor(255); //  5
    expectedResult << testingColor(200); //  6
    expectedResult << testingColor(200); //  7
    expectedResult << testingColor(255); //  8
    expectedResult << testingColor(200); //  9
    expectedResult << testingColor(200); // 10

    QVector<KisFillInterval> expectedForwardIntervals;
    expectedForwardIntervals << KisFillInterval(3,  4, 1);
    expectedForwardIntervals << KisFillInterval(6,  7, 1);
    expectedForwardIntervals << KisFillInterval(9, 16, 1);

    QVector<KisFillInterval> expectedBackwardIntervals;
    expectedBackwardIntervals << KisFillInterval(-10,  -1, 0);
    expectedBackwardIntervals << KisFillInterval(11, 16, 0);


    testFillGeneral(initialBackwardIntervals,
                    expectedResult,
                    expectedForwardIntervals,
                    expectedBackwardIntervals);
}

void KisScanlineFillTest::testFillBackwardCollisionOnTheRight()
{
    QVector<KisFillInterval> initialBackwardIntervals;
    initialBackwardIntervals << KisFillInterval(9, 20, 0);

    QVector<QColor> expectedResult;
    expectedResult << testingColor(200); //  0
    expectedResult << testingColor(255); //  1
    expectedResult << testingColor(255); //  2
    expectedResult << testingColor(200); //  3
    expectedResult << testingColor(200); //  4
    expectedResult << testingColor(255); //  5
    expectedResult << testingColor(200); //  6
    expectedResult << testingColor(200); //  7
    expectedResult << testingColor(255); //  8
    expectedResult << testingColor(  0); //  9
    expectedResult << testingColor(  0); // 10

    QVector<KisFillInterval> expectedForwardIntervals;
    expectedForwardIntervals << KisFillInterval(-10,  0, 1);
    expectedForwardIntervals << KisFillInterval(3,  4, 1);
    expectedForwardIntervals << KisFillInterval(6,  7, 1);

    QVector<KisFillInterval> expectedBackwardIntervals;
    expectedBackwardIntervals << KisFillInterval(-10,  -1, 0);
    expectedBackwardIntervals << KisFillInterval(11, 20, 0);

    testFillGeneral(initialBackwardIntervals,
                    expectedResult,
                    expectedForwardIntervals,
                    expectedBackwardIntervals);
}

void KisScanlineFillTest::testFillBackwardCollisionFull()
{
    QVector<KisFillInterval> initialBackwardIntervals;
    initialBackwardIntervals << KisFillInterval(-10, 20, 0);

    QVector<QColor> expectedResult;
    expectedResult << testingColor(  0); //  0
    expectedResult << testingColor(255); //  1
    expectedResult << testingColor(255); //  2
    expectedResult << testingColor(  0); //  3
    expectedResult << testingColor(  0); //  4
    expectedResult << testingColor(255); //  5
    expectedResult << testingColor(  0); //  6
    expectedResult << testingColor(  0); //  7
    expectedResult << testingColor(255); //  8
    expectedResult << testingColor(  0); //  9
    expectedResult << testingColor(  0); // 10

    QVector<KisFillInterval> expectedForwardIntervals;

    QVector<KisFillInterval> expectedBackwardIntervals;
    expectedBackwardIntervals << KisFillInterval(-10, -1, 0);
    expectedBackwardIntervals << KisFillInterval(11, 20, 0);

    testFillGeneral(initialBackwardIntervals,
                    expectedResult,
                    expectedForwardIntervals,
                    expectedBackwardIntervals);
}

void KisScanlineFillTest::testFillBackwardCollisionSanityCheck()
{
#if defined ENABLE_FILL_SANITY_CHECKS && defined ENABLE_CHECKS_FOR_TESTING
    QVector<KisFillInterval> initialBackwardIntervals;
    initialBackwardIntervals << KisFillInterval(0, 10, 0);

    QVector<QColor> expectedResult;
    QVector<KisFillInterval> expectedForwardIntervals;
    QVector<KisFillInterval> expectedBackwardIntervals;
    expectedBackwardIntervals << KisFillInterval(0, 10, 0);

    bool gotException = false;

    try {
        testFillGeneral(initialBackwardIntervals,
                        expectedResult,
                        expectedForwardIntervals,
                        expectedBackwardIntervals);
    } catch (...) {
        gotException = true;
    }

    QVERIFY(gotException);
#endif /* ENABLE_FILL_SANITY_CHECKS */
}

QTEST_KDEMAIN(KisScanlineFillTest, GUI)
