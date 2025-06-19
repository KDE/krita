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
    converter.setDocumentOffset(QPoint(20,20));
    converter.setCanvasWidgetSize(QSize(500,500));
    converter.setZoom(1.);

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
    converter.setDocumentOffset(QPoint(0,0));
    converter.setCanvasWidgetSize(QSize(500,500));

    converter.setZoom(1.);

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
    converter.setDocumentOffset(QPoint(20,30));
    converter.setCanvasWidgetSize(QSize(500,500));

    QRectF testRect(100,100,100,100);
    QTransform imageToWidget;
    QTransform documentToWidget;
    QTransform flakeToWidget;
    QTransform viewportToWidget;

    converter.setZoom(1.);

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
    converter.setDocumentOffset(QPoint(20,30));
    converter.setCanvasWidgetSize(QSize(500,500));

    QRectF testRect(100,100,100,100);
    QTransform imageToWidget;
    QTransform documentToWidget;
    QTransform viewportToWidget;

    converter.setZoom(0.5);

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
    converter.setDocumentOffset(QPoint(0,0));
    converter.setCanvasWidgetSize(widgetSize);

    converter.rotate(converter.widgetCenterPoint(), 30);
    converter.setZoom(1.);

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
    converter.setDocumentOffset(QPoint(200,100));
    converter.setCanvasWidgetSize(widgetSize);

//    QTransform imageToWidget;
//    QTransform documentToWidget;
//    QTransform flakeToWidget;
//    QTransform viewportToWidget;

    converter.mirror(converter.imageCenterInWidgetPixel(), true, false);
    converter.setZoom(1.);

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
    converter.setDocumentOffset(QPoint(-50,-50));
    converter.setCanvasWidgetSize(widgetSize);

//    QTransform imageToWidget;
//    QTransform documentToWidget;
//    QTransform flakeToWidget;
//    QTransform viewportToWidget;

    converter.mirror(converter.imageCenterInWidgetPixel(), true, false);
    converter.setZoom(1.);

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
    converter.setDocumentOffset(QPoint(0,0));
    converter.setCanvasWidgetSize(QSize(500,500));
    converter.setZoom(1.);

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
    converter.setDocumentOffset(QPoint(0,0));
    converter.setCanvasWidgetSize(QSize(500,500));
    converter.setZoom(1.);

    qDebug() << ppVar(converter.imageToWidget(QPointF(0,0)));

    converter.setZoom(0.5);

}

void KisCoordinatesConverterTest::testImageSizeChange()
{
    KisImageSP image;
    KisCoordinatesConverter converter;
    initImage(&image, &converter);

    converter.setImage(image);
    converter.setDocumentOffset(QPoint(0,0));
    converter.setCanvasWidgetSize(QSize(500,500));
    converter.setZoom(0.5);

    const QRect oldImageBounds = converter.imageRectInImagePixels();
    const QRect oldImageBoundsInWidget = converter.imageRectInWidgetPixels().toAlignedRect();

    image->resizeImage(QRect(0,0,500,500));
    image->waitForDone();
    QTest::qWait(100);

    /**
     * We do **not** directly link the converter to the image.
     * All updates to the converter should be propagated manually
     * by KisView
     */

    QCOMPARE(converter.imageRectInImagePixels(), oldImageBounds);
    QCOMPARE(converter.imageRectInWidgetPixels(), oldImageBoundsInWidget);

    converter.setImageBounds(image->bounds());

    QCOMPARE(converter.imageRectInImagePixels(), image->bounds());
    QCOMPARE(converter.imageRectInWidgetPixels(), QRect(0,0,250,250));
}

