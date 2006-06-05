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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <kunittest/runner.h>
#include <kunittest/module.h>

#include "kis_factory.h"
#include "kis_rgb_f16half_colorspace.h"
#include "kis_rgb_f16half_colorspace_tester.h"
#include "kis_rgb_f32_colorspace.h"
#include "KoIntegerMaths.h"
#include "kis_paint_device.h"

using namespace KUnitTest;

KUNITTEST_MODULE( kunittest_kis_rgb_f16half_colorspace_tester, "RGBA 16-bit float half colorspace tester" );
KUNITTEST_MODULE_REGISTER_TESTER( KisRgbF16HalfColorSpaceTester );

#define PIXEL_BLUE 0
#define PIXEL_GREEN 1
#define PIXEL_RED 2
#define PIXEL_ALPHA 3

#define NUM_CHANNELS 4
#define NUM_COLOUR_CHANNELS 3
#define CHANNEL_SIZE ((int)sizeof(half))

#define RED_CHANNEL 0
#define GREEN_CHANNEL 1
#define BLUE_CHANNEL 2
#define ALPHA_CHANNEL 3

//#define MAX_CHANNEL_VALUE 1.0f
//#define MIN_CHANNEL_VALUE 0.0f

#define CHANNEL_VALUE_ZERO ((half)0)
#define CHANNEL_VALUE_ONE ((half)1)

void KisRgbF16HalfColorSpaceTester::allTests()
{
    // We need this so that the colour profile loading can operate without crashing.
    KisFactory *factory = new KisFactory();

    testBasics();
    testToQImage();
    testCompositeOps();
    testMixColors();

    delete factory;
}

