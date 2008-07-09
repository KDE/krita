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

#include <QTest>
#include <QCoreApplication>

#include <kdebug.h>

#include "KoZoomHandler.h"
#include "KoGlobal.h"
#include "KoUnit.h"

#include "zoomhandler_test.h"

// Same as qfuzzycompare, but less precise because KoZoomHandler is a
// bit messy itself.
static inline bool fuzzyCompare(double p1, double p2)
{
    return qAbs(p1 - p2) < 0.0000001;
}


void zoomhandler_test::testConstruction()
{

    KoZoomHandler * zoomHandler = new KoZoomHandler();

    QVERIFY( zoomHandler->zoomFactorX() == 1 );
    QVERIFY( zoomHandler->zoomFactorY() == 1 );
    QVERIFY( ( int )INCH_TO_POINT( zoomHandler->resolutionX() ) == ( int )KoGlobal::dpiX() );
    QVERIFY( ( int )INCH_TO_POINT( zoomHandler->resolutionY() ) == ( int )KoGlobal::dpiY() );
    QVERIFY( ( int )INCH_TO_POINT( zoomHandler->zoomedResolutionX() ) == ( int )KoGlobal::dpiX() );
    QVERIFY( ( int )INCH_TO_POINT( zoomHandler->zoomedResolutionY() ) == ( int )KoGlobal::dpiY() );
    QVERIFY( zoomHandler->zoomMode() == KoZoomMode::ZOOM_CONSTANT );
    QVERIFY( zoomHandler->zoomInPercent() == 100 );
    delete zoomHandler;
}

void zoomhandler_test::testApi()
{
    KoZoomHandler zoomHandler;
    double x, y;

    zoomHandler.setResolution( 128, 129 );
    QVERIFY( zoomHandler.resolutionX() == 128 );
    QVERIFY( zoomHandler.resolutionY() == 129 );

    zoomHandler.setZoomedResolution( 50, 60 );
    QVERIFY( zoomHandler.zoomedResolutionX() == 50);
    QVERIFY( zoomHandler.zoomedResolutionY() == 60);

    zoomHandler.setZoom( 10 );
    QVERIFY( zoomHandler.zoomInPercent() == 10 );
    QVERIFY( zoomHandler.resolutionX() == 128 );
    QVERIFY( zoomHandler.resolutionY() == 129 );
    QVERIFY( zoomHandler.zoomedResolutionX() == 12.8 );
    QVERIFY( zoomHandler.zoomedResolutionY() == 12.9 );
    zoomHandler.zoom( &x, &y );
    QVERIFY( x == 12.8 && y == 12.9 );

    zoomHandler.setZoom( 100 );
    zoomHandler.setZoom( 0.1 );
    QVERIFY( zoomHandler.zoomInPercent() == 10 );
    QVERIFY( zoomHandler.resolutionX() == 128 );
    QVERIFY( zoomHandler.resolutionY() == 129 );
    QVERIFY( zoomHandler.zoomedResolutionX() == 12.8 );
    QVERIFY( zoomHandler.zoomedResolutionY() == 12.9 );
    zoomHandler.zoom( &x, &y );
    QVERIFY( x == 12.8 && y == 12.9 );

    zoomHandler.setZoomMode( KoZoomMode::ZOOM_CONSTANT );
    QVERIFY( zoomHandler.zoomMode() == KoZoomMode::ZOOM_CONSTANT );
    zoomHandler.setZoomMode( KoZoomMode::ZOOM_WIDTH );
    QVERIFY( zoomHandler.zoomMode() == KoZoomMode::ZOOM_WIDTH );
    zoomHandler.setZoomMode( KoZoomMode::ZOOM_PAGE );
    QVERIFY( zoomHandler.zoomMode() == KoZoomMode::ZOOM_PAGE );

}

void zoomhandler_test::testViewToDocument()
{
    KoZoomHandler zoomHandler;
    zoomHandler.setZoomAndResolution(100, 100, 100 );

    QVERIFY( zoomHandler.viewToDocument( QPointF( 0, 0 ) ) == QPointF( 0, 0 ) );
    // 100 view pixels are 72 postscript points at 100% zoom, 100ppi.
    QVERIFY( zoomHandler.viewToDocument( QRectF( 0, 0, 100, 100 ) ) == QRectF( 0, 0, 72, 72 ) );
    QVERIFY( zoomHandler.viewToDocumentX( 0 ) == 0 );
    QVERIFY( zoomHandler.viewToDocumentY( 0 ) == 0 );

}

void zoomhandler_test::testDocumentToView()
{
    KoZoomHandler zoomHandler;
    zoomHandler.setZoomAndResolution(100, 100, 100 );

    QVERIFY( zoomHandler.documentToView(  QPointF( 0,0 ) ) == QPointF( 0, 0 ) );
    QVERIFY( zoomHandler.documentToView(  QRectF( 0, 0, 72, 72 ) ) == QRectF( 0, 0, 100, 100) );
    QVERIFY( qFuzzyCompare( zoomHandler.documentToViewX( 72 ), 100 ) );
    QVERIFY( qFuzzyCompare( zoomHandler.documentToViewY( 72 ), 100 ) );

}


QTEST_APPLESS_MAIN(zoomhandler_test)

#include "zoomhandler_test.moc"
