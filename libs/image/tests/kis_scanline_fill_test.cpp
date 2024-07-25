/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_scanline_fill_test.h"

#include <testutil.h>

#include <simpletest.h>
#include <floodfill/kis_scanline_fill.h>
#include <floodfill/kis_fill_interval.h>
#include <floodfill/kis_fill_interval_map.h>

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_default_bounds.h"
#include "kis_pixel_selection.h"

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
    Q_FOREACH (const KisFillInterval &i, initialBackwardIntervals) {
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

void KisScanlineFillTest::testClearNonZeroComponent()
{
    const QRect rc1(10, 10, 10, 10);
    const QRect rc2(30, 10, 10, 10);
    const QRect boundingRect(0,0,100,100);

    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());

    dev->fill(rc1, KoColor(Qt::red, dev->colorSpace()));
    dev->fill(rc2, KoColor(Qt::green, dev->colorSpace()));

    QCOMPARE(dev->exactBounds(), rc1 | rc2);

    KisScanlineFill fill(dev, QPoint(10,10), boundingRect);
    fill.clearNonZeroComponent();

    QCOMPARE(dev->exactBounds(), rc2);
}

void KisScanlineFillTest::testExternalFill()
{
    const QRect rc1(10, 10, 10, 10);
    const QRect rc2(30, 10, 10, 10);
    const QRect boundingRect(0,0,100,100);

    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    KisPaintDeviceSP other = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());

    dev->fill(rc1, KoColor(Qt::red, dev->colorSpace()));
    dev->fill(rc2, KoColor(Qt::green, dev->colorSpace()));

    QCOMPARE(dev->exactBounds(), rc1 | rc2);

    KisScanlineFill fill(dev, QPoint(10,10), boundingRect);
    fill.fill(KoColor(Qt::blue, dev->colorSpace()), other);

    QCOMPARE(dev->exactBounds(), rc1 | rc2);
    QCOMPARE(other->exactBounds(), rc1);

    QColor c;

    dev->pixel(10, 10, &c);
    QCOMPARE(c, QColor(Qt::red));

    other->pixel(10, 10, &c);
    QCOMPARE(c, QColor(Qt::blue));
}

void KisScanlineFillTest::testGapClosingFillGeneral(QPoint seed, int gapSize)
{
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    QImage srcImage(TestUtil::fetchDataFileLazy("close_gap_low.png"));
    QVERIFY(!srcImage.isNull());

    QRect imageRect = srcImage.rect();

    dev->convertFromQImage(srcImage, 0, 0, 0);

    KisPixelSelectionSP pixelSelection = new KisPixelSelection(new KisSelectionDefaultBounds(dev));

    KisScanlineFill gc(dev, seed, imageRect);
    gc.setThreshold(1);
    gc.setOpacitySpread(100);
    gc.setCloseGap(gapSize);

    gc.fillSelection(pixelSelection);

    QImage resultImage =
        pixelSelection->convertToQImage(0,
                                        imageRect.x(), imageRect.y(),
                                        imageRect.width(), imageRect.height());

    QString referenceSuffix =
        QString("low_%1_%2_%3").arg(gapSize).arg(seed.x()).arg(seed.y());

    QVERIFY(TestUtil::checkQImage(resultImage,
                                  "fill_painter",
                                  "close_gap_",
                                  referenceSuffix));
}

void KisScanlineFillTest::testGapClosingFill()
{
    // The most basic case.
    testGapClosingFillGeneral(QPoint(52, 84), 1);

    // Progressively better closing.
    testGapClosingFillGeneral(QPoint(103, 94), 1);
    testGapClosingFillGeneral(QPoint(103, 94), 2);
    testGapClosingFillGeneral(QPoint(103, 94), 3);

    // Curved lines, filling into gaps.
    testGapClosingFillGeneral(QPoint(43, 30), 3);

    // No spilling at an extreme gap size.
    testGapClosingFillGeneral(QPoint(32, 28), 32);

    // Getting into a problematic territory (missed pixels).
    // NOTE: The algorithm WILL miss some currently.
    testGapClosingFillGeneral(QPoint(103, 94), 8);
    testGapClosingFillGeneral(QPoint(93, 79), 18);

    // Fill lines instead of transparent areas.
    testGapClosingFillGeneral(QPoint(146, 96), 1);

    // Expansion cases (filling away from a corner/gap).
    testGapClosingFillGeneral(QPoint(50, 57), 5);
    testGapClosingFillGeneral(QPoint(63, 53), 5);
    testGapClosingFillGeneral(QPoint(63, 53), 19);

    // A problematic expansion case (will not fully expand).
    // The test will help notice if it got better or worse.
    testGapClosingFillGeneral(QPoint(39, 56), 4);

    // Fill up to the image edges. Ensure no out of bounds access.
    testGapClosingFillGeneral(QPoint(147, 97), 32);
}

SIMPLE_TEST_MAIN(KisScanlineFillTest)
