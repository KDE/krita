/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_coordinates_converter_test.h"
#include <simpletest.h>

#include <QTransform>

#include <KoZoomHandler.h>
#include <KoColorSpaceRegistry.h>
#include <kis_image.h>

#include "kis_coordinates_converter.h"
#include "kis_filter_strategy.h"
#include <kis_algebra_2d.h>


void initImage(KisImageSP *image, KoZoomHandler *zoomHandler)
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    *image = new KisImage(0, 1000, 1000, cs, "projection test");
    (*image)->setResolution(100, 100);

    zoomHandler->setResolution(100, 100);
}

void KisCoordinatesConverterTest::testConversion()
{
    KisImageSP image;
    KisCoordinatesConverter converter;
    initImage(&image, &converter);

    converter.setImage(image);
    converter.setZoom(1.0);
    converter.setDocumentOffset(QPoint(20,20));
    converter.setCanvasWidgetSize(QSize(500,500));

    QRectF testRect(100,100,100,100);

    QCOMPARE(converter.imageToViewport(testRect), QRectF(80,80,100,100));
    QCOMPARE(converter.viewportToImage(testRect), QRectF(120,120,100,100));

    QCOMPARE(converter.widgetToViewport(testRect), QRectF(100,100,100,100));
    QCOMPARE(converter.viewportToWidget(testRect), QRectF(100,100,100,100));

    QCOMPARE(converter.widgetToDocument(testRect), QRectF(1.20,1.20,1,1));
    QCOMPARE(converter.documentToWidget(testRect), QRectF(9980,9980,10000,10000));

    QCOMPARE(converter.imageToDocument(testRect), QRectF(1,1,1,1));
    QCOMPARE(converter.documentToImage(testRect), QRectF(10000,10000,10000,10000));

    converter.setZoom(0.5);

    QCOMPARE(converter.imageToViewport(testRect), QRectF(30,30,50,50));
    QCOMPARE(converter.viewportToImage(testRect), QRectF(240,240,200,200));

    QCOMPARE(converter.widgetToViewport(testRect), QRectF(100,100,100,100));
    QCOMPARE(converter.viewportToWidget(testRect), QRectF(100,100,100,100));

    QCOMPARE(converter.widgetToDocument(testRect), QRectF(2.4,2.4,2,2));
    QCOMPARE(converter.documentToWidget(testRect), QRectF(4980,4980,5000,5000));

    QCOMPARE(converter.imageToDocument(testRect), QRectF(1,1,1,1));
    QCOMPARE(converter.documentToImage(testRect), QRectF(10000,10000,10000,10000));
}

void KisCoordinatesConverterTest::testImageCropping()
{
    KisImageSP image;
    KisCoordinatesConverter converter;
    initImage(&image, &converter);

    converter.setImage(image);
    converter.setZoom(1.0);
    converter.setDocumentOffset(QPoint(0,0));
    converter.setCanvasWidgetSize(QSize(500,500));

    // we do NOT crop here
    QCOMPARE(converter.viewportToImage(QRectF(900,900,200,200)),
             QRectF(900,900,200,200));
}

#define CHECK_TRANSFORM(trans,test,ref) QCOMPARE(trans.map(test).boundingRect(), ref)
//#define CHECK_TRANSFORM(trans,test,ref) dbgKrita << trans.map(test).boundingRect()

void KisCoordinatesConverterTest::testTransformations()
{
    KisImageSP image;
    KisCoordinatesConverter converter;
    initImage(&image, &converter);

    converter.setImage(image);
    converter.setZoom(1.0);
    converter.setDocumentOffset(QPoint(20,30));
    converter.setCanvasWidgetSize(QSize(500,500));

    QRectF testRect(100,100,100,100);
    QTransform imageToWidget;
    QTransform documentToWidget;
    QTransform flakeToWidget;
    QTransform viewportToWidget;

    imageToWidget = converter.imageToWidgetTransform();
    documentToWidget = converter.documentToWidgetTransform();
    flakeToWidget = converter.flakeToWidgetTransform();
    viewportToWidget = converter.viewportToWidgetTransform();

    CHECK_TRANSFORM(imageToWidget, testRect, QRectF(80,70,100,100));
    CHECK_TRANSFORM(documentToWidget, testRect, QRectF(9980,9970,10000,10000));
    CHECK_TRANSFORM(flakeToWidget, testRect, QRectF(80,70,100,100));
    CHECK_TRANSFORM(viewportToWidget, testRect, QRectF(100,100,100,100));

    converter.setZoom(0.5);

    imageToWidget = converter.imageToWidgetTransform();
    documentToWidget = converter.documentToWidgetTransform();
    flakeToWidget = converter.flakeToWidgetTransform();
    viewportToWidget = converter.viewportToWidgetTransform();

    CHECK_TRANSFORM(imageToWidget, testRect, QRectF(30,20,50,50));
    CHECK_TRANSFORM(documentToWidget, testRect, QRectF(4980,4970,5000,5000));
    CHECK_TRANSFORM(flakeToWidget, testRect, QRectF(80,70,100,100));
    CHECK_TRANSFORM(viewportToWidget, testRect, QRectF(100,100,100,100));
}

void KisCoordinatesConverterTest::testConsistency()
{
    KisImageSP image;
    KisCoordinatesConverter converter;
    initImage(&image, &converter);

    converter.setImage(image);
    converter.setZoom(0.5);
    converter.setDocumentOffset(QPoint(20,30));
    converter.setCanvasWidgetSize(QSize(500,500));

    QRectF testRect(100,100,100,100);
    QTransform imageToWidget;
    QTransform documentToWidget;
    QTransform viewportToWidget;

    imageToWidget = converter.imageToWidgetTransform();
    documentToWidget = converter.documentToWidgetTransform();
    viewportToWidget = converter.viewportToWidgetTransform();

    QRectF fromImage = converter.viewportToWidget(converter.imageToViewport(testRect));
    QRectF fromDocument = converter.documentToWidget(testRect);
    QRectF fromViewport = converter.viewportToWidget(testRect);

    CHECK_TRANSFORM(imageToWidget, testRect, fromImage);
    CHECK_TRANSFORM(documentToWidget, testRect, fromDocument);
    CHECK_TRANSFORM(viewportToWidget, testRect, fromViewport);
}

