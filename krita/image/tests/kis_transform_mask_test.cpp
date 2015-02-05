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

#include "kis_transform_mask_test.h"

#include <qtest_kde.h>

#include <KoColor.h>


#include "kis_transform_mask.h"
#include "kis_transform_mask_params_interface.h"

#include "testutil.h"

#include "kis_algebra_2d.h"
#include "kis_safe_transform.h"
#include "kis_clone_layer.h"



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

    qDebug() << ppVar(testRect);
    qDebug() << ppVar(fwdPoly);
    qDebug() << ppVar(fwdRect);
    qDebug() << ppVar(bwdPoly);
    qDebug() << ppVar(bwdRect);

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
    qDebug() << ppVar(testRect);
    qDebug() << ppVar(fwdPoly);
    qDebug() << ppVar(fwdRect);
    qDebug() << ppVar(bwdPoly);
    qDebug() << ppVar(bwdRect);
    qDebug() << ppVar(bwdNastyRect);
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

bool checkDeviceImpl(KisPaintDeviceSP device, KisImageSP image, const QString &testName, const QString &prefix) {
    return TestUtil::checkQImageExternal(device->convertToQImage(0, image->bounds()),
                                         "transform_mask_updates",
                                         prefix,
                                         testName, 1, 1, 100);
}

bool checkImage(KisImageSP image, const QString &testName, const QString &prefix) {
    return checkDeviceImpl(image->projection(), image, testName, prefix);
}

