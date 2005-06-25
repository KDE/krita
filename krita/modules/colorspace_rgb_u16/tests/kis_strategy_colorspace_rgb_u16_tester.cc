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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <kunittest/runner.h>
#include <kunittest/module.h>

#include "kis_factory.h"
#include "kis_strategy_colorspace_rgb_u16_tester.h"
#include "kis_strategy_colorspace_rgb_u16.h"
#include "kis_integer_maths.h"
#include "kis_paint_device.h"

using namespace KUnitTest;

KUNITTEST_MODULE( kunittest_kis_strategy_colorspace_rgb_u16_tester, "RGB 16-bit integer colorspace tester" );
KUNITTEST_MODULE_REGISTER_TESTER( KisStrategyColorSpaceRGBU16Tester );

#define PIXEL_BLUE 0
#define PIXEL_GREEN 1
#define PIXEL_RED 2
#define PIXEL_ALPHA 3

#define NUM_CHANNELS 4
#define NUM_COLOUR_CHANNELS 3
#define CHANNEL_SIZE 2

#define MAX_CHANNEL_VALUE UINT16_MAX
#define MIN_CHANNEL_VALUE UINT16_MIN

void KisStrategyColorSpaceRGBU16Tester::allTests()
{
	// We need this so that the colour profile loading can operate without crashing.
	KisFactory *factory = new KisFactory();

	testBasics();
	testToQImage();
        //testMixColors();

	delete factory;
}

