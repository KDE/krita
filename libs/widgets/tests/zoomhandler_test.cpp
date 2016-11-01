/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "zoomhandler_test.h"

#include <QTest>
#include <QCoreApplication>

#include <WidgetsDebug.h>

#include "KoZoomHandler.h"
#include "KoDpi.h"
#include "KoUnit.h"


void zoomhandler_test::testConstruction()
{

    KoZoomHandler * zoomHandler = new KoZoomHandler();

    QCOMPARE( zoomHandler->zoomFactorX(), 1. );
    QCOMPARE( zoomHandler->zoomFactorY(), 1. );
    QCOMPARE( ( int )INCH_TO_POINT( zoomHandler->resolutionX() ), ( int )KoDpi::dpiX() );
    QCOMPARE( ( int )INCH_TO_POINT( zoomHandler->resolutionY() ), ( int )KoDpi::dpiY() );
    QCOMPARE( ( int )INCH_TO_POINT( zoomHandler->zoomedResolutionX() ), ( int )KoDpi::dpiX() );
    QCOMPARE( ( int )INCH_TO_POINT( zoomHandler->zoomedResolutionY() ), ( int )KoDpi::dpiY() );
    QCOMPARE( zoomHandler->zoomMode(), KoZoomMode::ZOOM_CONSTANT );
    QCOMPARE( zoomHandler->zoomInPercent(), 100 );
    delete zoomHandler;
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
    zoomHandler.setZoomMode( KoZoomMode::ZOOM_WIDTH );
    QCOMPARE( zoomHandler.zoomMode(), KoZoomMode::ZOOM_WIDTH );
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
