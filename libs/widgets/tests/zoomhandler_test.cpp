/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "zoomhandler_test.h"

#include <simpletest.h>
#include <QCoreApplication>

#include <WidgetsDebug.h>

#include "KoZoomHandler.h"
#include "KoUnit.h"


void zoomhandler_test::testConstruction()
{

    QScopedPointer<KoZoomHandler> zoomHandler(new KoZoomHandler());

    QCOMPARE( zoomHandler->zoomFactorX(), 1. );
    QCOMPARE( zoomHandler->zoomFactorY(), 1. );
    QCOMPARE( ( int )INCH_TO_POINT( zoomHandler->resolutionX() ), ( int )72 );
    QCOMPARE( ( int )INCH_TO_POINT( zoomHandler->resolutionY() ), ( int )72 );
    QCOMPARE( ( int )INCH_TO_POINT( zoomHandler->zoomedResolutionX() ), ( int )72 );
    QCOMPARE( ( int )INCH_TO_POINT( zoomHandler->zoomedResolutionY() ), ( int )72 );
    QCOMPARE( zoomHandler->zoomMode(), KoZoomMode::ZOOM_CONSTANT );
    QCOMPARE( zoomHandler->zoomInPercent(), 100 );
}

void zoomhandler_test::testApi()
{
    KoZoomHandler zoomHandler;
    qreal x, y;

    zoomHandler.setResolution( 128, 129 );
    QCOMPARE( zoomHandler.resolutionX(), 128. );
    QCOMPARE( zoomHandler.resolutionY(), 129. );

    zoomHandler.setZoomedResolution( 50, 60 );
    QCOMPARE( zoomHandler.zoomedResolutionX(), 50.);
    QCOMPARE( zoomHandler.zoomedResolutionY(), 60.);

    zoomHandler.setZoom( 0.2 ); // is 20%
    QCOMPARE( zoomHandler.zoomInPercent(), 20);
    QCOMPARE( zoomHandler.resolutionX(), 128. );
    QCOMPARE( zoomHandler.resolutionY(), 129. );
    QCOMPARE( zoomHandler.zoomedResolutionX(), 25.6 );
    QCOMPARE( zoomHandler.zoomedResolutionY(), 25.8 );
    zoomHandler.zoom( &x, &y );
    QVERIFY( x == 25.6 && y == 25.8 );

    zoomHandler.setZoom( 1. );
    zoomHandler.setZoom( 0.2 );
    QCOMPARE( zoomHandler.zoomInPercent(), 20 );
    QCOMPARE( zoomHandler.resolutionX(), 128. );
    QCOMPARE( zoomHandler.resolutionY(), 129. );
    QCOMPARE( zoomHandler.zoomedResolutionX(), 25.6 );
    QCOMPARE( zoomHandler.zoomedResolutionY(), 25.8 );
    zoomHandler.zoom( &x, &y );
    QVERIFY( x == 25.6 && y == 25.8 );

    zoomHandler.setZoomMode( KoZoomMode::ZOOM_CONSTANT );
    QCOMPARE( zoomHandler.zoomMode(), KoZoomMode::ZOOM_CONSTANT );
    zoomHandler.setZoomMode( KoZoomMode::ZOOM_PAGE );
    QCOMPARE( zoomHandler.zoomMode(), KoZoomMode::ZOOM_PAGE );

}

void zoomhandler_test::testViewToDocument()
{
    KoZoomHandler zoomHandler;
    zoomHandler.setZoom( 1.0 );
    zoomHandler.setDpi( 100, 100 );

    QCOMPARE( zoomHandler.viewToDocument( QPointF( 0, 0 ) ), QPointF( 0, 0 ) );
    // 100 view pixels are 72 postscript points at 100% zoom, 100ppi.
    QCOMPARE( zoomHandler.viewToDocument( QRectF( 0, 0, 100, 100 ) ), QRectF( 0, 0, 72, 72 ) );
    QCOMPARE( zoomHandler.viewToDocumentX( 0 ), 0. );
    QCOMPARE( zoomHandler.viewToDocumentY( 0 ), 0. );

}

void zoomhandler_test::testDocumentToView()
{
    KoZoomHandler zoomHandler;
    zoomHandler.setZoom( 1.0 );
    zoomHandler.setDpi( 100, 100 );

    QCOMPARE( zoomHandler.documentToView(  QPointF( 0,0 ) ), QPointF( 0, 0 ) );
    QCOMPARE( zoomHandler.documentToView(  QRectF( 0, 0, 72, 72 ) ), QRectF( 0, 0, 100, 100) );
    QCOMPARE( zoomHandler.documentToViewX( 72 ), 100. );
    QCOMPARE( zoomHandler.documentToViewY( 72 ), 100. );

}


QTEST_APPLESS_MAIN(zoomhandler_test)
