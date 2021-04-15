/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_transform_mask_test.h"

#include <simpletest.h>

#include <KoColor.h>


#include "kis_transform_mask.h"
#include "kis_transform_mask_params_interface.h"

#include "testutil.h"
#include "kistest.h"

#include "kis_algebra_2d.h"
#include "kis_safe_transform.h"
#include "kis_clone_layer.h"
#include "kis_group_layer.h"
#include "kis_paint_device_debug_utils.h"



inline QString toOctaveFormat(const QTransform &t)
{
    QString s("T = [%1 %2 %3; %4 %5 %6; %7 %8 %9]");
    s = s
        .arg(t.m11()).arg(t.m12()).arg(t.m13())
        .arg(t.m21()).arg(t.m22()).arg(t.m23())
        .arg(t.m31()).arg(t.m32()).arg(t.m33());

    return s;
}

void KisTransformMaskTest::testSafeTransform()
{
    QTransform transform(-0.177454, -0.805953, -0.00213713,
                         -1.9295, -0.371835, -0.00290463,
                         3075.05, 2252.32, 7.62371);

    QRectF testRect(0, 1024, 512, 512);
    KisSafeTransform t2(transform, QRect(0, 0, 2048, 2048), testRect.toRect());

    QPolygonF fwdPoly = t2.mapForward(testRect);
    QRectF fwdRect = t2.mapRectForward(testRect);

    QPolygonF bwdPoly = t2.mapBackward(fwdPoly);
    QRectF bwdRect = t2.mapRectBackward(fwdRect);

    QPolygon ref;

    ref.clear();
    ref << QPoint(284, 410);
    ref << QPoint(10, 613);
    ref << QPoint(35, 532);
    ref << QPoint(236, 403);
    ref << QPoint(284, 410);
    QCOMPARE(fwdPoly.toPolygon(), ref);
    QCOMPARE(fwdRect.toRect(), QRect(10,403,274,211));

    ref.clear();
    ref << QPoint(512, 1024);
    ref << QPoint(512, 1536);
    ref << QPoint(0, 1536);
    ref << QPoint(0, 1024);
    ref << QPoint(512, 1024);
    QCOMPARE(bwdPoly.toPolygon(), ref);
    QCOMPARE(bwdRect.toRect(), QRect(0, 994, 1198, 584));

/*
    QImage image(2500, 2500, QImage::Format_ARGB32);
    QPainter gc(&image);
    gc.setPen(Qt::cyan);

    gc.setOpacity(0.7);

    gc.setBrush(Qt::red);
    gc.drawPolygon(t2.srcClipPolygon());

    gc.setBrush(Qt::green);
    gc.drawPolygon(t2.dstClipPolygon());

    dbgKrita << ppVar(testRect);
    dbgKrita << ppVar(fwdPoly);
    dbgKrita << ppVar(fwdRect);
    dbgKrita << ppVar(bwdPoly);
    dbgKrita << ppVar(bwdRect);

    gc.setBrush(Qt::yellow);
    gc.drawPolygon(testRect);

    gc.setBrush(Qt::red);
    gc.drawPolygon(fwdRect);
    gc.setBrush(Qt::blue);
    gc.drawPolygon(fwdPoly);

    gc.setBrush(Qt::magenta);
    gc.drawPolygon(bwdRect);
    gc.setBrush(Qt::cyan);
    gc.drawPolygon(bwdPoly);

    gc.end();
    image.save("polygons_safety.png");
*/
}

void KisTransformMaskTest::testSafeTransformUnity()
{
    QTransform transform;

    QRectF testRect(0, 1024, 512, 512);
    KisSafeTransform t2(transform, QRect(0, 0, 2048, 2048), testRect.toRect());

    QPolygonF fwdPoly = t2.mapForward(testRect);
    QRectF fwdRect = t2.mapRectForward(testRect);

    QPolygonF bwdPoly = t2.mapBackward(fwdPoly);
    QRectF bwdRect = t2.mapRectBackward(fwdRect);

    QCOMPARE(testRect, fwdRect);
    QCOMPARE(testRect, bwdRect);
    QCOMPARE(fwdPoly, QPolygonF(testRect));
    QCOMPARE(bwdPoly, QPolygonF(testRect));
}

