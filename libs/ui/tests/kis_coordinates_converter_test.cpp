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


SIMPLE_TEST_MAIN(KisCoordinatesConverterTest)