void KisStrategyColorSpaceRGBU16Tester::testBasics()
{
	KisStrategyColorSpaceSP cs = new KisStrategyColorSpaceRGBU16();

	CHECK(cs -> alpha(), true);
	CHECK(cs -> nChannels(), NUM_CHANNELS);
	CHECK(cs -> nColorChannels(), NUM_COLOUR_CHANNELS);
	CHECK(cs -> pixelSize(), NUM_CHANNELS * CHANNEL_SIZE);

	vKisChannelInfoSP channels = cs -> channels();

	// Red
	CHECK(channels[0] -> pos(), PIXEL_RED * CHANNEL_SIZE);
	CHECK(channels[0] -> size(), CHANNEL_SIZE);
	CHECK(channels[0] -> channelType(), COLOR);

	// Green
	CHECK(channels[1] -> pos(), PIXEL_GREEN * CHANNEL_SIZE);
	CHECK(channels[1] -> size(), CHANNEL_SIZE);
	CHECK(channels[1] -> channelType(), COLOR);

	// Blue
	CHECK(channels[2] -> pos(), PIXEL_BLUE * CHANNEL_SIZE);
	CHECK(channels[2] -> size(), CHANNEL_SIZE);
	CHECK(channels[2] -> channelType(), COLOR);

	// Alpha
	CHECK(channels[3] -> pos(), PIXEL_ALPHA * CHANNEL_SIZE);
	CHECK(channels[3] -> size(), CHANNEL_SIZE);
	CHECK(channels[3] -> channelType(), ALPHA);

	KisPaintDeviceSP pd = new KisPaintDevice(cs, "test");

	KisStrategyColorSpaceRGBU16::Pixel defaultPixel;

	memcpy(&defaultPixel, pd -> dataManager() -> defaultPixel(), sizeof(defaultPixel));

	CHECK((int)defaultPixel.red, 0);
	CHECK((int)defaultPixel.green, 0);
	CHECK((int)defaultPixel.blue, 0);
	CHECK((int)defaultPixel.alpha, 0);

	Q_UINT16 pixel[NUM_CHANNELS];

	cs -> nativeColor(qRgb(255, 255, 255), reinterpret_cast<Q_UINT8 *>(pixel));

	CHECK((int)pixel[PIXEL_RED], MAX_CHANNEL_VALUE);
	CHECK((int)pixel[PIXEL_GREEN], MAX_CHANNEL_VALUE);
	CHECK((int)pixel[PIXEL_BLUE], MAX_CHANNEL_VALUE);

	cs -> nativeColor(qRgb(0, 0, 0), reinterpret_cast<Q_UINT8 *>(pixel));

	CHECK((int)pixel[PIXEL_RED], MIN_CHANNEL_VALUE);
	CHECK((int)pixel[PIXEL_GREEN], MIN_CHANNEL_VALUE);
	CHECK((int)pixel[PIXEL_BLUE], MIN_CHANNEL_VALUE);

	cs -> nativeColor(qRgb(128, 64, 192), reinterpret_cast<Q_UINT8 *>(pixel));

	CHECK((int)pixel[PIXEL_RED], (int)UINT8_TO_UINT16(128));
	CHECK((int)pixel[PIXEL_GREEN], (int)UINT8_TO_UINT16(64));
	CHECK((int)pixel[PIXEL_BLUE], (int)UINT8_TO_UINT16(192));

	cs -> nativeColor(qRgb(255, 255, 255), OPACITY_OPAQUE, reinterpret_cast<Q_UINT8 *>(pixel));

	CHECK((int)pixel[PIXEL_RED], MAX_CHANNEL_VALUE);
	CHECK((int)pixel[PIXEL_GREEN], MAX_CHANNEL_VALUE);
	CHECK((int)pixel[PIXEL_BLUE], MAX_CHANNEL_VALUE);
	CHECK((int)pixel[PIXEL_ALPHA], MAX_CHANNEL_VALUE);

	cs -> nativeColor(qRgb(255, 255, 255), OPACITY_TRANSPARENT, reinterpret_cast<Q_UINT8 *>(pixel));

	CHECK((int)pixel[PIXEL_RED], MAX_CHANNEL_VALUE);
	CHECK((int)pixel[PIXEL_GREEN], MAX_CHANNEL_VALUE);
	CHECK((int)pixel[PIXEL_BLUE], MAX_CHANNEL_VALUE);
	CHECK((int)pixel[PIXEL_ALPHA], MIN_CHANNEL_VALUE);

	cs -> nativeColor(qRgb(255, 255, 255), OPACITY_OPAQUE / 2, reinterpret_cast<Q_UINT8 *>(pixel));

	CHECK((int)pixel[PIXEL_RED], MAX_CHANNEL_VALUE);
	CHECK((int)pixel[PIXEL_GREEN], MAX_CHANNEL_VALUE);
	CHECK((int)pixel[PIXEL_BLUE], MAX_CHANNEL_VALUE);
	CHECK((int)pixel[PIXEL_ALPHA], (int)UINT8_TO_UINT16(OPACITY_OPAQUE / 2));

	pixel[PIXEL_RED] = MAX_CHANNEL_VALUE;
	pixel[PIXEL_GREEN] = MAX_CHANNEL_VALUE;
	pixel[PIXEL_BLUE] = MAX_CHANNEL_VALUE;

	QColor c;

	cs -> toQColor(reinterpret_cast<const Q_UINT8 *>(pixel), &c);

	CHECK(c.red(), 255);
	CHECK(c.green(), 255);
	CHECK(c.blue(), 255);

	pixel[PIXEL_RED] = MIN_CHANNEL_VALUE;
	pixel[PIXEL_GREEN] = MIN_CHANNEL_VALUE;
	pixel[PIXEL_BLUE] = MIN_CHANNEL_VALUE;

	cs -> toQColor(reinterpret_cast<const Q_UINT8 *>(pixel), &c);

	CHECK(c.red(), 0);
	CHECK(c.green(), 0);
	CHECK(c.blue(), 0);

	pixel[PIXEL_RED] = MAX_CHANNEL_VALUE / 4;
	pixel[PIXEL_GREEN] = MAX_CHANNEL_VALUE / 2;
	pixel[PIXEL_BLUE] = (3 * MAX_CHANNEL_VALUE) / 4;

	cs -> toQColor(reinterpret_cast<const Q_UINT8 *>(pixel), &c);

	CHECK(c.red(), (int)UINT16_TO_UINT8(MAX_CHANNEL_VALUE / 4));
	CHECK(c.green(), (int)UINT16_TO_UINT8(MAX_CHANNEL_VALUE / 2));
	CHECK(c.blue(), (int)UINT16_TO_UINT8((3 * MAX_CHANNEL_VALUE) / 4));

	pixel[PIXEL_RED] = MAX_CHANNEL_VALUE;
	pixel[PIXEL_GREEN] = MAX_CHANNEL_VALUE;
	pixel[PIXEL_BLUE] = MAX_CHANNEL_VALUE;
	pixel[PIXEL_ALPHA] = MAX_CHANNEL_VALUE;

	QUANTUM opacity;

	cs -> toQColor(reinterpret_cast<const Q_UINT8 *>(pixel), &c, &opacity);

	CHECK(c.red(), 255);
	CHECK(c.green(), 255);
	CHECK(c.blue(), 255);
	CHECK(opacity, OPACITY_OPAQUE);

	pixel[PIXEL_ALPHA] = MAX_CHANNEL_VALUE;

	cs -> toQColor(reinterpret_cast<const Q_UINT8 *>(pixel), &c, &opacity);

	CHECK(c.red(), 255);
	CHECK(c.green(), 255);
	CHECK(c.blue(), 255);
	CHECK(opacity, OPACITY_OPAQUE);

	pixel[PIXEL_RED] = MIN_CHANNEL_VALUE;
	pixel[PIXEL_GREEN] = MIN_CHANNEL_VALUE;
	pixel[PIXEL_BLUE] = MIN_CHANNEL_VALUE;
	pixel[PIXEL_ALPHA] = MIN_CHANNEL_VALUE;

	cs -> toQColor(reinterpret_cast<const Q_UINT8 *>(pixel), &c, &opacity);

	CHECK(c.red(), 0);
	CHECK(c.green(), 0);
	CHECK(c.blue(), 0);
	CHECK(opacity, OPACITY_TRANSPARENT);

	pixel[PIXEL_RED] = MAX_CHANNEL_VALUE / 4;
	pixel[PIXEL_GREEN] = MAX_CHANNEL_VALUE / 2;
	pixel[PIXEL_BLUE] = (3 * MAX_CHANNEL_VALUE) / 4;
	pixel[PIXEL_ALPHA] = MAX_CHANNEL_VALUE / 2;

	cs -> toQColor(reinterpret_cast<const Q_UINT8 *>(pixel), &c, &opacity);

	CHECK(c.red(), (int)UINT16_TO_UINT8(MAX_CHANNEL_VALUE / 4));
	CHECK(c.green(), (int)UINT16_TO_UINT8(MAX_CHANNEL_VALUE / 2));
	CHECK(c.blue(), (int)UINT16_TO_UINT8((3 * MAX_CHANNEL_VALUE) / 4));
	CHECK((int)opacity, (int)UINT16_TO_UINT8(MAX_CHANNEL_VALUE / 2));

	#define NUM_PIXELS 4

	KisStrategyColorSpaceRGBU16::Pixel pixels[NUM_PIXELS] = {
		{MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE / 4},
		{MAX_CHANNEL_VALUE / 4, MAX_CHANNEL_VALUE / 2, MAX_CHANNEL_VALUE / 3, MAX_CHANNEL_VALUE / 2},
		{MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE, MIN_CHANNEL_VALUE},
		{MIN_CHANNEL_VALUE, MIN_CHANNEL_VALUE, MIN_CHANNEL_VALUE, MAX_CHANNEL_VALUE}
	};

	cs -> setAlpha(reinterpret_cast<Q_UINT8 *>(pixels), OPACITY_OPAQUE / 2, NUM_PIXELS);

	CHECK((int)pixels[0].red, MAX_CHANNEL_VALUE);
	CHECK((int)pixels[0].green, MAX_CHANNEL_VALUE);
	CHECK((int)pixels[0].blue, MAX_CHANNEL_VALUE);
	CHECK((int)pixels[0].alpha, (int)UINT8_TO_UINT16(OPACITY_OPAQUE / 2));

	CHECK((int)pixels[1].red, MAX_CHANNEL_VALUE / 3);
	CHECK((int)pixels[1].green, MAX_CHANNEL_VALUE / 2);
	CHECK((int)pixels[1].blue, MAX_CHANNEL_VALUE / 4);
	CHECK((int)pixels[1].alpha, (int)UINT8_TO_UINT16(OPACITY_OPAQUE / 2));

	CHECK((int)pixels[2].red, MAX_CHANNEL_VALUE);
	CHECK((int)pixels[2].green, MAX_CHANNEL_VALUE);
	CHECK((int)pixels[2].blue, MAX_CHANNEL_VALUE);
	CHECK((int)pixels[2].alpha, (int)UINT8_TO_UINT16(OPACITY_OPAQUE / 2));

	CHECK((int)pixels[3].red, MIN_CHANNEL_VALUE);
	CHECK((int)pixels[3].green, MIN_CHANNEL_VALUE);
	CHECK((int)pixels[3].blue, MIN_CHANNEL_VALUE);
	CHECK((int)pixels[3].alpha, (int)UINT8_TO_UINT16(OPACITY_OPAQUE / 2));

	/*
	virtual Q_INT8 difference(const Q_UINT8 *src1, const Q_UINT8 *src2);
	virtual KisCompositeOpList userVisiblecompositeOps() const;
	virtual void adjustBrightnessContrast(const Q_UINT8 *src, Q_UINT8 *dst, Q_INT8 brightness, Q_INT8 contrast, Q_INT32 nPixels) const;
	*/
}

