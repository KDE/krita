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

#include <kunittest/runner.h>
#include <kunittest/module.h>

#include "kis_factory.h"
#include "kis_strategy_colorspace_rgb_tester.h"
#include "kis_rgb_colorspace.h"

using namespace KUnitTest;

KUNITTEST_MODULE( kunittest_kis_strategy_colorspace_rgb_tester, "RGB ColorSpace Tester" );
KUNITTEST_MODULE_REGISTER_TESTER( KisRgbColorSpaceTester );

void KisRgbColorSpaceTester::allTests()
{
    // We need this so that the color profile loading can operate without crashing.
    KisFactory *factory = new KisFactory();

    testBasics();
    testMixColors();

    delete factory;
}

#define NUM_CHANNELS 4

#define RED_CHANNEL 0
#define GREEN_CHANNEL 1
#define BLUE_CHANNEL 2
#define ALPHA_CHANNEL 3

void KisRgbColorSpaceTester::testBasics()
{
    KoColorProfile *defProfile = new KoColorProfile(cmsCreate_sRGBProfile());  
    KisRgbColorSpace *cs = new KisRgbColorSpace(defProfile);

    quint8 pixel[NUM_CHANNELS];

    pixel[PIXEL_RED] = 255;
    pixel[PIXEL_GREEN] = 128;
    pixel[PIXEL_BLUE] = 64;
    pixel[PIXEL_ALPHA] = 0;

    QString valueText = cs->channelValueText(pixel, RED_CHANNEL);
    CHECK(valueText, QString("255"));

    valueText = cs->channelValueText(pixel, GREEN_CHANNEL);
    CHECK(valueText, QString("128"));

    valueText = cs->channelValueText(pixel, BLUE_CHANNEL);
    CHECK(valueText, QString("64"));

    valueText = cs->channelValueText(pixel, ALPHA_CHANNEL);
    CHECK(valueText, QString("0"));

    valueText = cs->normalisedChannelValueText(pixel, RED_CHANNEL);
    CHECK(valueText, QString().setNum(1.0));

    valueText = cs->normalisedChannelValueText(pixel, GREEN_CHANNEL);
    CHECK(valueText, QString().setNum(128.0 / 255.0));

    valueText = cs->normalisedChannelValueText(pixel, BLUE_CHANNEL);
    CHECK(valueText, QString().setNum(64.0 / 255.0));

    valueText = cs->normalisedChannelValueText(pixel, ALPHA_CHANNEL);
    CHECK(valueText, QString().setNum(0.0));

    cs->setPixel(pixel, 128, 192, 64, 99);
    CHECK((uint)pixel[PIXEL_RED], 128u);
    CHECK((uint)pixel[PIXEL_GREEN], 192u);
    CHECK((uint)pixel[PIXEL_BLUE], 64u);
    CHECK((uint)pixel[PIXEL_ALPHA], 99u);

    quint8 red;
    quint8 green;
    quint8 blue;
    quint8 alpha;

    cs->getPixel(pixel, &red, &green, &blue, &alpha);
    CHECK((uint)red, 128u);
    CHECK((uint)green, 192u);
    CHECK((uint)blue, 64u);
    CHECK((uint)alpha, 99u);
}

void KisRgbColorSpaceTester::testMixColors()
{
    KoColorProfile *defProfile = new KoColorProfile(cmsCreate_sRGBProfile());
    KisRgbColorSpace *cs = new KisRgbColorSpace(defProfile);


    // Test mixColors.
    quint8 pixel1[4];
    quint8 pixel2[4];
    quint8 outputPixel[4];

    pixel1[PIXEL_RED] = 255;
    pixel1[PIXEL_GREEN] = 255;
    pixel1[PIXEL_BLUE] = 255;
    pixel1[PIXEL_ALPHA] = 255;

    pixel2[PIXEL_RED] = 0;
    pixel2[PIXEL_GREEN] = 0;
    pixel2[PIXEL_BLUE] = 0;
    pixel2[PIXEL_ALPHA] = 0;

    const quint8 *pixelPtrs[2];
    quint8 weights[2];

    pixelPtrs[0] = pixel1;
    pixelPtrs[1] = pixel2;

    weights[0] = 255;
    weights[1] = 0;

    cs->mixColors(pixelPtrs, weights, 2, outputPixel);

    CHECK((int)outputPixel[PIXEL_RED], 255);
    CHECK((int)outputPixel[PIXEL_GREEN], 255);
    CHECK((int)outputPixel[PIXEL_BLUE], 255);
    CHECK((int)outputPixel[PIXEL_ALPHA], 255);

    weights[0] = 0;
    weights[1] = 255;

    cs->mixColors(pixelPtrs, weights, 2, outputPixel);

    CHECK((int)outputPixel[PIXEL_RED], 0);
    CHECK((int)outputPixel[PIXEL_GREEN], 0);
    CHECK((int)outputPixel[PIXEL_BLUE], 0);
    CHECK((int)outputPixel[PIXEL_ALPHA], 0);

    weights[0] = 128;
    weights[1] = 127;

    cs->mixColors(pixelPtrs, weights, 2, outputPixel);
     
    CHECK((int)outputPixel[PIXEL_RED], 255);
    CHECK((int)outputPixel[PIXEL_GREEN], 255);
    CHECK((int)outputPixel[PIXEL_BLUE], 255);
    CHECK((int)outputPixel[PIXEL_ALPHA], 128);

    pixel1[PIXEL_RED] = 200;
    pixel1[PIXEL_GREEN] = 100;
    pixel1[PIXEL_BLUE] = 50;
    pixel1[PIXEL_ALPHA] = 255;

    pixel2[PIXEL_RED] = 100;
    pixel2[PIXEL_GREEN] = 200;
    pixel2[PIXEL_BLUE] = 20;
    pixel2[PIXEL_ALPHA] = 255;

    cs->mixColors(pixelPtrs, weights, 2, outputPixel);

    CHECK((int)outputPixel[PIXEL_RED], 150);
    CHECK((int)outputPixel[PIXEL_GREEN], 150);
    CHECK((int)outputPixel[PIXEL_BLUE], 35);
    CHECK((int)outputPixel[PIXEL_ALPHA], 255);

    pixel1[PIXEL_RED] = 0;
    pixel1[PIXEL_GREEN] = 0;
    pixel1[PIXEL_BLUE] = 0;
    pixel1[PIXEL_ALPHA] = 0;

    pixel2[PIXEL_RED] = 255;
    pixel2[PIXEL_GREEN] = 255;
    pixel2[PIXEL_BLUE] = 255;
    pixel2[PIXEL_ALPHA] = 254;

    weights[0] = 89;
    weights[1] = 166;

    cs->mixColors(pixelPtrs, weights, 2, outputPixel);

    CHECK((int)outputPixel[PIXEL_RED], 255);
    CHECK((int)outputPixel[PIXEL_GREEN], 255);
    CHECK((int)outputPixel[PIXEL_BLUE], 255);
    CHECK((int)outputPixel[PIXEL_ALPHA], 165);
}