void KisCoordinatesConverterTest::testRotation()
{
    KisImageSP image;
    KisCoordinatesConverter converter;
    initImage(&image, &converter);

    QSize widgetSize(1000,500);
    QRectF testRect(800, 100, 300, 300);

    converter.setImage(image);
    converter.setZoom(1.);
    converter.setDocumentOffset(QPoint(0,0));
    converter.setCanvasWidgetSize(widgetSize);

    converter.rotate(converter.widgetCenterPoint(), 30);

    QTransform viewportToWidget = converter.viewportToWidgetTransform();

    QRectF boundingRect = viewportToWidget.mapRect(testRect);
    QRectF directRect = converter.viewportToWidget(testRect);

    QCOMPARE(boundingRect, directRect);

    QRectF referenceRect(QPointF(742.82,53.5898), QSizeF(409.808,409.808));

#define FUZZY(a,b) ((a)-(b) < 0.01)

    QVERIFY(FUZZY(boundingRect.top(), referenceRect.top()));
    QVERIFY(FUZZY(boundingRect.left(), referenceRect.left()));
    QVERIFY(FUZZY(boundingRect.width(), referenceRect.width()));
    QVERIFY(FUZZY(boundingRect.height(), referenceRect.height()));
}

void KisCoordinatesConverterTest::testMirroring()
{
    KisImageSP image;
    KisCoordinatesConverter converter;
    initImage(&image, &converter);

    QSize widgetSize(500,400);
//    QSize flakeSize(1000,1000);
    QRectF testRect(300, 100, 200, 200);

    converter.setImage(image);
    converter.setZoom(1.0);
    converter.setDocumentOffset(QPoint(200,100));
    converter.setCanvasWidgetSize(widgetSize);

//    QTransform imageToWidget;
//    QTransform documentToWidget;
//    QTransform flakeToWidget;
//    QTransform viewportToWidget;

    converter.mirror(converter.imageCenterInWidgetPixel(), true, false);

    // image pixels == flake pixels

    QRectF viewportRect = converter.imageToViewport(testRect);
    QRectF widgetRect = converter.viewportToWidget(viewportRect);

    QCOMPARE(widgetRect, QRectF(300,0,200,200));
    QCOMPARE(viewportRect, QRectF(0,0,200,200));

    QRectF roundTrip = converter.viewportToWidget(widgetRect);
    QCOMPARE(roundTrip, viewportRect);
}

void KisCoordinatesConverterTest::testMirroringCanvasBiggerThanImage()
{
    KisImageSP image;
    KisCoordinatesConverter converter;
    initImage(&image, &converter);

    QSize widgetSize(2000,2000);
//    QSize flakeSize(1000,1000);
    QRectF testRect(300, 100, 200, 200);

    converter.setImage(image);
    converter.setZoom(1.0);
    converter.setDocumentOffset(QPoint(-50,-50));
    converter.setCanvasWidgetSize(widgetSize);

//    QTransform imageToWidget;
//    QTransform documentToWidget;
//    QTransform flakeToWidget;
//    QTransform viewportToWidget;

    converter.mirror(converter.imageCenterInWidgetPixel(), true, false);

    // image pixels == flake pixels

    QRectF viewportRect = converter.imageToViewport(testRect);
    QRectF widgetRect = converter.viewportToWidget(viewportRect);

    QCOMPARE(widgetRect, QRectF(550,150,200,200));
    QCOMPARE(viewportRect, QRectF(300,100,200,200));
}

void KisCoordinatesConverterTest::testCanvasOffset()
{
    KisImageSP image;
    KisCoordinatesConverter converter;
    initImage(&image, &converter);

    converter.setImage(image);
    converter.setZoom(1.);
    converter.setDocumentOffset(QPoint(0,0));
    converter.setCanvasWidgetSize(QSize(500,500));

    qDebug() << ppVar(converter.imageToWidget(QPointF(0,0)));

    converter.setZoom(0.5);

    qDebug() << ppVar(converter.imageToWidget(QPointF(0,0)));

    {
        // move the image to the top and left direction (the offset is inverted)
        converter.setDocumentOffset(QPoint(0, 0));
        qDebug() << "before mirror" << ppVar(converter.documentOffset());

        qDebug() << "  " << ppVar(converter.imageToWidget(QPointF(0, 0)));
        qDebug() << "  " << ppVar(converter.imageToWidget(QPointF(1000, 0)));

        converter.mirror(converter.widgetCenterPoint(), true, false);
        qDebug() << "after mirror" << ppVar(converter.documentOffset());

        qDebug() << "  " << ppVar(converter.imageToWidget(QPointF(0, 0)));
        qDebug() << "  " << ppVar(converter.imageToWidget(QPointF(1000, 0)));

        converter.mirror(converter.widgetCenterPoint(), false, false);
    }

    {
        // move the image to the top and left direction (the offset is inverted)
        converter.setDocumentOffset(QPoint(100, 200));
        qDebug() << "before mirror" << ppVar(converter.documentOffset());

        qDebug() << "  " << ppVar(converter.imageToWidget(QPointF(0, 0)));
        qDebug() << "  " << ppVar(converter.imageToWidget(QPointF(1000, 0)));

        converter.mirror(converter.widgetCenterPoint(), true, false);
        qDebug() << "after mirror" << ppVar(converter.documentOffset());

        qDebug() << "  " << ppVar(converter.imageToWidget(QPointF(0, 0)));
        qDebug() << "  " << ppVar(converter.imageToWidget(QPointF(1000, 0)));

        converter.mirror(converter.widgetCenterPoint(), false, false);
    }

    {
        // move the image to the top and left direction (the offset is inverted)
        converter.setDocumentOffset(QPoint(100, 200));
        qDebug() << "before rotate" << ppVar(converter.documentOffset());

        qDebug() << "  " << ppVar(converter.imageToWidget(QPointF(0, 0)));
        qDebug() << "  " << ppVar(converter.imageToWidget(QPointF(1000, 0)));

        converter.rotate(converter.widgetCenterPoint(), 30);
        qDebug() << "after rotate" << ppVar(converter.documentOffset());

        qDebug() << "  " << ppVar(converter.imageToWidget(QPointF(0, 0)));
        qDebug() << "  " << ppVar(converter.imageToWidget(QPointF(1000, 0)));
        qDebug() << "  " << ppVar(converter.imageToWidget(QPointF(1000, 1000)));
        qDebug() << "  " << ppVar(converter.imageToWidget(QPointF(0, 1000)));

        converter.mirror(converter.widgetCenterPoint(), false, false);
    }
}

void KisCoordinatesConverterTest::testImageSmallerThanCanvas()
{
    KisImageSP image;
    KisCoordinatesConverter converter;
    initImage(&image, &converter);

    converter.setImage(image);
    converter.setZoom(1.);
    converter.setDocumentOffset(QPoint(0,0));
    converter.setCanvasWidgetSize(QSize(500,500));


    qDebug() << ppVar(converter.imageToWidget(QPointF(0,0)));

    converter.setZoom(0.5);

}

