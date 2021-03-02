/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_paint_layer_test.h"
#include <simpletest.h>
#include <QImage>
#include <QCoreApplication>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kundo2command.h"

#include "kis_group_layer.h"
#include "kis_types.h"
#include "kis_paint_layer.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_transparency_mask.h"
#include <testutil.h>
#include "kis_selection.h"
#include "kis_fill_painter.h"
#include "kis_pixel_selection.h"
#include <kis_iterator_ng.h>
#include "kis_layer_projection_plane.h"
#include "kis_psd_layer_style.h"
#include "filter/kis_filter_registry.h"
#include "kis_keyframe_channel.h"
#include "kis_paint_device_debug_utils.h"

void KisPaintLayerTest::testProjection()
{

    QImage qimage(QString(FILES_DATA_DIR) + '/' + "hakonepa.png");
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, qimage.width(), qimage.height(), cs, "merge test");

    KisPaintLayerSP layer = new KisPaintLayer(image, "test", OPACITY_OPAQUE_U8);
    layer->paintDevice()->convertFromQImage(qimage, 0, 0, 0);
    image->addNode(layer.data());

    // Make sure the projection and the paint device are the same -- we don't have masks yet
    QVERIFY(layer->paintDevice().data() == layer->projection().data());

    KisTransparencyMaskSP transparencyMask = new KisTransparencyMask(image, "tmask");
    transparencyMask->initSelection(layer);
    transparencyMask->selection()->pixelSelection()->invert();
    image->addNode(transparencyMask.data(), layer.data());

    // Now there are masks. Verify that
    Q_ASSERT(layer->hasEffectMasks());

    // And now we're going to update the projection, but nothing is dirty yet
    layer->projectionPlane()->recalculate(qimage.rect(), layer);

    // Which also means that the projection is no longer the paint device
    QVERIFY(layer->paintDevice().data() != layer->projection().data());

    // Now the machinery will start to roll
    layer->setDirty(qimage.rect());

    // And now we're going to update the projection, but nothing is dirty yet
    layer->projectionPlane()->recalculate(qimage.rect(), layer);

    // Which also means that the projection is no longer the paint device
    QVERIFY(layer->paintDevice().data() != layer->projection().data());

    // And the projection is no longer 0, because while we've updated it, nothing is dirty,
    // so nothing gets updated
    QVERIFY(layer->projection().data() != 0);

    // The selection is initially empty, so after an update, all pixels are still visible
    layer->projectionPlane()->recalculate(qimage.rect(), layer);

    // We've inverted the mask, so now nothing is seen
    KisSequentialConstIterator it(layer->projection(), qimage.rect());
    while (it.nextPixel()) {
        QVERIFY(cs->opacityU8(it.oldRawData()) == OPACITY_TRANSPARENT_U8);
    };

    // Now fill the layer with some opaque pixels
    transparencyMask->select(qimage.rect());
    transparencyMask->setDirty(qimage.rect());
    image->waitForDone();

    layer->projection()->convertToQImage(0, 0, 0, qimage.width(), qimage.height()).save("aaa.png");
    // Nothing is transparent anymore, so the projection and the paint device should be identical again
    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, qimage, layer->projection()->convertToQImage(0, 0, 0, qimage.width(), qimage.height()))) {
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }

}

void KisPaintLayerTest::testKeyframing()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 512, 512, cs, "");
    KisPaintLayerSP layer = new KisPaintLayer(image, "", OPACITY_OPAQUE_U8);
    KisPaintDeviceSP dev = layer->paintDevice();

    KisKeyframeChannel *contentChannel = layer->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    QVERIFY(!contentChannel);

    layer->enableAnimation();

    contentChannel = layer->getKeyframeChannel("content", true);
    QVERIFY(contentChannel);
    QCOMPARE(contentChannel->keyframeCount(), 1);

    KUndo2Command parentCommand;

    contentChannel->addKeyframe(7, &parentCommand);
    QCOMPARE(contentChannel->keyframeCount(), 2);
    QVERIFY(contentChannel->keyframeAt(0) != contentChannel->keyframeAt(7));

    KisPaintLayerSP layerCopy = new KisPaintLayer(*layer.data());
}

void KisPaintLayerTest::testLayerStyles()
{
    /**
     * FIXME: Right now the layer style plugin is stored in 'filters'
     *        directory, so we need to load filters to get it registered
     *        in the factory.
     */
    KisFilterRegistry::instance();

    const QRect imageRect(0, 0, 200, 200);
    const QRect rFillRect(10, 10, 100, 100);
    const QRect tMaskRect(50, 50, 20, 20);
    const QRect partialSelectionRect(90, 50, 20, 20);


    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, imageRect.width(), imageRect.height(), cs, "styles test");

    KisPaintLayerSP layer = new KisPaintLayer(image, "test", OPACITY_OPAQUE_U8);
    image->addNode(layer);

    layer->paintDevice()->fill(rFillRect, KoColor(Qt::red, cs));

    layer->setDirty();
    image->waitForDone();
    KIS_DUMP_DEVICE_2(layer->projection(), imageRect, "00L_initial", "dd");

    KisPSDLayerStyleSP style(new KisPSDLayerStyle());
    style->dropShadow()->setNoise(30);
    style->dropShadow()->setEffectEnabled(true);

    layer->setLayerStyle(style);

    layer->setDirty();
    image->waitForDone();
    KIS_DUMP_DEVICE_2(image->projection(), imageRect, "02P_styled", "dd");

    KisTransparencyMaskSP transparencyMask = new KisTransparencyMask(image, "tmask");

    KisSelectionSP selection = new KisSelection();
    selection->pixelSelection()->select(tMaskRect, OPACITY_OPAQUE_U8);
    transparencyMask->setSelection(selection);
    image->addNode(transparencyMask, layer);

    layer->setDirty();
    image->waitForDone();
    KIS_DUMP_DEVICE_2(image->projection(), imageRect, "03P_styled", "dd");

    selection->pixelSelection()->select(partialSelectionRect, OPACITY_OPAQUE_U8);
    layer->setDirty(partialSelectionRect);

    layer->setDirty();
    image->waitForDone();
    KIS_DUMP_DEVICE_2(image->projection(), imageRect, "04P_partial", "dd");
}


SIMPLE_TEST_MAIN(KisPaintLayerTest)