void KisStrategyColorSpaceRGBU16Tester::testMixColors()
{
	KisStrategyColorSpaceSP cs = new KisStrategyColorSpaceRGBU16();

	// Test mixColors.
	Q_UINT16 pixel1[4];
	Q_UINT16 pixel2[4];
	Q_UINT16 outputPixel[4];

	pixel1[PIXEL_RED] = 255;
	pixel1[PIXEL_GREEN] = 255;
	pixel1[PIXEL_BLUE] = 255;
	pixel1[PIXEL_ALPHA] = 255;

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

	cs -> mixColors(pixelPtrs, weights, 2, reinterpret_cast<Q_UINT8 *>(outputPixel));

	CHECK((int)outputPixel[PIXEL_RED], 255);
	CHECK((int)outputPixel[PIXEL_GREEN], 255);
	CHECK((int)outputPixel[PIXEL_BLUE], 255);
	CHECK((int)outputPixel[PIXEL_ALPHA], 255);

	weights[0] = 0;
	weights[1] = 255;

	cs -> mixColors(pixelPtrs, weights, 2, reinterpret_cast<Q_UINT8 *>(outputPixel));

	CHECK((int)outputPixel[PIXEL_RED], 0);
	CHECK((int)outputPixel[PIXEL_GREEN], 0);
	CHECK((int)outputPixel[PIXEL_BLUE], 0);
	CHECK((int)outputPixel[PIXEL_ALPHA], 0);

	weights[0] = 128;
	weights[1] = 127;

	cs -> mixColors(pixelPtrs, weights, 2, reinterpret_cast<Q_UINT8 *>(outputPixel));
	 
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

	cs -> mixColors(pixelPtrs, weights, 2, reinterpret_cast<Q_UINT8 *>(outputPixel));

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

	cs -> mixColors(pixelPtrs, weights, 2, reinterpret_cast<Q_UINT8 *>(outputPixel));

	CHECK((int)outputPixel[PIXEL_RED], 255);
	CHECK((int)outputPixel[PIXEL_GREEN], 255);
	CHECK((int)outputPixel[PIXEL_BLUE], 255);
	CHECK((int)outputPixel[PIXEL_ALPHA], 165);
}

