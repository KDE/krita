/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_colorize_mask_test.h"

#include <QTest>

#include <testutil.h>
#include "lazybrush/kis_colorize_mask.h"
#include "kis_paint_device_debug_utils.h"
#include "kis_global.h"
#include "lazybrush/kis_lazy_fill_tools.h"
#include "kundo2command.h"
#include "kistest.h"

#include <KoColor.h>

struct ColorizeMaskTester
{
    ColorizeMaskTester()
        : refRect(0,0,200,200),
          p(refRect)
    {
        KisPaintDeviceSP src = p.layer->paintDevice();

        fillRect = kisGrowRect(refRect, -20);
        internalFillRect = kisGrowRect(fillRect, -10);
        src->fill(fillRect, KoColor(Qt::black, src->colorSpace()));
        src->fill(internalFillRect, KoColor(Qt::transparent, src->colorSpace()));
        src->fill(QRect(100, 10, 10, 130), KoColor(Qt::black, src->colorSpace()));

        // KIS_DUMP_DEVICE_2(src, refRect, "src", "dd");

        mask = new KisColorizeMask(p.image, "mask1");
        p.image->addNode(mask, p.layer);

        mask->initializeCompositeOp();

        {
            KisPaintDeviceSP key1 = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
            key1->fill(QRect(50,50,10,20), KoColor(Qt::black, key1->colorSpace()));
            mask->testingAddKeyStroke(key1, KoColor(Qt::green, src->colorSpace()));
            // KIS_DUMP_DEVICE_2(key1, refRect, "key1", "dd");
        }

        {
            KisPaintDeviceSP key2 = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
            key2->fill(QRect(150,50,10,20), KoColor(Qt::black, key2->colorSpace()));
            mask->testingAddKeyStroke(key2, KoColor(Qt::red, src->colorSpace()));
            // KIS_DUMP_DEVICE_2(key2, refRect, "key2", "dd");
        }

        {
            KisPaintDeviceSP key3 = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
            key3->fill(QRect(0,0,10,10), KoColor(Qt::black, key3->colorSpace()));
            mask->testingAddKeyStroke(key3, KoColor(Qt::blue, src->colorSpace()), true);
            // KIS_DUMP_DEVICE_2(key3, refRect, "key3", "dd");
        }

        mask->resetCache();

        mask->testingRegenerateMask();
        p.image->waitForDone();

        // KIS_DUMP_DEVICE_2(mask->coloringProjection(), refRect, "coloring", "dd");
        // KIS_DUMP_DEVICE_2(mask->paintDevice(), refRect, "paintDevice", "dd");
        // KIS_DUMP_DEVICE_2(mask->testingFilteredSource(), refRect, "filteredSource", "dd");
    }

    QRect refRect;
    TestUtil::MaskParent p;
    KisColorizeMaskSP mask;

    QRect fillRect;
    QRect internalFillRect;
};


