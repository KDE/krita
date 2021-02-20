/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_transparency_mask_test.h"

#include <QTest>
#include "kis_transparency_mask.h"
#include "kis_paint_layer.h"
#include "kis_image.h"
#include "kis_fill_painter.h"
#include <testutil.h>
#include "kis_selection.h"
#include "kis_pixel_selection.h"
#define IMAGE_WIDTH 1000
#define IMAGE_HEIGHT 1000

KisPaintDeviceSP createDevice()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    KisFillPainter gc(dev);
    KoColor c(Qt::red, dev->colorSpace());
    gc.fillRect(0, 0, 100, 100, c);
    c = KoColor(Qt::blue, dev->colorSpace());
    gc.fillRect(100, 0, 100, 100, c);
    gc.end();

    return dev;
}

#define initImage(image, layer, device, mask) do {                      \
    image = new KisImage(0, IMAGE_WIDTH, IMAGE_HEIGHT, 0, "tests");     \
    device = createDevice();                                            \
    layer = new KisPaintLayer(image, "paint1", 100, device);                  \
    mask = new KisTransparencyMask(image, "tmask");                                   \
    image->addNode(layer);                                              \
    image->addNode(mask, layer);                                        \
    } while(0)

void KisTransparencyMaskTest::testApply()
{
    QPoint errpoint;

    KisImageSP image;
    KisPaintLayerSP layer;
    KisPaintDeviceSP dev;
    KisTransparencyMaskSP mask;


    QRect applyRect(0, 0, 200, 100);

    // Everything is selected
    initImage(image, layer, dev, mask);
    mask->initSelection(layer);
    mask->apply(dev, applyRect, applyRect, KisNode::N_FILTHY);
    QImage qimage = dev->convertToQImage(0, 0, 0, 200, 100);

    if (!TestUtil::compareQImages(errpoint,
                                  QImage(QString(FILES_DATA_DIR) + '/' + "transparency_mask_test_2.png"),
                                  qimage)) {
        QFAIL(QString("Failed to mask out image, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }

    // Invert the mask, so that nothing will be selected, then select a rect
    initImage(image, layer, dev, mask);
    mask->initSelection(layer);
    mask->selection()->pixelSelection()->invert();
    mask->apply(dev, applyRect, applyRect, KisNode::N_FILTHY);
    qimage = dev->convertToQImage(0, 0, 0, 200, 100);

    if (!TestUtil::compareQImages(errpoint,
                                  QImage(QString(FILES_DATA_DIR) + '/' + "transparency_mask_test_1.png"),
                                  qimage)) {
        QFAIL(QString("Failed to mask in image, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }

    initImage(image, layer, dev, mask);
    mask->initSelection(layer);
    mask->selection()->pixelSelection()->invert();
    mask->select(QRect(50, 0, 100, 100));
    mask->apply(dev, applyRect, applyRect, KisNode::N_FILTHY);
    qimage = dev->convertToQImage(0, 0, 0, 200, 100);

    if (!TestUtil::compareQImages(errpoint,
                                  QImage(QString(FILES_DATA_DIR) + '/' + "transparency_mask_test_3.png"),
                                  qimage)) {

        QFAIL(QString("Failed to apply partial mask, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }

}

#include "kis_full_refresh_walker.h"
#include "kis_async_merger.h"

void KisTransparencyMaskTest::testMoveParentLayer()
{
    KisImageSP image;
    KisPaintLayerSP layer;
    KisPaintDeviceSP dev;
    KisTransparencyMaskSP mask;

    initImage(image, layer, dev, mask);
    mask->initSelection(layer);
    mask->selection()->pixelSelection()->invert();
    mask->select(QRect(50, 50, 100, 100));

    KisFullRefreshWalker walker(image->bounds());
    KisAsyncMerger merger;

    walker.collectRects(layer, image->bounds());
    merger.startMerge(walker);

    // image->projection()->convertToQImage(0, 0,0,300,300).save("proj_before.png");

    QRect initialRect(0,0,200,100);
    QCOMPARE(layer->exactBounds(), initialRect);
    QCOMPARE(image->projection()->exactBounds(), QRect(50,50,100,50));


    layer->setX(100);
    layer->setY(100);

    dbgKrita << "Sel. rect before:" << mask->selection()->selectedExactRect();

    mask->setX(100);
    mask->setY(100);

    dbgKrita << "Sel. rect after:" << mask->selection()->selectedExactRect();

    QRect finalRect(100,100,200,100);
    QCOMPARE(layer->exactBounds(), finalRect);

    walker.collectRects(layer, initialRect | finalRect);
    merger.startMerge(walker);

    // image->projection()->convertToQImage(0, 0,0,300,300).save("proj_after.png");
    QCOMPARE(image->projection()->exactBounds(), QRect(150,150,100,50));
}

void KisTransparencyMaskTest::testMoveMaskItself()
{
    KisImageSP image;
    KisPaintLayerSP layer;
    KisPaintDeviceSP dev;
    KisTransparencyMaskSP mask;

    initImage(image, layer, dev, mask);
    mask->initSelection(layer);
    mask->selection()->pixelSelection()->invert();
    mask->select(QRect(50, 50, 100, 100));

    KisFullRefreshWalker walker(image->bounds());
    KisAsyncMerger merger;

    walker.collectRects(layer, image->bounds());
    merger.startMerge(walker);

    // image->projection()->convertToQImage(0, 0,0,300,300).save("proj_before.png");

    QRect initialRect(0,0,200,100);
    QCOMPARE(layer->exactBounds(), initialRect);
    QCOMPARE(image->projection()->exactBounds(), QRect(50,50,100,50));


    //layer->setX(100);
    //layer->setY(100);

    dbgKrita << "Sel. rect before:" << mask->selection()->selectedExactRect();

    mask->setX(50);
    mask->setY(25);

    dbgKrita << "Sel. rect after:" << mask->selection()->selectedExactRect();


    QCOMPARE(mask->selection()->selectedExactRect(), QRect(100, 75, 100, 100));
    QCOMPARE(layer->paintDevice()->exactBounds(), initialRect);
    QCOMPARE(layer->projection()->exactBounds(), QRect(50, 50, 100, 50));

    dbgKrita << "";

    QRect updateRect(0,0,300,300);

    walker.collectRects(mask, updateRect);
    merger.startMerge(walker);

    // image->projection()->convertToQImage(0, 0,0,300,300).save("proj_after.png");

    QCOMPARE(layer->paintDevice()->exactBounds(), initialRect);
    QCOMPARE(layer->projection()->exactBounds(), QRect(100, 75, 100, 25));
}

QTEST_MAIN(KisTransparencyMaskTest)
