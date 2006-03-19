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
#include "kis_strategy_colorspace_rgb_f32_tester.h"
#include "kis_rgb_f32_colorspace.h"
#include "kis_integer_maths.h"
#include "kis_paint_device.h"

using namespace KUnitTest;

KUNITTEST_MODULE( kunittest_kis_strategy_colorspace_rgb_f32_tester, "RGBA 32-bit float colorspace tester" );
KUNITTEST_MODULE_REGISTER_TESTER( KisRgbF32ColorSpaceTester );

#define PIXEL_BLUE 0
#define PIXEL_GREEN 1
#define PIXEL_RED 2
#define PIXEL_ALPHA 3

#define NUM_CHANNELS 4
#define NUM_COLOUR_CHANNELS 3
#define CHANNEL_SIZE ((int)sizeof(float))

#define RED_CHANNEL 0
#define GREEN_CHANNEL 1
#define BLUE_CHANNEL 2
#define ALPHA_CHANNEL 3

#define MAX_CHANNEL_VALUE 1.0f
#define MIN_CHANNEL_VALUE 0.0f


void KisRgbF32ColorSpaceTester::allTests()
{
    // We need this so that the colour profile loading can operate without crashing.
    KisFactory *factory = new KisFactory();

    testBasics();
    testToQImage();
    testCompositeOps();
        testMixColors();

    delete factory;
}