void KisRgbF16HalfColorSpaceTester::testBasics()
{


    KoColorProfile *profile = new KoColorProfile(cmsCreate_sRGBProfile());

    KisRgbF16HalfColorSpace *cs = new KisRgbF16HalfColorSpace(profile);
    KisAbstractColorSpace * csSP = cs;

    CHECK(cs->hasAlpha(), true);
    CHECK(cs->nChannels(), NUM_CHANNELS);
    CHECK(cs->nColorChannels(), NUM_COLOUR_CHANNELS);
    CHECK(cs->pixelSize(), NUM_CHANNELS * CHANNEL_SIZE);

    Q3ValueVector<KoChannelInfo *> channels = cs->channels();

    // Red
    CHECK(channels[0]->pos(), PIXEL_RED * CHANNEL_SIZE);
    CHECK(channels[0]->size(), CHANNEL_SIZE);
    CHECK(channels[0]->channelType(), COLOR);

    // Green
    CHECK(channels[1]->pos(), PIXEL_GREEN * CHANNEL_SIZE);
    CHECK(channels[1]->size(), CHANNEL_SIZE);
    CHECK(channels[1]->channelType(), COLOR);

    // Blue
    CHECK(channels[2]->pos(), PIXEL_BLUE * CHANNEL_SIZE);
    CHECK(channels[2]->size(), CHANNEL_SIZE);
    CHECK(channels[2]->channelType(), COLOR);

    // Alpha
    CHECK(channels[3]->pos(), PIXEL_ALPHA * CHANNEL_SIZE);
    CHECK(channels[3]->size(), CHANNEL_SIZE);
    CHECK(channels[3]->channelType(), ALPHA);

    KisPaintDeviceSP pd = new KisPaintDevice(cs, "test");

    KisRgbF16HalfColorSpace::Pixel defaultPixel;

    memcpy(&defaultPixel, pd->dataManager()->defaultPixel(), sizeof(defaultPixel));

    CHECK(defaultPixel.red, CHANNEL_VALUE_ZERO);
    CHECK(defaultPixel.green, CHANNEL_VALUE_ZERO);
    CHECK(defaultPixel.blue, CHANNEL_VALUE_ZERO);
    CHECK(defaultPixel.alpha, F16HALF_OPACITY_TRANSPARENT);

    half pixel[NUM_CHANNELS];

    cs->fromQColor(qRgb(255, 255, 255), reinterpret_cast<quint8 *>(pixel));

    CHECK(pixel[PIXEL_RED], CHANNEL_VALUE_ONE);
    CHECK(pixel[PIXEL_GREEN], CHANNEL_VALUE_ONE);
    CHECK(pixel[PIXEL_BLUE], CHANNEL_VALUE_ONE);

    cs->fromQColor(qRgb(0, 0, 0), reinterpret_cast<quint8 *>(pixel));

    CHECK(pixel[PIXEL_RED], CHANNEL_VALUE_ZERO);
    CHECK(pixel[PIXEL_GREEN], CHANNEL_VALUE_ZERO);
    CHECK(pixel[PIXEL_BLUE], CHANNEL_VALUE_ZERO);

    cs->fromQColor(qRgb(128, 64, 192), reinterpret_cast<quint8 *>(pixel));

    CHECK(pixel[PIXEL_RED], UINT8_TO_HALF(128));
    CHECK(pixel[PIXEL_GREEN], UINT8_TO_HALF(64));
    CHECK(pixel[PIXEL_BLUE], UINT8_TO_HALF(192));

    cs->fromQColor(qRgb(255, 255, 255), OPACITY_OPAQUE, reinterpret_cast<quint8 *>(pixel));

    CHECK(pixel[PIXEL_RED], CHANNEL_VALUE_ONE);
    CHECK(pixel[PIXEL_GREEN], CHANNEL_VALUE_ONE);
    CHECK(pixel[PIXEL_BLUE], CHANNEL_VALUE_ONE);
    CHECK(pixel[PIXEL_ALPHA], CHANNEL_VALUE_ONE);

    cs->fromQColor(qRgb(255, 255, 255), OPACITY_TRANSPARENT, reinterpret_cast<quint8 *>(pixel));

    CHECK(pixel[PIXEL_RED], CHANNEL_VALUE_ONE);
    CHECK(pixel[PIXEL_GREEN], CHANNEL_VALUE_ONE);
    CHECK(pixel[PIXEL_BLUE], CHANNEL_VALUE_ONE);
    CHECK(pixel[PIXEL_ALPHA], F16HALF_OPACITY_TRANSPARENT);

    cs->fromQColor(qRgb(255, 255, 255), OPACITY_OPAQUE / 2, reinterpret_cast<quint8 *>(pixel));

    CHECK(pixel[PIXEL_RED], CHANNEL_VALUE_ONE);
    CHECK(pixel[PIXEL_GREEN], CHANNEL_VALUE_ONE);
    CHECK(pixel[PIXEL_BLUE], CHANNEL_VALUE_ONE);
    CHECK(pixel[PIXEL_ALPHA], UINT8_TO_HALF(OPACITY_OPAQUE / 2));

    pixel[PIXEL_RED] = CHANNEL_VALUE_ONE;
    pixel[PIXEL_GREEN] = CHANNEL_VALUE_ONE;
    pixel[PIXEL_BLUE] = CHANNEL_VALUE_ONE;

    QColor c;

    cs->toQColor(reinterpret_cast<const quint8 *>(pixel), &c);

    CHECK(c.red(), 255);
    CHECK(c.green(), 255);
    CHECK(c.blue(), 255);

    pixel[PIXEL_RED] = CHANNEL_VALUE_ZERO;
    pixel[PIXEL_GREEN] = CHANNEL_VALUE_ZERO;
    pixel[PIXEL_BLUE] = CHANNEL_VALUE_ZERO;

    cs->toQColor(reinterpret_cast<const quint8 *>(pixel), &c);

    CHECK(c.red(), 0);
    CHECK(c.green(), 0);
    CHECK(c.blue(), 0);

    pixel[PIXEL_RED] = CHANNEL_VALUE_ONE / 4;
    pixel[PIXEL_GREEN] = CHANNEL_VALUE_ONE / 2;
    pixel[PIXEL_BLUE] = (3 * CHANNEL_VALUE_ONE) / 4;

    cs->toQColor(reinterpret_cast<const quint8 *>(pixel), &c);

    CHECK(c.red(), (int)HALF_TO_UINT8(CHANNEL_VALUE_ONE / 4));
    CHECK(c.green(), (int)HALF_TO_UINT8(CHANNEL_VALUE_ONE / 2));
    CHECK(c.blue(), (int)HALF_TO_UINT8((3 * CHANNEL_VALUE_ONE) / 4));

    pixel[PIXEL_RED] = CHANNEL_VALUE_ONE;
    pixel[PIXEL_GREEN] = CHANNEL_VALUE_ONE;
    pixel[PIXEL_BLUE] = CHANNEL_VALUE_ONE;
    pixel[PIXEL_ALPHA] = CHANNEL_VALUE_ONE;

    quint8 opacity;

    cs->toQColor(reinterpret_cast<const quint8 *>(pixel), &c, &opacity);

    CHECK(c.red(), 255);
    CHECK(c.green(), 255);
    CHECK(c.blue(), 255);
    CHECK(opacity, OPACITY_OPAQUE);

    pixel[PIXEL_ALPHA] = F16HALF_OPACITY_OPAQUE;

    cs->toQColor(reinterpret_cast<const quint8 *>(pixel), &c, &opacity);

    CHECK(c.red(), 255);
    CHECK(c.green(), 255);
    CHECK(c.blue(), 255);
    CHECK(opacity, OPACITY_OPAQUE);

    pixel[PIXEL_RED] = CHANNEL_VALUE_ZERO;
    pixel[PIXEL_GREEN] = CHANNEL_VALUE_ZERO;
    pixel[PIXEL_BLUE] = CHANNEL_VALUE_ZERO;
    pixel[PIXEL_ALPHA] = F16HALF_OPACITY_TRANSPARENT;

    cs->toQColor(reinterpret_cast<const quint8 *>(pixel), &c, &opacity);

    CHECK(c.red(), 0);
    CHECK(c.green(), 0);
    CHECK(c.blue(), 0);
    CHECK(opacity, OPACITY_TRANSPARENT);

    pixel[PIXEL_RED] = CHANNEL_VALUE_ONE / 4;
    pixel[PIXEL_GREEN] = CHANNEL_VALUE_ONE / 2;
    pixel[PIXEL_BLUE] = (3 * CHANNEL_VALUE_ONE) / 4;
    pixel[PIXEL_ALPHA] = CHANNEL_VALUE_ONE / 2;

    cs->toQColor(reinterpret_cast<const quint8 *>(pixel), &c, &opacity);

    CHECK(c.red(), (int)HALF_TO_UINT8(CHANNEL_VALUE_ONE / 4));
    CHECK(c.green(), (int)HALF_TO_UINT8(CHANNEL_VALUE_ONE / 2));
    CHECK(c.blue(), (int)HALF_TO_UINT8((3 * CHANNEL_VALUE_ONE) / 4));
    CHECK((int)opacity, (int)HALF_TO_UINT8(CHANNEL_VALUE_ONE / 2));

    #define NUM_PIXELS 4

    KisRgbF16HalfColorSpace::Pixel pixels[NUM_PIXELS] = {
        {CHANNEL_VALUE_ONE, CHANNEL_VALUE_ONE, CHANNEL_VALUE_ONE, CHANNEL_VALUE_ONE / 4},
        {CHANNEL_VALUE_ONE / 4, CHANNEL_VALUE_ONE / 2, CHANNEL_VALUE_ONE / 3, CHANNEL_VALUE_ONE / 2},
        {CHANNEL_VALUE_ONE, CHANNEL_VALUE_ONE, CHANNEL_VALUE_ONE, CHANNEL_VALUE_ZERO},
        {CHANNEL_VALUE_ZERO, CHANNEL_VALUE_ZERO, CHANNEL_VALUE_ZERO, CHANNEL_VALUE_ONE}
    };

    cs->setAlpha(reinterpret_cast<quint8 *>(pixels), OPACITY_OPAQUE / 2, NUM_PIXELS);

    CHECK(pixels[0].red, CHANNEL_VALUE_ONE);
    CHECK(pixels[0].green, CHANNEL_VALUE_ONE);
    CHECK(pixels[0].blue, CHANNEL_VALUE_ONE);
    CHECK(pixels[0].alpha, UINT8_TO_HALF(OPACITY_OPAQUE / 2));

    CHECK(pixels[1].red, (half)(CHANNEL_VALUE_ONE / 3));
    CHECK(pixels[1].green, (half)(CHANNEL_VALUE_ONE / 2));
    CHECK(pixels[1].blue, (half)(CHANNEL_VALUE_ONE / 4));
    CHECK(pixels[1].alpha, UINT8_TO_HALF(OPACITY_OPAQUE / 2));

    CHECK(pixels[2].red, CHANNEL_VALUE_ONE);
    CHECK(pixels[2].green, CHANNEL_VALUE_ONE);
    CHECK(pixels[2].blue, CHANNEL_VALUE_ONE);
    CHECK(pixels[2].alpha, UINT8_TO_HALF(OPACITY_OPAQUE / 2));

    CHECK(pixels[3].red, CHANNEL_VALUE_ZERO);
    CHECK(pixels[3].green, CHANNEL_VALUE_ZERO);
    CHECK(pixels[3].blue, CHANNEL_VALUE_ZERO);
    CHECK(pixels[3].alpha, UINT8_TO_HALF(OPACITY_OPAQUE / 2));

    pixel[PIXEL_RED] = CHANNEL_VALUE_ONE;
    pixel[PIXEL_GREEN] = CHANNEL_VALUE_ONE / 2;
    pixel[PIXEL_BLUE] = CHANNEL_VALUE_ONE / 4;
    pixel[PIXEL_ALPHA] = CHANNEL_VALUE_ZERO;

    QString valueText = cs->channelValueText(reinterpret_cast<quint8 *>(pixel), RED_CHANNEL);
    CHECK(valueText, QString().setNum(CHANNEL_VALUE_ONE));

    valueText = cs->channelValueText(reinterpret_cast<quint8 *>(pixel), GREEN_CHANNEL);
    CHECK(valueText, QString().setNum(CHANNEL_VALUE_ONE / 2));

    valueText = cs->channelValueText(reinterpret_cast<quint8 *>(pixel), BLUE_CHANNEL);
    CHECK(valueText, QString().setNum(CHANNEL_VALUE_ONE / 4));

    valueText = cs->channelValueText(reinterpret_cast<quint8 *>(pixel), ALPHA_CHANNEL);
    CHECK(valueText, QString().setNum(CHANNEL_VALUE_ZERO));

    valueText = cs->normalisedChannelValueText(reinterpret_cast<quint8 *>(pixel), RED_CHANNEL);
    CHECK(valueText, QString().setNum(CHANNEL_VALUE_ONE));

    valueText = cs->normalisedChannelValueText(reinterpret_cast<quint8 *>(pixel), GREEN_CHANNEL);
    CHECK(valueText, QString().setNum(CHANNEL_VALUE_ONE / 2));

    valueText = cs->normalisedChannelValueText(reinterpret_cast<quint8 *>(pixel), BLUE_CHANNEL);
    CHECK(valueText, QString().setNum(CHANNEL_VALUE_ONE / 4));

    valueText = cs->normalisedChannelValueText(reinterpret_cast<quint8 *>(pixel), ALPHA_CHANNEL);
    CHECK(valueText, QString().setNum(CHANNEL_VALUE_ZERO));

    cs->setPixel(reinterpret_cast<quint8 *>(pixel), 0.128, 0.192, 0.64, 0.99);
    CHECK(pixel[PIXEL_RED], (half)0.128f);
    CHECK(pixel[PIXEL_GREEN], (half)0.192f);
    CHECK(pixel[PIXEL_BLUE], (half)0.64f);
    CHECK(pixel[PIXEL_ALPHA], (half)0.99f);

    half red;
    half green;
    half blue;
    half alpha;

    cs->getPixel(reinterpret_cast<const quint8 *>(pixel), &red, &green, &blue, &alpha);
    CHECK(red, (half)0.128f);
    CHECK(green, (half)0.192f);
    CHECK(blue, (half)0.64f);
    CHECK(alpha, (half)0.99f);

    CHECK(HALF_TO_UINT8(-0.5), 0u);
    CHECK(HALF_TO_UINT8(0), 0u);
    CHECK_TOLERANCE(HALF_TO_UINT8(0.5), UINT8_MAX / 2, 1u);
    CHECK(HALF_TO_UINT8(1), UINT8_MAX);
    CHECK(HALF_TO_UINT8(1.5), UINT8_MAX);

    CHECK(HALF_TO_UINT16(-0.5), 0u);
    CHECK(HALF_TO_UINT16(0), 0u);
    CHECK_TOLERANCE(HALF_TO_UINT16(0.5), UINT16_MAX / 2, 1u);
    CHECK(HALF_TO_UINT16(1), UINT16_MAX);
    CHECK(HALF_TO_UINT16(1.5), UINT16_MAX);
}