void KisTransformMaskTest::testSafeTransformSingleVanishingPoint()
{
    // rotation around 0X has a single vanishing point for 0Y axis
    QTransform transform(1, 0, 0,
                         -0.870208, -0.414416, -0.000955222,
                         132.386, 1082.91, 1.99439);

    QTransform R; R.rotateRadians(M_PI / 4.0);
    //transform *= R;

    QRectF testRect(1536, 1024, 512, 512);
    KisSafeTransform t2(transform, QRect(0, 0, 2048, 2048), testRect.toRect());

    QPolygonF fwdPoly = t2.mapForward(testRect);
    QRectF fwdRect = t2.mapRectForward(testRect);

    QPolygonF bwdPoly = t2.mapBackward(fwdPoly);
    QRectF bwdRect = t2.mapRectBackward(fwdRect);

    /**
     * A special weird rect that crosses the vanishing point,
     * which is (911.001, 433.84) in this case
     */
    QRectF fwdNastyRect(800, 100, 400, 600);
    //QRectF fwdNastyRect(100, 400, 1000, 800);
    QRectF bwdNastyRect = t2.mapRectBackward(fwdNastyRect);

/*
    dbgKrita << ppVar(testRect);
    dbgKrita << ppVar(fwdPoly);
    dbgKrita << ppVar(fwdRect);
    dbgKrita << ppVar(bwdPoly);
    dbgKrita << ppVar(bwdRect);
    dbgKrita << ppVar(bwdNastyRect);
*/

    QPolygon ref;

    ref.clear();
    ref << QPoint(765,648);
    ref << QPoint(1269, 648);
    ref << QPoint(1601, 847);
    ref << QPoint(629, 847);
    ref << QPoint(765, 648);
    QCOMPARE(fwdPoly.toPolygon(), ref);
    QCOMPARE(fwdRect.toRect(), QRect(629,648,971,199));

    ref.clear();
    ref << QPoint(1536,1024);
    ref << QPoint(2048,1024);
    ref << QPoint(2048,1536);
    ref << QPoint(1536,1536);
    ref << QPoint(1536,1024);
    QCOMPARE(bwdPoly.toPolygon(), ref);
    QCOMPARE(bwdRect.toRect(), QRect(1398,1024,650,512));

    QCOMPARE(bwdNastyRect.toRect(), QRect(1463,0,585,1232));
}