void KisCoordinatesConverterTest::testImageResolutionChange()
{
    KisImageSP image;
    KisCoordinatesConverter converter;
    initImage(&image, &converter);

    converter.setImage(image);
    converter.setZoom(0.5);
    converter.setDocumentOffset(QPoint(0,0));
    converter.setCanvasWidgetSize(QSize(500,500));

    qreal oldXScale = 0.0, oldYScale = 0.0;
    converter.imageScale(&oldXScale, &oldYScale);
    const QRectF oldImageRectInDocumentPixels = converter.imageRectInDocumentPixels();

    /**
     * The center of the image should be kept unchanged when resolution changes
     */
    const QPointF oldImageCenterInWidgetPixels = converter.imageCenterInWidgetPixel();
    qDebug() << "0" << ppVar(converter.imageCenterInWidgetPixel());

    KisFilterStrategy *strategy = new KisBilinearFilterStrategy();
    image->scaleImage(image->size(), 200, 200, strategy);
    image->waitForDone();
    QTest::qWait(100);

    /**
     * We do **not** directly link the converter to the image.
     * All updates to the converter should be propagated manually
     * by KisView
     */

    qreal newXScale = 0.0, newYScale = 0.0;

    converter.imageScale(&newXScale, &newYScale);
    QCOMPARE(newXScale, oldXScale);
    QCOMPARE(newYScale, oldYScale);
    QCOMPARE(converter.imageRectInDocumentPixels(), oldImageRectInDocumentPixels);
    QCOMPARE(converter.imageCenterInWidgetPixel(), oldImageCenterInWidgetPixels);

    converter.setImageResolution(image->xRes(), image->yRes());

    qDebug() << "1" << ppVar(converter.imageCenterInWidgetPixel());

    converter.imageScale(&newXScale, &newYScale);
    QCOMPARE(newXScale, 0.5 * oldXScale);
    QCOMPARE(newYScale, 0.5 * oldYScale);
    QCOMPARE(converter.imageRectInDocumentPixels(), QRectF(0,0,5,5));
    // the position of image center should be the same
    QCOMPARE(converter.imageCenterInWidgetPixel(), oldImageCenterInWidgetPixels);
}

void KisCoordinatesConverterTest::testOffsetLimits()
{
    KisImageSP image;
    KisCoordinatesConverter converter;
    initImage(&image, &converter);

    const QRect widgetRect(0,0,500,500);
    converter.setImage(image);
    converter.setZoom(0.5);
    converter.setDocumentOffset(QPoint(0,0));
    converter.setCanvasWidgetSize(widgetRect.size()); // should be initialized first

    const QPointF widgetSizeVector(widgetRect.width(), widgetRect.height());

    // initial offset is null, hence the topleft is at the origin of the widget
    QCOMPARE(converter.imageToWidget(QPointF(0,0)), QPointF(0,0));

    // when the scrollbars offset in minimal, the topleft of the image is moved
    // to the bottom-right corner of the widget
    converter.setDocumentOffset(converter.minimumOffset());
    QCOMPARE(converter.imageToWidget(QPointF(0,0)), 0.9 * widgetSizeVector);

    // when the scrollbars offset in maximal, the topleft of the image is moved
    // under the left side of the widget and we see only the bottom-right corner
    // of the image.
    converter.setDocumentOffset(converter.maximumOffset());
    QCOMPARE(converter.imageToWidget(QPointF(1000,1000)), 0.1 * widgetSizeVector);
}

void KisCoordinatesConverterTest::testOffsetLimitsCropping()
{
    KisImageSP image;
    KisCoordinatesConverter converter;
    initImage(&image, &converter);

    const QRect widgetRect(0,0,500,500);
    converter.setImage(image);
    converter.setZoom(0.5);
    converter.setDocumentOffset(QPoint(0,0));
    converter.setCanvasWidgetSize(widgetRect.size()); // should be initialized first

    QCOMPARE(converter.documentOffset(), QPoint(0,0));
    QCOMPARE(converter.imageToWidget(QPointF(0,0)), QPointF(0,0));
    QCOMPARE(converter.imageToWidget(QPointF(1000,1000)), QPointF(500,500));

    // This mirror operation cause clipping of the offset, since
    // the left edge of the document shoudl appear at position
    // 480, which is higer than the limit of 450.
    converter.mirror(QPointF(490, 0), true, false);

    QCOMPARE(converter.documentOffset(), QPoint(-450,0));
    QCOMPARE(converter.imageToWidget(QPointF(0,0)), QPointF(950,0));
    QCOMPARE(converter.imageToWidget(QPointF(1000,1000)), QPointF(450,500));

    // return the canvas back to the topleft (though mirrored)
    converter.setDocumentOffset(QPoint(0,0));
    QCOMPARE(converter.documentOffset(), QPoint(0,0));
    QCOMPARE(converter.imageToWidget(QPointF(0,0)), QPointF(500,0));
    QCOMPARE(converter.imageToWidget(QPointF(1000,1000)), QPointF(0,500));

    // Now mirror on the left side. The offset should be corrected
    // to the right
    converter.mirror(QPointF(10, 0), false, false);

    QCOMPARE(converter.documentOffset(), QPoint(450,0));
    QCOMPARE(converter.imageToWidget(QPointF(0,0)), QPointF(-450,0));
    QCOMPARE(converter.imageToWidget(QPointF(1000,1000)), QPointF(50,500));
}

using PointPairs = QVector<std::pair<QPointF, QPointF>>;

