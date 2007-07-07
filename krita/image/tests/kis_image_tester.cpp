/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
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

#include <QApplication>

#include <qtest_kde.h>

#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>

#include "kis_image_tester.h"
#include "kis_image.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include <kdebug.h>

#define IMAGE_WIDTH 1
#define IMAGE_HEIGHT 1

void KisImageTester::layerTests()
{
    KisImageSP image = new KisImage( 0, IMAGE_WIDTH, IMAGE_WIDTH, 0, "layer tests" );
    QVERIFY( image->rootLayer() != 0 );
    QVERIFY( image->rootLayer()->firstChild() == 0 );

    KisLayerSP layer = new KisPaintLayer( image, "layer 1", OPACITY_OPAQUE );
    image->addLayer( layer );

    QVERIFY( image->rootLayer()->firstChild() == layer );
}

void KisImageTester::mergeTests()
{
    KoColorSpaceRegistry * reg = KoColorSpaceRegistry::instance();
    QVERIFY2( reg, "Could not get colorspace registry" );

    KoColorSpace * colorSpace = reg->colorSpace("RGBA", 0);

    KisImageSP image = new KisImage(0, IMAGE_WIDTH, IMAGE_HEIGHT, colorSpace, "merge test");

    KoColor mergedPixel = image->mergedPixel(0, 0);

    QColor color;
    quint8 opacity;

    mergedPixel.toQColor(&color, &opacity);

    QCOMPARE(opacity, OPACITY_TRANSPARENT);

    KisPaintLayer * layer = new KisPaintLayer(image, "layer 1", OPACITY_OPAQUE);
    image->addLayer(layer, image->rootLayer(), 0);

    layer->paintDevice()->setPixel(0, 0, QColor(255, 128, 64), OPACITY_OPAQUE);
    layer->setDirty();

    mergedPixel = image->mergedPixel(0, 0);
    mergedPixel.toQColor(&color, &opacity);

    QCOMPARE(opacity, OPACITY_OPAQUE);
    QCOMPARE(color.red(), 255);
    QCOMPARE(color.green(), 128);
    QCOMPARE(color.blue(), 64);

    KisPaintLayer * layer2 = new KisPaintLayer(image, "layer 2", OPACITY_OPAQUE / 2);
    image->addLayer(layer2, image->rootLayer(), layer);
    layer2->setDirty();

    layer2->paintDevice()->setPixel(0, 0, QColor(255, 255, 255), OPACITY_OPAQUE);

    mergedPixel = image->mergedPixel(0, 0);
    mergedPixel.toQColor(&color, &opacity);

    // Does not work. See BUG: 147193
    kDebug() << "XXXXXXXXXXXXXXXX: BUG: 147193" << endl;
    QCOMPARE(( uint ) opacity, ( uint ) OPACITY_OPAQUE);
    QCOMPARE(( uint ) color.red(), ( uint )255);
    QCOMPARE(( uint ) color.green(), ( uint )( 128 + ((255 - 128) / 2) ) );
    QCOMPARE(( uint ) color.blue(), ( uint ) ( 64 + ((255 - 64) / 2) ) );
}


QTEST_KDEMAIN(KisImageTester, NoGUI)
#include "kis_image_tester.moc"