#define PIXELS_WIDTH 2
#define PIXELS_HEIGHT 2

void KisStrategyColorSpaceRGBU16Tester::testToQImage()
{
	KisStrategyColorSpaceSP cs = new KisStrategyColorSpaceRGBU16();

	KisStrategyColorSpaceRGBU16::Pixel pixels[PIXELS_WIDTH * PIXELS_HEIGHT] = {
		{MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE / 4},
		{MAX_CHANNEL_VALUE / 4, MAX_CHANNEL_VALUE / 2, MAX_CHANNEL_VALUE / 3, MAX_CHANNEL_VALUE / 2},
		{MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE, MIN_CHANNEL_VALUE},
		{MIN_CHANNEL_VALUE, MIN_CHANNEL_VALUE, MIN_CHANNEL_VALUE, MAX_CHANNEL_VALUE}
	};

	QImage image = cs -> convertToQImage(reinterpret_cast<const Q_UINT8 *>(pixels), PIXELS_WIDTH, PIXELS_HEIGHT, 0, 0);

	QRgb c = image.pixel(0, 0);

	CHECK(qRed(c), (int)UINT16_TO_UINT8(MAX_CHANNEL_VALUE));
	CHECK(qGreen(c), (int)UINT16_TO_UINT8(MAX_CHANNEL_VALUE));
	CHECK(qBlue(c), (int)UINT16_TO_UINT8(MAX_CHANNEL_VALUE));
	CHECK(qAlpha(c), (int)UINT16_TO_UINT8(MAX_CHANNEL_VALUE / 4));

	c = image.pixel(1, 0);

	CHECK(qRed(c), (int)UINT16_TO_UINT8(MAX_CHANNEL_VALUE / 3));
	CHECK(qGreen(c), (int)UINT16_TO_UINT8(MAX_CHANNEL_VALUE / 2));
	CHECK(qBlue(c), (int)UINT16_TO_UINT8(MAX_CHANNEL_VALUE / 4));
	CHECK(qAlpha(c), (int)UINT16_TO_UINT8(MAX_CHANNEL_VALUE / 2));

	c = image.pixel(0, 1);

	CHECK(qRed(c), (int)UINT16_TO_UINT8(MAX_CHANNEL_VALUE));
	CHECK(qGreen(c), (int)UINT16_TO_UINT8(MAX_CHANNEL_VALUE));
	CHECK(qBlue(c), (int)UINT16_TO_UINT8(MAX_CHANNEL_VALUE));
	CHECK(qAlpha(c), (int)UINT16_TO_UINT8(MIN_CHANNEL_VALUE));

	c = image.pixel(1, 1);

	CHECK(qRed(c), (int)UINT16_TO_UINT8(MIN_CHANNEL_VALUE));
	CHECK(qGreen(c), (int)UINT16_TO_UINT8(MIN_CHANNEL_VALUE));
	CHECK(qBlue(c), (int)UINT16_TO_UINT8(MIN_CHANNEL_VALUE));
	CHECK(qAlpha(c), (int)UINT16_TO_UINT8(MAX_CHANNEL_VALUE));
}

