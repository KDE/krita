/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_coordinates_converter_test.h"
#include <simpletest.h>

#include <algorithm>

#include <QTransform>

#include <KoZoomHandler.h>
#include <KoColorSpaceRegistry.h>
#include <kis_image.h>

#include "kis_coordinates_converter.h"
#include "kis_filter_strategy.h"
#include <kis_algebra_2d.h>
#include <KoViewTransformStillPoint.h>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
Q_DECLARE_METATYPE(KoZoomMode::Mode)
#endif

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

    converter.rotate(std::nullopt, 30);

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

    converter.mirror(converter.makeViewStillPoint(converter.imageCenterInWidgetPixel()), true, false);

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

    converter.mirror(converter.makeViewStillPoint(converter.imageCenterInWidgetPixel()), true, false);

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

        converter.mirror(std::nullopt, true, false);
        qDebug() << "after mirror" << ppVar(converter.documentOffset());

        qDebug() << "  " << ppVar(converter.imageToWidget(QPointF(0, 0)));
        qDebug() << "  " << ppVar(converter.imageToWidget(QPointF(1000, 0)));

        converter.mirror(std::nullopt, false, false);
    }

    {
        // move the image to the top and left direction (the offset is inverted)
        converter.setDocumentOffset(QPoint(100, 200));
        qDebug() << "before mirror" << ppVar(converter.documentOffset());

        qDebug() << "  " << ppVar(converter.imageToWidget(QPointF(0, 0)));
        qDebug() << "  " << ppVar(converter.imageToWidget(QPointF(1000, 0)));

        converter.mirror(std::nullopt, true, false);
        qDebug() << "after mirror" << ppVar(converter.documentOffset());

        qDebug() << "  " << ppVar(converter.imageToWidget(QPointF(0, 0)));
        qDebug() << "  " << ppVar(converter.imageToWidget(QPointF(1000, 0)));

        converter.mirror(std::nullopt, false, false);
    }

    {
        // move the image to the top and left direction (the offset is inverted)
        converter.setDocumentOffset(QPoint(100, 200));
        qDebug() << "before rotate" << ppVar(converter.documentOffset());

        qDebug() << "  " << ppVar(converter.imageToWidget(QPointF(0, 0)));
        qDebug() << "  " << ppVar(converter.imageToWidget(QPointF(1000, 0)));

        converter.rotate(std::nullopt, 30);
        qDebug() << "after rotate" << ppVar(converter.documentOffset());

        qDebug() << "  " << ppVar(converter.imageToWidget(QPointF(0, 0)));
        qDebug() << "  " << ppVar(converter.imageToWidget(QPointF(1000, 0)));
        qDebug() << "  " << ppVar(converter.imageToWidget(QPointF(1000, 1000)));
        qDebug() << "  " << ppVar(converter.imageToWidget(QPointF(0, 1000)));

        converter.mirror(std::nullopt, false, false);
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

void KisCoordinatesConverterTest::testOffsetLimits_data()
{
    QTest::addColumn<QRect>("extraRreferencesRect");

    QTest::addRow("no-ref") << QRect();
    QTest::addRow("ref-topleft") << QRect(-100, -100, 200, 200);
    QTest::addRow("ref-bottomright") << QRect(900, 900, 200, 200);
}

void KisCoordinatesConverterTest::testOffsetLimits()
{
    QFETCH(QRect, extraRreferencesRect);

    KisImageSP image;
    KisCoordinatesConverter converter;
    initImage(&image, &converter);

    const QRect widgetRect(0,0,500,500);
    converter.setImage(image);
    converter.setZoom(0.5);
    converter.setDocumentOffset(QPoint(0,0));
    converter.setCanvasWidgetSize(widgetRect.size()); // should be initialized first
    if (!extraRreferencesRect.isEmpty()) {
        converter.setExtraReferencesBounds(extraRreferencesRect);
    }

    const QPointF widgetSizeVector(widgetRect.width(), widgetRect.height());

    // initial offset is null, hence the topleft is at the origin of the widget
    QCOMPARE(converter.imageToWidget(QPointF(0,0)), QPointF(0,0));

    const QRectF sampleImageRect = image->bounds() | extraRreferencesRect;

    // when the scrollbars offset in minimal, the topleft of the image is moved
    // to the bottom-right corner of the widget
    converter.setDocumentOffset(converter.minimumOffset());
    QCOMPARE(converter.imageToWidget(sampleImageRect.topLeft()), 0.9 * widgetSizeVector);

    // when the scrollbars offset in maximal, the topleft of the image is moved
    // under the left side of the widget and we see only the bottom-right corner
    // of the image.
    converter.setDocumentOffset(converter.maximumOffset());
    QCOMPARE(converter.imageToWidget(sampleImageRect.bottomRight()), 0.1 * widgetSizeVector);
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
    // the left edge of the document should appear at position
    // 480, which is higher than the limit of 450.
    converter.mirror(converter.makeViewStillPoint(QPointF(490, 0)), true, false);

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
    converter.mirror(converter.makeViewStillPoint(QPointF(10, 0)), false, false);

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
        << 0.35 // the only difference is that the perceived zoom is halved
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
        << 0.25 // the only difference is that the perceived zoom is halved
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
        << 0.25 // the only difference is that the perceived zoom is halved
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
        << 0.35 // the only difference is that the perceived zoom is halved
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

    const KoViewTransformStillPoint stillPoint = converter.makeViewStillPoint(widgetStillPoint);
    converter.setZoom(newZoomMode, newConstantZoom, newScreenResolution.x(), newScreenResolution.y(), stillPoint);

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
    converter.setZoom(zoomMode, originalZoom, image->xRes(), image->yRes(), std::nullopt);
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
    converter.setZoom(zoomMode, originalZoom, image->xRes(), image->yRes(), std::nullopt);
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
    converter.setZoom(zoomMode, originalZoom, image->xRes(), image->yRes(), std::nullopt);
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

    const KoViewTransformStillPoint stillPoint = converter.makeViewStillPoint(-originalOffset);

    // that should be exactly the topleft of the document
    QCOMPARE(stillPoint.docPoint(), QPointF());

    converter.setZoom(KoZoomMode::ZOOM_CONSTANT, originalZoom, newScreenResolution, newScreenResolution, stillPoint);

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

void KisCoordinatesConverterTest::testHiDPICanvasSize_data()
{
    QTest::addColumn<qreal>("devicePixelRatio");
    QTest::addColumn<QSize>("widgetSize");
    QTest::addColumn<QPointF>("offset");
    QTest::addColumn<qreal>("zoom");
    QTest::addColumn<QSizeF>("expectedWidgetSize");
    QTest::addColumn<QPointF>("expectedOffsetF");
    QTest::addColumn<QPoint>("expectedOffset");
    QTest::addColumn<QSize>("expectedViewportDevicePixelSize");
    QTest::addColumn<PointPairs>("expectedTestPoints");

    QTest::newRow("lodpi")
        << 1.0
        << QSize(701, 503)
        << QPointF(-101, -103)
        << 1.0 // zoom
        << QSizeF(701, 503)
        << QPointF(-101, -103)
        << QPoint(-101, -103)
        << QSize(701, 503)
        << PointPairs{
            {{0,0},     {101,103}},
            {{100,100}, {201,203}},
            {{1000,1000}, {1101,1103}},
           };

    QTest::newRow("hidpi-i2")
        << 2.0
        << QSize(701, 503)
        << QPointF(-101, -103)
        << 1.0 // zoom
        << QSizeF(701, 503)
        << QPointF(-101, -103)
        << QPoint(-101, -103)
        << QSize(1402, 1006) // only viewport texture size is changed in integer scaling mode!
        << PointPairs{
            {{0,0},     {101,103}},
            {{100,100}, {201,203}},
            {{1000,1000}, {1101,1103}},
            };

    QTest::newRow("hidpi-i3")
        << 3.0
        << QSize(701, 503)
        << QPointF(-101, -103)
        << 1.0 // zoom
        << QSizeF(701, 503)
        << QPointF(-101, -103)
        << QPoint(-101, -103)
        << QSize(2103, 1509) // only viewport texture size is changed in integer scaling mode!
        << PointPairs{
            {{0,0},     {101,103}},
            {{100,100}, {201,203}},
            {{1000,1000}, {1101,1103}},
            };

    auto frac = [] (int whole, int nom, int denom) {
        return denom == 0 ? whole :
            qreal(whole * denom + nom) / denom;
    };

    QTest::newRow("hidpi-f1.5")
        << 1.5
        << QSize(701, 503)
        << QPointF(-101, -103)
        << 1.0 // zoom
        << QSizeF(frac(700, 2, 3), frac(502, 2, 3))
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        // Qt5 rounds negative numbers to the positive direction, causing qRound(-151.5) = -151
        << QPointF(-frac(100, 2, 3), -frac(102, 2, 3)) // offset is rounded to device pixels!
        << QPoint(-100, -102)
#else
        << QPointF(-frac(101, 1, 3), -frac(103, 1, 3)) // offset is rounded to device pixels!
        << QPoint(-101, -103)
#endif
        << QSize(1051, 754) // the size is floored to device pixels!
        << PointPairs{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            // Qt5 rounds negative numbers to the positive direction, hence different results
            {{0,0},      {frac(100, 2, 3), frac(102, 2, 3)}},
            {{100,100},  {frac(200, 2, 3), frac(202, 2, 3)}},
            {{1000,1000},{frac(1100, 2, 3), frac(1102, 2, 3)}},
#else
            {{0,0},      {frac(101, 1, 3), frac(103, 1, 3)}},
            {{100,100},  {frac(201, 1, 3), frac(203, 1, 3)}},
            {{1000,1000},{frac(1101, 1, 3), frac(1103, 1, 3)}},
#endif
            };

    QTest::newRow("hidpi-f1.75")
        << 1.75
        << QSize(701, 503)
        << QPointF(-101, -103)
        << 1.0 // zoom
        << QSizeF(frac(700, 4, 7), frac(502, 6, 7))
        << QPointF(-frac(101, 1, 7), -frac(102, 6, 7)) // offset is rounded to device pixels!
        << QPoint(-101, -102) // scroll bars value is floored! (TODO: verify it is correct?)
        << QSize(1226, 880) // the size is floored to device pixels!
        << PointPairs{
            {{0,0},      {frac(101, 1, 7), frac(102, 6, 7)}},
            {{100,100},  {frac(201, 1, 7), frac(202, 6, 7)}},
            {{1000,1000},{frac(1101, 1, 7), frac(1102, 6, 7)}},
            };

    QTest::newRow("hidpi-f1.25")
        << 1.25
        << QSize(701, 503)
        << QPointF(-101, -103)
        << 1.0 // zoom
        << QSizeF(700.8, 502.4)
        << QPointF(-100.8, -103.2) // offset is rounded to device pixels!
        << QPoint(-100, -103) // scroll bars value is floored! (TODO: verify it is correct?)
        << QSize(876, 628) // the size is floored to device pixels!
        << PointPairs{
            {{0,0},      {100.8, 103.2}},
            {{100,100},  {200.8, 203.2}},
            {{1000,1000},{1100.8, 1103.2}},
            };
}

void KisCoordinatesConverterTest::testHiDPICanvasSize()
{
    QFETCH(qreal, devicePixelRatio);
    QFETCH(QSize, widgetSize);
    QFETCH(qreal, zoom);
    QFETCH(QPointF, offset);
    QFETCH(QSizeF, expectedWidgetSize);
    QFETCH(QPointF, expectedOffsetF);
    QFETCH(QPoint, expectedOffset);
    QFETCH(QSize, expectedViewportDevicePixelSize);
    QFETCH(PointPairs, expectedTestPoints);

    KisImageSP image;
    KisCoordinatesConverter converter;
    initImage(&image, &converter);

    converter.setDevicePixelRatio(devicePixelRatio);
    converter.setImage(image);
    converter.setCanvasWidgetSize(widgetSize);
    converter.setZoom(zoom);
    converter.setDocumentOffset(offset);

    QCOMPARE(converter.getCanvasWidgetSize(), expectedWidgetSize);
    QCOMPARE(converter.documentOffsetF(), expectedOffsetF);
    QCOMPARE(converter.documentOffset(), expectedOffset);
    QCOMPARE(converter.viewportDevicePixelSize(), expectedViewportDevicePixelSize);

    {
        // verify if image origin is aligned to the device pixels
        const QPointF imageTopLeftInWidgetPixels = converter.imageToWidget(QPointF(0, 0));
        const QPointF imageTopLeftInDevicePixels = imageTopLeftInWidgetPixels * devicePixelRatio;

        QCOMPARE(imageTopLeftInDevicePixels.x() - qRound(imageTopLeftInDevicePixels.x()), 0.0);
        QCOMPARE(imageTopLeftInDevicePixels.y() - qRound(imageTopLeftInDevicePixels.y()), 0.0);

        // TODO: also make sure that the bottom-right corner of the image is also aligned to
        //       pixel grid
    }

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

void KisCoordinatesConverterTest::testZoomLimits_data()
{
    QTest::addColumn<QSize>("originalImageSize");
    QTest::addColumn<QPointF>("originalImageResolution");
    QTest::addColumn<QPointF>("originalScreenResolution");
    QTest::addColumn<PointPairs>("originalTestPoints");
    QTest::addColumn<qreal>("expectedOriginalMinZoom");
    QTest::addColumn<qreal>("expectedOriginalMaxZoom");
    QTest::addColumn<QVector<qreal>>("expectedOriginalZoomLevels");

    QTest::addColumn<QSize>("finalImageSize");
    QTest::addColumn<QPointF>("finalImageResolution");
    QTest::addColumn<QPointF>("finalScreenResolution");
    QTest::addColumn<PointPairs>("expectedTestPoints");
    QTest::addColumn<qreal>("expectedFinalMinZoom");
    QTest::addColumn<qreal>("expectedFinalMaxZoom");
    QTest::addColumn<QVector<qreal>>("expectedFinalZoomLevels");

    QTest::newRow("image-resize")
        << QSize(1000, 1000)
        << QPointF(100, 100)
        << QPointF(100, 100)
        << PointPairs{
            {{0,0},     {0,0}},
            {{100,100}, {100,100}},
           }
        << 0.1
        << 90.0
        << QVector<qreal>{0.125, 0.166667, 0.25, 0.333333, 0.5, 0.666667, 1, 1.33333, 2, 2.66667, 4, 5.33333, 8, 10.6667, 16, 21.3333, 32, 42.6667, 64}

        << QSize(2000, 2000)
        << QPointF(100, 100)
        << QPointF(100, 100)
        << PointPairs{
            {{0,0},     {0,0}},
            {{100,100}, {100,100}},
           }
        << 0.05
        << 90.0
        << QVector<qreal>{0.0625, 0.0833333, 0.125, 0.166667, 0.25, 0.333333, 0.5, 0.666667, 1, 1.33333, 2, 2.66667, 4, 5.33333, 8, 10.6667, 16, 21.3333, 32, 42.6667, 64};

    QTest::newRow("image-resolution-change")
        << QSize(1000, 1000)
        << QPointF(100, 100)
        << QPointF(100, 100)
        << PointPairs{
            {{0,0},     {0,0}},
            {{100,100}, {100,100}},
           }
        << 0.1
        << 90.0
        << QVector<qreal>{0.125, 0.166667, 0.25, 0.333333, 0.5, 0.666667, 1, 1.33333, 2, 2.66667, 4, 5.33333, 8, 10.6667, 16, 21.3333, 32, 42.6667, 64}

        << QSize(1000, 1000)
        << QPointF(50, 50) // lower image DPI to increase physical screen size
        << QPointF(100, 100)
        << PointPairs{
            {{0,0},     {0,0}},
            {{100,100}, {200,200}},
           }
        << 0.05
        << 90.0
        << QVector<qreal>{0.0625, 0.0833333, 0.125, 0.166667, 0.25, 0.333333, 0.5, 0.666667, 1, 1.33333, 2, 2.66667, 4, 5.33333, 8, 10.6667, 16, 21.3333, 32, 42.6667, 64};

        QTest::newRow("screen-resolution-change")
        << QSize(1000, 1000)
        << QPointF(100, 100)
        << QPointF(100, 100)
        << PointPairs{
            {{0,0},     {0,0}},
            {{100,100}, {100,100}},
           }
        << 0.1
        << 90.0
        << QVector<qreal>{0.125, 0.166667, 0.25, 0.333333, 0.5, 0.666667, 1, 1.33333, 2, 2.66667, 4, 5.33333, 8, 10.6667, 16, 21.3333, 32, 42.6667, 64}

        << QSize(1000, 1000)
        << QPointF(100, 100)
        << QPointF(200, 200)  // increase screen dpi to increase the size of the image
        << PointPairs{
            {{0,0},     {0,0}},
            {{100,100}, {200,200}},
           }
        << 0.05
        << 90.0
        << QVector<qreal>{0.0625, 0.0833333, 0.125, 0.166667, 0.25, 0.333333, 0.5, 0.666667, 1, 1.33333, 2, 2.66667, 4, 5.33333, 8, 10.6667, 16, 21.3333, 32, 42.6667, 64};

}

void KisCoordinatesConverterTest::testZoomLimits()
{
    QFETCH(QSize, originalImageSize);
    QFETCH(QPointF, originalImageResolution);
    QFETCH(QPointF, originalScreenResolution);
    QFETCH(PointPairs, originalTestPoints);
    QFETCH(qreal, expectedOriginalMinZoom);
    QFETCH(qreal, expectedOriginalMaxZoom);
    QFETCH(QVector<qreal>, expectedOriginalZoomLevels);

    QFETCH(QSize, finalImageSize);
    QFETCH(QPointF, finalImageResolution);
    QFETCH(QPointF, finalScreenResolution);
    QFETCH(PointPairs, expectedTestPoints);
    QFETCH(qreal, expectedFinalMinZoom);
    QFETCH(qreal, expectedFinalMaxZoom);
    QFETCH(QVector<qreal>, expectedFinalZoomLevels);

    KisImageSP image;
    KisCoordinatesConverter converter;
    initImage(&image, &converter);

    image->resizeImage(QRect(QPoint(), originalImageSize));
    image->setResolution(originalImageResolution.x(), originalImageResolution.y());
    image->waitForDone();
    converter.setResolution(originalScreenResolution.x(), originalScreenResolution.y());

    converter.setImage(image);
    converter.setCanvasWidgetSize(QSize(700,500));
    converter.setZoom(1.0);
    converter.setDocumentOffset(QPointF());

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

    QCOMPARE(converter.minZoom(), expectedOriginalMinZoom);
    QCOMPARE(converter.maxZoom(), expectedOriginalMaxZoom);

    auto fuzzyCompareZoom = [] (qreal lhs, qreal rhs) {
        return qRound(lhs * 10000) == qRound(rhs * 10000);
    };

    auto compareZoomLevels = [&] (const QVector<qreal> &real, const QVector<qreal> &expected) {
        if (real.size() != expected.size()) {
            qWarning() << "Zoom level vectors have different size!";
            qWarning() << "    " << ppVar(real);
            qWarning() << "    " << ppVar(expected);
            return false;
        }

        auto mismatch = std::mismatch(real.begin(), real.end(),
            expected.begin(),
            fuzzyCompareZoom);

        if (mismatch.first != real.end()) {
            const int index = std::distance(real.begin(), mismatch.first);

            qWarning() << "Zoom level mismatch at index" << index;
            qWarning() << "    real:    " << *mismatch.first;
            qWarning() << "    expected:" << *mismatch.second;
            qWarning() << "   " << ppVar(real);
            qWarning() << "   " << ppVar(expected);
            return false;
        }

        return true;
    };

    if (!compareZoomLevels(converter.standardZoomLevels(), expectedOriginalZoomLevels)) {
        QFAIL("Failed to compare original zoom levels");
    }

    if (originalImageSize != finalImageSize) {
        image->resizeImage(QRect(QPoint(), finalImageSize));
        image->waitForDone();
        converter.setImageBounds(image->bounds(), QPoint(), QPoint());
    }

    if (originalImageResolution != finalImageResolution) {
        KisFilterStrategy *strategy = new KisBilinearFilterStrategy();
        image->scaleImage(image->bounds().size(), finalImageResolution.x(), finalImageResolution.y(), strategy);
        image->waitForDone();
        converter.setImageResolution(image->xRes(), image->yRes());
        converter.setDocumentOffset(QPointF());
    }

    if (originalScreenResolution != finalScreenResolution) {
        converter.setZoom(converter.zoomMode(),
                          converter.zoom(),
                          finalScreenResolution.x(),
                          finalScreenResolution.y(),
                          std::nullopt);
        converter.setDocumentOffset(QPointF());
    }

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

    QCOMPARE(converter.minZoom(), expectedFinalMinZoom);
    QCOMPARE(converter.maxZoom(), expectedFinalMaxZoom);

    if (!compareZoomLevels(converter.standardZoomLevels(), expectedFinalZoomLevels)) {
        QFAIL("Failed to compare original zoom levels");
    }

}
void KisCoordinatesConverterTest::testZoomLimitsEnforcement_data()
{
    QTest::addColumn<QSize>("originalImageSize");
    QTest::addColumn<QSize>("originalCanvasSize");
    QTest::addColumn<qreal>("expectedOriginalMinZoom");
    QTest::addColumn<qreal>("expectedOriginalMaxZoom");
    QTest::addColumn<KoZoomMode::Mode>("requestedZoomMode");
    QTest::addColumn<qreal>("requestedZoom");
    QTest::addColumn<KoZoomMode::Mode>("expectedFinalZoomMode");
    QTest::addColumn<qreal>("expectedFinalZoom");

    QTest::newRow("const-below-min")
        << QSize(1000, 1000)
        << QSize(700, 500)
        << 0.1
        << 90.0
        << KoZoomMode::ZOOM_CONSTANT
        << 0.0001
        << KoZoomMode::ZOOM_CONSTANT
        << 0.1;

    QTest::newRow("const-above-max")
        << QSize(1000, 1000)
        << QSize(700, 500)
        << 0.1
        << 90.0
        << KoZoomMode::ZOOM_CONSTANT
        << 190.0
        << KoZoomMode::ZOOM_CONSTANT
        << 90.0;

    QTest::newRow("page-below-min")
        << QSize(1000, 1000)
        << QSize(50, 50)
        << 0.1
        << 90.0
        << KoZoomMode::ZOOM_PAGE
        << 777.0
        << KoZoomMode::ZOOM_PAGE
        << 0.05; // fit-modes are allowed to ignore zoom limits
}

void KisCoordinatesConverterTest::testZoomLimitsEnforcement()
{
    /// Zoom limits enforcement is applied only to KoZoomMode::ZOOM_CONSTANT
    /// mode. All the fit-modes are allowed to zoom as much as needed.

    QFETCH(QSize, originalImageSize);
    QFETCH(QSize, originalCanvasSize);
    QFETCH(qreal, expectedOriginalMinZoom);
    QFETCH(qreal, expectedOriginalMaxZoom);

    QFETCH(KoZoomMode::Mode, requestedZoomMode);
    QFETCH(qreal, requestedZoom);

    QFETCH(KoZoomMode::Mode, expectedFinalZoomMode);
    QFETCH(qreal, expectedFinalZoom);

    KisImageSP image;
    KisCoordinatesConverter converter;
    initImage(&image, &converter);

    image->resizeImage(QRect(QPoint(), originalImageSize));
    image->waitForDone();

    converter.setImage(image);
    converter.setCanvasWidgetSize(originalCanvasSize);

    QCOMPARE(converter.minZoom(), expectedOriginalMinZoom);
    QCOMPARE(converter.maxZoom(), expectedOriginalMaxZoom);

    converter.setZoom(requestedZoomMode, requestedZoom,
        converter.resolutionX(), converter.resolutionY(),
        converter.makeViewStillPoint(converter.imageCenterInWidgetPixel()));

    QCOMPARE(converter.zoomMode(), expectedFinalZoomMode);
    QCOMPARE(converter.zoom(), expectedFinalZoom);
}

void KisCoordinatesConverterTest::testFindNextZoom_data()
{
    QTest::addColumn<bool>("findNext");
    QTest::addColumn<qreal>("startZoom");
    QTest::addColumn<qreal>("expectedZoom");

    QTest::newRow("next-below-minimum")
        << true
        << 0.03
        << 0.0625;

    QTest::newRow("next-middle")
        << true
        << 0.07
        << 0.0833333;

    QTest::newRow("next-collision")
        << true
        << 0.0833333
        << 0.125;

    QTest::newRow("next-above-maximum")
        << true
        << 65.0
        << 65.0; // keep the current level

    QTest::newRow("prev-below-minimum")
        << false
        << 0.03
        << 0.03;

    QTest::newRow("prev-middle")
        << false
        << 0.07
        << 0.0625;

    QTest::newRow("prev-collision")
        << false
        << 0.0833333
        << 0.0625;

    QTest::newRow("prev-above-maximum")
        << false
        << 65.0
        << 64.0;


}

void KisCoordinatesConverterTest::testFindNextZoom()
{
    QFETCH(bool, findNext);
    QFETCH(qreal, startZoom);
    QFETCH(qreal, expectedZoom);

    const QVector<qreal> levels{0.0625, 0.0833333, 0.125, 0.166667, 0.25, 0.333333, 0.5, 0.666667, 1, 1.33333, 2, 2.66667, 4, 5.33333, 8, 10.6667, 16, 21.3333, 32, 42.6667, 64};

    const qreal resultZoom =
        findNext ?
        KisCoordinatesConverter::findNextZoom(startZoom, levels) :
        KisCoordinatesConverter::findPrevZoom(startZoom, levels);

    QCOMPARE(resultZoom, expectedZoom);
}

void KisCoordinatesConverterTest::testZoomTo_data()
{
    QTest::addColumn<QPointF>("originalOffset");
    QTest::addColumn<qreal>("originalZoom");
    QTest::addColumn<PointPairs>("originalTestPoints");
    QTest::addColumn<QRectF>("zoomToRect");
    QTest::addColumn<qreal>("expectedZoom");
    QTest::addColumn<PointPairs>("expectedTestPoints");

    QTest::newRow("constant")
        << QPointF(-100,-100)
        << 1.0
        << PointPairs{
                      {{0,0},     {100,100}},
                      {{100,100}, {200,200}},
                     }
        << QRectF(200,200,100,100)
        << 5.0
        << PointPairs{
                      {{100,100},     {100, 0}}, // the selected portion takes the whole canvas
                      {{200,200},     {600, 500}},
                     };
}

void KisCoordinatesConverterTest::testZoomTo()
{
    QFETCH(QPointF, originalOffset);
    QFETCH(qreal, originalZoom);
    QFETCH(PointPairs, originalTestPoints);
    QFETCH(QRectF, zoomToRect);
    QFETCH(qreal, expectedZoom);
    QFETCH(PointPairs, expectedTestPoints);

    KisImageSP image;
    KisCoordinatesConverter converter;
    initImage(&image, &converter);

    converter.setImage(image);
    converter.setZoom(originalZoom);
    converter.setDocumentOffset(originalOffset);
    converter.setCanvasWidgetSize(QSize(700, 500));

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

    converter.zoomTo(zoomToRect);

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

enum TestHiDPIMode
{
    ZoomPageTo7,
    ZoomPageToWidth,
    ScrollToFraction,
    ScrollAway,
    ImageResolution,
    ImageBounds,
    ZoomTo,
    Rotate,
    RotateOrthogonal
};

Q_DECLARE_METATYPE(TestHiDPIMode)

void KisCoordinatesConverterTest::testHiDPIOffsetSnapping_data()
{
    QTest::addColumn<TestHiDPIMode>("testingMode");
    QTest::addColumn<bool>("expectsPixelAlignment");

    QTest::addRow("page-to-const7") << ZoomPageTo7 << true;
    QTest::addRow("page-to-width") << ZoomPageToWidth << true;
    QTest::addRow("scroll-to-fraction") << ScrollToFraction << true;
    QTest::addRow("scroll-away") << ScrollAway << true;
    QTest::addRow("image-resolution") << ImageResolution << true;
    QTest::addRow("image-bounds") << ImageBounds << true;
    QTest::addRow("zoom-to") << ZoomTo << true;
    QTest::addRow("rotate-orthogonal") << RotateOrthogonal << true;
    /**
     * In non-orthogonal rotation mode we disable pixel alignment
     * to make rotations smoother.
     */
    QTest::addRow("rotate") << Rotate << false;
}

void KisCoordinatesConverterTest::testHiDPIOffsetSnapping()
{
    QFETCH(TestHiDPIMode, testingMode);
    QFETCH(bool, expectsPixelAlignment);

    KisImageSP image;
    KisCoordinatesConverter converter;
    initImage(&image, &converter);

    KisFilterStrategy *strategy = new KisBilinearFilterStrategy();
    image->scaleImage(QSize(77, 83), image->xRes(), image->yRes(), strategy);
    image->waitForDone();
    QTest::qWait(100);

    converter.setDevicePixelRatio(1.5);
    converter.setImage(image);
    converter.setCanvasWidgetSizeKeepZoom(QSize(700, 500));

    auto roundTo5thDigit = [] (qreal x) {
        return qRound(x * 100000.0) / 100000.0;
    };

    {
        const QPointF imageTopLeftInDevicePixelsHW = converter.imageRectInWidgetPixels().topLeft() * converter.devicePixelRatio();
        QCOMPARE(roundTo5thDigit(imageTopLeftInDevicePixelsHW.x()), qRound(imageTopLeftInDevicePixelsHW.x()));
        QCOMPARE(roundTo5thDigit(imageTopLeftInDevicePixelsHW.y()), qRound(imageTopLeftInDevicePixelsHW.y()));
    }



    switch (testingMode) {
        case ZoomPageTo7:
            converter.setZoom(KoZoomMode::ZOOM_CONSTANT, 0.17,
                converter.resolutionX(), converter.resolutionY(), converter.makeViewStillPoint(converter.imageCenterInWidgetPixel()));
            break;
        case ZoomPageToWidth:
            converter.setZoom(KoZoomMode::ZOOM_WIDTH, 1.0,
                converter.resolutionX(), converter.resolutionY(), converter.makeViewStillPoint(converter.imageCenterInWidgetPixel()));
            break;
        case ScrollToFraction:
            converter.setDocumentOffset(QPointF(-100.33, -200.77));
            break;
        case ScrollAway:
            converter.setCanvasWidgetSizeKeepZoom(QSize(701, 503));
            converter.setDocumentOffset(QPointF(10000.33, 20000.22));
            break;
        case ImageResolution:
            converter.setImageResolution(7.77, 3.13);
            break;
        case ImageBounds:
            converter.setImageBounds(QRect(0,0,113, 117), QPointF(77,17), QPointF(111, 110));
            break;
        case ZoomTo:
            converter.zoomTo(QRectF(13.33, 17.77, 333.17, 234.13));
            break;
        case Rotate:
            converter.rotate(converter.makeViewStillPoint(converter.imageCenterInWidgetPixel()), 17.3);
            break;
        case RotateOrthogonal:
            converter.rotate(converter.makeViewStillPoint(converter.imageCenterInWidgetPixel()), 90.0);
            break;
    }

    {
        const QPointF imageTopLeftInDevicePixelsHW = converter.imageRectInWidgetPixels().topLeft() * converter.devicePixelRatio();
        if (expectsPixelAlignment) {
            QCOMPARE(roundTo5thDigit(imageTopLeftInDevicePixelsHW.x()), qRound(imageTopLeftInDevicePixelsHW.x()));
            QCOMPARE(roundTo5thDigit(imageTopLeftInDevicePixelsHW.y()), qRound(imageTopLeftInDevicePixelsHW.y()));
        } else {
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
            QCOMPARE_NE(roundTo5thDigit(imageTopLeftInDevicePixelsHW.x()), qRound(imageTopLeftInDevicePixelsHW.x()));
            QCOMPARE_NE(roundTo5thDigit(imageTopLeftInDevicePixelsHW.y()), qRound(imageTopLeftInDevicePixelsHW.y()));
#else
            QVERIFY(roundTo5thDigit(imageTopLeftInDevicePixelsHW.x()) != qRound(imageTopLeftInDevicePixelsHW.x()));
            QVERIFY(roundTo5thDigit(imageTopLeftInDevicePixelsHW.y()) != qRound(imageTopLeftInDevicePixelsHW.y()));
#endif
        }
    }
}

void KisCoordinatesConverterTest::testPreferredCenterTransformations_data()
{
    QTest::addColumn<QString>("testingMode");
    QTest::addColumn<bool>("useCustomStillPoint");

    QTest::addRow("iterative-zoom") << "zoom" << false;
    QTest::addRow("iterative-zoom-still-point") << "zoom" << true;
    QTest::addRow("iterative-rotate") << "rotate" << false;
    QTest::addRow("iterative-rotate-still-point") << "rotate" << true;
    QTest::addRow("reset-rotation") << "reset-rotation" << false;
    QTest::addRow("reset-rotation-still-point") << "reset-rotation" << true;
    QTest::addRow("mirror") << "mirror" << false;
    QTest::addRow("mirror-still-point") << "mirror" << true;

    QTest::addRow("zoom-width") << "zoom-width" << false;
    QTest::addRow("pan") << "pan" << false;
    QTest::addRow("change-widget-size-page") << "change-widget-size-page" << false;
    QTest::addRow("change-widget-size-const") << "change-widget-size-const" << false;
    QTest::addRow("change-image-size") << "change-image-size" << false;
    QTest::addRow("change-image-resolution") << "change-image-resolution" << false;
}

void KisCoordinatesConverterTest::testPreferredCenterTransformations()
{
    QFETCH(QString, testingMode);
    QFETCH(bool, useCustomStillPoint);

    KisImageSP image;
    KisCoordinatesConverter converter;
    initImage(&image, &converter);

    converter.setImage(image);
    converter.setCanvasWidgetSizeKeepZoom(QSize(700, 500));

    KoViewTransformStillPoint expectedStillPoint;
    std::optional<KoViewTransformStillPoint> realStillPoint;

    if (!useCustomStillPoint) {
        expectedStillPoint = {converter.imageRectInDocumentPixels().center(), converter.imageCenterInWidgetPixel()};
        realStillPoint = std::nullopt;
    } else {
        expectedStillPoint = converter.makeViewStillPoint({100.0,100.0});
        realStillPoint = expectedStillPoint;
    }

    bool ignoreStillPointComparison = false;

    if (testingMode == "zoom") {
        for (int i = 0; i < 100; i++) {
            const qreal zoom = 0.033 + 0.0073 * i;
            converter.setZoom(KoZoomMode::ZOOM_CONSTANT,
                               zoom,
                               converter.resolutionX(),
                               converter.resolutionY(),
                               realStillPoint);
        }
    } else if (testingMode == "rotate") {
        for (int i = 0; i < 100; i++) {
            converter.rotate(realStillPoint, M_PI / 113.0);
        }
    } else if (testingMode == "reset-rotation") {
        converter.rotate(realStillPoint, M_PI / 6);
        converter.resetRotation(realStillPoint);
    } else if (testingMode == "mirror") {
        converter.mirror(realStillPoint, true, false);
    } else if (testingMode == "zoom-width") {
        converter.setZoom(KoZoomMode::ZOOM_WIDTH, 1.0,
            converter.resolutionX(), converter.resolutionY(),
            std::nullopt);
    } else if (testingMode == "pan") {
        converter.setDocumentOffset(converter.documentOffsetF() + QPointF(100.0, 0.0));

        /// the actual still point is checked in a  different test,
        /// here we only need to test the preferred center
        ignoreStillPointComparison = true;
    } else if (testingMode == "change-widget-size-page") {
        const qreal oldZoom = converter.zoom();
        const QPointF oldOffset = converter.documentOffsetF();
        converter.setCanvasWidgetSizeKeepZoom(QSize(1100,400));
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
        QCOMPARE_NE(converter.zoom(), oldZoom);
        QCOMPARE_NE(converter.documentOffsetF(), oldOffset);
#else
        QVERIFY(!qFuzzyCompare(converter.zoom(), oldZoom));
        QVERIFY(converter.documentOffsetF() != oldOffset);
#endif

        // update the expected still point to the ceter of the "new" widget
        expectedStillPoint.second = converter.widgetCenterPoint();

    } else if (testingMode == "change-widget-size-const") {
        converter.setZoom(KoZoomMode::ZOOM_CONSTANT,
            converter.zoom(),
            converter.resolutionX(),
            converter.resolutionY(),
            std::nullopt);

        const qreal oldZoom = converter.zoom();
        const QPointF oldOffset = converter.documentOffsetF();
        converter.setCanvasWidgetSizeKeepZoom(QSize(1100,400));
        QCOMPARE(converter.zoom(), oldZoom);
        QCOMPARE(converter.documentOffsetF(), oldOffset);
    } else if (testingMode == "change-image-size") {
        converter.setImageBounds(QRect(0,0,113, 117), QPointF(77,17), QPointF(111, 110));

        /// the actual still point is checked in a  different test,
        /// here we only need to test the preferred center
        ignoreStillPointComparison = true;
    } else if (testingMode == "change-image-resolution") {
        converter.setDocumentOffset(converter.documentOffsetF() + QPointF(100.0, 0.0));
        converter.setImageResolution(7.77, 3.13);

        /// the actual still point is checked in a  different test,
        /// here we only need to test the preferred center
        ignoreStillPointComparison = true;
    } else {
        qFatal("Unknown tesing mode");
    }

    if (!ignoreStillPointComparison &&
        kisDistance(converter.documentToWidget(expectedStillPoint.docPoint()), expectedStillPoint.viewPoint()) > 1.0) {
        qDebug() << "Failed to compare the final image center:";
        qDebug() << "    " << ppVar(expectedStillPoint);
        qDebug() << "    " << ppVar(converter.documentToWidget(expectedStillPoint.docPoint()));
        QFAIL("The image center is not preserved");
    }

    if (kisDistance(converter.imageToWidget(converter.preferredTransformationCenter()), converter.widgetCenterPoint()) > 1.0) {
        qDebug() << "Failed to compare the preferred center position";
        qDebug() << "    " << ppVar(converter.preferredTransformationCenter());
        qDebug() << "    " << ppVar(converter.imageToWidget(converter.preferredTransformationCenter()));
        qDebug() << "    " << ppVar(converter.widgetCenterPoint());
        QFAIL("The preferred center position does not point to the center of the widget");
    }
}

SIMPLE_TEST_MAIN(KisCoordinatesConverterTest)

