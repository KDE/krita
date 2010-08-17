/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_coordinates_converter_test.h"
#include <QTest>

#include <qtest_kde.h>

#include <KoZoomHandler.h>
#include <KoColorSpaceRegistry.h>
#include <kis_image.h>

#include "kis_coordinates_converter.h"


void initImage(KisImageSP *image, KoZoomHandler **zoomHandler)
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    *image = new KisImage(0, 1000, 1000, cs, "projection test");
    (*image)->setResolution(300, 300);

    *zoomHandler = new KoZoomHandler();
    (*zoomHandler)->setResolution(300, 300);
}

void KisCoordinatesConverterTest::testConversion()
{
    KisImageSP image;
    KoZoomHandler *zoomHandler;
    initImage(&image, &zoomHandler);

    QRect testRect(100,100,100,100);
    KisCoordinatesConverter converter(image, zoomHandler);

    converter.setDocumentOrigin(QPoint(10,10));
    converter.setDocumentOffset(QPoint(30,30));

    zoomHandler->setZoom(1.);

    QCOMPARE(converter.imageToViewport(testRect), QRectF(70,70,100,100));
    QCOMPARE(converter.viewportToImage(testRect), QRect(130,130,100,100));

    QCOMPARE(converter.widgetToViewport(testRect), QRectF(60,60,100,100));
    QCOMPARE(converter.viewportToWidget(testRect), QRectF(140,140,100,100));

    QCOMPARE(converter.imageToWidget(testRect), QRectF(110,110,100,100));
    QCOMPARE(converter.widgetToImage(testRect), QRect(90,90,100,100));

    zoomHandler->setZoom(0.5);

    QCOMPARE(converter.imageToViewport(testRect), QRectF(20,20,50,50));
    QCOMPARE(converter.viewportToImage(testRect), QRect(260,260,200,200));

    QCOMPARE(converter.widgetToViewport(testRect), QRectF(60,60,100,100));
    QCOMPARE(converter.viewportToWidget(testRect), QRectF(140,140,100,100));

    QCOMPARE(converter.imageToWidget(testRect), QRectF(60,60,50,50));
    QCOMPARE(converter.widgetToImage(testRect), QRect(180,180,200,200));

    delete zoomHandler;
}

void KisCoordinatesConverterTest::testImageCropping()
{
    KisImageSP image;
    KoZoomHandler *zoomHandler;
    initImage(&image, &zoomHandler);

    KisCoordinatesConverter converter(image, zoomHandler);

    converter.setDocumentOrigin(QPoint(0,0));
    converter.setDocumentOffset(QPoint(0,0));

    zoomHandler->setZoom(1.);

    // we do crop here
    QCOMPARE(converter.viewportToImage(QRectF(900,900,200,200)),
             QRect(900,900,100,100));

    QCOMPARE(converter.widgetToImage(QRectF(900,900,200,200)),
             QRect(900,900,100,100));

}


QTEST_KDEMAIN(KisCoordinatesConverterTest, GUI)
#include "kis_coordinates_converter_test.moc"

