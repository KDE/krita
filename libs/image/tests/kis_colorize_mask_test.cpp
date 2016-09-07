/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_colorize_mask_test.h"

#include <QTest>

#include "testutil.h"
#include "lazybrush/kis_colorize_mask.h"
#include "kis_paint_device_debug_utils.h"
#include "kis_global.h"
#include "lazybrush/kis_lazy_fill_tools.h"
#include "kundo2command.h"

#include <KoColor.h>


void KisColorizeMaskTest::test()
{
    QRect refRect(0,0,200,200);
    TestUtil::MaskParent p(refRect);

    KisPaintDeviceSP src = p.layer->paintDevice();

    QRect fillRect = kisGrowRect(refRect, -20);
    QRect internalFillRect = kisGrowRect(fillRect, -10);
    src->fill(fillRect, KoColor(Qt::black, src->colorSpace()));
    src->fill(internalFillRect, KoColor(Qt::transparent, src->colorSpace()));
    src->fill(QRect(100, 10, 10, 130), KoColor(Qt::black, src->colorSpace()));

    // KIS_DUMP_DEVICE_2(src, refRect, "src", "dd");

    KisColorizeMaskSP mask = new KisColorizeMask();
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

    QList<KisLazyFillTools::KeyStroke> strokes;

    strokes = mask->testingKeyStrokes();

    // Check initial bounding rects

    QCOMPARE(mask->paintDevice()->exactBounds(), QRect(0,0,160,70));
    QCOMPARE(mask->coloringProjection()->exactBounds(), internalFillRect);
    QCOMPARE(mask->testingFilteredSource()->exactBounds(), refRect);

    QCOMPARE(strokes[0].dev->exactBounds(), QRect(50,50,10,20));
    QCOMPARE(strokes[1].dev->exactBounds(), QRect(150,50,10,20));
    QCOMPARE(strokes[2].dev->exactBounds(), QRect(0,0,10,10));

    // Move the mask: the filled area is also expected to be moved!
    mask->setX(5);
    mask->setY(7);

    // KIS_DUMP_DEVICE_2(mask->coloringProjection(), refRect, "coloring2", "dd");
    // KIS_DUMP_DEVICE_2(mask->paintDevice(), refRect, "paintDevice2", "dd");
    // KIS_DUMP_DEVICE_2(mask->testingFilteredSource(), refRect, "filteredSource2", "dd");

    QCOMPARE(mask->paintDevice()->exactBounds(), QRect(5,7,160,70));
    QCOMPARE(mask->coloringProjection()->exactBounds(), internalFillRect.translated(5, 7));
    QCOMPARE(mask->testingFilteredSource()->exactBounds(), refRect);

    QCOMPARE(strokes[0].dev->exactBounds(), QRect(55,57,10,20));
    QCOMPARE(strokes[1].dev->exactBounds(), QRect(155,57,10,20));
    QCOMPARE(strokes[2].dev->exactBounds(), QRect(5,7,10,10));

    // Test changing mask color space

    const KoColorSpace *oldCS = KoColorSpaceRegistry::instance()->rgb8();
    const KoColorSpace *newCS = KoColorSpaceRegistry::instance()->lab16();

    QCOMPARE(mask->colorSpace(), oldCS);
    QCOMPARE(mask->paintDevice()->colorSpace(), oldCS);
    QCOMPARE(mask->coloringProjection()->colorSpace(), oldCS);

    QCOMPARE(strokes[0].color.colorSpace(), oldCS);
    QCOMPARE(strokes[1].color.colorSpace(), oldCS);
    QCOMPARE(strokes[2].color.colorSpace(), oldCS);

    KUndo2Command *cmd = mask->setColorSpace(newCS);
    cmd->redo();
    strokes = mask->testingKeyStrokes();

    QCOMPARE(mask->colorSpace(), newCS);
    QCOMPARE(mask->paintDevice()->colorSpace(), newCS);
    QCOMPARE(mask->coloringProjection()->colorSpace(), newCS);

    QCOMPARE(strokes[0].color.colorSpace(), newCS);
    QCOMPARE(strokes[1].color.colorSpace(), newCS);
    QCOMPARE(strokes[2].color.colorSpace(), newCS);

    cmd->undo();
    strokes = mask->testingKeyStrokes();

    QCOMPARE(mask->colorSpace(), oldCS);
    QCOMPARE(mask->paintDevice()->colorSpace(), oldCS);
    QCOMPARE(mask->coloringProjection()->colorSpace(), oldCS);

    QCOMPARE(strokes[0].color.colorSpace(), oldCS);
    QCOMPARE(strokes[1].color.colorSpace(), oldCS);
    QCOMPARE(strokes[2].color.colorSpace(), oldCS);

    cmd->redo();
    strokes = mask->testingKeyStrokes();

    QCOMPARE(mask->colorSpace(), newCS);
    QCOMPARE(mask->paintDevice()->colorSpace(), newCS);
    QCOMPARE(mask->coloringProjection()->colorSpace(), newCS);

    QCOMPARE(strokes[0].color.colorSpace(), newCS);
    QCOMPARE(strokes[1].color.colorSpace(), newCS);
    QCOMPARE(strokes[2].color.colorSpace(), newCS);
}

QTEST_MAIN(KisColorizeMaskTest)
