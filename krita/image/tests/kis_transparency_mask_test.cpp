/*
 *  Copyright (c) 2007 Boudewijn Rempt boud@valdyas.org
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

#include "kis_transparency_mask_test.h"

#include <qtest_kde.h>
#include "kis_transparency_mask.h"
#include "kis_paint_layer.h"
#include "kis_image.h"
#include "kis_fill_painter.h"
#include "testutil.h"
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

void KisTransparencyMaskTest::testCreation()
{
    KisTransparencyMask test;
}

#define initImage(image, layer, device, mask) do {                      \
    image = new KisImage(0, IMAGE_WIDTH, IMAGE_HEIGHT, 0, "tests");     \
    device = createDevice();                                            \
    layer = new KisPaintLayer(KisImageWSP(0), "", 100, device);         \
    mask = new KisTransparencyMask();                                   \
    image->addNode(layer);                                              \
    image->addNode(mask, layer);                                        \
    } while(0)

void KisTransparencyMaskTest::testApply()
{
    QPoint errpoint;

    KisImageWSP image;
    KisPaintLayerSP layer;
    KisPaintDeviceSP dev;
    KisTransparencyMaskSP mask;

    // Nothing is selected -- therefore everything should be filtered out on apply
    initImage(image, layer, dev, mask);
    mask->apply(dev, QRect(0, 0, 200, 100));
    QImage qimage = dev->convertToQImage(0, 0, 0, 200, 100);

    if (!TestUtil::compareQImages(errpoint,
                                  QImage(QString(FILES_DATA_DIR) + QDir::separator() + "transparency_mask_test_2.png"),
                                  qimage)) {
        QFAIL(QString("Failed to mask out image, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }

    // Invert the mask -- everything is selected
    initImage(image, layer, dev, mask);
    mask->selection()->getOrCreatePixelSelection()->invert();
    mask->apply(dev, QRect(0, 0, 200, 100));
    qimage = dev->convertToQImage(0, 0, 0, 200, 100);

    if (!TestUtil::compareQImages(errpoint,
                                  QImage(QString(FILES_DATA_DIR) + QDir::separator() + "transparency_mask_test_1.png"),
                                  qimage)) {
        QFAIL(QString("Failed to mask in image, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }

    // Invert back, and select a small area
    initImage(image, layer, dev, mask);
    mask->selection()->getOrCreatePixelSelection()->invert();
    mask->select(QRect(50, 0, 100, 100));
    mask->apply(dev, QRect(0, 0, 200, 100));
    qimage = dev->convertToQImage(0, 0, 0, 200, 100);

    if (!TestUtil::compareQImages(errpoint,
                                  QImage(QString(FILES_DATA_DIR) + QDir::separator() + "transparency_mask_test_3.png"),
                                  qimage)) {

        QFAIL(QString("Failed to apply partial mask, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }

}

QTEST_KDEMAIN(KisTransparencyMaskTest, GUI)
#include "kis_transparency_mask_test.moc"
