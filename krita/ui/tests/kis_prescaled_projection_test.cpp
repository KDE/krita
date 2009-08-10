/*
 * Copyright (C) 2007 Boudewijn Rempt <boud@kde.org>
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

#include "kis_prescaled_projection_test.h"
#include <QTest>
#include <QCoreApplication>

#include <qtest_kde.h>

#include <QSize>
#include <QImage>

#include <KoZoomHandler.h>
#include <KoColorSpaceRegistry.h>
#include <KoCompositeOp.h>
#include <KoColorSpaceConstants.h>

#include <kis_paint_device.h>
#include <kis_types.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_group_layer.h>


#include "canvas/kis_prescaled_projection.h"


bool KisPrescaledProjectionTest::testProjectionScenario(KisPrescaledProjection & projection,
        KoZoomHandler * viewConverter,
        const QString & name)
{

    qDebug() << 1;
    projection.resizePrescaledImage(QSize(1000, 1000));
    projection.prescaledQImage().save(name + "_prescaled_projection_01.png");

    qDebug() << 21;
    viewConverter->setZoom(0.5);
    projection.preScale();
    projection.prescaledQImage().save(name + "_prescaled_projection_021.png");

    qDebug() << 22;
    viewConverter->setZoom(0.6);
    projection.preScale();
    projection.prescaledQImage().save(name + "_prescaled_projection_022.png");

    qDebug() << 23;
    viewConverter->setZoom(0.71);
    projection.preScale();
    projection.prescaledQImage().save(name + "_prescaled_projection_023.png");

    qDebug() << 24;
    viewConverter->setZoom(0.84);
    projection.preScale();
    projection.prescaledQImage().save(name + "_prescaled_projection_024.png");

    qDebug() << 25;
    viewConverter->setZoom(0.9);
    projection.preScale();
    projection.prescaledQImage().save(name + "_prescaled_projection_025.png");

    qDebug() << 3;
    viewConverter->setZoom(1.9);
    projection.preScale();
    projection.prescaledQImage().save(name + "_prescaled_projection_03.png");

    qDebug() << 4;
    viewConverter->setZoom(2.0);
    projection.preScale();
    projection.prescaledQImage().save(name + "_prescaled_projection_04.png");

    viewConverter->setZoom(2.5);
    projection.preScale();
    projection.prescaledQImage().save(name + "_prescaled_projection_05.png");

    viewConverter->setZoom(16.0);
    projection.preScale();
    projection.prescaledQImage().save(name + "_prescaled_projection_06.png");

    viewConverter->setZoom(1.0);
    projection.preScale();
    projection.prescaledQImage().save(name + "_prescaled_projection_07.png");

    projection.documentOffsetMoved(QPoint(50, 50));
    projection.prescaledQImage().save(name + "_prescaled_projection_08.png");

    projection.documentOffsetMoved(QPoint(100, 100));
    projection.prescaledQImage().save(name + "_prescaled_projection_081.png");

    projection.documentOffsetMoved(QPoint(200, 200));
    projection.prescaledQImage().save(name + "_prescaled_projection_082.png");

    projection.documentOffsetMoved(QPoint(250, 250));
    projection.prescaledQImage().save(name + "_prescaled_projection_083.png");

    projection.documentOffsetMoved(QPoint(150, 200));
    projection.prescaledQImage().save(name + "_prescaled_projection_084.png");

    projection.documentOffsetMoved(QPoint(100, 200));
    projection.prescaledQImage().save(name + "_prescaled_projection_085.png");

    projection.documentOffsetMoved(QPoint(50, 200));
    projection.prescaledQImage().save(name + "_prescaled_projection_086.png");

    projection.documentOffsetMoved(QPoint(0, 200));
    projection.prescaledQImage().save(name + "_prescaled_projection_087.png");

    projection.resizePrescaledImage(QSize(750, 750));
    projection.prescaledQImage().save(name + "_prescaled_projection_09.png");

    viewConverter->setZoom(1.0);
    projection.preScale();
    projection.prescaledQImage().save(name + "_prescaled_projection_10.png");

    projection.resizePrescaledImage(QSize(350, 350));
    projection.prescaledQImage().save(name + "_prescaled_projection_11.png");

    projection.documentOffsetMoved(QPoint(100, 100));
    projection.prescaledQImage().save(name + "_prescaled_projection_12.png");

    viewConverter->setZoom(0.75);
    projection.preScale();
    projection.prescaledQImage().save(name + "_prescaled_projection_13.png");

    projection.documentOffsetMoved(QPoint(10, 10));
    projection.prescaledQImage().save(name + "_prescaled_projection_14.png");

    projection.documentOffsetMoved(QPoint(0, 0));
    projection.prescaledQImage().save(name + "_prescaled_projection_15.png");

    projection.documentOffsetMoved(QPoint(10, 10));
    projection.prescaledQImage().save(name + "_prescaled_projection_16.png");

    projection.documentOffsetMoved(QPoint(30, 50));
    projection.prescaledQImage().save(name + "_prescaled_projection_17.png");

    return true;
}

void KisPrescaledProjectionTest::testCreation()
{
    KisPrescaledProjection * prescaledProjection = 0;
    prescaledProjection = new KisPrescaledProjection();
    QVERIFY(prescaledProjection != 0);
    QVERIFY(prescaledProjection->drawCheckers() == false);
    QVERIFY(prescaledProjection->prescaledPixmap().isNull());
    QVERIFY(prescaledProjection->prescaledQImage().isNull());
    delete prescaledProjection;
}


void KisPrescaledProjectionTest::testCoordinateConversionRoundTrip()
{
    KisPrescaledProjection projection;

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 100, 100, cs, "projection test");
    image->setResolution(300, 300);

    KoZoomHandler * viewConverter = new KoZoomHandler();
    viewConverter->setResolution(120, 120);

    projection.setViewConverter(viewConverter);
    projection.setImage(image);
    projection.resizePrescaledImage(QSize(100, 100));

    QRect viewRect = toAlignedRectWorkaround(projection.viewRectFromImagePixels(QRect(0, 0, 100, 100)));

    /**
     * FIXME:
     * Actually, we compare with wrong rect here, i think.
     * Right rect here is QRect(0,0,40,40) as
     * 100 / 300 * 120 == 40
     * Current value '41', i guess, is a legacy of
     * "hystory reasons" caused QRect::right() to be
     * one pixel smaller than QRectF::right()
     */
    QCOMPARE(viewRect, QRect(0, 0, 41, 41));

    QRect viewRect2 = toAlignedRectWorkaround(projection.viewRectFromImagePixels(QRect(0, 0, 200, 200)));
    /**
     * Here too
     */
    QCOMPARE(viewRect2, QRect(0, 0, 81, 81));

    QRect imageRect = projection.imageRectFromViewPortPixels(viewRect);
    QCOMPARE(imageRect, QRect(0, 0, 100, 100));

    QRect viewRect3 = toAlignedRectWorkaround(projection.viewRectFromImagePixels(imageRect));
    QCOMPARE(viewRect3, viewRect);
}