void KisCoordinatesConverterTest::testZoomMode_data()
{
    QTest::addColumn<int>("zoomMargin");
    QTest::addColumn<QSize>("widgetSize");
    QTest::addColumn<QPointF>("originalOffset");
    QTest::addColumn<qreal>("originalZoom");
    QTest::addColumn<PointPairs>("originalTestPoints");
    QTest::addColumn<KoZoomMode::Mode>("newZoomMode");
    QTest::addColumn<qreal>("newConstantZoom");
    QTest::addColumn<QPointF>("widgetStillPoint");
    QTest::addColumn<QPointF>("newScreenResolution");
    QTest::addColumn<qreal>("expectedZoom");
    QTest::addColumn<PointPairs>("expectedTestPoints");

    const qreal unusedNewConstantZoom = 7777.7;
    const QPointF unusedWidgetStillPoint(8888.8, 9999.9);
    const QPointF defaultDisplayResolution(100.0, 100.0);

    QTest::newRow("constant")
        << 0
        << QSize(700, 500)
        << QPointF(-100,-100)
        << 1.0
        << PointPairs{
                      {{0,0},     {100,100}},
                      {{100,100}, {200,200}},
                      {{200,200}, {300,300}} // still point
                     }
        << KoZoomMode::ZOOM_CONSTANT
        << 0.7
        << QPointF(300, 300) // widget still point
        << defaultDisplayResolution
        << 0.7
        << PointPairs{
                      {{0,0},     {160, 160}},
                      {{100,100}, {230,230}},
                      {{1000,0},  {860,160}},
                      {{200,200}, {300,300}}, // still point
                     };

    QTest::newRow("constant-double-resolution")
        << 0
        << QSize(700, 500)
        << QPointF(-100,-100)
        << 1.0
        << PointPairs{
                    {{0,0},     {100,100}},
                    {{100,100}, {200,200}},
                    {{200,200}, {300,300}} // still point
                    }
        << KoZoomMode::ZOOM_CONSTANT
        << 0.7
        << QPointF(300, 300) // widget still point
        << 2.0 * defaultDisplayResolution
        << 0.7
        << PointPairs{
                    {{0,0},     {20, 20}},
                    {{100,100}, {160,160}},
                    {{1000,0},  {1420,20}},
                    {{200,200}, {300,300}}, // still point
                    };

    QTest::newRow("width-shrink")
        << 0
        << QSize(700, 500)
        << QPointF(-100,-100)
        << 1.0
        << PointPairs{
                        {{0,0},     {100,100}},
                        {{100,100}, {200,200}},
                        {{500,500}, {600,600}} // the ceter of the image is at random point
                        }
        << KoZoomMode::ZOOM_WIDTH
        << unusedNewConstantZoom
        << unusedWidgetStillPoint
        << defaultDisplayResolution
        << 0.7
        << PointPairs{
                        {{0,0},     {0,-100}}, // the left image border is aligned to the left
                        {{100,100}, {70,-30}},
                        {{1000,0},  {700,-100}}, // the right image border is aligned to the right
                        {{500,500}, {350,250}}, // the center of the image is at center of the widget
                        };

    QTest::newRow("width-shrink-double-resolution")
        << 0
        << QSize(700, 500)
        << QPointF(-100,-100)
        << 1.0
        << PointPairs{
                        {{0,0},     {100,100}},
                        {{100,100}, {200,200}},
                        {{500,500}, {600,600}} // the ceter of the image is at random point
                        }
        << KoZoomMode::ZOOM_WIDTH
        << unusedNewConstantZoom
        << unusedWidgetStillPoint
        << 2.0 * defaultDisplayResolution
        << 0.35 // the only difference is that the percieved zoom is halved
        << PointPairs{
                        {{0,0},     {0,-100}}, // the left image border is aligned to the left
                        {{100,100}, {70,-30}},
                        {{1000,0},  {700,-100}}, // the right image border is aligned to the right
                        {{500,500}, {350,250}}, // the center of the image is at center of the widget
                        };

    QTest::newRow("width-grow")
        << 0
        << QSize(700, 500)
        << QPointF(-100,-100)
        << 0.1
        << PointPairs{
                      {{0,0},     {100,100}},
                      {{100,100}, {110,110}},
                      {{1000,1000}, {200,200}},
                      {{500,500}, {150,150}} // the ceter of the image is at random point
                     }
        << KoZoomMode::ZOOM_WIDTH
        << unusedNewConstantZoom
        << unusedWidgetStillPoint
        << defaultDisplayResolution
        << 0.7
        << PointPairs{
                      {{0,0},     {0,-100}}, // the left image border is aligned to the left
                      {{100,100}, {70,-30}},
                      {{1000,0},  {700,-100}}, // the right image border is aligned to the right
                      {{500,500}, {350,250}}, // the center of the image is at center of the widget
                     };

    QTest::newRow("width-shrink-with-margin")
            << 50
            << QSize(700, 500)
            << QPointF(-100,-100)
            << 1.0
            << PointPairs{
                          {{0,0},     {100,100}},
                          {{100,100}, {200,200}},
                          {{500,500}, {600,600}} // the ceter of the image is at random point
                         }
            << KoZoomMode::ZOOM_WIDTH
            << unusedNewConstantZoom
            << unusedWidgetStillPoint
            << defaultDisplayResolution
            << 0.6
            << PointPairs{
                          {{0,0},     {50,-50}}, // the left image border is aligned to the left
                          {{100,100}, {110,10}},
                          {{1000,0},  {650,-50}}, // the right image border is aligned to the right
                          {{500,500}, {350,250}}, // the center of the image is at center of the widget
                         };

    QTest::newRow("height-shrink")
        << 0
        << QSize(700, 500)
        << QPointF(-100,-100)
        << 1.0
        << PointPairs{
                      {{0,0},     {100,100}},
                      {{100,100}, {200,200}},
                      {{500,500}, {600,600}} // the ceter of the image is at random point
                     }
        << KoZoomMode::ZOOM_HEIGHT
        << unusedNewConstantZoom
        << unusedWidgetStillPoint
        << defaultDisplayResolution
        << 0.5
        << PointPairs{
                      {{0,0},     {100,0}}, // the top image border is aligned to the top
                      {{100,100}, {150,50}},
                      {{0,1000},  {100,500}}, // the bottom image border is aligned to the bottom
                      {{500,500}, {350,250}}, // the center of the image is at center of the widget
                     };

    QTest::newRow("height-shrink-double-resolution")
        << 0
        << QSize(700, 500)
        << QPointF(-100,-100)
        << 1.0
        << PointPairs{
                    {{0,0},     {100,100}},
                    {{100,100}, {200,200}},
                    {{500,500}, {600,600}} // the ceter of the image is at random point
                    }
        << KoZoomMode::ZOOM_HEIGHT
        << unusedNewConstantZoom
        << unusedWidgetStillPoint
        << 2.0 * defaultDisplayResolution
        << 0.25 // the only difference is that the percieved zoom is halved
        << PointPairs{
                    {{0,0},     {100,0}}, // the top image border is aligned to the top
                    {{100,100}, {150,50}},
                    {{0,1000},  {100,500}}, // the bottom image border is aligned to the bottom
                    {{500,500}, {350,250}}, // the center of the image is at center of the widget
                    };

    QTest::newRow("height-grow")
        << 0
        << QSize(700, 500)
        << QPointF(-100,-100)
        << 0.1
        << PointPairs{
                    {{0,0},     {100,100}},
                    {{100,100}, {110,110}},
                    {{1000,1000}, {200,200}},
                    {{500,500}, {150,150}} // the ceter of the image is at random point
                    }
        << KoZoomMode::ZOOM_HEIGHT
        << unusedNewConstantZoom
        << unusedWidgetStillPoint
        << defaultDisplayResolution
        << 0.5
        << PointPairs{
                      {{0,0},     {100,0}}, // the top image border is aligned to the top
                      {{100,100}, {150,50}},
                      {{0,1000},  {100,500}}, // the bottom image border is aligned to the bottom
                      {{500,500}, {350,250}}, // the center of the image is at center of the widget
                     };

    QTest::newRow("height-shrink-with-margin")
        << 50
        << QSize(700, 500)
        << QPointF(-100,-100)
        << 1.0
        << PointPairs{
                    {{0,0},     {100,100}},
                    {{100,100}, {200,200}},
                    {{500,500}, {600,600}} // the ceter of the image is at random point
                    }
        << KoZoomMode::ZOOM_HEIGHT
        << unusedNewConstantZoom
        << unusedWidgetStillPoint
        << defaultDisplayResolution
        << 0.4
        << PointPairs{
            {{0,0},     {150,50}}, // the top image border is aligned to the top
            {{100,100}, {190,90}},
            {{0,1000},  {150,450}}, // the bottom image border is aligned to the bottom
            {{500,500}, {350,250}}, // the center of the image is at center of the widget
           };

    QTest::newRow("page-vertically")
        << 0
        << QSize(700, 500)
        << QPointF(-100,-100)
        << 1.0
        << PointPairs{
                      {{0,0},     {100,100}},
                      {{100,100}, {200,200}},
                      {{500,500}, {600,600}}, // center of the image should become in the center of the canvas
                     }
        << KoZoomMode::ZOOM_PAGE
        << unusedNewConstantZoom
        << unusedWidgetStillPoint
        << defaultDisplayResolution
        << 0.5
        << PointPairs{
            {{0,0},     {100,0}}, // the top image border is aligned to the top
            {{100,100}, {150,50}},
            {{1000,1000},  {600,500}}, // the bottom image border is aligned to the bottom
            {{500,500}, {350,250}}, // center of the image should become in the center of the canvas
           };

    QTest::newRow("page-vertically-double-resolution")
        << 0
        << QSize(700, 500)
        << QPointF(-100,-100)
        << 1.0
        << PointPairs{
                      {{0,0},     {100,100}},
                      {{100,100}, {200,200}},
                      {{500,500}, {600,600}}, // center of the image should become in the center of the canvas
                     }
        << KoZoomMode::ZOOM_PAGE
        << unusedNewConstantZoom
        << unusedWidgetStillPoint
        << 2.0 * defaultDisplayResolution
        << 0.25 // the only difference is that the percieved zoom is halved
        << PointPairs{
            {{0,0},     {100,0}}, // the top image border is aligned to the top
            {{100,100}, {150,50}},
            {{1000,1000},  {600,500}}, // the bottom image border is aligned to the bottom
            {{500,500}, {350,250}}, // center of the image should become in the center of the canvas
           };

    QTest::newRow("page-vertically-margin")
        << 50
        << QSize(700, 500)
        << QPointF(-100,-100)
        << 1.0
        << PointPairs{
                        {{0,0},     {100,100}},
                        {{100,100}, {200,200}},
                        {{500,500}, {600,600}}, // center of the image should become in the center of the canvas
                    }
        << KoZoomMode::ZOOM_PAGE
        << unusedNewConstantZoom
        << unusedWidgetStillPoint
        << defaultDisplayResolution
        << 0.4
        << PointPairs{
            {{0,0},     {150,50}}, // the top image border is aligned to the top
            {{100,100}, {190,90}},
            {{1000,1000},  {550,450}}, // the bottom image border is aligned to the bottom
            {{500,500}, {350,250}}, // center of the image should become in the center of the canvas
            };

    QTest::newRow("page-horizontally")
        << 0
        << QSize(700, 800)
        << QPointF(-100,-100)
        << 1.0
        << PointPairs{
                        {{0,0},     {100,100}},
                        {{100,100}, {200,200}},
                        {{500,500}, {600,600}}, // center of the image should become in the center of the canvas
                        }
        << KoZoomMode::ZOOM_PAGE
        << unusedNewConstantZoom
        << unusedWidgetStillPoint
        << defaultDisplayResolution
        << 0.7
        << PointPairs{
            {{0,0},     {0,50}}, // the left image border is aligned to the left
            {{100,100}, {70,120}},
            {{1000,1000},  {700,750}}, // the right image border is aligned to the right
            {{500,500}, {350,400}}, // center of the image should become in the center of the canvas
            };

    QTest::newRow("page-horizontally-double-resolution")
        << 0
        << QSize(700, 800)
        << QPointF(-100,-100)
        << 1.0
        << PointPairs{
                        {{0,0},     {100,100}},
                        {{100,100}, {200,200}},
                        {{500,500}, {600,600}}, // center of the image should become in the center of the canvas
                        }
        << KoZoomMode::ZOOM_PAGE
        << unusedNewConstantZoom
        << unusedWidgetStillPoint
        << 2.0 * defaultDisplayResolution
        << 0.35 // the only difference is that the percieved zoom is halved
        << PointPairs{
            {{0,0},     {0,50}}, // the left image border is aligned to the left
            {{100,100}, {70,120}},
            {{1000,1000},  {700,750}}, // the right image border is aligned to the right
            {{500,500}, {350,400}}, // center of the image should become in the center of the canvas
            };

    QTest::newRow("page-horizontally-margin")
            << 50
            << QSize(700, 800)
            << QPointF(-100,-100)
            << 1.0
            << PointPairs{
                            {{0,0},     {100,100}},
                            {{100,100}, {200,200}},
                            {{500,500}, {600,600}}, // center of the image should become in the center of the canvas
                            }
            << KoZoomMode::ZOOM_PAGE
            << unusedNewConstantZoom
            << unusedWidgetStillPoint
            << defaultDisplayResolution
            << 0.6
            << PointPairs{
                {{0,0},     {50,100}}, // the left image border is aligned to the left
                {{100,100}, {110,160}},
                {{1000,1000},  {650,700}}, // the right image border is aligned to the right
                {{500,500}, {350,400}}, // center of the image should become in the center of the canvas
                };
}

