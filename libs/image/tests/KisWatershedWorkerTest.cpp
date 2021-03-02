/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisWatershedWorkerTest.h"

#include <simpletest.h>

#include "kis_paint_device.h"
#include "kis_painter.h"

#include "kis_paint_device_debug_utils.h"

#include "kis_gaussian_kernel.h"
#include "krita_utils.h"

#include "lazybrush/kis_lazy_fill_tools.h"
#include "testutil.h"
#include "testing_timed_default_bounds.h"


#include <lazybrush/KisWatershedWorker.h>

inline KisPaintDeviceSP loadTestImage(const QString &name, bool convertToAlpha)
{
    QImage image(TestUtil::fetchDataFileLazy(name));
    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    dev->convertFromQImage(image, 0);
    if (convertToAlpha) {
        dev = KisPainter::convertToAlphaAsAlpha(dev);
    }

    return dev;
}

void KisWatershedWorkerTest::testWorker()
{
    KisPaintDeviceSP mainDev = loadTestImage("fill1_main.png", false);
    KisPaintDeviceSP aLabelDev = loadTestImage("fill1_a_extra.png", true);
    KisPaintDeviceSP bLabelDev = loadTestImage("fill1_b.png", true);
    KisPaintDeviceSP resultColoring = new KisPaintDevice(mainDev->colorSpace());

    KisPaintDeviceSP filteredMainDev = KisPainter::convertToAlphaAsGray(mainDev);
    const QRect filterRect = filteredMainDev->exactBounds();

    mainDev->setDefaultBounds(new TestUtil::TestingTimedDefaultBounds(filterRect));
    aLabelDev->setDefaultBounds(mainDev->defaultBounds());
    bLabelDev->setDefaultBounds(mainDev->defaultBounds());
    resultColoring->setDefaultBounds(mainDev->defaultBounds());
    filteredMainDev->setDefaultBounds(mainDev->defaultBounds());

    KisGaussianKernel::applyLoG(filteredMainDev,
                                filterRect,
                                2,
                                -1.0,
                                QBitArray(), 0);

    KisLazyFillTools::normalizeAlpha8Device(filteredMainDev, filterRect);

    KIS_DUMP_DEVICE_2(filteredMainDev, filterRect, "main", "dd");
    KIS_DUMP_DEVICE_2(aLabelDev, filterRect, "alabel", "dd");

    KisWatershedWorker worker(filteredMainDev, resultColoring, filterRect);
    worker.addKeyStroke(aLabelDev, KoColor(Qt::red, mainDev->colorSpace()));
    worker.addKeyStroke(bLabelDev, KoColor(Qt::blue, mainDev->colorSpace()));
    worker.run(0.7);

}

void KisWatershedWorkerTest::testWorkerSmall()
{
    KisPaintDeviceSP mainDev = loadTestImage("fill5_main.png", false);
    KisPaintDeviceSP aLabelDev = loadTestImage("fill5_a.png", true);
    KisPaintDeviceSP bLabelDev = loadTestImage("fill5_b.png", true);
    KisPaintDeviceSP resultColoring = new KisPaintDevice(mainDev->colorSpace());

    KisPaintDeviceSP filteredMainDev = KisPainter::convertToAlphaAsGray(mainDev);
    const QRect filterRect = filteredMainDev->exactBounds();
//    KisGaussianKernel::applyLoG(filteredMainDev,
//                                filterRect,
//                                2,
//                                QBitArray(), 0);

    KisLazyFillTools::normalizeAndInvertAlpha8Device(filteredMainDev, filterRect);

    KIS_DUMP_DEVICE_2(filteredMainDev, filterRect, "main", "dd");
    KIS_DUMP_DEVICE_2(aLabelDev, filterRect, "alabel", "dd");
    KIS_DUMP_DEVICE_2(bLabelDev, filterRect, "blabel", "dd");

    KisWatershedWorker worker(filteredMainDev, resultColoring, filterRect);
    worker.addKeyStroke(aLabelDev, KoColor(Qt::red, mainDev->colorSpace()));
    worker.addKeyStroke(bLabelDev, KoColor(Qt::blue, mainDev->colorSpace()));
    worker.run();

    QCOMPARE(worker.testingGroupPositiveEdge(1, 0), 35);
    QCOMPARE(worker.testingGroupNegativeEdge(1, 0), 0);
    QCOMPARE(worker.testingGroupForeignEdge(1, 0), 5);

    QCOMPARE(worker.testingGroupPositiveEdge(1, 255), 3);
    QCOMPARE(worker.testingGroupNegativeEdge(1, 255), 15);
    QCOMPARE(worker.testingGroupForeignEdge(1, 255), 8);

    QCOMPARE(worker.testingGroupPositiveEdge(2, 0), 22);
    QCOMPARE(worker.testingGroupNegativeEdge(2, 0), 0);
    QCOMPARE(worker.testingGroupForeignEdge(2, 0), 6);

    QCOMPARE(worker.testingGroupPositiveEdge(2, 255), 1);
    QCOMPARE(worker.testingGroupNegativeEdge(2, 255), 6);
    QCOMPARE(worker.testingGroupForeignEdge(2, 255), 7);
}