bool doPartialTests(const QString &prefix, KisImageSP image, KisLayerSP paintLayer,
                    KisLayerSP visibilityToggleLayer, KisTransformMaskSP mask)
{
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
            result &= checkImage(image, testName, prefix);
        }
    }

    // initial update of the mask to clear the unused portions of the projection
    // (it updates only when we call set dirty on the mask itself, which happens
    // in Krita right after the addition of the mask onto a layer)

    mask->setDirty();
    image->waitForDone();
    testName = QString("tm_%1_initial_mask_visible_on").arg(testIndex++);
    result &= checkImage(image, testName, prefix);

    // start layer visibility testing

    paintLayer->setVisible(false);
    paintLayer->setDirty();
    image->waitForDone();
    testName = QString("tm_%1_layer_visible_off").arg(testIndex++);
    result &= checkImage(image, testName, prefix);

    paintLayer->setVisible(true);
    paintLayer->setDirty();
    image->waitForDone();
    testName = QString("tm_%1_layer_visible_on").arg(testIndex++);
    result &= checkImage(image, testName, prefix);

    if (paintLayer != visibilityToggleLayer) {
        visibilityToggleLayer->setVisible(false);
        visibilityToggleLayer->setDirty();
        image->waitForDone();
        testName = QString("tm_%1_extra_layer_visible_off").arg(testIndex++);
        result &= checkImage(image, testName, prefix);


        visibilityToggleLayer->setVisible(true);
        visibilityToggleLayer->setDirty();
        image->waitForDone();
        testName = QString("tm_%1_extra_layer_visible_on").arg(testIndex++);
        result &= checkImage(image, testName, prefix);
    }

    // toggle mask visibility

    mask->setVisible(false);
    mask->setDirty();
    image->waitForDone();
    testName = QString("tm_%1_mask_visible_off").arg(testIndex++);
    result &= checkImage(image, testName, prefix);

    mask->setVisible(true);
    mask->setDirty();
    image->waitForDone();
    testName = QString("tm_%1_mask_visible_on").arg(testIndex++);
    result &= checkImage(image, testName, prefix);

    // entire bounds update

    // no clearing, just don't hang up

    paintLayer->setDirty(refRect);
    image->waitForDone();
    testName = QString("tm_%1_layer_dirty_bounds").arg(testIndex++);
    result &= checkImage(image, testName, prefix);

    // no clearing, just don't hang up

    mask->setDirty(refRect);
    image->waitForDone();
    testName = QString("tm_%1_mask_dirty_bounds").arg(testIndex++);
    result &= checkImage(image, testName, prefix);

    if (paintLayer != visibilityToggleLayer) {
        // no clearing, just don't hang up

        visibilityToggleLayer->setDirty(refRect);
        image->waitForDone();
        testName = QString("tm_%1_extra_layer_dirty_bounds").arg(testIndex++);
        result &= checkImage(image, testName, prefix);
    }

    QRect fillRect;

    // partial updates outside

    fillRect = QRect(-100, 0.5 * refRect.height(), 50, 100);
    paintLayer->paintDevice()->fill(fillRect, KoColor(Qt::red, image->colorSpace()));
    paintLayer->setDirty(fillRect);
    image->waitForDone();
    testName = QString("tm_%1_layer_dirty_outside_%2_%3").arg(testIndex++).arg(fillRect.x()).arg(fillRect.y());
    result &= checkImage(image, testName, prefix);

    fillRect = QRect(0.5 * refRect.width(), -100, 100, 50);
    paintLayer->paintDevice()->fill(fillRect, KoColor(Qt::red, image->colorSpace()));
    paintLayer->setDirty(fillRect);
    image->waitForDone();
    testName = QString("tm_%1_layer_dirty_outside_%2_%3").arg(testIndex++).arg(fillRect.x()).arg(fillRect.y());
    result &= checkImage(image, testName, prefix);

    fillRect = QRect(refRect.width() + 50, 0.2 * refRect.height(), 50, 100);
    paintLayer->paintDevice()->fill(fillRect, KoColor(Qt::red, image->colorSpace()));
    paintLayer->setDirty(fillRect);
    image->waitForDone();
    testName = QString("tm_%1_layer_dirty_outside_%2_%3").arg(testIndex++).arg(fillRect.x()).arg(fillRect.y());
    result &= checkImage(image, testName, prefix);

    // partial update inside

    fillRect = QRect(0.5 * refRect.width() - 50, 0.5 * refRect.height() - 50, 100, 100);
    paintLayer->paintDevice()->fill(fillRect, KoColor(Qt::red, image->colorSpace()));
    paintLayer->setDirty(fillRect);
    image->waitForDone();
    testName = QString("tm_%1_layer_dirty_inside_%2_%3").arg(testIndex++).arg(fillRect.x()).arg(fillRect.y());
    result &= checkImage(image, testName, prefix);

    // clear explicitly
    image->projection()->clear();

    mask->setDirty();
    image->waitForDone();
    testName = QString("tm_%1_mask_dirty_bounds").arg(testIndex++);
    result &= checkImage(image, testName, prefix);


    KisDumbTransformMaskParams *params =
        dynamic_cast<KisDumbTransformMaskParams*>(mask->transformParams().data());

    QTransform t = params->testingGetTransform();
    t *= QTransform::fromTranslate(400, 300);
    params->testingSetTransform(t);
    mask->setTransformParams(mask->transformParams());

    mask->setDirty();
    image->waitForDone();
    testName = QString("tm_%1_mask_dirty_after_offset").arg(testIndex++);
    result &= checkImage(image, testName, prefix);

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

    KisTransformMaskSP mask = new KisTransformMask();
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

    KisTransformMaskSP mask = new KisTransformMask();
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
    QRect refRect(0,0,512,512);
    QRect fillRect(400,400,100,100);
    TestUtil::MaskParent p(refRect);

    p.layer->paintDevice()->fill(fillRect, KoColor(Qt::red, p.layer->colorSpace()));

    KisPaintLayerSP player = new KisPaintLayer(p.image, "bg", OPACITY_OPAQUE_U8, p.image->colorSpace());
    p.image->addNode(player, p.image->root(), KisNodeSP());

    KisCloneLayerSP clone = new KisCloneLayer(p.layer, p.image, "clone", OPACITY_OPAQUE_U8);
    p.image->addNode(clone, p.image->root());

    KisTransformMaskSP mask = new KisTransformMask();
    p.image->addNode(mask, clone);

    QTransform transform(1, 0, 0,
                         0, 1, 0,
                         0, -150, 1);

    mask->setTransformParams(KisTransformMaskParamsInterfaceSP(
                                 new KisDumbTransformMaskParams(transform)));

    p.layer->setDirty(refRect);
    p.image->waitForDone();
    QVERIFY(checkImage(p.image, "0_initial", "clone_offset_simple"));

    clone->setX(-300);
    clone->setDirty();
    p.image->waitForDone();
    QVERIFY(checkImage(p.image, "1_after_offset", "clone_offset_simple"));

    mask->setDirty();
    p.image->waitForDone();
    QVERIFY(checkImage(p.image, "2_after_offset_dirty_mask", "clone_offset_simple"));

    QTest::qWait(4000);
    QVERIFY(checkImage(p.image, "3_delayed_regeneration", "clone_offset_simple"));

    KisPaintDeviceSP previewDevice = mask->buildPreviewDevice();
    QVERIFY(checkDeviceImpl(previewDevice, p.image, "4_preview_device", "clone_offset_simple"));


    QVERIFY(doPartialTests("clone_offset_complex", p.image, p.layer, clone, mask));
}

QTEST_KDEMAIN(KisTransformMaskTest, GUI)