void KisRgbF16HalfColorSpaceTester::testMixColors()
{
    KoColorProfile *profile = new KoColorProfile(cmsCreate_sRGBProfile());
    KisAbstractColorSpace * cs = new KisRgbF16HalfColorSpace(profile);

    // Test mixColors.
    half pixel1[NUM_CHANNELS];
    half pixel2[NUM_CHANNELS];
    half outputPixel[NUM_CHANNELS];

    outputPixel[PIXEL_RED] = 0;
    outputPixel[PIXEL_GREEN] = 0;
    outputPixel[PIXEL_BLUE] = 0;
    outputPixel[PIXEL_ALPHA] = 0;

    pixel1[PIXEL_RED] = CHANNEL_VALUE_ONE;
    pixel1[PIXEL_GREEN] = CHANNEL_VALUE_ONE;
    pixel1[PIXEL_BLUE] = CHANNEL_VALUE_ONE;
    pixel1[PIXEL_ALPHA] = CHANNEL_VALUE_ONE;

    pixel2[PIXEL_RED] = 0;
    pixel2[PIXEL_GREEN] = 0;
    pixel2[PIXEL_BLUE] = 0;
    pixel2[PIXEL_ALPHA] = 0;

    const quint8 *pixelPtrs[2];
    quint8 weights[2];

    pixelPtrs[0] = reinterpret_cast<const quint8 *>(pixel1);
    pixelPtrs[1] = reinterpret_cast<const quint8 *>(pixel2);

    weights[0] = 255;
    weights[1] = 0;

    cs->mixColors(pixelPtrs, weights, 2, reinterpret_cast<quint8 *>(outputPixel));

    CHECK(outputPixel[PIXEL_RED], CHANNEL_VALUE_ONE);
    CHECK(outputPixel[PIXEL_GREEN], CHANNEL_VALUE_ONE);
    CHECK(outputPixel[PIXEL_BLUE], CHANNEL_VALUE_ONE);
    CHECK(outputPixel[PIXEL_ALPHA], CHANNEL_VALUE_ONE);

    weights[0] = 0;
    weights[1] = 255;

    cs->mixColors(pixelPtrs, weights, 2, reinterpret_cast<quint8 *>(outputPixel));

    CHECK(outputPixel[PIXEL_RED], (half)0.0f);
    CHECK(outputPixel[PIXEL_GREEN], (half)0.0f);
    CHECK(outputPixel[PIXEL_BLUE], (half)0.0f);
    CHECK(outputPixel[PIXEL_ALPHA], (half)0.0f);

    weights[0] = 128;
    weights[1] = 127;

    cs->mixColors(pixelPtrs, weights, 2, reinterpret_cast<quint8 *>(outputPixel));

    CHECK(outputPixel[PIXEL_RED], CHANNEL_VALUE_ONE);
    CHECK(outputPixel[PIXEL_GREEN], CHANNEL_VALUE_ONE);
    CHECK(outputPixel[PIXEL_BLUE], CHANNEL_VALUE_ONE);
    CHECK(outputPixel[PIXEL_ALPHA], (half)((128 * CHANNEL_VALUE_ONE) / 255));

    pixel1[PIXEL_RED] = 20000;
    pixel1[PIXEL_GREEN] = 10000;
    pixel1[PIXEL_BLUE] = 5000;
    pixel1[PIXEL_ALPHA] = CHANNEL_VALUE_ONE;

    pixel2[PIXEL_RED] = 10000;
    pixel2[PIXEL_GREEN] = 20000;
    pixel2[PIXEL_BLUE] = 2000;
    pixel2[PIXEL_ALPHA] = CHANNEL_VALUE_ONE;

    cs->mixColors(pixelPtrs, weights, 2, reinterpret_cast<quint8 *>(outputPixel));

    CHECK_TOLERANCE(outputPixel[PIXEL_RED], (128 * 20000 + 127 * 10000) / 255, 5);
    CHECK_TOLERANCE(outputPixel[PIXEL_GREEN], (128 * 10000 + 127 * 20000) / 255, 5);
    CHECK_TOLERANCE(outputPixel[PIXEL_BLUE], (128 * 5000 + 127 * 2000) / 255, 5);
    CHECK(outputPixel[PIXEL_ALPHA], CHANNEL_VALUE_ONE);

    pixel1[PIXEL_RED] = 0;
    pixel1[PIXEL_GREEN] = 0;
    pixel1[PIXEL_BLUE] = 0;
    pixel1[PIXEL_ALPHA] = 0;

    pixel2[PIXEL_RED] = CHANNEL_VALUE_ONE;
    pixel2[PIXEL_GREEN] = CHANNEL_VALUE_ONE;
    pixel2[PIXEL_BLUE] = CHANNEL_VALUE_ONE;
    pixel2[PIXEL_ALPHA] = CHANNEL_VALUE_ONE;

    weights[0] = 89;
    weights[1] = 166;

    cs->mixColors(pixelPtrs, weights, 2, reinterpret_cast<quint8 *>(outputPixel));

    CHECK(outputPixel[PIXEL_RED], CHANNEL_VALUE_ONE);
    CHECK(outputPixel[PIXEL_GREEN], CHANNEL_VALUE_ONE);
    CHECK(outputPixel[PIXEL_BLUE], CHANNEL_VALUE_ONE);
    CHECK_TOLERANCE(outputPixel[PIXEL_ALPHA], (89 * 0 + 166 * CHANNEL_VALUE_ONE) / 255, 5);
}