bool doPartialTests(const QString &prefix, KisImageSP image, KisLayerSP paintLayer,
                    KisLayerSP visibilityToggleLayer, KisTransformMaskSP mask)
{
    TestUtil::ReferenceImageChecker chk(prefix, "transform_mask_updates");

    bool result = true;

    QRect refRect = image->bounds();

    int testIndex = 1;
    QString testName;

    for (int y = 0; y < refRect.height(); y += 512) {
        for (int x = 0; x < refRect.width(); x += 512) {
            QRect rc(x, y, 512, 512);

            if (rc.right() > refRect.right()) {
                rc.setRight(refRect.right());
                if (rc.isEmpty()) continue;
            }

            if (rc.bottom() > refRect.bottom()) {
                rc.setBottom(refRect.bottom());
                if (rc.isEmpty()) continue;
            }

            paintLayer->setDirty(rc);
            image->waitForDone();
            testName = QString("tm_%1_partial_%2_%3").arg(testIndex++).arg(x).arg(y);
            result &= chk.checkImage(image, testName);
        }
    }

    // initial update of the mask to clear the unused portions of the projection
    // (it updates only when we call set dirty on the mask itself, which happens
    // in Krita right after the addition of the mask onto a layer)

    mask->setDirty();
    image->waitForDone();
    testName = QString("tm_%1_initial_mask_visible_on").arg(testIndex++);
    result &= chk.checkImage(image, testName);

    // start layer visibility testing

    paintLayer->setVisible(false);
    paintLayer->setDirty();
    image->waitForDone();
    testName = QString("tm_%1_layer_visible_off").arg(testIndex++);
    result &= chk.checkImage(image, testName);

    paintLayer->setVisible(true);
    paintLayer->setDirty();
    image->waitForDone();
    testName = QString("tm_%1_layer_visible_on").arg(testIndex++);
    result &= chk.checkImage(image, testName);

    if (paintLayer != visibilityToggleLayer) {
        visibilityToggleLayer->setVisible(false);
        visibilityToggleLayer->setDirty();
        image->waitForDone();
        testName = QString("tm_%1_extra_layer_visible_off").arg(testIndex++);
        result &= chk.checkImage(image, testName);


        visibilityToggleLayer->setVisible(true);
        visibilityToggleLayer->setDirty();
        image->waitForDone();
        testName = QString("tm_%1_extra_layer_visible_on").arg(testIndex++);
        result &= chk.checkImage(image, testName);
    }

    // toggle mask visibility

    mask->setVisible(false);
    mask->setDirty();
    image->waitForDone();
    testName = QString("tm_%1_mask_visible_off").arg(testIndex++);
    result &= chk.checkImage(image, testName);

    mask->setVisible(true);
    mask->setDirty();
    image->waitForDone();
    testName = QString("tm_%1_mask_visible_on").arg(testIndex++);
    result &= chk.checkImage(image, testName);

    // entire bounds update

    // no clearing, just don't hang up

    paintLayer->setDirty(refRect);
    image->waitForDone();
    testName = QString("tm_%1_layer_dirty_bounds").arg(testIndex++);
    result &= chk.checkImage(image, testName);

    // no clearing, just don't hang up

    mask->setDirty(refRect);
    image->waitForDone();
    testName = QString("tm_%1_mask_dirty_bounds").arg(testIndex++);
    result &= chk.checkImage(image, testName);

    if (paintLayer != visibilityToggleLayer) {
        // no clearing, just don't hang up

        visibilityToggleLayer->setDirty(refRect);
        image->waitForDone();
        testName = QString("tm_%1_extra_layer_dirty_bounds").arg(testIndex++);
        result &= chk.checkImage(image, testName);
    }

    QRect fillRect;

    // partial updates outside

    fillRect = QRect(-100, 0.5 * refRect.height(), 50, 100);
    paintLayer->paintDevice()->fill(fillRect, KoColor(Qt::red, image->colorSpace()));
    paintLayer->setDirty(fillRect);
    image->waitForDone();
    testName = QString("tm_%1_layer_dirty_outside_%2_%3").arg(testIndex++).arg(fillRect.x()).arg(fillRect.y());
    result &= chk.checkImage(image, testName);

    fillRect = QRect(0.5 * refRect.width(), -100, 100, 50);
    paintLayer->paintDevice()->fill(fillRect, KoColor(Qt::red, image->colorSpace()));
    paintLayer->setDirty(fillRect);
    image->waitForDone();
    testName = QString("tm_%1_layer_dirty_outside_%2_%3").arg(testIndex++).arg(fillRect.x()).arg(fillRect.y());
    result &= chk.checkImage(image, testName);

    fillRect = QRect(refRect.width() + 50, 0.2 * refRect.height(), 50, 100);
    paintLayer->paintDevice()->fill(fillRect, KoColor(Qt::red, image->colorSpace()));
    paintLayer->setDirty(fillRect);
    image->waitForDone();
    testName = QString("tm_%1_layer_dirty_outside_%2_%3").arg(testIndex++).arg(fillRect.x()).arg(fillRect.y());
    result &= chk.checkImage(image, testName);

    // partial update inside

    fillRect = QRect(0.5 * refRect.width() - 50, 0.5 * refRect.height() - 50, 100, 100);
    paintLayer->paintDevice()->fill(fillRect, KoColor(Qt::red, image->colorSpace()));
    paintLayer->setDirty(fillRect);
    image->waitForDone();
    testName = QString("tm_%1_layer_dirty_inside_%2_%3").arg(testIndex++).arg(fillRect.x()).arg(fillRect.y());
    result &= chk.checkImage(image, testName);

    // clear explicitly
    image->projection()->clear();

    mask->setDirty();
    image->waitForDone();
    testName = QString("tm_%1_mask_dirty_bounds").arg(testIndex++);
    result &= chk.checkImage(image, testName);


    KisDumbTransformMaskParams *params =
        dynamic_cast<KisDumbTransformMaskParams*>(mask->transformParams().data());

    QTransform t = params->testingGetTransform();
    t *= QTransform::fromTranslate(400, 300);
    params->testingSetTransform(t);
    mask->setTransformParams(mask->transformParams());

    mask->setDirty();
    image->waitForDone();
    testName = QString("tm_%1_mask_dirty_after_offset").arg(testIndex++);
    result &= chk.checkImage(image, testName);

    return result;
}

void KisTransformMaskTest::testMaskOnPaintLayer()
{
    QImage refImage(TestUtil::fetchDataFileLazy("test_transform_quality.png"));
    QRect refRect = refImage.rect();
    TestUtil::MaskParent p(refRect);

    p.layer->paintDevice()->convertFromQImage(refImage, 0);

    KisPaintLayerSP player = new KisPaintLayer(p.image, "bg", OPACITY_OPAQUE_U8, p.image->colorSpace());
    p.image->addNode(player, p.image->root(), KisNodeSP());

    KisTransformMaskSP mask = new KisTransformMask(p.image, "mask");
    p.image->addNode(mask, p.layer);

    QTransform transform(-0.177454, -0.805953, -0.00213713,
                         -1.9295, -0.371835, -0.00290463,
                         3075.05, 2252.32, 7.62371);

    mask->setTransformParams(KisTransformMaskParamsInterfaceSP(
                                 new KisDumbTransformMaskParams(transform)));

    QVERIFY(doPartialTests("pl", p.image, p.layer, p.layer, mask));
}

