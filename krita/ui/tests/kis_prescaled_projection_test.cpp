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

#include <kis_config.h>
#include <kis_types.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>
#include <kis_group_layer.h>

#include "canvas/kis_coordinates_converter.h"
#include "canvas/kis_prescaled_projection.h"

#include "../../sdk/tests/testutil.h"

bool KisPrescaledProjectionTest::testProjectionScenario(KisPrescaledProjection & projection,
        KoZoomHandler * viewConverter,
        const QString & name)
{

    projection.notifyCanvasSizeChanged(QSize(1000, 1000));
    projection.prescaledQImage().save(name + "_prescaled_projection_01.png");

    viewConverter->setZoom(0.5);
    projection.preScale();
    projection.prescaledQImage().save(name + "_prescaled_projection_021.png");

    viewConverter->setZoom(0.6);
    projection.preScale();
    projection.prescaledQImage().save(name + "_prescaled_projection_022.png");

    viewConverter->setZoom(0.71);
    projection.preScale();
    projection.prescaledQImage().save(name + "_prescaled_projection_023.png");

    viewConverter->setZoom(0.84);
    projection.preScale();
    projection.prescaledQImage().save(name + "_prescaled_projection_024.png");

    viewConverter->setZoom(0.9);
    projection.preScale();
    projection.prescaledQImage().save(name + "_prescaled_projection_025.png");

    viewConverter->setZoom(1.9);
    projection.preScale();
    projection.prescaledQImage().save(name + "_prescaled_projection_03.png");

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

    projection.viewportMoved(QPoint(50, 50));
    projection.prescaledQImage().save(name + "_prescaled_projection_08.png");

    projection.viewportMoved(QPoint(100, 100));
    projection.prescaledQImage().save(name + "_prescaled_projection_081.png");

    projection.viewportMoved(QPoint(200, 200));
    projection.prescaledQImage().save(name + "_prescaled_projection_082.png");

    projection.viewportMoved(QPoint(250, 250));
    projection.prescaledQImage().save(name + "_prescaled_projection_083.png");

    projection.viewportMoved(QPoint(150, 200));
    projection.prescaledQImage().save(name + "_prescaled_projection_084.png");

    projection.viewportMoved(QPoint(100, 200));
    projection.prescaledQImage().save(name + "_prescaled_projection_085.png");

    projection.viewportMoved(QPoint(50, 200));
    projection.prescaledQImage().save(name + "_prescaled_projection_086.png");

    projection.viewportMoved(QPoint(0, 200));
    projection.prescaledQImage().save(name + "_prescaled_projection_087.png");

    projection.notifyCanvasSizeChanged(QSize(750, 750));
    projection.prescaledQImage().save(name + "_prescaled_projection_09.png");

    viewConverter->setZoom(1.0);
    projection.preScale();
    projection.prescaledQImage().save(name + "_prescaled_projection_10.png");

    projection.notifyCanvasSizeChanged(QSize(350, 350));
    projection.prescaledQImage().save(name + "_prescaled_projection_11.png");

    projection.viewportMoved(QPoint(100, 100));
    projection.prescaledQImage().save(name + "_prescaled_projection_12.png");

    viewConverter->setZoom(0.75);
    projection.preScale();
    projection.prescaledQImage().save(name + "_prescaled_projection_13.png");

    projection.viewportMoved(QPoint(10, 10));
    projection.prescaledQImage().save(name + "_prescaled_projection_14.png");

    projection.viewportMoved(QPoint(0, 0));
    projection.prescaledQImage().save(name + "_prescaled_projection_15.png");

    projection.viewportMoved(QPoint(10, 10));
    projection.prescaledQImage().save(name + "_prescaled_projection_16.png");

    projection.viewportMoved(QPoint(30, 50));
    projection.prescaledQImage().save(name + "_prescaled_projection_17.png");

    return true;
}

