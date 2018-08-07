/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_file_layer_test.h"

#include <QTest>

#include <KoColor.h>

#include <kis_file_layer.h>
#include <kis_transform_mask.h>
#include <kis_transform_mask_params_interface.h>

#include <testutil.h>

#include "config-limit-long-tests.h"

void waitForMaskUpdates(KisNodeSP root) {
#ifdef LIMIT_LONG_TESTS
    KisLayerUtils::forceAllDelayedNodesUpdate(root);
    QTest::qWait(100);
#else /* LIMIT_LONG_TESTS */
    Q_UNUSED(root);
    QTest::qWait(100);
#endif /* LIMIT_LONG_TESTS */
}

void KisFileLayerTest::testFileLayerPlusTransformMaskOffImage()
{
    TestUtil::ReferenceImageChecker chk("flayer_tmask_offimage", "file_layer");

    QRect refRect(0,0,640,441);
    QRect fillRect(400,400,100,100);
    TestUtil::MaskParent p(refRect);

    QString refName(TestUtil::fetchDataFileLazy("hakonepa.png"));
    KisLayerSP flayer = new KisFileLayer(p.image, "", refName, KisFileLayer::None, "flayer", OPACITY_OPAQUE_U8);

    p.image->addNode(flayer, p.image->root(), KisNodeSP());

    waitForMaskUpdates(p.image->root());
    p.image->waitForDone();

    KisTransformMaskSP mask1 = new KisTransformMask();
    p.image->addNode(mask1, flayer);

    mask1->setName("mask1");

    flayer->setDirty(refRect);
    p.image->waitForDone();
    chk.checkImage(p.image, "00_initial_layer_update");

    waitForMaskUpdates(p.image->root());
    p.image->waitForDone();
    chk.checkImage(p.image, "00X_initial_layer_update");


    flayer->setX(580);
    flayer->setY(400);

    flayer->setDirty(refRect);
    p.image->waitForDone();
    chk.checkImage(p.image, "01_file_layer_moved");

    waitForMaskUpdates(p.image->root());
    p.image->waitForDone();
    chk.checkImage(p.image, "01X_file_layer_moved");


    QTransform transform = QTransform::fromTranslate(-580, -400);
    mask1->setTransformParams(KisTransformMaskParamsInterfaceSP(
                                  new KisDumbTransformMaskParams(transform)));


    /**
     * NOTE: here we see our image cropped by 1.5 image size rect!
     *       That is expected and controlled by
     *       KisImageConfig::transformMaskOffBoundsReadArea()
     *       parameter
     */

    mask1->setDirty(refRect);
    p.image->waitForDone();
    chk.checkImage(p.image, "02_mask1_moved_mask_update");

    waitForMaskUpdates(p.image->root());
    p.image->waitForDone();
    chk.checkImage(p.image, "02X_mask1_moved_mask_update");

    QVERIFY(chk.testPassed());
}

void KisFileLayerTest::testFileLayerPlusTransformMaskSmallFileBigOffset()
{
    TestUtil::ReferenceImageChecker chk("flayer_tmask_huge_offset", "file_layer");

    QRect refRect(0,0,2000,1500);
    QRect fillRect(400,400,100,100);
    TestUtil::MaskParent p(refRect);

    QString refName(TestUtil::fetchDataFileLazy("file_layer_source.png"));
    KisLayerSP flayer = new KisFileLayer(p.image, "", refName, KisFileLayer::None, "flayer", OPACITY_OPAQUE_U8);

    p.image->addNode(flayer, p.image->root(), KisNodeSP());

    waitForMaskUpdates(p.image->root());
    p.image->waitForDone();

    // check whether the default bounds of the file layer are
    // initialized properly
    QCOMPARE(flayer->original()->defaultBounds()->bounds(), p.image->bounds());

    KisTransformMaskSP mask1 = new KisTransformMask();
    p.image->addNode(mask1, flayer);

    mask1->setName("mask1");

    flayer->setDirty(refRect);
    p.image->waitForDone();
    chk.checkImage(p.image, "00_initial_layer_update");

    waitForMaskUpdates(p.image->root());
    p.image->waitForDone();
    chk.checkImage(p.image, "00X_initial_layer_update");

    QTransform transform;

    transform = QTransform::fromTranslate(1200, 300);
    mask1->setTransformParams(KisTransformMaskParamsInterfaceSP(
                                  new KisDumbTransformMaskParams(transform)));

    mask1->setDirty(refRect);
    p.image->waitForDone();
    chk.checkImage(p.image, "01_mask1_moved_mask_update");

    waitForMaskUpdates(p.image->root());
    p.image->waitForDone();
    chk.checkImage(p.image, "01X_mask1_moved_mask_update");

    QVERIFY(chk.testPassed());
}

KISTEST_MAIN(KisFileLayerTest)