void KisTransformMaskTest::testMaskOnCloneLayer()
{
    QImage refImage(TestUtil::fetchDataFileLazy("test_transform_quality.png"));
    QRect refRect = refImage.rect();
    TestUtil::MaskParent p(refRect);

    p.layer->paintDevice()->convertFromQImage(refImage, 0);

    KisPaintLayerSP player = new KisPaintLayer(p.image, "bg", OPACITY_OPAQUE_U8, p.image->colorSpace());
    p.image->addNode(player, p.image->root(), KisNodeSP());

    KisCloneLayerSP clone = new KisCloneLayer(p.layer, p.image, "clone", OPACITY_OPAQUE_U8);
    p.image->addNode(clone, p.image->root());

    KisTransformMaskSP mask = new KisTransformMask(p.image, "mask");
    p.image->addNode(mask, clone);

    QTransform transform(-0.177454, -0.805953, -0.00213713,
                         -1.9295, -0.371835, -0.00290463,
                         3075.05, 2252.32, 7.62371);

    mask->setTransformParams(KisTransformMaskParamsInterfaceSP(
                                 new KisDumbTransformMaskParams(transform)));

    QVERIFY(doPartialTests("cl", p.image, p.layer, clone, mask));
}

void KisTransformMaskTest::testMaskOnCloneLayerWithOffset()
{
    TestUtil::ReferenceImageChecker chk("clone_offset_simple", "transform_mask_updates");

    QRect refRect(0,0,512,512);
    QRect fillRect(400,400,100,100);
    TestUtil::MaskParent p(refRect);

    p.layer->paintDevice()->fill(fillRect, KoColor(Qt::red, p.layer->colorSpace()));

    KisPaintLayerSP player = new KisPaintLayer(p.image, "bg", OPACITY_OPAQUE_U8, p.image->colorSpace());
    p.image->addNode(player, p.image->root(), KisNodeSP());

    KisCloneLayerSP clone = new KisCloneLayer(p.layer, p.image, "clone", OPACITY_OPAQUE_U8);
    p.image->addNode(clone, p.image->root());

    KisTransformMaskSP mask = new KisTransformMask(p.image, "mask");
    p.image->addNode(mask, clone);

    QTransform transform(1, 0, 0,
                         0, 1, 0,
                         0, -150, 1);

    mask->setTransformParams(KisTransformMaskParamsInterfaceSP(
                                 new KisDumbTransformMaskParams(transform)));

    p.layer->setDirty(refRect);
    p.image->waitForDone();
    chk.checkImage(p.image, "0_initial");

    clone->setX(-300);
    clone->setDirty();
    p.image->waitForDone();
    chk.checkImage(p.image, "1_after_offset");

    mask->setDirty();
    p.image->waitForDone();
    chk.checkImage(p.image, "2_after_offset_dirty_mask");

    QTest::qWait(4000);
    chk.checkImage(p.image, "3_delayed_regeneration");

    KisPaintDeviceSP previewDevice = mask->buildPreviewDevice();
    chk.checkDevice(previewDevice, p.image, "4_preview_device");

    QVERIFY(chk.testPassed());

    QVERIFY(doPartialTests("clone_offset_complex", p.image, p.layer, clone, mask));
}

#define CHECK_MASK1_TOGGLE
#define CHECK_MASK2_TOGGLE
#define CHECK_HIDE_ALL
#define CHECK_HIDE_ALL_AFTER_MOVE