void KisPrescaledProjectionTest::testCreation()
{
    KisPrescaledProjection * prescaledProjection = 0;
    prescaledProjection = new KisPrescaledProjection();
    QVERIFY(prescaledProjection != 0);
    QVERIFY(prescaledProjection->prescaledQImage().isNull());
    delete prescaledProjection;
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

    KisPaintLayerSP layer = new KisPaintLayer(image, "test", OPACITY_OPAQUE_U8, cs);
    image->addNode(layer.data(), image->rootLayer(), 0);
    layer->paintDevice()->convertFromQImage(qimage, 0);

    KisPrescaledProjection projection;
    KisCoordinatesConverter converter;
    
    converter.setImage(image);
    projection.setCoordinatesConverter(&converter);
    projection.setImage(image);

    // pixel-for-pixel, at 100% zoom
    converter.setResolution(image->xRes(), image->yRes());

    testProjectionScenario(projection, &converter, "pixel_for_pixel");

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
    image->setResolution(100, 100);

    KisPaintLayerSP layer = new KisPaintLayer(image, "test", OPACITY_OPAQUE_U8, cs);
    image->addNode(layer.data(), image->rootLayer(), 0);
    layer->paintDevice()->convertFromQImage(qimage, 0);

    KisPrescaledProjection projection;
    KisCoordinatesConverter converter;
    
    converter.setImage(image);
    projection.setCoordinatesConverter(&converter);

    projection.setImage(image);

    testProjectionScenario(projection, &converter, "120dpi");

}

//#include <valgrind/callgrind.h>

void KisPrescaledProjectionTest::benchmarkUpdate()
{
    QImage referenceImage(QString(FILES_DATA_DIR) + QDir::separator() + "lena.png");
    QRect imageRect = QRect(QPoint(0,0), referenceImage.size());

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, imageRect.width(), imageRect.height(), cs, "projection test");

    // set up 300dpi
    image->setResolution(300 / 72 , 300 / 72);

    KisPaintLayerSP layer = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8, cs);
    layer->paintDevice()->convertFromQImage(referenceImage, 0);

    image->addNode(layer, image->rootLayer(), 0);

    KisPrescaledProjection projection;

    KisCoordinatesConverter converter;
    converter.setImage(image);
    projection.setCoordinatesConverter(&converter);
    projection.setImage(image);

    // Emulate "Use same aspect as pixels"
    converter.setResolution(image->xRes(), image->yRes());

    converter.setZoom(1.0);

    KisUpdateInfoSP info = projection.updateCache(image->bounds());
    projection.recalculateCache(info);

    QCOMPARE(imageRect, QRect(0,0,512,512));

    QRect dirtyRect(0,0,20,20);
    const qint32 numShifts = 25;
    const QPoint offset(dirtyRect.width(),dirtyRect.height());

    //CALLGRIND_START_INSTRUMENTATION;

    QBENCHMARK {
        for(qint32 i = 0; i < numShifts; i++) {
            KisUpdateInfoSP tempInfo = projection.updateCache(dirtyRect);
            projection.recalculateCache(tempInfo);

            dirtyRect.translate(offset);
        }
    }

    //CALLGRIND_STOP_INSTRUMENTATION;

}


void KisPrescaledProjectionTest::testScaling()
{
    QImage sourceImage(QString(FILES_DATA_DIR) + QDir::separator() + "lena.png");

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, sourceImage.width(), sourceImage.height(), cs, "projection test");
    image->setResolution(100, 100);

    KisPaintLayerSP layer = new KisPaintLayer(image, "test", OPACITY_OPAQUE_U8, cs);
    layer->paintDevice()->convertFromQImage(sourceImage, 0);

    image->addNode(layer, image->rootLayer(), 0);

    KisCoordinatesConverter converter;
    converter.setResolution(100, 100);
    converter.setZoom(1.);
    converter.setImage(image);
    converter.setCanvasWidgetSize(QSize(100,100));

    KisPrescaledProjection projection;
    projection.setCoordinatesConverter(&converter);
    projection.setImage(image);

    converter.setDocumentOffset(QPoint(100,100));
    converter.setDocumentOrigin(QPoint(0,0));


    projection.notifyCanvasSizeChanged(QSize(100,100));
    projection.notifyZoomChanged();

    QImage result = projection.prescaledQImage();
    QImage reference = sourceImage.copy(QRect(100,100,100,100));

    QPoint pt;
    QVERIFY(TestUtil::compareQImages(pt, result, reference));


    // Test scrolling
    converter.setDocumentOffset(QPoint(150,150));
    projection.viewportMoved(QPoint(-50,-50));

    result = projection.prescaledQImage();
    reference = sourceImage.copy(QRect(150,150,100,100));

    QVERIFY(TestUtil::compareQImages(pt, result, reference));
}


QTEST_KDEMAIN(KisPrescaledProjectionTest, GUI)

#include "kis_prescaled_projection_test.moc"