void KisCoordinatesConverterTest::testZoomMode()
{
    QFETCH(int, zoomMargin);
    QFETCH(QSize, widgetSize);
    QFETCH(QPointF, originalOffset);
    QFETCH(qreal, originalZoom);
    QFETCH(PointPairs, originalTestPoints);
    QFETCH(KoZoomMode::Mode, newZoomMode);
    QFETCH(qreal, newConstantZoom);
    QFETCH(QPointF, widgetStillPoint);
    QFETCH(QPointF, newScreenResolution);
    QFETCH(qreal, expectedZoom);
    QFETCH(PointPairs, expectedTestPoints);

    KisImageSP image;
    KisCoordinatesConverter converter;
    initImage(&image, &converter);

    const QRect widgetRect(QPoint(), widgetSize);
    converter.setZoomMarginSize(zoomMargin);
    converter.setImage(image);
    converter.setZoom(originalZoom);
    converter.setDocumentOffset(originalOffset);
    converter.setCanvasWidgetSize(widgetRect.size());

    for (const auto &pair : std::as_const(originalTestPoints)) {
        const QPointF realPointWidgetPos = converter.imageToWidget(pair.first);

        if (!KisAlgebra2D::fuzzyPointCompare(realPointWidgetPos, pair.second)) {
            qWarning() << "Failed to compare test points before transformation:";
            qWarning() << "    image point:          " << pair.first;
            qWarning() << "    real widget point:    " << realPointWidgetPos;
            qWarning() << "    expected widget point:" << pair.second;
            QFAIL("faled to compare points before transforamation");
        }
    }

    converter.setZoom(newZoomMode, newConstantZoom, newScreenResolution.x(), newScreenResolution.y(), widgetStillPoint);

    QCOMPARE(converter.zoom(), expectedZoom);
    QCOMPARE(converter.zoomMode(), newZoomMode);
    for (const auto &pair : std::as_const(expectedTestPoints)) {
        const QPointF realPointWidgetPos = converter.imageToWidget(pair.first);

        if (!KisAlgebra2D::fuzzyPointCompare(realPointWidgetPos, pair.second)) {
            qWarning() << "Failed to compare test points after transformation:";
            qWarning() << "    image point:          " << pair.first;
            qWarning() << "    real widget point:    " << realPointWidgetPos;
            qWarning() << "    expected widget point:" << pair.second;
            QFAIL("faled to compare points after transforamation");
        }
    }
}

