/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_projection_test.h"
#include <QTest>


#include <KoColor.h>
#include <KoColorSpace.h>

#include <kis_types.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_fill_painter.h>
#include <kis_iterator_ng.h>

void KisProjectionTest::testDirty()
{
    KisImageSP image = new KisImage(0, 1000, 1000, 0, "layer tests");

    // Two layers so the single-layer-is-rootlayer optimization doesn't kick in
    KisLayerSP layer = new KisPaintLayer(image, "layer 1", OPACITY_OPAQUE_U8);
    KisLayerSP layer2 = new KisPaintLayer(image, "layer 2", OPACITY_OPAQUE_U8);
    image->addNode(layer);
    image->addNode(layer2);
    KisFillPainter gc(layer2->paintDevice());
    KoColor c(Qt::red, layer2->colorSpace());
    gc.fillRect(0, 0, 1000, 1000, c);
    gc.end();
    layer2->setDirty(gc.takeDirtyRegion());

    // wait a little for the projection to finish
    QTest::qSleep(250);

    // Check that the projection is totally redistribute
    KisSequentialConstIterator it(image->projection(), QRect(0, 0, 1000, 1000));
    while (it.nextPixel()) {
        QColor c;
        image->colorSpace()->toQColor(it.oldRawData(), &c, image->profile());
        QVERIFY(c == Qt::red);
    }
}

QTEST_MAIN(KisProjectionTest)