void KisRgbF32ColorSpaceTester::testBasics()
{
    KisProfile *profile = new KisProfile(cmsCreate_sRGBProfile());

    KisRgbF32ColorSpace *cs = new KisRgbF32ColorSpace(profile);
    KisAbstractColorSpace * csSP = cs;

    CHECK(cs->hasAlpha(), true);
    CHECK(cs->nChannels(), NUM_CHANNELS);
    CHECK(cs->nColorChannels(), NUM_COLOUR_CHANNELS);
    CHECK(cs->pixelSize(), NUM_CHANNELS * CHANNEL_SIZE);

    QValueVector<KisChannelInfo *> channels = cs->channels();

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

    KisRgbF32ColorSpace::Pixel defaultPixel;

    memcpy(&defaultPixel, pd->dataManager()->defaultPixel(), sizeof(defaultPixel));

    CHECK(defaultPixel.red, 0.0f);
    CHECK(defaultPixel.green, 0.0f);
    CHECK(defaultPixel.blue, 0.0f);
    CHECK(defaultPixel.alpha, F32_OPACITY_TRANSPARENT);

    float pixel[NUM_CHANNELS];

    cs->fromQColor(qRgb(255, 255, 255), reinterpret_cast<Q_UINT8 *>(pixel));

    CHECK(pixel[PIXEL_RED], MAX_CHANNEL_VALUE);
    CHECK(pixel[PIXEL_GREEN], MAX_CHANNEL_VALUE);
    CHECK(pixel[PIXEL_BLUE], MAX_CHANNEL_VALUE);

    cs->fromQColor(qRgb(0, 0, 0), reinterpret_cast<Q_UINT8 *>(pixel));

    CHECK(pixel[PIXEL_RED], MIN_CHANNEL_VALUE);
    CHECK(pixel[PIXEL_GREEN], MIN_CHANNEL_VALUE);
    CHECK(pixel[PIXEL_BLUE], MIN_CHANNEL_VALUE);

    cs->fromQColor(qRgb(128, 64, 192), reinterpret_cast<Q_UINT8 *>(pixel));

    CHECK(pixel[PIXEL_RED], UINT8_TO_FLOAT(128));
    CHECK(pixel[PIXEL_GREEN], UINT8_TO_FLOAT(64));
    CHECK(pixel[PIXEL_BLUE], UINT8_TO_FLOAT(192));

    cs->fromQColor(qRgb(255, 255, 255), OPACITY_OPAQUE, reinterpret_cast<Q_UINT8 *>(pixel));

    CHECK(pixel[PIXEL_RED], MAX_CHANNEL_VALUE);
    CHECK(pixel[PIXEL_GREEN], MAX_CHANNEL_VALUE);
    CHECK(pixel[PIXEL_BLUE], MAX_CHANNEL_VALUE);
    CHECK(pixel[PIXEL_ALPHA], MAX_CHANNEL_VALUE);

    cs->fromQColor(qRgb(255, 255, 255), OPACITY_TRANSPARENT, reinterpret_cast<Q_UINT8 *>(pixel));

    CHECK(pixel[PIXEL_RED], MAX_CHANNEL_VALUE);
    CHECK(pixel[PIXEL_GREEN], MAX_CHANNEL_VALUE);
    CHECK(pixel[PIXEL_BLUE], MAX_CHANNEL_VALUE);
    CHECK(pixel[PIXEL_ALPHA], F32_OPACITY_TRANSPARENT);

    cs->fromQColor(qRgb(255, 255, 255), OPACITY_OPAQUE / 2, reinterpret_cast<Q_UINT8 *>(pixel));

    CHECK(pixel[PIXEL_RED], MAX_CHANNEL_VALUE);
    CHECK(pixel[PIXEL_GREEN], MAX_CHANNEL_VALUE);
    CHECK(pixel[PIXEL_BLUE], MAX_CHANNEL_VALUE);
    CHECK(pixel[PIXEL_ALPHA], UINT8_TO_FLOAT(OPACITY_OPAQUE / 2));

    pixel[PIXEL_RED] = MAX_CHANNEL_VALUE;
    pixel[PIXEL_GREEN] = MAX_CHANNEL_VALUE;
    pixel[PIXEL_BLUE] = MAX_CHANNEL_VALUE;

    QColor c;

    cs->toQColor(reinterpret_cast<const Q_UINT8 *>(pixel), &c);

    CHECK(c.red(), 255);
    CHECK(c.green(), 255);
    CHECK(c.blue(), 255);

    pixel[PIXEL_RED] = MIN_CHANNEL_VALUE;
    pixel[PIXEL_GREEN] = MIN_CHANNEL_VALUE;
    pixel[PIXEL_BLUE] = MIN_CHANNEL_VALUE;

    cs->toQColor(reinterpret_cast<const Q_UINT8 *>(pixel), &c);

    CHECK(c.red(), 0);
    CHECK(c.green(), 0);
    CHECK(c.blue(), 0);

    pixel[PIXEL_RED] = MAX_CHANNEL_VALUE / 4;
    pixel[PIXEL_GREEN] = MAX_CHANNEL_VALUE / 2;
    pixel[PIXEL_BLUE] = (3 * MAX_CHANNEL_VALUE) / 4;

    cs->toQColor(reinterpret_cast<const Q_UINT8 *>(pixel), &c);

    CHECK(c.red(), (int)FLOAT_TO_UINT8(MAX_CHANNEL_VALUE / 4));
    CHECK(c.green(), (int)FLOAT_TO_UINT8(MAX_CHANNEL_VALUE / 2));
    CHECK(c.blue(), (int)FLOAT_TO_UINT8((3 * MAX_CHANNEL_VALUE) / 4));

    pixel[PIXEL_RED] = MAX_CHANNEL_VALUE;
    pixel[PIXEL_GREEN] = MAX_CHANNEL_VALUE;
    pixel[PIXEL_BLUE] = MAX_CHANNEL_VALUE;
    pixel[PIXEL_ALPHA] = MAX_CHANNEL_VALUE;

    Q_UINT8 opacity;

    cs->toQColor(reinterpret_cast<const Q_UINT8 *>(pixel), &c, &opacity);

    CHECK(c.red(), 255);
    CHECK(c.green(), 255);
    CHECK(c.blue(), 255);
    CHECK(opacity, OPACITY_OPAQUE);

    pixel[PIXEL_ALPHA] = F32_OPACITY_OPAQUE;

    cs->toQColor(reinterpret_cast<const Q_UINT8 *>(pixel), &c, &opacity);

    CHECK(c.red(), 255);
    CHECK(c.green(), 255);
    CHECK(c.blue(), 255);
    CHECK(opacity, OPACITY_OPAQUE);

    pixel[PIXEL_RED] = MIN_CHANNEL_VALUE;
    pixel[PIXEL_GREEN] = MIN_CHANNEL_VALUE;
    pixel[PIXEL_BLUE] = MIN_CHANNEL_VALUE;
    pixel[PIXEL_ALPHA] = F32_OPACITY_TRANSPARENT;

    cs->toQColor(reinterpret_cast<const Q_UINT8 *>(pixel), &c, &opacity);

    CHECK(c.red(), 0);
    CHECK(c.green(), 0);
    CHECK(c.blue(), 0);
    CHECK(opacity, OPACITY_TRANSPARENT);

    pixel[PIXEL_RED] = MAX_CHANNEL_VALUE / 4;
    pixel[PIXEL_GREEN] = MAX_CHANNEL_VALUE / 2;
    pixel[PIXEL_BLUE] = (3 * MAX_CHANNEL_VALUE) / 4;
    pixel[PIXEL_ALPHA] = MAX_CHANNEL_VALUE / 2;

    cs->toQColor(reinterpret_cast<const Q_UINT8 *>(pixel), &c, &opacity);

    CHECK(c.red(), (int)FLOAT_TO_UINT8(MAX_CHANNEL_VALUE / 4));
    CHECK(c.green(), (int)FLOAT_TO_UINT8(MAX_CHANNEL_VALUE / 2));
    CHECK(c.blue(), (int)FLOAT_TO_UINT8((3 * MAX_CHANNEL_VALUE) / 4));
    CHECK((int)opacity, (int)FLOAT_TO_UINT8(MAX_CHANNEL_VALUE / 2));

    #define NUM_PIXELS 4

    KisRgbF32ColorSpace::Pixel pixels[NUM_PIXELS] = {
        {MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE / 4},
        {MAX_CHANNEL_VALUE / 4, MAX_CHANNEL_VALUE / 2, MAX_CHANNEL_VALUE / 3, MAX_CHANNEL_VALUE / 2},
        {MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE, MIN_CHANNEL_VALUE},
        {MIN_CHANNEL_VALUE, MIN_CHANNEL_VALUE, MIN_CHANNEL_VALUE, MAX_CHANNEL_VALUE}
    };

    cs->setAlpha(reinterpret_cast<Q_UINT8 *>(pixels), OPACITY_OPAQUE / 2, NUM_PIXELS);

    CHECK(pixels[0].red, MAX_CHANNEL_VALUE);
    CHECK(pixels[0].green, MAX_CHANNEL_VALUE);
    CHECK(pixels[0].blue, MAX_CHANNEL_VALUE);
    CHECK(pixels[0].alpha, UINT8_TO_FLOAT(OPACITY_OPAQUE / 2));

    CHECK(pixels[1].red, MAX_CHANNEL_VALUE / 3);
    CHECK(pixels[1].green, MAX_CHANNEL_VALUE / 2);
    CHECK(pixels[1].blue, MAX_CHANNEL_VALUE / 4);
    CHECK(pixels[1].alpha, UINT8_TO_FLOAT(OPACITY_OPAQUE / 2));

    CHECK(pixels[2].red, MAX_CHANNEL_VALUE);
    CHECK(pixels[2].green, MAX_CHANNEL_VALUE);
    CHECK(pixels[2].blue, MAX_CHANNEL_VALUE);
    CHECK(pixels[2].alpha, UINT8_TO_FLOAT(OPACITY_OPAQUE / 2));

    CHECK(pixels[3].red, MIN_CHANNEL_VALUE);
    CHECK(pixels[3].green, MIN_CHANNEL_VALUE);
    CHECK(pixels[3].blue, MIN_CHANNEL_VALUE);
    CHECK(pixels[3].alpha, UINT8_TO_FLOAT(OPACITY_OPAQUE / 2));

    pixel[PIXEL_RED] = MAX_CHANNEL_VALUE;
    pixel[PIXEL_GREEN] = MAX_CHANNEL_VALUE / 2;
    pixel[PIXEL_BLUE] = MAX_CHANNEL_VALUE / 4;
    pixel[PIXEL_ALPHA] = MIN_CHANNEL_VALUE;

    QString valueText = cs->channelValueText(reinterpret_cast<Q_UINT8 *>(pixel), RED_CHANNEL);
    CHECK(valueText, QString().setNum(MAX_CHANNEL_VALUE));

    valueText = cs->channelValueText(reinterpret_cast<Q_UINT8 *>(pixel), GREEN_CHANNEL);
    CHECK(valueText, QString().setNum(MAX_CHANNEL_VALUE / 2));

    valueText = cs->channelValueText(reinterpret_cast<Q_UINT8 *>(pixel), BLUE_CHANNEL);
    CHECK(valueText, QString().setNum(MAX_CHANNEL_VALUE / 4));

    valueText = cs->channelValueText(reinterpret_cast<Q_UINT8 *>(pixel), ALPHA_CHANNEL);
    CHECK(valueText, QString().setNum(MIN_CHANNEL_VALUE));

    valueText = cs->normalisedChannelValueText(reinterpret_cast<Q_UINT8 *>(pixel), RED_CHANNEL);
    CHECK(valueText, QString().setNum(MAX_CHANNEL_VALUE));

    valueText = cs->normalisedChannelValueText(reinterpret_cast<Q_UINT8 *>(pixel), GREEN_CHANNEL);
    CHECK(valueText, QString().setNum(MAX_CHANNEL_VALUE / 2));

    valueText = cs->normalisedChannelValueText(reinterpret_cast<Q_UINT8 *>(pixel), BLUE_CHANNEL);
    CHECK(valueText, QString().setNum(MAX_CHANNEL_VALUE / 4));

    valueText = cs->normalisedChannelValueText(reinterpret_cast<Q_UINT8 *>(pixel), ALPHA_CHANNEL);
    CHECK(valueText, QString().setNum(MIN_CHANNEL_VALUE));

    cs->setPixel(reinterpret_cast<Q_UINT8 *>(pixel), 0.128, 0.192, 0.64, 0.99);
    CHECK(pixel[PIXEL_RED], 0.128f);
    CHECK(pixel[PIXEL_GREEN], 0.192f);
    CHECK(pixel[PIXEL_BLUE], 0.64f);
    CHECK(pixel[PIXEL_ALPHA], 0.99f);

    float red;
    float green;
    float blue;
    float alpha;

    cs->getPixel(reinterpret_cast<const Q_UINT8 *>(pixel), &red, &green, &blue, &alpha);
    CHECK(red, 0.128f);
    CHECK(green, 0.192f);
    CHECK(blue, 0.64f);
    CHECK(alpha, 0.99f);

    CHECK(FLOAT_TO_UINT8(-0.5), 0u);
    CHECK(FLOAT_TO_UINT8(0), 0u);
    CHECK_TOLERANCE(FLOAT_TO_UINT8(0.5), UINT8_MAX / 2, 1u);
    CHECK(FLOAT_TO_UINT8(1), UINT8_MAX);
    CHECK(FLOAT_TO_UINT8(1.5), UINT8_MAX);

    CHECK(FLOAT_TO_UINT16(-0.5), 0u);
    CHECK(FLOAT_TO_UINT16(0), 0u);
    CHECK_TOLERANCE(FLOAT_TO_UINT16(0.5), UINT16_MAX / 2, 1u);
    CHECK(FLOAT_TO_UINT16(1), UINT16_MAX);
    CHECK(FLOAT_TO_UINT16(1.5), UINT16_MAX);
}