void KisCoordinatesConverterTest::testChangeCanvasSize_data()
{
    QTest::addColumn<KoZoomMode::Mode>("zoomMode");
    QTest::addColumn<QSize>("originalWidgetSize");
    QTest::addColumn<qreal>("originalZoom");
    QTest::addColumn<QPointF>("originalOffset");
    QTest::addColumn<PointPairs>("originalTestPoints");
    QTest::addColumn<QSize>("newWidgetSize");
    QTest::addColumn<qreal>("expectedZoom");
    QTest::addColumn<PointPairs>("expectedTestPoints");

    const QPointF unusedOriginalDocumentOffset(8888.8, 9999.9);

    QTest::newRow("constant")
        << KoZoomMode::ZOOM_CONSTANT
        << QSize(700, 500)
        << 0.5
        << QPointF(-100, -100)
        << PointPairs{
            {{0,0},     {100,100}},
            {{100,100}, {150,150}},
            {{200,200}, {200,200}}
           }
        << QSize(200, 200) // nothing changes with the widget resize!
        << 0.5
        << PointPairs{
            {{0,0},     {100,100}},
            {{100,100}, {150,150}},
            {{200,200}, {200,200}}
           };

    QTest::newRow("page-vertically")
           << KoZoomMode::ZOOM_PAGE
           << QSize(700, 500)
           << 0.5
           << unusedOriginalDocumentOffset
           << PointPairs{
               {{0,0},     {100,0}},
               {{100,100}, {150,50}},
               {{500,500}, {350,250}} // centers are aligned
              }
           << QSize(200, 100)
           << 0.1
           << PointPairs{
               {{0,0},     {50,0}},
               {{100,100}, {60,10}},
               {{500,500}, {100,50}} // centers are aligned
              };

    QTest::newRow("page-horizontally")
        << KoZoomMode::ZOOM_PAGE
        << QSize(700, 500)
        << 0.5
        << unusedOriginalDocumentOffset
        << PointPairs{
            {{0,0},     {100,0}},
            {{100,100}, {150,50}},
            {{500,500}, {350,250}} // centers are aligned
            }
        << QSize(100, 200)
        << 0.1
        << PointPairs{
            {{0,0},     {0, 50}},
            {{100,100}, {10, 60}},
            {{500,500}, {50, 100}} // centers are aligned
            };

    QTest::newRow("width")
        << KoZoomMode::ZOOM_WIDTH
        << QSize(700, 500)
        << 0.7
        << unusedOriginalDocumentOffset
        << PointPairs{
            {{0,0},     {0,-100}},
            {{100,100}, {70,-30}},
            {{500,500}, {350,250}} // centers are aligned
            }
        << QSize(200, 100)
        << 0.2
        << PointPairs{
            {{0,0},     {0,-50}},
            {{100,100}, {20,-30}},
            {{500,500}, {100,50}} // centers are aligned
            };

    QTest::newRow("height")
           << KoZoomMode::ZOOM_HEIGHT
           << QSize(700, 500)
           << 0.5
           << unusedOriginalDocumentOffset
           << PointPairs{
               {{0,0},     {100,0}},
               {{100,100}, {150,50}},
               {{500,500}, {350,250}} // centers are aligned
              }
           << QSize(200, 100)
           << 0.1
           << PointPairs{
               {{0,0},     {50,0}},
               {{100,100}, {60,10}},
               {{500,500}, {100,50}} // centers are aligned
              };
}

void KisCoordinatesConverterTest::testChangeCanvasSize()
{
    QFETCH(KoZoomMode::Mode, zoomMode);
    QFETCH(qreal, originalZoom);
    QFETCH(QPointF, originalOffset);
    QFETCH(QSize, originalWidgetSize);
    QFETCH(PointPairs, originalTestPoints);
    QFETCH(QSize, newWidgetSize);
    QFETCH(qreal, expectedZoom);
    QFETCH(PointPairs, expectedTestPoints);

    KisImageSP image;
    KisCoordinatesConverter converter;
    initImage(&image, &converter);

    converter.setImage(image);
    converter.setCanvasWidgetSize(originalWidgetSize);
    converter.setZoom(zoomMode, originalZoom, image->xRes(), image->yRes(), QRectF(QPointF(), originalWidgetSize).center());
    if (zoomMode == KoZoomMode::ZOOM_CONSTANT) {
        converter.setDocumentOffset(originalOffset);
    }

    QCOMPARE(converter.zoom(), originalZoom);
    QCOMPARE(converter.zoomMode(), zoomMode);

    for (const auto &pair : std::as_const(originalTestPoints)) {
        const QPointF realPointWidgetPos = converter.imageToWidget(pair.first);

        if (!KisAlgebra2D::fuzzyPointCompare(realPointWidgetPos, pair.second)) {
            qWarning() << "Failed to compare test points before transformation:";
            qWarning() << "    image point:          " << pair.first;
            qWarning() << "    real widget point:    " << realPointWidgetPos;
            qWarning() << "    expected widget point:" << pair.second;
            QFAIL("faled to compare points before transforamation");
        }
    }

    converter.setCanvasWidgetSizeKeepZoom(newWidgetSize);

    QCOMPARE(converter.zoom(), expectedZoom);
    QCOMPARE(converter.zoomMode(), zoomMode);

    for (const auto &pair : std::as_const(expectedTestPoints)) {
        const QPointF realPointWidgetPos = converter.imageToWidget(pair.first);

        if (!KisAlgebra2D::fuzzyPointCompare(realPointWidgetPos, pair.second)) {
            qWarning() << "Failed to compare test points after transformation:";
            qWarning() << "    image point:          " << pair.first;
            qWarning() << "    real widget point:    " << realPointWidgetPos;
            qWarning() << "    expected widget point:" << pair.second;
            QFAIL("faled to compare points after transforamation");
        }
    }
}

