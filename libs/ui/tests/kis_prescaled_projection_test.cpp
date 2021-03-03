/*
 * SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_prescaled_projection_test.h"
#include <simpletest.h>
#include <QCoreApplication>

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
#include <kis_update_info.h>

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
    QImage qimage(QString(FILES_DATA_DIR) + '/' + "carrot.png");

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
    projection.setMonitorProfile(0,
                                 KoColorConversionTransformation::internalRenderingIntent(),
                                 KoColorConversionTransformation::internalConversionFlags());
    projection.setImage(image);

    // pixel-for-pixel, at 100% zoom
    converter.setResolution(image->xRes(), image->yRes());

    testProjectionScenario(projection, &converter, "pixel_for_pixel");

}


void KisPrescaledProjectionTest::testScalingUndeferredSmoothing()
{
    // Set up a nice image
    QImage qimage(QString(FILES_DATA_DIR) + '/' + "carrot.png");

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
    projection.setMonitorProfile(0,
                                 KoColorConversionTransformation::internalRenderingIntent(),
                                 KoColorConversionTransformation::internalConversionFlags());
    projection.setImage(image);

    testProjectionScenario(projection, &converter, "120dpi");

}

//#include <valgrind/callgrind.h>

void KisPrescaledProjectionTest::benchmarkUpdate()
{
    QImage referenceImage(QString(FILES_DATA_DIR) + '/' + "carrot.png");
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
    projection.setMonitorProfile(0,
                                 KoColorConversionTransformation::internalRenderingIntent(),
                                 KoColorConversionTransformation::internalConversionFlags());
    projection.setImage(image);

    // Emulate "Use same aspect as pixels"
    converter.setResolution(image->xRes(), image->yRes());

    converter.setZoom(1.0);

    KisUpdateInfoSP info = projection.updateCache(image->bounds());
    projection.recalculateCache(info);

    QEXPECT_FAIL("", "We expected the image rect to be (0,0,512,512), but it is (0,0 308x245)", Continue);
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


class PrescaledProjectionTester
{
public:
    PrescaledProjectionTester() {
        sourceImage = QImage(QString(FILES_DATA_DIR) + '/' + "carrot.png");

        const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
        image = new KisImage(0, sourceImage.width(), sourceImage.height(), cs, "projection test");
        image->setResolution(100, 100);

        layer = new KisPaintLayer(image, "test", OPACITY_OPAQUE_U8, cs);
        layer->paintDevice()->convertFromQImage(sourceImage, 0);

        image->addNode(layer, image->rootLayer(), 0);

        converter.setResolution(100, 100);
        converter.setZoom(1.);
        converter.setImage(image);
        converter.setCanvasWidgetSize(QSize(100,100));
        converter.setDocumentOffset(QPoint(100,100));

        projection.setCoordinatesConverter(&converter);
        projection.setMonitorProfile(0,
                                     KoColorConversionTransformation::internalRenderingIntent(),
                                     KoColorConversionTransformation::internalConversionFlags());
        projection.setImage(image);
        projection.notifyCanvasSizeChanged(QSize(100,100));
        projection.notifyZoomChanged();
    }

    QImage sourceImage;
    KisImageSP image;
    KisPaintLayerSP layer;
    KisCoordinatesConverter converter;
    KisPrescaledProjection projection;
};

void KisPrescaledProjectionTest::testScrollingZoom100()
{
    PrescaledProjectionTester t;

    QImage result = t.projection.prescaledQImage();
    QImage reference = t.sourceImage.copy(QRect(100,100,100,100));

    QPoint pt;
    QVERIFY(TestUtil::compareQImages(pt, result, reference));

    // Test actual scrolling
    t.converter.setDocumentOffset(QPoint(150,150));
    t.projection.viewportMoved(QPoint(-50,-50));

    result = t.projection.prescaledQImage();
    reference = t.sourceImage.copy(QRect(150,150,100,100));

    QEXPECT_FAIL("", "Images should be the same, but aren't", Continue);
    QVERIFY(TestUtil::compareQImages(pt, result, reference));
}

void KisPrescaledProjectionTest::testScrollingZoom50()
{
    PrescaledProjectionTester t;

    t.converter.setDocumentOffset(QPoint(0,0));

    t.converter.setCanvasWidgetSize(QSize(300,300));
    t.projection.notifyCanvasSizeChanged(QSize(300,300));

    QEXPECT_FAIL("", "Images should be the same, but aren't", Continue);
    QVERIFY(TestUtil::checkQImage(t.projection.prescaledQImage(),
                                  "prescaled_projection_test",
                                  "testScrollingZoom50",
                                  "initial"));

    t.converter.setZoom(0.5);
    t.projection.notifyZoomChanged();

    QEXPECT_FAIL("", "Images should be the same, but aren't", Continue);
    QVERIFY(TestUtil::checkQImage(t.projection.prescaledQImage(),
                                  "prescaled_projection_test",
                                  "testScrollingZoom50",
                                  "zoom50"));

    t.converter.setDocumentOffset(QPoint(50,50));
    t.projection.viewportMoved(QPoint(-50,-50));

    QEXPECT_FAIL("", "Images should be the same, but aren't", Continue);
    QVERIFY(TestUtil::checkQImage(t.projection.prescaledQImage(),
                                  "prescaled_projection_test",
                                  "testScrollingZoom50",
                                  "zoom50_moved50"));
}

void KisPrescaledProjectionTest::testUpdates()
{
    PrescaledProjectionTester t;

    t.converter.setDocumentOffset(QPoint(10,10));

    t.converter.setCanvasWidgetSize(2*QSize(300,300));
    t.projection.notifyCanvasSizeChanged(2*QSize(300,300));


    t.converter.setZoom(0.50);
    t.projection.notifyZoomChanged();

    QEXPECT_FAIL("", "Images should be the same, but aren't", Continue);
    QVERIFY(TestUtil::checkQImage(t.projection.prescaledQImage(),
                                  "prescaled_projection_test",
                                  "testUpdates",
                                  "zoom50"));

    t.layer->setVisible(false);
    KisUpdateInfoSP info = t.projection.updateCache(t.image->bounds());
    t.projection.recalculateCache(info);

    QEXPECT_FAIL("", "Images should be the same, but aren't", Continue);
    QVERIFY(TestUtil::checkQImage(t.projection.prescaledQImage(),
                                  "prescaled_projection_test",
                                  "testUpdates",
                                  "cleared"));

    t.layer->setVisible(true);
    t.image->refreshGraph();

    // Update incrementally

    const int step = 73;
    const int patchOffset = -7;
    const int patchSize = 93;

    QList<KisUpdateInfoSP> infos;

    for(int y = 0; y < t.image->height(); y+=step) {
        for(int x = 0; x < t.image->width(); x+=step) {
            QRect patchRect(x - patchOffset, y - patchOffset,
                            patchSize, patchSize);

            infos.append(t.projection.updateCache(patchRect));
        }
    }

    Q_FOREACH (KisUpdateInfoSP info, infos) {
        t.projection.recalculateCache(info);
    }

    QEXPECT_FAIL("", "Testcase for bug: https://bugs.kde.org/show_bug.cgi?id=289915", Continue);
    QVERIFY(TestUtil::checkQImage(t.projection.prescaledQImage(),
                                  "prescaled_projection_test",
                                  "testUpdates",
                                  "zoom50", 1));
}

void KisPrescaledProjectionTest::testQtScaling()
{
    // See: https://bugreports.qt.nokia.com/browse/QTBUG-22827

    /**
     * Currently we rely on this behavior, so let's test for it.
     */

    // Create a canvas image
    QImage canvas(6, 6, QImage::Format_ARGB32);
    canvas.fill(0);

    // Image we are going to scale down
    QImage image(7, 7, QImage::Format_ARGB32);
    QPainter imagePainter(&image);
    imagePainter.fillRect(QRect(0,0,7,7),Qt::green);
    imagePainter.end();


    QPainter gc(&canvas);

    // Scale down transformation
    qreal scale = 3.49/7.0;
    gc.setTransform(QTransform::fromScale(scale,scale));

    // Draw a rect scale*(7x7)
    gc.fillRect(QRectF(0,0,7,7), Qt::red);

    // Draw an image scale*(7x7)
    gc.drawImage(QPointF(), image, QRectF(0,0,7,7));

    gc.end();

    // Create an expected result
    QImage expectedResult(6, 6, QImage::Format_ARGB32);
    expectedResult.fill(0);
    QPainter expectedPainter(&expectedResult);
    expectedPainter.fillRect(QRect(0,0,4,4), Qt::red);
    expectedPainter.fillRect(QRect(0,0,3,3), Qt::green);
    expectedPainter.end();

    QEXPECT_FAIL("", "Images should be the same, but aren't", Continue);
    QCOMPARE(canvas, expectedResult);
}

SIMPLE_TEST_MAIN(KisPrescaledProjectionTest)