void KisColorizeMaskTest::test()
{
    ColorizeMaskTester t;

    QList<KisLazyFillTools::KeyStroke> strokes;

    strokes = t.mask->fetchKeyStrokesDirect();

    const QRect expectedFillRect(25,25,145,150);

    // Check initial bounding rects

    QCOMPARE(t.mask->paintDevice()->exactBounds(), QRect(0,0,160,70));
    QCOMPARE(t.mask->coloringProjection()->exactBounds(), expectedFillRect);
    QCOMPARE(t.mask->testingFilteredSource()->exactBounds(), t.refRect);

    QCOMPARE(strokes[0].dev->exactBounds(), QRect(50,50,10,20));
    QCOMPARE(strokes[1].dev->exactBounds(), QRect(150,50,10,20));
    QCOMPARE(strokes[2].dev->exactBounds(), QRect(0,0,10,10));

    // Move the t.mask: the filled area is also expected to be moved!
    t.mask->setX(5);
    t.mask->setY(7);

    // KIS_DUMP_DEVICE_2(t.mask->coloringProjection(), t.refRect, "coloring2", "dd");
    // KIS_DUMP_DEVICE_2(t.mask->paintDevice(), t.refRect, "paintDevice2", "dd");
    // KIS_DUMP_DEVICE_2(t.mask->testingFilteredSource(), t.refRect, "filteredSource2", "dd");

    QCOMPARE(t.mask->paintDevice()->exactBounds(), QRect(5,7,160,70));
    QCOMPARE(t.mask->coloringProjection()->exactBounds(), expectedFillRect.translated(5, 7));
    QCOMPARE(t.mask->testingFilteredSource()->exactBounds(), t.refRect);

    QCOMPARE(strokes[0].dev->exactBounds(), QRect(55,57,10,20));
    QCOMPARE(strokes[1].dev->exactBounds(), QRect(155,57,10,20));
    QCOMPARE(strokes[2].dev->exactBounds(), QRect(5,7,10,10));

    // Test changing t.mask color space

    const KoColorSpace *oldCS = KoColorSpaceRegistry::instance()->rgb8();
    const KoColorSpace *newCS = KoColorSpaceRegistry::instance()->lab16();

    QCOMPARE(t.mask->colorSpace(), oldCS);
    QCOMPARE(t.mask->paintDevice()->colorSpace(), oldCS);
    QCOMPARE(t.mask->coloringProjection()->colorSpace(), oldCS);

    QCOMPARE(strokes[0].color.colorSpace(), oldCS);
    QCOMPARE(strokes[1].color.colorSpace(), oldCS);
    QCOMPARE(strokes[2].color.colorSpace(), oldCS);

    QScopedPointer<KUndo2Command> cmd(t.mask->setColorSpace(newCS));
    cmd->redo();
    strokes = t.mask->fetchKeyStrokesDirect();

    QCOMPARE(t.mask->colorSpace(), newCS);
    QCOMPARE(t.mask->paintDevice()->colorSpace(), newCS);
    QCOMPARE(t.mask->coloringProjection()->colorSpace(), newCS);

    QCOMPARE(strokes[0].color.colorSpace(), newCS);
    QCOMPARE(strokes[1].color.colorSpace(), newCS);
    QCOMPARE(strokes[2].color.colorSpace(), newCS);

    cmd->undo();
    strokes = t.mask->fetchKeyStrokesDirect();

    QCOMPARE(t.mask->colorSpace(), oldCS);
    QCOMPARE(t.mask->paintDevice()->colorSpace(), oldCS);
    QCOMPARE(t.mask->coloringProjection()->colorSpace(), oldCS);

    QCOMPARE(strokes[0].color.colorSpace(), oldCS);
    QCOMPARE(strokes[1].color.colorSpace(), oldCS);
    QCOMPARE(strokes[2].color.colorSpace(), oldCS);

    cmd->redo();
    strokes = t.mask->fetchKeyStrokesDirect();

    QCOMPARE(t.mask->colorSpace(), newCS);
    QCOMPARE(t.mask->paintDevice()->colorSpace(), newCS);
    QCOMPARE(t.mask->coloringProjection()->colorSpace(), newCS);

    QCOMPARE(strokes[0].color.colorSpace(), newCS);
    QCOMPARE(strokes[1].color.colorSpace(), newCS);
    QCOMPARE(strokes[2].color.colorSpace(), newCS);
}

#include "processing/kis_crop_processing_visitor.h"

void KisColorizeMaskTest::testCrop()
{
    ColorizeMaskTester t;
    QList<KisLazyFillTools::KeyStroke> strokes;
    strokes = t.mask->fetchKeyStrokesDirect();

    const QRect expectedFillRect(25,25,145,150);

    // Check initial bounding rects

    QCOMPARE(t.mask->paintDevice()->exactBounds(), QRect(0,0,160,70));
    QCOMPARE(t.mask->coloringProjection()->exactBounds(), expectedFillRect);
    QCOMPARE(t.mask->testingFilteredSource()->exactBounds(), t.refRect);

    // KIS_DUMP_DEVICE_2(t.mask->coloringProjection(), t.refRect, "coloring3", "dd");
    // KIS_DUMP_DEVICE_2(t.mask->paintDevice(), t.refRect, "paintDevice3", "dd");
    // KIS_DUMP_DEVICE_2(t.mask->testingFilteredSource(), t.refRect, "filteredSource3", "dd");

    QCOMPARE(strokes[0].dev->exactBounds(), QRect(50,50,10,20));
    QCOMPARE(strokes[1].dev->exactBounds(), QRect(150,50,10,20));
    QCOMPARE(strokes[2].dev->exactBounds(), QRect(0,0,10,10));

    QRect cropRect(5,5,150,55);
    KisCropProcessingVisitor visitor(cropRect, true, true);
    t.mask->accept(visitor, t.p.image->undoAdapter());

    // KIS_DUMP_DEVICE_2(t.mask->coloringProjection(), t.refRect, "coloring4", "dd");
    // KIS_DUMP_DEVICE_2(t.mask->paintDevice(), t.refRect, "paintDevice4", "dd");
    // KIS_DUMP_DEVICE_2(t.mask->testingFilteredSource(), t.refRect, "filteredSource4", "dd");

    QCOMPARE(t.mask->paintDevice()->exactBounds(), QRect(0,0,150,55));
    QCOMPARE(t.mask->coloringProjection()->exactBounds(), (expectedFillRect & cropRect).translated(-cropRect.topLeft()));
    QCOMPARE(t.mask->testingFilteredSource()->exactBounds(), t.refRect);

    QCOMPARE(strokes[0].dev->exactBounds(), QRect(45,45,10,10));
    QCOMPARE(strokes[1].dev->exactBounds(), QRect(145,45,5,10));
    QCOMPARE(strokes[2].dev->exactBounds(), QRect(0,0,5,5));
}

KISTEST_MAIN(KisColorizeMaskTest)