void KisPrescaledProjectionTest::testScalingUndeferredSmoothingPixelForPixel()
{
    // Set up a nice image
    QImage qimage(QString(FILES_DATA_DIR) + QDir::separator() + "lena.png");

    // Undo adapter not necessary
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, qimage.width(), qimage.height(), cs, "projection test");

    // 300 dpi recalculated to pixels per point (of which there are 72
    // to the inch)
    image->setResolution(100 / 72 , 100 / 72);

    KisPaintLayerSP layer = new KisPaintLayer(image, "test", OPACITY_OPAQUE, cs);
    image->addNode(layer.data(), image->rootLayer(), 0);
    layer->paintDevice()->convertFromQImage(qimage, "");

    KisPrescaledProjection projection;
    KoZoomHandler * viewConverter = new KoZoomHandler();
    projection.setViewConverter(viewConverter);
    projection.setImage(image);

    // pixel-for-pixel, at 100% zoom
    viewConverter->setResolution(image->xRes(), image->yRes());

    testProjectionScenario(projection, viewConverter, "pixel_for_pixel");

}


void KisPrescaledProjectionTest::testScalingUndeferredSmoothing()
{
    // Set up a nice image
    QImage qimage(QString(FILES_DATA_DIR) + QDir::separator() + "lena.png");

    // Undo adapter not necessary
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, qimage.width(), qimage.height(), cs, "projection test");

    // 300 dpi recalculated to pixels per point (of which there are 72
    // to the inch)
    image->setResolution(300 / 72 , 300 / 72);

    KisPaintLayerSP layer = new KisPaintLayer(image, "test", OPACITY_OPAQUE, cs);
    image->addNode(layer.data(), image->rootLayer(), 0);
    layer->paintDevice()->convertFromQImage(qimage, "");

    KisPrescaledProjection projection;
    KoZoomHandler * viewConverter = new KoZoomHandler();
    projection.setViewConverter(viewConverter);
    projection.setImage(image);

    testProjectionScenario(projection, viewConverter, "120dpi");

}

QTEST_KDEMAIN(KisPrescaledProjectionTest, GUI)

#include "kis_prescaled_projection_test.moc"