void KisRgbF32ColorSpaceTester::testMixColors()
{
    KisProfile *profile = new KisProfile(cmsCreate_sRGBProfile());

    KisAbstractColorSpace * cs = new KisRgbF32ColorSpace(profile);

    // Test mixColors.
    float pixel1[NUM_CHANNELS];
    float pixel2[NUM_CHANNELS];
    float outputPixel[NUM_CHANNELS];

    outputPixel[PIXEL_RED] = 0;
    outputPixel[PIXEL_GREEN] = 0;
    outputPixel[PIXEL_BLUE] = 0;
    outputPixel[PIXEL_ALPHA] = 0;

    pixel1[PIXEL_RED] = MAX_CHANNEL_VALUE;
    pixel1[PIXEL_GREEN] = MAX_CHANNEL_VALUE;
    pixel1[PIXEL_BLUE] = MAX_CHANNEL_VALUE;
    pixel1[PIXEL_ALPHA] = MAX_CHANNEL_VALUE;

    pixel2[PIXEL_RED] = 0;
    pixel2[PIXEL_GREEN] = 0;
    pixel2[PIXEL_BLUE] = 0;
    pixel2[PIXEL_ALPHA] = 0;

    const Q_UINT8 *pixelPtrs[2];
    Q_UINT8 weights[2];

    pixelPtrs[0] = reinterpret_cast<const Q_UINT8 *>(pixel1);
    pixelPtrs[1] = reinterpret_cast<const Q_UINT8 *>(pixel2);

    weights[0] = 255;
    weights[1] = 0;

    cs->mixColors(pixelPtrs, weights, 2, reinterpret_cast<Q_UINT8 *>(outputPixel));

    CHECK(outputPixel[PIXEL_RED], MAX_CHANNEL_VALUE);
    CHECK(outputPixel[PIXEL_GREEN], MAX_CHANNEL_VALUE);
    CHECK(outputPixel[PIXEL_BLUE], MAX_CHANNEL_VALUE);
    CHECK(outputPixel[PIXEL_ALPHA], MAX_CHANNEL_VALUE);

    weights[0] = 0;
    weights[1] = 255;

    cs->mixColors(pixelPtrs, weights, 2, reinterpret_cast<Q_UINT8 *>(outputPixel));

    CHECK(outputPixel[PIXEL_RED], 0.0f);
    CHECK(outputPixel[PIXEL_GREEN], 0.0f);
    CHECK(outputPixel[PIXEL_BLUE], 0.0f);
    CHECK(outputPixel[PIXEL_ALPHA], 0.0f);

    weights[0] = 128;
    weights[1] = 127;

    cs->mixColors(pixelPtrs, weights, 2, reinterpret_cast<Q_UINT8 *>(outputPixel));

    CHECK(outputPixel[PIXEL_RED], MAX_CHANNEL_VALUE);
    CHECK(outputPixel[PIXEL_GREEN], MAX_CHANNEL_VALUE);
    CHECK(outputPixel[PIXEL_BLUE], MAX_CHANNEL_VALUE);
    CHECK(outputPixel[PIXEL_ALPHA], (128 * MAX_CHANNEL_VALUE) / 255);

    pixel1[PIXEL_RED] = 20000;
    pixel1[PIXEL_GREEN] = 10000;
    pixel1[PIXEL_BLUE] = 5000;
    pixel1[PIXEL_ALPHA] = MAX_CHANNEL_VALUE;

    pixel2[PIXEL_RED] = 10000;
    pixel2[PIXEL_GREEN] = 20000;
    pixel2[PIXEL_BLUE] = 2000;
    pixel2[PIXEL_ALPHA] = MAX_CHANNEL_VALUE;

    cs->mixColors(pixelPtrs, weights, 2, reinterpret_cast<Q_UINT8 *>(outputPixel));

    CHECK_TOLERANCE(outputPixel[PIXEL_RED], (128 * 20000 + 127 * 10000) / 255, 5);
    CHECK_TOLERANCE(outputPixel[PIXEL_GREEN], (128 * 10000 + 127 * 20000) / 255, 5);
    CHECK_TOLERANCE(outputPixel[PIXEL_BLUE], (128 * 5000 + 127 * 2000) / 255, 5);
    CHECK(outputPixel[PIXEL_ALPHA], MAX_CHANNEL_VALUE);

    pixel1[PIXEL_RED] = 0;
    pixel1[PIXEL_GREEN] = 0;
    pixel1[PIXEL_BLUE] = 0;
    pixel1[PIXEL_ALPHA] = 0;

    pixel2[PIXEL_RED] = MAX_CHANNEL_VALUE;
    pixel2[PIXEL_GREEN] = MAX_CHANNEL_VALUE;
    pixel2[PIXEL_BLUE] = MAX_CHANNEL_VALUE;
    pixel2[PIXEL_ALPHA] = MAX_CHANNEL_VALUE;

    weights[0] = 89;
    weights[1] = 166;

    cs->mixColors(pixelPtrs, weights, 2, reinterpret_cast<Q_UINT8 *>(outputPixel));

    CHECK(outputPixel[PIXEL_RED], MAX_CHANNEL_VALUE);
    CHECK(outputPixel[PIXEL_GREEN], MAX_CHANNEL_VALUE);
    CHECK(outputPixel[PIXEL_BLUE], MAX_CHANNEL_VALUE);
    CHECK_TOLERANCE(outputPixel[PIXEL_ALPHA], (89 * 0 + 166 * MAX_CHANNEL_VALUE) / 255, 5);
}