#define PIXELS_WIDTH 2
#define PIXELS_HEIGHT 2

void KisRgbF16HalfColorSpaceTester::testToQImage()
{
    KoColorProfile *profile = new KoColorProfile(cmsCreate_sRGBProfile());

    KisAbstractColorSpace * cs = new KisRgbF16HalfColorSpace(profile);

    KisRgbF16HalfColorSpace::Pixel pixels[PIXELS_WIDTH * PIXELS_HEIGHT] = {
        {CHANNEL_VALUE_ONE, CHANNEL_VALUE_ONE, CHANNEL_VALUE_ONE, CHANNEL_VALUE_ONE / 4},
        {CHANNEL_VALUE_ONE / 4, CHANNEL_VALUE_ONE / 2, CHANNEL_VALUE_ONE / 3, CHANNEL_VALUE_ONE / 2},
        {CHANNEL_VALUE_ONE, CHANNEL_VALUE_ONE, CHANNEL_VALUE_ONE, CHANNEL_VALUE_ZERO},
        {CHANNEL_VALUE_ZERO, CHANNEL_VALUE_ZERO, CHANNEL_VALUE_ZERO, CHANNEL_VALUE_ONE}
    };

    QImage image = cs->convertToQImage(reinterpret_cast<const quint8 *>(pixels), PIXELS_WIDTH, PIXELS_HEIGHT, 0, 0);

    QRgb c = image.pixel(0, 0);

    // Exposure comes into play here.
    /*
    CHECK(qRed(c), (int)HALF_TO_UINT8(CHANNEL_VALUE_ONE));
    CHECK(qGreen(c), (int)HALF_TO_UINT8(CHANNEL_VALUE_ONE));
    CHECK(qBlue(c), (int)HALF_TO_UINT8(CHANNEL_VALUE_ONE));
    CHECK(qAlpha(c), (int)HALF_TO_UINT8(CHANNEL_VALUE_ONE / 4));

    c = image.pixel(1, 0);

    CHECK(qRed(c), (int)HALF_TO_UINT8(CHANNEL_VALUE_ONE / 3));
    CHECK(qGreen(c), (int)HALF_TO_UINT8(CHANNEL_VALUE_ONE / 2));
    CHECK(qBlue(c), (int)HALF_TO_UINT8(CHANNEL_VALUE_ONE / 4));
    CHECK(qAlpha(c), (int)HALF_TO_UINT8(CHANNEL_VALUE_ONE / 2));

    c = image.pixel(0, 1);

    CHECK(qRed(c), (int)HALF_TO_UINT8(CHANNEL_VALUE_ONE));
    CHECK(qGreen(c), (int)HALF_TO_UINT8(CHANNEL_VALUE_ONE));
    CHECK(qBlue(c), (int)HALF_TO_UINT8(CHANNEL_VALUE_ONE));
    CHECK(qAlpha(c), (int)HALF_TO_UINT8(CHANNEL_VALUE_ZERO));

    c = image.pixel(1, 1);

    CHECK(qRed(c), (int)HALF_TO_UINT8(CHANNEL_VALUE_ZERO));
    CHECK(qGreen(c), (int)HALF_TO_UINT8(CHANNEL_VALUE_ZERO));
    CHECK(qBlue(c), (int)HALF_TO_UINT8(CHANNEL_VALUE_ZERO));
    CHECK(qAlpha(c), (int)HALF_TO_UINT8(CHANNEL_VALUE_ONE));
    */
}