void KisWatershedWorkerTest::testWorkerSmallWithAllies()
{
    KisPaintDeviceSP mainDev = loadTestImage("fill5_main.png", false);
    KisPaintDeviceSP aLabelDev = loadTestImage("fill5_a_extra.png", true);
    KisPaintDeviceSP bLabelDev = loadTestImage("fill5_b.png", true);
    KisPaintDeviceSP resultColoring = new KisPaintDevice(mainDev->colorSpace());

    KisPaintDeviceSP filteredMainDev = KisPainter::convertToAlphaAsGray(mainDev);
    const QRect filterRect = filteredMainDev->exactBounds();
//    KisGaussianKernel::applyLoG(filteredMainDev,
//                                filterRect,
//                                2,
//                                QBitArray(), 0);

    KisLazyFillTools::normalizeAndInvertAlpha8Device(filteredMainDev, filterRect);

    KIS_DUMP_DEVICE_2(filteredMainDev, filterRect, "main", "dd");
    KIS_DUMP_DEVICE_2(aLabelDev, filterRect, "alabel", "dd");
    KIS_DUMP_DEVICE_2(bLabelDev, filterRect, "blabel", "dd");

    KisWatershedWorker worker(filteredMainDev, resultColoring, filterRect);
    worker.addKeyStroke(aLabelDev, KoColor(Qt::red, mainDev->colorSpace()));
    worker.addKeyStroke(bLabelDev, KoColor(Qt::blue, mainDev->colorSpace()));
    worker.run();

    QCOMPARE(worker.testingGroupPositiveEdge(1, 0), 29);
    QCOMPARE(worker.testingGroupNegativeEdge(1, 0), 0);
    QCOMPARE(worker.testingGroupForeignEdge(1, 0), 0);
    QCOMPARE(worker.testingGroupAllyEdge(1, 0), 1);

    QCOMPARE(worker.testingGroupPositiveEdge(1, 255), 2);
    QCOMPARE(worker.testingGroupNegativeEdge(1, 255), 11);
    QCOMPARE(worker.testingGroupForeignEdge(1, 255), 4);
    QCOMPARE(worker.testingGroupAllyEdge(1, 255), 5);
    QCOMPARE(worker.testingGroupConflicts(1, 255, 3), 4);

    QCOMPARE(worker.testingGroupPositiveEdge(2, 0), 16);
    QCOMPARE(worker.testingGroupNegativeEdge(2, 0), 0);
    QCOMPARE(worker.testingGroupForeignEdge(2, 0), 5);
    QCOMPARE(worker.testingGroupAllyEdge(2, 0), 1);
    QCOMPARE(worker.testingGroupConflicts(2, 0, 3), 5);

    QCOMPARE(worker.testingGroupPositiveEdge(2, 255), 1);
    QCOMPARE(worker.testingGroupNegativeEdge(2, 255), 7);
    QCOMPARE(worker.testingGroupForeignEdge(2, 255), 1);
    QCOMPARE(worker.testingGroupAllyEdge(2, 255), 5);
    QCOMPARE(worker.testingGroupConflicts(2, 255, 3), 1);

    QCOMPARE(worker.testingGroupPositiveEdge(3, 0), 13);
    QCOMPARE(worker.testingGroupNegativeEdge(3, 0), 0);
    QCOMPARE(worker.testingGroupForeignEdge(3, 0), 5);
    QCOMPARE(worker.testingGroupAllyEdge(3, 0), 0);
    QCOMPARE(worker.testingGroupConflicts(3, 0, 2), 5);

    QCOMPARE(worker.testingGroupPositiveEdge(3, 255), 1);
    QCOMPARE(worker.testingGroupNegativeEdge(3, 255), 4);
    QCOMPARE(worker.testingGroupForeignEdge(3, 255), 5);
    QCOMPARE(worker.testingGroupAllyEdge(3, 255), 0);
    QCOMPARE(worker.testingGroupConflicts(3, 255, 1), 4);
    QCOMPARE(worker.testingGroupConflicts(3, 255, 2), 1);

    worker.testingTryRemoveGroup(2, 0);

    // check the group was really removed!
    QCOMPARE(worker.testingGroupPositiveEdge(2, 0), 0);
    QCOMPARE(worker.testingGroupNegativeEdge(2, 0), 0);
    QCOMPARE(worker.testingGroupForeignEdge(2, 0), 0);
    QCOMPARE(worker.testingGroupAllyEdge(2, 0), 0);
    QCOMPARE(worker.testingGroupConflicts(2, 0, 3), 0);
}

SIMPLE_TEST_MAIN(KisWatershedWorkerTest)