#define PIXELS_WIDTH 2
#define PIXELS_HEIGHT 2

void KisRgbF32ColorSpaceTester::testToQImage()
{
    KisProfile *profile = new KisProfile(cmsCreate_sRGBProfile());

    KisAbstractColorSpace * cs = new KisRgbF32ColorSpace(profile);

    KisRgbF32ColorSpace::Pixel pixels[PIXELS_WIDTH * PIXELS_HEIGHT] = {
        {MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE / 4},
        {MAX_CHANNEL_VALUE / 4, MAX_CHANNEL_VALUE / 2, MAX_CHANNEL_VALUE / 3, MAX_CHANNEL_VALUE / 2},
        {MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE, MIN_CHANNEL_VALUE},
        {MIN_CHANNEL_VALUE, MIN_CHANNEL_VALUE, MIN_CHANNEL_VALUE, MAX_CHANNEL_VALUE}
    };

    QImage image = cs->convertToQImage(reinterpret_cast<const Q_UINT8 *>(pixels), PIXELS_WIDTH, PIXELS_HEIGHT, 0, 0);

    QRgb c = image.pixel(0, 0);

    // Exposure comes into play here.
    /*
    CHECK(qRed(c), (int)FLOAT_TO_UINT8(MAX_CHANNEL_VALUE));
    CHECK(qGreen(c), (int)FLOAT_TO_UINT8(MAX_CHANNEL_VALUE));
    CHECK(qBlue(c), (int)FLOAT_TO_UINT8(MAX_CHANNEL_VALUE));
    CHECK(qAlpha(c), (int)FLOAT_TO_UINT8(MAX_CHANNEL_VALUE / 4));

    c = image.pixel(1, 0);

    CHECK(qRed(c), (int)FLOAT_TO_UINT8(MAX_CHANNEL_VALUE / 3));
    CHECK(qGreen(c), (int)FLOAT_TO_UINT8(MAX_CHANNEL_VALUE / 2));
    CHECK(qBlue(c), (int)FLOAT_TO_UINT8(MAX_CHANNEL_VALUE / 4));
    CHECK(qAlpha(c), (int)FLOAT_TO_UINT8(MAX_CHANNEL_VALUE / 2));

    c = image.pixel(0, 1);

    CHECK(qRed(c), (int)FLOAT_TO_UINT8(MAX_CHANNEL_VALUE));
    CHECK(qGreen(c), (int)FLOAT_TO_UINT8(MAX_CHANNEL_VALUE));
    CHECK(qBlue(c), (int)FLOAT_TO_UINT8(MAX_CHANNEL_VALUE));
    CHECK(qAlpha(c), (int)FLOAT_TO_UINT8(MIN_CHANNEL_VALUE));

    c = image.pixel(1, 1);

    CHECK(qRed(c), (int)FLOAT_TO_UINT8(MIN_CHANNEL_VALUE));
    CHECK(qGreen(c), (int)FLOAT_TO_UINT8(MIN_CHANNEL_VALUE));
    CHECK(qBlue(c), (int)FLOAT_TO_UINT8(MIN_CHANNEL_VALUE));
    CHECK(qAlpha(c), (int)FLOAT_TO_UINT8(MAX_CHANNEL_VALUE));
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

void KisRgbF32ColorSpaceTester::testCompositeOps()
{
    KisProfile *profile = new KisProfile(cmsCreate_sRGBProfile());

    KisRgbF32ColorSpace *cs = new KisRgbF32ColorSpace(profile);

    KisRgbF32ColorSpace::Pixel srcPixel;
    KisRgbF32ColorSpace::Pixel dstPixel;

    srcPixel.red = UINT8_TO_FLOAT(102);
    srcPixel.green = UINT8_TO_FLOAT(170);
    srcPixel.blue = UINT8_TO_FLOAT(238);
    srcPixel.alpha = F32_OPACITY_OPAQUE;

    dstPixel = srcPixel;

    cs->compositeDivide(reinterpret_cast<Q_UINT8 *>(&dstPixel), 1, reinterpret_cast<const Q_UINT8 *>(&srcPixel),
                1, 0, 0, 1, 1, F32_OPACITY_OPAQUE);
    /*
    CHECK(dstPixel.red, (Q_UINT16)UINT8_TO_UINT16(253));
    CHECK(dstPixel.green, (Q_UINT16)UINT8_TO_UINT16(254));
    CHECK(dstPixel.blue, (Q_UINT16)UINT8_TO_UINT16(254));
    CHECK(dstPixel.alpha, KisRgbF32ColorSpace::F32_OPACITY_OPAQUE);

    Q_UINT16 srcColor = 43690;
    Q_UINT16 dstColor = 43690;

    srcColor = QMIN((dstColor * (65535u + 1u) + (srcColor / 2u)) / (1u + srcColor), 65535u);

    CHECK((int)srcColor, 65534);

    Q_UINT16 newColor = UINT16_BLEND(srcColor, dstColor, 65535u);

    CHECK((int)newColor, 65534);
    */

    /*
    KisRgbF32ColorSpace::Pixel srcPixels[NUM_ROWS * NUM_COLUMNS] = {
        {MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE / 4},
        {MAX_CHANNEL_VALUE / 4, MAX_CHANNEL_VALUE / 2, MAX_CHANNEL_VALUE / 3, MAX_CHANNEL_VALUE / 2},
        {MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE, MIN_CHANNEL_VALUE},
        {MIN_CHANNEL_VALUE, MIN_CHANNEL_VALUE, MIN_CHANNEL_VALUE, MAX_CHANNEL_VALUE}
    };

    KisRgbF32ColorSpace::Pixel dstPixels[NUM_ROWS * NUM_COLUMNS] = {
        {MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE / 4},
        {MAX_CHANNEL_VALUE / 4, MAX_CHANNEL_VALUE / 2, MAX_CHANNEL_VALUE / 3, MAX_CHANNEL_VALUE / 2},
        {MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE, MIN_CHANNEL_VALUE},
        {MIN_CHANNEL_VALUE, MIN_CHANNEL_VALUE, MIN_CHANNEL_VALUE, MAX_CHANNEL_VALUE}
    };

    cs->compositeOver(reinterpret_cast<Q_UINT8 *>(dstPixels), DST_ROW_STRIDE, reinterpret_cast<const Q_UINT8 *>(srcPixels),
                SRC_ROW_STRIDE, mask, MASK_ROW_STRIDE, NUM_ROWS, NUM_COLUMNS, opacity);
    */

    delete cs;
}