void KisCoordinatesConverterTest::testChangeImageResolution_data()
{
    QTest::addColumn<KoZoomMode::Mode>("zoomMode");
    QTest::addColumn<qreal>("originalZoom");
    QTest::addColumn<QPointF>("originalOffset");
    QTest::addColumn<PointPairs>("originalTestPoints");
    QTest::addColumn<QPointF>("newImageResolution");
    QTest::addColumn<qreal>("expectedZoom");
    QTest::addColumn<qreal>("expectedEffectiveZoom");
    QTest::addColumn<PointPairs>("expectedTestPoints");

    const QPointF unusedOriginalDocumentOffset(8888.8, 9999.9);

    QTest::newRow("constant")
        << KoZoomMode::ZOOM_CONSTANT
        << 0.5
        << QPointF(-100, -100)
        << PointPairs{
            {{500,500}, {350,350}}, // still point
            {{0,0},     {100,100}},
            {{100,100}, {150,150}},
           }
        << QPointF(200, 200)
        << 0.5
        << 0.25
        << PointPairs{
            {{500,500}, {350,350}}, // still point
            {{0,0},     {225,225}},
            {{100,100}, {250,250}},
           };

    // all "page-style" zoom modes reset into "constant" mode
    QTest::newRow("page-vertically")
           << KoZoomMode::ZOOM_PAGE
           << 0.5
           << unusedOriginalDocumentOffset
           << PointPairs{
               {{500,500}, {350,250}}, // centers are aligned
               {{0,0},     {100,0}},
               {{100,100}, {150,50}},
              }
           << QPointF(200, 200)
           << 0.5
           << 0.25
           << PointPairs{
               {{500,500}, {350,250}}, // centers are still aligned
               {{0,0},     {225,125}}, // but the top side is not aligned anymore!
               {{100,100}, {250,150}},
              };
}

void KisCoordinatesConverterTest::testChangeImageResolution()
{
    QFETCH(KoZoomMode::Mode, zoomMode);
    QFETCH(qreal, originalZoom);
    QFETCH(QPointF, originalOffset);
    QFETCH(PointPairs, originalTestPoints);
    QFETCH(QPointF, newImageResolution);
    QFETCH(qreal, expectedZoom);
    QFETCH(qreal, expectedEffectiveZoom);
    QFETCH(PointPairs, expectedTestPoints);

    const QSize widgetSize(700, 500);

    KisImageSP image;
    KisCoordinatesConverter converter;
    initImage(&image, &converter);

    converter.setImage(image);
    converter.setCanvasWidgetSize(widgetSize);
    converter.setZoom(zoomMode, originalZoom, image->xRes(), image->yRes(), QRectF(QPointF(), widgetSize).center());
    if (zoomMode == KoZoomMode::ZOOM_CONSTANT) {
        converter.setDocumentOffset(originalOffset);
    }

    QCOMPARE(converter.zoom(), originalZoom);
    QCOMPARE(converter.zoomMode(), zoomMode);

    for (const auto &pair : std::as_const(originalTestPoints)) {
        const QPointF realPointWidgetPos = converter.imageToWidget(pair.first);

        if (!KisAlgebra2D::fuzzyPointCompare(realPointWidgetPos, pair.second)) {
            qWarning() << "Failed to compare test points before transformation:";
            qWarning() << "    image point:          " << pair.first;
            qWarning() << "    real widget point:    " << realPointWidgetPos;
            qWarning() << "    expected widget point:" << pair.second;
            QFAIL("faled to compare points before transforamation");
        }
    }

    converter.setImageResolution(newImageResolution.x(), newImageResolution.y());

    /**
     * Zoom mode should reset into "constant mode to let the user see that the
     * image has actually changed
     */
    QCOMPARE(converter.zoomMode(), KoZoomMode::ZOOM_CONSTANT);
    QCOMPARE(converter.zoom(), expectedZoom);
    QCOMPARE(converter.effectiveZoom(), expectedEffectiveZoom);

    for (const auto &pair : std::as_const(expectedTestPoints)) {
        const QPointF realPointWidgetPos = converter.imageToWidget(pair.first);

        if (!KisAlgebra2D::fuzzyPointCompare(realPointWidgetPos, pair.second)) {
            qWarning() << "Failed to compare test points after transformation:";
            qWarning() << "    image point:          " << pair.first;
            qWarning() << "    real widget point:    " << realPointWidgetPos;
            qWarning() << "    expected widget point:" << pair.second;
            QFAIL("faled to compare points after transforamation");
        }
    }
}

void KisCoordinatesConverterTest::testChangeImageSize_data()
{
    QTest::addColumn<KoZoomMode::Mode>("zoomMode");
    QTest::addColumn<qreal>("originalZoom");
    QTest::addColumn<QPointF>("originalOffset");
    QTest::addColumn<PointPairs>("originalTestPoints");
    QTest::addColumn<QSize>("newImageSize");
    QTest::addColumn<QPointF>("oldImageStillPoint");
    QTest::addColumn<QPointF>("newImageStillPoint");
    QTest::addColumn<qreal>("expectedZoom");
    QTest::addColumn<PointPairs>("expectedTestPoints");

    const QPointF unusedOriginalDocumentOffset(8888.8, 9999.9);

    QTest::newRow("constant")
        << KoZoomMode::ZOOM_CONSTANT
        << 0.25
        << QPointF(-100, -100)
        << PointPairs{
            {{900,900}, {325,325}}, // still point
            {{0,0},     {100,100}},
            {{100,100}, {125,125}},
           }
        << QSize(200, 200) // crop the bottom-right corner of the image
        << QPointF(900,900)
        << QPointF(100,100)
        << 0.25
        << PointPairs{
            {{100,100}, {325,325}}, // still point
            {{0,0},     {300,300}},
           };

    // all "page-style" zoom modes reset into "constant" mode
    QTest::newRow("page-vertically")
           << KoZoomMode::ZOOM_PAGE
           << 0.5
           << unusedOriginalDocumentOffset
           << PointPairs{
               {{500,500}, {350,250}}, // centers are aligned
               {{0,0},     {100,0}},
               {{100,100}, {150,50}},
               {{900,900}, {550,450}}, // still point
              }
           << QSize(200, 200) // crop the bottom-right corner of the image
           << QPointF(900,900)
           << QPointF(100,100)
           << 0.5
           << PointPairs{
               {{100,100}, {550,450}}, // still point
               {{0,0},     {500,400}},
              };
}