void KisTransformMaskTest::testMultipleMasks()
{
    TestUtil::ReferenceImageChecker chk("multiple_masks", "transform_mask_updates");

    QRect refRect(0,0,512,512);
    QRect fillRect(400,400,100,100);
    TestUtil::MaskParent p(refRect);

    p.layer->paintDevice()->fill(fillRect, KoColor(Qt::red, p.layer->colorSpace()));

    KisPaintLayerSP player = new KisPaintLayer(p.image, "bg", OPACITY_OPAQUE_U8, p.image->colorSpace());
    p.image->addNode(player, p.image->root(), KisNodeSP());

    //KisCloneLayerSP clone = new KisCloneLayer(p.layer, p.image, "clone", OPACITY_OPAQUE_U8);
    //p.image->addNode(clone, p.image->root());

    KisTransformMaskSP mask1 = new KisTransformMask(p.image, "mask1");
    p.image->addNode(mask1, p.layer);

    KisTransformMaskSP mask2 = new KisTransformMask(p.image, "mask2");
    p.image->addNode(mask2, p.layer);

    p.layer->setDirty(refRect);
    p.image->waitForDone();
    chk.checkImage(p.image, "00_initial_layer_update");

    QTransform transform;

    transform = QTransform::fromTranslate(-150, 0);
    mask1->setTransformParams(KisTransformMaskParamsInterfaceSP(
                                  new KisDumbTransformMaskParams(transform)));

    p.layer->setDirty(refRect);
    p.image->waitForDone();
    chk.checkImage(p.image, "01_mask1_moved_layer_update");

    QTest::qWait(4000);
    p.image->waitForDone();
    chk.checkImage(p.image, "01X_mask1_moved_layer_update");


    transform = QTransform::fromTranslate(0, -150);
    mask2->setTransformParams(KisTransformMaskParamsInterfaceSP(
                                  new KisDumbTransformMaskParams(transform)));

    p.layer->setDirty(refRect);
    p.image->waitForDone();
    chk.checkImage(p.image, "02_mask2_moved_layer_update");

    QTest::qWait(4000);
    p.image->waitForDone();
    chk.checkImage(p.image, "02X_mask2_moved_layer_update");

#ifdef CHECK_MASK1_TOGGLE

    {
        mask1->setVisible(false);
        mask1->setDirty(refRect);
        p.image->waitForDone();
        chk.checkImage(p.image, "03_mask1_tg_off_refRect");

        QTest::qWait(4000);
        p.image->waitForDone();
        chk.checkImage(p.image, "03X_mask1_tg_off_refRect");


        mask1->setVisible(true);
        mask1->setDirty(refRect);
        p.image->waitForDone();
        chk.checkImage(p.image, "04_mask1_tg_on_refRect");

        QTest::qWait(4000);
        p.image->waitForDone();
        chk.checkImage(p.image, "04X_mask1_tg_on_refRect");


        mask1->setVisible(false);
        mask1->setDirty();
        p.image->waitForDone();
        chk.checkImage(p.image, "05_mask1_tg_off_default_rect");

        QTest::qWait(4000);
        p.image->waitForDone();
        chk.checkImage(p.image, "05X_mask1_tg_off_default_rect");


        mask1->setVisible(true);
        mask1->setDirty();
        p.image->waitForDone();
        chk.checkImage(p.image, "06_mask1_tg_on_default_rect");

        QTest::qWait(4000);
        p.image->waitForDone();
        chk.checkImage(p.image, "06X_mask1_tg_on_default_rect");
    }
#endif /* CHECK_MASK1_TOGGLE */


#ifdef CHECK_MASK2_TOGGLE

    {
        mask2->setVisible(false);
        mask2->setDirty(refRect);
        p.image->waitForDone();
        chk.checkImage(p.image, "07_mask2_tg_off_refRect");

        QTest::qWait(4000);
        p.image->waitForDone();
        chk.checkImage(p.image, "07X_mask2_tg_off_refRect");


        mask2->setVisible(true);
        mask2->setDirty(refRect);
        p.image->waitForDone();
        chk.checkImage(p.image, "08_mask2_tg_on_refRect");

        QTest::qWait(4000);
        p.image->waitForDone();
        chk.checkImage(p.image, "08X_mask2_tg_on_refRect");

        mask2->setVisible(false);
        mask2->setDirty(refRect);
        p.image->waitForDone();
        chk.checkImage(p.image, "09_mask2_tg_off_default_rect");

        QTest::qWait(4000);
        p.image->waitForDone();
        chk.checkImage(p.image, "09X_mask2_tg_off_default_rect");


        mask2->setVisible(true);
        mask2->setDirty(refRect);
        p.image->waitForDone();
        chk.checkImage(p.image, "10_mask2_tg_on_default_rect");

        QTest::qWait(4000);
        p.image->waitForDone();
        chk.checkImage(p.image, "10X_mask2_tg_on_default_rect");

    }

#endif /* CHECK_MASK2_TOGGLE */


#ifdef CHECK_HIDE_ALL

    {
        mask1->setVisible(false);
        mask1->setDirty();
        p.image->waitForDone();
        chk.checkImage(p.image, "11.1_hide_both_update_default_mask1");

        QTest::qWait(4000);
        p.image->waitForDone();
        chk.checkImage(p.image, "11.1X_hide_both_update_default_mask1");

        mask2->setVisible(false);
        mask2->setDirty();
        p.image->waitForDone();
        chk.checkImage(p.image, "11.2_hide_both_update_default_mask2");

        QTest::qWait(4000);
        p.image->waitForDone();
        chk.checkImage(p.image, "11.2X_hide_both_update_default_mask2");

        mask1->setVisible(true);
        mask1->setDirty();
        p.image->waitForDone();
        chk.checkImage(p.image, "12_sh_mask1_on");

        QTest::qWait(4000);
        p.image->waitForDone();
        chk.checkImage(p.image, "12X_sh_mask1_on");

        mask1->setVisible(false);
        mask1->setDirty();
        p.image->waitForDone();
        chk.checkImage(p.image, "13_sh_mask1_off");

        QTest::qWait(4000);
        p.image->waitForDone();
        chk.checkImage(p.image, "13X_sh_mask1_off");

        mask2->setVisible(true);
        mask2->setDirty();
        p.image->waitForDone();
        chk.checkImage(p.image, "14_sh_mask2_on");

        QTest::qWait(4000);
        p.image->waitForDone();
        chk.checkImage(p.image, "14X_sh_mask2_on");


        mask2->setVisible(false);
        mask2->setDirty();
        p.image->waitForDone();
        chk.checkImage(p.image, "15_sh_mask2_off");

        QTest::qWait(4000);
        p.image->waitForDone();
        chk.checkImage(p.image, "15X_sh_mask2_off");
    }

#endif /* CHECK_HIDE_ALL */

#ifdef CHECK_HIDE_ALL_AFTER_MOVE

    {
        transform = QTransform::fromTranslate(50, -150);
        mask2->setTransformParams(KisTransformMaskParamsInterfaceSP(
                                      new KisDumbTransformMaskParams(transform)));

        mask2->setDirty();
        p.image->waitForDone();
        chk.checkImage(p.image, "20_moved_mask2");

        QTest::qWait(4000);
        p.image->waitForDone();
        chk.checkImage(p.image, "20X_moved_mask2");
    }

    {
        mask1->setVisible(false);
        mask1->setDirty();
        p.image->waitForDone();
        chk.checkImage(p.image, "21.1_hide_both_update_default_mask1");

        QTest::qWait(4000);
        p.image->waitForDone();
        chk.checkImage(p.image, "21.1X_hide_both_update_default_mask1");

        mask2->setVisible(false);
        mask2->setDirty();
        p.image->waitForDone();
        chk.checkImage(p.image, "21.2_hide_both_update_default_mask2");

        QTest::qWait(4000);
        p.image->waitForDone();
        chk.checkImage(p.image, "21.2X_hide_both_update_default_mask2");

        mask1->setVisible(true);
        mask1->setDirty();
        p.image->waitForDone();
        chk.checkImage(p.image, "22_sh_mask1_on");

        QTest::qWait(4000);
        p.image->waitForDone();
        chk.checkImage(p.image, "22X_sh_mask1_on");

        mask1->setVisible(false);
        mask1->setDirty();
        p.image->waitForDone();
        chk.checkImage(p.image, "23_sh_mask1_off");

        QTest::qWait(4000);
        p.image->waitForDone();
        chk.checkImage(p.image, "23X_sh_mask1_off");

        mask2->setVisible(true);
        mask2->setDirty();
        p.image->waitForDone();
        chk.checkImage(p.image, "24_sh_mask2_on");

        QTest::qWait(4000);
        p.image->waitForDone();
        chk.checkImage(p.image, "24X_sh_mask2_on");


        mask2->setVisible(false);
        mask2->setDirty();
        p.image->waitForDone();
        chk.checkImage(p.image, "25_sh_mask2_off");

        QTest::qWait(4000);
        p.image->waitForDone();
        chk.checkImage(p.image, "25X_sh_mask2_off");
    }

#endif /* CHECK_HIDE_ALL_AFTER_MOVE */

    QVERIFY(chk.testPassed());
}

