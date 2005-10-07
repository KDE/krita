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

#include <qapplication.h>

#include <kunittest/runner.h>
#include <kunittest/module.h>

#include "kis_image_tester.h"
#include "kis_image.h"
#include "kis_factory.h"
#include "kis_rgb_colorspace.h"
#include "kis_colorspace_factory_registry.h"
#include "kis_color.h"
#include "kis_layer.h"

using namespace KUnitTest;

KUNITTEST_MODULE(kunittest_kis_image_tester, "KisImage Tester");
KUNITTEST_MODULE_REGISTER_TESTER(KisImageTester);

void KisImageTester::allTests()
{
    if (qApp != 0) {

        // We need this so that the colour profile loading can operate without crashing.
        KisFactory *factory = new KisFactory();

        mergeTests();

        delete factory;
    } else {
        SKIP("Skipping KisImage tests because we are being run from the command line. KisImage contains a QPixmap which requires a gui to be available. Use kunittestguimodrunner.");
    }
}

#define IMAGE_WIDTH 1
#define IMAGE_HEIGHT 1

void KisImageTester::mergeTests()
{
    KisColorSpace * colorSpace = KisColorSpaceFactoryRegistry::instance() -> getColorSpace(KisID("RGBA", ""), "");

    KisImageSP image = new KisImage(0, IMAGE_WIDTH, IMAGE_HEIGHT, colorSpace, "merge test");

    KisColor mergedPixel = image -> mergedPixel(0, 0);

    QColor colour;
    Q_UINT8 opacity;

    mergedPixel.toQColor(&colour, &opacity);

    CHECK(opacity, OPACITY_TRANSPARENT);

    KisLayerSP layer = new KisLayer(image, "layer 1", OPACITY_OPAQUE);
    image -> add(layer, 0);

    layer -> setPixel(0, 0, QColor(255, 128, 64), OPACITY_OPAQUE);

    mergedPixel = image -> mergedPixel(0, 0);
    mergedPixel.toQColor(&colour, &opacity);

    CHECK(opacity, OPACITY_OPAQUE);
    CHECK(colour.red(), 255);
    CHECK(colour.green(), 128);
    CHECK(colour.blue(), 64);

    layer = new KisLayer(image, "layer 2", OPACITY_OPAQUE / 2);
    image -> add(layer, 0);

    layer -> setPixel(0, 0, QColor(255, 255, 255), OPACITY_OPAQUE);

    mergedPixel = image -> mergedPixel(0, 0);
    mergedPixel.toQColor(&colour, &opacity);

    CHECK(opacity, OPACITY_OPAQUE);
    CHECK(colour.red(), 255);
    CHECK(colour.green(), 128 + ((255 - 128) / 2));
    CHECK(colour.blue(), 64 + ((255 - 64) / 2));
}


