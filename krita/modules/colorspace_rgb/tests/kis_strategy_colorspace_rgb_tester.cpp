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
#include "kis_strategy_colorspace_rgb_tester.h"
#include "kis_strategy_colorspace_rgb.h"

using namespace KUnitTest;

KUNITTEST_MODULE( kunittest_kis_strategy_colorspace_rgb_tester, "RGB Colorspace Tester" );
KUNITTEST_MODULE_REGISTER_TESTER( KisStrategyColorSpaceRGBTester );

void KisStrategyColorSpaceRGBTester::allTests()
{
	// We need this so that the colour profile loading can operate without crashing.
	KisFactory *factory = new KisFactory();

	testMixColors();

	delete factory;
}

void KisStrategyColorSpaceRGBTester::testMixColors()
{
	KisStrategyColorSpaceSP cs = new KisStrategyColorSpaceRGB();

	// Test mixColors.
	Q_UINT8 pixel1[4];
	Q_UINT8 pixel2[4];
	Q_UINT8 outputPixel[4];

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

	pixelPtrs[0] = pixel1;
	pixelPtrs[1] = pixel2;

	weights[0] = 255;
	weights[1] = 0;

	cs -> mixColors(pixelPtrs, weights, 2, outputPixel);

	CHECK((int)outputPixel[PIXEL_RED], 255);
	CHECK((int)outputPixel[PIXEL_GREEN], 255);
	CHECK((int)outputPixel[PIXEL_BLUE], 255);
	CHECK((int)outputPixel[PIXEL_ALPHA], 255);

	weights[0] = 0;
	weights[1] = 255;

	cs -> mixColors(pixelPtrs, weights, 2, outputPixel);

	CHECK((int)outputPixel[PIXEL_RED], 0);
	CHECK((int)outputPixel[PIXEL_GREEN], 0);
	CHECK((int)outputPixel[PIXEL_BLUE], 0);
	CHECK((int)outputPixel[PIXEL_ALPHA], 0);

	weights[0] = 128;
	weights[1] = 127;

	cs -> mixColors(pixelPtrs, weights, 2, outputPixel);
	 
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

	cs -> mixColors(pixelPtrs, weights, 2, outputPixel);

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

	cs -> mixColors(pixelPtrs, weights, 2, outputPixel);

	CHECK((int)outputPixel[PIXEL_RED], 255);
	CHECK((int)outputPixel[PIXEL_GREEN], 255);
	CHECK((int)outputPixel[PIXEL_BLUE], 255);
	CHECK((int)outputPixel[PIXEL_ALPHA], 165);
}