void KisCoordinatesConverterTest::testImageResolutionChange()
{
    KisImageSP image;
    KisCoordinatesConverter converter;
    initImage(&image, &converter);

    converter.setImage(image);
    converter.setDocumentOffset(QPoint(0,0));
    converter.setCanvasWidgetSize(QSize(500,500));
    converter.setZoom(0.5);

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
    converter.setDocumentOffset(QPoint(0,0));
    converter.setCanvasWidgetSize(widgetRect.size()); // should be initialized first
    converter.setZoom(0.5);

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
    converter.setDocumentOffset(QPoint(0,0));
    converter.setCanvasWidgetSize(widgetRect.size()); // should be initialized first
    converter.setZoom(0.5);

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

void KisCoordinatesConverterTest::testZoomConstant()
{
    KisImageSP image;
    KisCoordinatesConverter converter;
    initImage(&image, &converter);

    const QRect widgetRect(0,0,700,500);
    converter.setImage(image);
    converter.setDocumentOffset(QPoint(0,0));
    converter.setCanvasWidgetSize(widgetRect.size()); // should be initialized first
    converter.setZoom(0.5);

    // still point is QPointF(50,50)
    QCOMPARE(converter.imageToWidget(QPointF(100,100)), QPointF(50,50)); // still point
    QCOMPARE(converter.imageToWidget(QPointF(0,0)), QPointF(0,0));

    converter.setZoom(KoZoomMode::ZOOM_CONSTANT, 0.75, image->xRes(), image->yRes(), QPointF(50,50));

    QCOMPARE(converter.imageToWidget(QPointF(100,100)), QPointF(50,50)); // still point
    QCOMPARE(converter.imageToWidget(QPointF(0,0)), QPointF(-25,-25));
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
    QTest::addColumn<qreal>("expectedZoom");
    QTest::addColumn<PointPairs>("expectedTestPoints");

    QTest::newRow("width-shrink-relative-to-widget")
        << 0
        << QSize(700, 500)
        << QPointF(-100,-100)
        << 1.0
        << PointPairs{
                      {{0,0},     {100,100}},
                      {{100,100}, {200,200}},
                      {{250,150}, {350,250}} // "still point" is the center of the widget(!)
                     }
        << KoZoomMode::ZOOM_WIDTH
        << 0.7
        << PointPairs{
                      {{0,0},     {0,145}}, // the left image border is aligned to the left
                      {{100,100}, {70,215}},
                      {{1000,0},  {700,145}}, // the right image border is aligned to the right
                      {{250,150}, {175,250}}, // "still point" has Y coordinate unchanged
                     };

    QTest::newRow("width-grow-relative-to-image")
        << 0
        << QSize(700, 500)
        << QPointF(-100,-100)
        << 0.1
        << PointPairs{
                      {{0,0},     {100,100}},
                      {{100,100}, {110,110}},
                      {{1000,1000}, {200,200}},
                      {{500,500}, {150,150}} // "still point" points to the center of the image(!)
                     }
        << KoZoomMode::ZOOM_WIDTH
        << 0.7
        << PointPairs{
                      {{0,0},     {0,-200}}, // the left image border is aligned to the left
                      {{100,100}, {70,-130}},
                      {{1000,0},  {700,-200}}, // the right image border is aligned to the right
                      {{500,500}, {350,150}}, // "still point" has Y coordinate unchanged
                     };

    QTest::newRow("width-shrink-width-margin")
            << 50
            << QSize(700, 500)
            << QPointF(-100,-100)
            << 1.0
            << PointPairs{
                          {{0,0},     {100,100}},
                          {{100,100}, {200,200}},
                          {{250,150}, {350,250}} // "still point" is the center of the widget(!)
                         }
            << KoZoomMode::ZOOM_WIDTH
            << 0.6
            << PointPairs{
                          {{0,0},     {50,160}}, // the left image border is aligned to the left
                          {{100,100}, {110,220}},
                          {{1000,0},  {650,160}}, // the right image border is aligned to the right
                          {{250,150}, {200,250}}, // "still point" has Y coordinate unchanged
                         };

    QTest::newRow("height-shrink-relative-to-widget")
        << 0
        << QSize(700, 500)
        << QPointF(-100,-100)
        << 1.0
        << PointPairs{
                      {{0,0},     {100,100}},
                      {{100,100}, {200,200}},
                      {{250,150}, {350,250}} // "still point" is the center of the widget(!)
                     }
        << KoZoomMode::ZOOM_HEIGHT
        << 0.5
        << PointPairs{
                      {{0,0},     {225,0}}, // the top image border is aligned to the top
                      {{100,100}, {275,50}},
                      {{0,1000},  {225,500}}, // the bottom image border is aligned to the bottom
                      {{250,150}, {350,75}}, // "still point" has X coordinate unchanged
                     };

    QTest::newRow("height-grow-relative-to-image")
        << 0
        << QSize(700, 500)
        << QPointF(-100,-100)
        << 0.1
        << PointPairs{
                    {{0,0},     {100,100}},
                    {{100,100}, {110,110}},
                    {{1000,1000}, {200,200}},
                    {{500,500}, {150,150}} // "still point" points to the center of the image(!)
                    }
        << KoZoomMode::ZOOM_HEIGHT
        << 0.5
        << PointPairs{
                      {{0,0},     {-100,0}}, // the top image border is aligned to the top
                      {{100,100}, {-50,50}},
                      {{0,1000},  {-100,500}}, // the bottom image border is aligned to the bottom
                      {{500,500}, {150,250}}, // "still point" has X coordinate unchanged
                     };

    QTest::newRow("height-shrink-width-margin")
        << 50
        << QSize(700, 500)
        << QPointF(-100,-100)
        << 1.0
        << PointPairs{
                    {{0,0},     {100,100}},
                    {{100,100}, {200,200}},
                    {{250,150}, {350,250}} // "still point" is the center of the widget(!)
                    }
        << KoZoomMode::ZOOM_HEIGHT
        << 0.4
        << PointPairs{
            {{0,0},     {250,50}}, // the top image border is aligned to the top
            {{100,100}, {290,90}},
            {{0,1000},  {250,450}}, // the bottom image border is aligned to the bottom
            {{250,150}, {350,110}}, // "still point" has X coordinate unchanged
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
        << 0.5
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
        << 0.4
        << PointPairs{
            {{0,0},     {150,50}}, // the top image border is aligned to the top
            {{100,100}, {190,90}},
            {{1000,1000},  {550,450}}, // the bottom image border is aligned to the bottom
            {{500,500}, {350,250}}, // center of the image should become in the center of the canvas
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
    QFETCH(qreal, expectedZoom);
    QFETCH(PointPairs, expectedTestPoints);

    KisImageSP image;
    KisCoordinatesConverter converter;
    initImage(&image, &converter);

    const QRect widgetRect(QPoint(), widgetSize);
    converter.setZoomMarginSize(zoomMargin);
    converter.setImage(image);
    converter.setDocumentOffset(originalOffset);
    converter.setCanvasWidgetSize(widgetRect.size());
    converter.setZoom(originalZoom);

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

    converter.setZoom(newZoomMode, 777.7, image->xRes(), image->yRes(), QPointF(50,50));

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

SIMPLE_TEST_MAIN(KisCoordinatesConverterTest)