void KisCoordinatesConverterTest::testChangeImageSize()
{
    QFETCH(KoZoomMode::Mode, zoomMode);
    QFETCH(qreal, originalZoom);
    QFETCH(QPointF, originalOffset);
    QFETCH(PointPairs, originalTestPoints);
    QFETCH(QSize, newImageSize);
    QFETCH(QPointF, oldImageStillPoint);
    QFETCH(QPointF, newImageStillPoint);
    QFETCH(qreal, expectedZoom);
    QFETCH(PointPairs, expectedTestPoints);

    const QSize widgetSize(700, 500);

    KisImageSP image;
    KisCoordinatesConverter converter;
    initImage(&image, &converter);

    converter.setImage(image);
    converter.setCanvasWidgetSize(widgetSize);
    converter.setZoom(zoomMode, originalZoom, image->xRes(), image->yRes(), QRectF(QPointF(), widgetSize).center());
    if (zoomMode == KoZoomMode::ZOOM_CONSTANT) {
        converter.setDocumentOffset(originalOffset);
    }

    QCOMPARE(converter.zoom(), originalZoom);
    QCOMPARE(converter.zoomMode(), zoomMode);

    for (const auto &pair : std::as_const(originalTestPoints)) {
        const QPointF realPointWidgetPos = converter.imageToWidget(pair.first);

        if (!KisAlgebra2D::fuzzyPointCompare(realPointWidgetPos, pair.second)) {
            qWarning() << "Failed to compare test points before transformation:";
            qWarning() << "    image point:          " << pair.first;
            qWarning() << "    real widget point:    " << realPointWidgetPos;
            qWarning() << "    expected widget point:" << pair.second;
            QFAIL("faled to compare points before transforamation");
        }
    }

    converter.setImageBounds(QRect(QPoint(), newImageSize), oldImageStillPoint, newImageStillPoint);

    /**
     * Zoom mode should reset into "constant mode to let the user see that the
     * image has actually changed
     */
    QCOMPARE(converter.zoomMode(), KoZoomMode::ZOOM_CONSTANT);
    QCOMPARE(converter.zoom(), expectedZoom);

    for (const auto &pair : std::as_const(expectedTestPoints)) {
        const QPointF realPointWidgetPos = converter.imageToWidget(pair.first);

        if (!KisAlgebra2D::fuzzyPointCompare(realPointWidgetPos, pair.second)) {
            qWarning() << "Failed to compare test points after transformation:";
            qWarning() << "    image point:          " << pair.first;
            qWarning() << "    real widget point:    " << realPointWidgetPos;
            qWarning() << "    expected widget point:" << pair.second;
            QFAIL("faled to compare points after transforamation");
        }
    }
}

void KisCoordinatesConverterTest::testResolutionModes_data()
{
    QTest::addColumn<qreal>("originalImageResolution");
    QTest::addColumn<qreal>("originalScreenResolution");
    QTest::addColumn<qreal>("originalZoom");
    QTest::addColumn<qreal>("originalEffectiveZoom");
    QTest::addColumn<QPointF>("originalOffset");
    QTest::addColumn<PointPairs>("originalTestPoints");

    QTest::addColumn<qreal>("newScreenResolution");
    QTest::addColumn<qreal>("expectedZoom");
    QTest::addColumn<qreal>("expectedEffectiveZoom");
    QTest::addColumn<PointPairs>("expectedTestPoints");
    QTest::addColumn<QRectF>("expectedImageRectInWidgetPixels");

    QTest::newRow("use-display-pixels")
        << 2.0 // 144 pixels per inch
        << 2.0 // 144 pixels per inch
        << 1.0 // zoom
        << 1.0 // effective zoom is the same
        << QPointF(-100, -100)
        << PointPairs{
            {{0,0},     {100,100}},
            {{100,100}, {200,200}},
           }
        << 4.0 // 288 pixels per inch, hence image size should double in screen pixels
        << 1.0 // zoom is still 100%
        << 2.0 // though effective scale factor is 200%
        << PointPairs{
            {{0,0},     {100,100}},
            {{100,100}, {300,300}},
           }
        << QRectF(100, 100, 2000, 2000); // the size of the image is doubled in screen pixels
}

void KisCoordinatesConverterTest::testResolutionModes()
{
    QFETCH(qreal, originalImageResolution);
    QFETCH(qreal, originalScreenResolution);
    QFETCH(qreal, originalZoom);
    QFETCH(qreal, originalEffectiveZoom);
    QFETCH(QPointF, originalOffset);
    QFETCH(PointPairs, originalTestPoints);
    QFETCH(qreal, newScreenResolution);
    QFETCH(qreal, expectedZoom);
    QFETCH(qreal, expectedEffectiveZoom);
    QFETCH(PointPairs, expectedTestPoints);
    QFETCH(QRectF, expectedImageRectInWidgetPixels);

    const QSize widgetSize(700, 500);

    KisImageSP image;
    KisCoordinatesConverter converter;
    initImage(&image, &converter);

    image->setResolution(originalImageResolution, originalImageResolution);
    converter.setResolution(originalScreenResolution, originalScreenResolution);
    converter.setImage(image);
    converter.setCanvasWidgetSize(widgetSize);
    converter.setZoom(originalZoom);
    converter.setDocumentOffset(originalOffset);

    QCOMPARE(converter.zoom(), originalZoom);
    QCOMPARE(converter.effectiveZoom(), originalEffectiveZoom);
    QCOMPARE(converter.zoomMode(), KoZoomMode::ZOOM_CONSTANT);

    for (const auto &pair : std::as_const(originalTestPoints)) {
        const QPointF realPointWidgetPos = converter.imageToWidget(pair.first);

        if (!KisAlgebra2D::fuzzyPointCompare(realPointWidgetPos, pair.second)) {
            qWarning() << "Failed to compare test points before transformation:";
            qWarning() << "    image point:          " << pair.first;
            qWarning() << "    real widget point:    " << realPointWidgetPos;
            qWarning() << "    expected widget point:" << pair.second;
            QFAIL("faled to compare points before transforamation");
        }
    }

    converter.setZoom(KoZoomMode::ZOOM_CONSTANT, originalZoom, newScreenResolution, newScreenResolution, -originalOffset);

    QCOMPARE(converter.zoom(), expectedZoom);
    QCOMPARE(converter.effectiveZoom(), expectedEffectiveZoom);
    QCOMPARE(converter.imageRectInWidgetPixels(), expectedImageRectInWidgetPixels);

    for (const auto &pair : std::as_const(expectedTestPoints)) {
        const QPointF realPointWidgetPos = converter.imageToWidget(pair.first);

        if (!KisAlgebra2D::fuzzyPointCompare(realPointWidgetPos, pair.second)) {
            qWarning() << "Failed to compare test points after transformation:";
            qWarning() << "    image point:          " << pair.first;
            qWarning() << "    real widget point:    " << realPointWidgetPos;
            qWarning() << "    expected widget point:" << pair.second;
            QFAIL("faled to compare points after transforamation");
        }
    }
}

SIMPLE_TEST_MAIN(KisCoordinatesConverterTest)