#define NUM_ROWS 2
#define NUM_COLUMNS 2
#define SRC_ROW_STRIDE (NUM_COLUMNS * CHANNEL_SIZE)
#define DST_ROW_STRIDE (NUM_COLUMNS * CHANNEL_SIZE)
#define MASK_ROW_STRIDE NUM_COLUMNS

/*
1 alpha 1    0 alpha 1
1 alpha 0.5  0 alpha 1
1 alpha 0.5  0 alpha 0.5
1 alpha 0    0 alpha 0.5

*/

void  KisRgbF16HalfColorSpaceTester::testCompositeOps()
{
    KoColorProfile *profile = new KoColorProfile(cmsCreate_sRGBProfile());

    KisRgbF16HalfColorSpace *cs = new KisRgbF16HalfColorSpace(profile);

    KisRgbF16HalfColorSpace::Pixel srcPixel;
    KisRgbF16HalfColorSpace::Pixel dstPixel;

    srcPixel.red = UINT8_TO_HALF(102);
    srcPixel.green = UINT8_TO_HALF(170);
    srcPixel.blue = UINT8_TO_HALF(238);
    srcPixel.alpha = F16HALF_OPACITY_OPAQUE;

    dstPixel = srcPixel;

    cs->compositeDivide(reinterpret_cast<quint8 *>(&dstPixel), 1, reinterpret_cast<const quint8 *>(&srcPixel),
                1, 0, 0, 1, 1, F16HALF_OPACITY_OPAQUE);
    /*
    CHECK(dstPixel.red, (quint16)UINT8_TO_UINT16(253));
    CHECK(dstPixel.green, (quint16)UINT8_TO_UINT16(254));
    CHECK(dstPixel.blue, (quint16)UINT8_TO_UINT16(254));
    CHECK(dstPixel.alpha, KisRgbF16HalfColorSpace::F16HALF_OPACITY_OPAQUE);

    quint16 srcColor = 43690;
    quint16 dstColor = 43690;

    srcColor = qMin((dstColor * (65535u + 1u) + (srcColor / 2u)) / (1u + srcColor), 65535u);

    CHECK((int)srcColor, 65534);

    quint16 newColor = UINT16_BLEND(srcColor, dstColor, 65535u);

    CHECK((int)newColor, 65534);
    */

    /*
    KisRgbF16HalfColorSpace::Pixel srcPixels[NUM_ROWS * NUM_COLUMNS] = {
        {CHANNEL_VALUE_ONE, CHANNEL_VALUE_ONE, CHANNEL_VALUE_ONE, CHANNEL_VALUE_ONE / 4},
        {CHANNEL_VALUE_ONE / 4, CHANNEL_VALUE_ONE / 2, CHANNEL_VALUE_ONE / 3, CHANNEL_VALUE_ONE / 2},
        {CHANNEL_VALUE_ONE, CHANNEL_VALUE_ONE, CHANNEL_VALUE_ONE, CHANNEL_VALUE_ZERO},
        {CHANNEL_VALUE_ZERO, CHANNEL_VALUE_ZERO, CHANNEL_VALUE_ZERO, CHANNEL_VALUE_ONE}
    };

    KisRgbF16HalfColorSpace::Pixel dstPixels[NUM_ROWS * NUM_COLUMNS] = {
        {CHANNEL_VALUE_ONE, CHANNEL_VALUE_ONE, CHANNEL_VALUE_ONE, CHANNEL_VALUE_ONE / 4},
        {CHANNEL_VALUE_ONE / 4, CHANNEL_VALUE_ONE / 2, CHANNEL_VALUE_ONE / 3, CHANNEL_VALUE_ONE / 2},
        {CHANNEL_VALUE_ONE, CHANNEL_VALUE_ONE, CHANNEL_VALUE_ONE, CHANNEL_VALUE_ZERO},
        {CHANNEL_VALUE_ZERO, CHANNEL_VALUE_ZERO, CHANNEL_VALUE_ZERO, CHANNEL_VALUE_ONE}
    };

    cs->compositeOver(reinterpret_cast<quint8 *>(dstPixels), DST_ROW_STRIDE, reinterpret_cast<const quint8 *>(srcPixels),
                SRC_ROW_STRIDE, mask, MASK_ROW_STRIDE, NUM_ROWS, NUM_COLUMNS, opacity);
    */

    delete cs;
}