void KisTransformMaskTest::testMaskWithOffset()
{
    TestUtil::ReferenceImageChecker chk("mask_with_offset", "transform_mask_updates");

    QRect refRect(0,0,512,512);
    QRect fillRect(400,400,100,100);
    TestUtil::MaskParent p(refRect);

    p.layer->paintDevice()->fill(fillRect, KoColor(Qt::red, p.layer->colorSpace()));

    KisPaintLayerSP player = new KisPaintLayer(p.image, "bg", OPACITY_OPAQUE_U8, p.image->colorSpace());
    p.image->addNode(player, p.image->root(), KisNodeSP());

    KisTransformMaskSP mask1 = new KisTransformMask(p.image, "mask1");
    p.image->addNode(mask1, p.layer);

    p.layer->setDirty(refRect);
    p.image->waitForDone();
    chk.checkImage(p.image, "00_initial_layer_update");

    QTest::qWait(4000);
    p.image->waitForDone();
    chk.checkImage(p.image, "00X_initial_layer_update");


    QTransform transform;

    transform = QTransform::fromTranslate(-150, 0);
    mask1->setTransformParams(KisTransformMaskParamsInterfaceSP(
                                  new KisDumbTransformMaskParams(transform)));

    p.layer->setDirty(refRect);
    p.image->waitForDone();
    chk.checkImage(p.image, "01_mask1_moved_layer_update");

    QTest::qWait(4000);
    p.image->waitForDone();
    chk.checkImage(p.image, "01X_mask1_moved_layer_update");

    mask1->setY(-150);

    mask1->setDirty(refRect);
    p.image->waitForDone();
    chk.checkImage(p.image, "02_mask1_y_offset");

    QTest::qWait(4000);
    p.image->waitForDone();
    chk.checkImage(p.image, "02X_mask1_y_offset");

    QVERIFY(chk.testPassed());
}

void KisTransformMaskTest::testWeirdFullUpdates()
{
    //TestUtil::ExternalImageChecker chk("mask_with_offset", "transform_mask_updates");

    QRect imageRect(0,0,512,512);
    QRect fillRect(10, 10, 236, 236);
    TestUtil::MaskParent p(imageRect);

    p.layer->paintDevice()->fill(fillRect, KoColor(Qt::red, p.layer->colorSpace()));

    KisPaintLayerSP player1 = new KisPaintLayer(p.image, "pl1", OPACITY_OPAQUE_U8, p.image->colorSpace());
    player1->paintDevice()->fill(fillRect, KoColor(Qt::red, p.layer->colorSpace()));
    p.image->addNode(player1, p.image->root());

    KisTransformMaskSP mask1 = new KisTransformMask(p.image, "mask1");
    QTransform transform1 =
        QTransform::fromTranslate(256, 0);
    mask1->setTransformParams(KisTransformMaskParamsInterfaceSP(
                                  new KisDumbTransformMaskParams(transform1)));

    p.image->addNode(mask1, player1);


    KisPaintLayerSP player2 = new KisPaintLayer(p.image, "pl2", OPACITY_OPAQUE_U8, p.image->colorSpace());
    player2->paintDevice()->fill(fillRect, KoColor(Qt::red, p.layer->colorSpace()));
    p.image->addNode(player2, p.image->root());

    KisTransformMaskSP mask2 = new KisTransformMask(p.image, "mask2");
    QTransform transform2 =
        QTransform::fromTranslate(0, 256);
    mask2->setTransformParams(KisTransformMaskParamsInterfaceSP(
                                  new KisDumbTransformMaskParams(transform2)));

    p.image->addNode(mask2, player2);


    KisPaintLayerSP player3 = new KisPaintLayer(p.image, "pl3", OPACITY_OPAQUE_U8, p.image->colorSpace());
    player3->paintDevice()->fill(fillRect, KoColor(Qt::red, p.layer->colorSpace()));
    p.image->addNode(player3, p.image->root());

    KisTransformMaskSP mask3 = new KisTransformMask(p.image, "mask3");
    QTransform transform3 =
        QTransform::fromTranslate(256, 256);
    mask3->setTransformParams(KisTransformMaskParamsInterfaceSP(
                                  new KisDumbTransformMaskParams(transform3)));

    p.image->addNode(mask3, player3);



    //p.image->initialRefreshGraph();

    p.image->refreshGraphAsync(0, QRect(0,0,256,256), QRect());
    p.image->waitForDone();

    QVERIFY(player1->projection()->extent().isEmpty());
    QVERIFY(player1->projection()->exactBounds().isEmpty());

    QVERIFY(player2->projection()->extent().isEmpty());
    QVERIFY(player2->projection()->exactBounds().isEmpty());

    QVERIFY(player3->projection()->extent().isEmpty());
    QVERIFY(player3->projection()->exactBounds().isEmpty());

    QCOMPARE(p.image->projection()->exactBounds(), QRect(QRect(10,10,236,236)));



    p.image->refreshGraphAsync(0, QRect(0,256,256,256), QRect());
    p.image->waitForDone();

    QVERIFY(player1->projection()->extent().isEmpty());
    QVERIFY(player1->projection()->exactBounds().isEmpty());

    QVERIFY(!player2->projection()->extent().isEmpty());
    QVERIFY(!player2->projection()->exactBounds().isEmpty());

    QVERIFY(player3->projection()->extent().isEmpty());
    QVERIFY(player3->projection()->exactBounds().isEmpty());

    QCOMPARE(p.image->projection()->exactBounds(), QRect(QRect(10,10,236,492)));


    p.image->refreshGraphAsync(0, QRect(256,0,256,256), QRect());
    p.image->waitForDone();

    QVERIFY(!player1->projection()->extent().isEmpty());
    QVERIFY(!player1->projection()->exactBounds().isEmpty());

    QVERIFY(!player2->projection()->extent().isEmpty());
    QVERIFY(!player2->projection()->exactBounds().isEmpty());

    QVERIFY(player3->projection()->extent().isEmpty());
    QVERIFY(player3->projection()->exactBounds().isEmpty());

    QCOMPARE(p.image->projection()->exactBounds(), QRect(QRect(10,10,492,492)));
    QVERIFY((p.image->projection()->region() & QRect(256,256,256,256)).isEmpty());



    p.image->refreshGraphAsync(0, QRect(256,256,256,256), QRect());
    p.image->waitForDone();

    QVERIFY(!player1->projection()->extent().isEmpty());
    QVERIFY(!player1->projection()->exactBounds().isEmpty());

    QVERIFY(!player2->projection()->extent().isEmpty());
    QVERIFY(!player2->projection()->exactBounds().isEmpty());

    QVERIFY(!player3->projection()->extent().isEmpty());
    QVERIFY(!player3->projection()->exactBounds().isEmpty());

    QCOMPARE(p.image->projection()->exactBounds(), QRect(QRect(10,10,492,492)));
    QVERIFY(!(p.image->projection()->region() & QRect(256,256,256,256)).isEmpty());

    p.image->waitForDone();

    KIS_DUMP_DEVICE_2(p.image->projection(), imageRect, "image_proj", "dd");

}

void KisTransformMaskTest::testTransformHiddenPartsOfTheGroup()
{
    //TestUtil::ExternalImageChecker chk("mask_with_offset", "transform_mask_updates");

    QRect imageRect(0,0,512,512);
    QRect fillRect(10, 10, 236, 236);
    QRect outsideFillRect = fillRect.translated(0, -1.5 * 256);

    TestUtil::MaskParent p(imageRect);

    //p.layer->paintDevice()->fill(fillRect, KoColor(Qt::green, p.layer->colorSpace()));

    p.image->initialRefreshGraph();

    KisGroupLayerSP glayer = new KisGroupLayer(p.image, "gl", OPACITY_OPAQUE_U8);
    p.image->addNode(glayer, p.image->root());

    KisPaintLayerSP player1 = new KisPaintLayer(p.image, "pl1", OPACITY_OPAQUE_U8, p.image->colorSpace());
    player1->paintDevice()->fill(fillRect, KoColor(Qt::red, p.layer->colorSpace()));
    player1->paintDevice()->fill(outsideFillRect, KoColor(Qt::blue, p.layer->colorSpace()));
    p.image->addNode(player1, glayer);

    player1->setDirty();
    p.image->waitForDone();

    QCOMPARE(p.image->projection()->exactBounds(), fillRect);

    //KIS_DUMP_DEVICE_2(p.image->projection(), imageRect, "image_proj_initial", "dd");

    KisTransformMaskSP mask1 = new KisTransformMask(p.image, "mask1");
    QTransform transform1 =
        QTransform::fromTranslate(0, 1.5 * 256);
    mask1->setTransformParams(KisTransformMaskParamsInterfaceSP(
                                  new KisDumbTransformMaskParams(transform1)));

    p.image->addNode(mask1, player1);

    mask1->setDirty();
    p.image->waitForDone();

    /**
     * Transform mask i sexpected to crop the externals of the layer by 50%
     * far behind the layer border! Take care!
     */
    QCOMPARE(p.image->projection()->exactBounds(), QRect(10, 128, 236, 384));

    //KIS_DUMP_DEVICE_2(p.image->projection(), imageRect, "image_proj_mask", "dd");
}

KISTEST_MAIN(KisTransformMaskTest)
