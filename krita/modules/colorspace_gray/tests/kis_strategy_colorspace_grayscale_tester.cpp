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
#include "kis_strategy_colorspace_grayscale_tester.h"
#include "kis_strategy_colorspace_grayscale.h"

using namespace KUnitTest;

KUNITTEST_MODULE( kunittest_kis_strategy_colorspace_grayscale_tester, "Greyscale Colorspace Tester" );
KUNITTEST_MODULE_REGISTER_TESTER( KisStrategyColorSpaceGrayscaleTester );

void KisStrategyColorSpaceGrayscaleTester::allTests()
{
	// We need this so that the colour profile loading can operate without crashing.
	KisFactory *factory = new KisFactory();

	testMixColors();

	delete factory;
}

#define MAX_CHANNEL_GRAYSCALEA 2

void KisStrategyColorSpaceGrayscaleTester::testMixColors()
{
	KisStrategyColorSpaceSP cs = new KisStrategyColorSpaceGrayscale();

	Q_UINT8 pixel1[MAX_CHANNEL_GRAYSCALEA];
	Q_UINT8 pixel2[MAX_CHANNEL_GRAYSCALEA];
	Q_UINT8 outputPixel[MAX_CHANNEL_GRAYSCALEA];

	pixel1[KisStrategyColorSpaceGrayscale::PIXEL_GRAY] = 255;
	pixel1[KisStrategyColorSpaceGrayscale::PIXEL_GRAY_ALPHA] = 255;

	pixel2[KisStrategyColorSpaceGrayscale::PIXEL_GRAY] = 0;
	pixel2[KisStrategyColorSpaceGrayscale::PIXEL_GRAY_ALPHA] = 0;

	const Q_UINT8 *pixelPtrs[2];
	Q_UINT8 weights[2];

	pixelPtrs[0] = pixel1;
	pixelPtrs[1] = pixel2;

	weights[0] = 255;
	weights[1] = 0;

	cs -> mixColors(pixelPtrs, weights, 2, outputPixel);

	CHECK((int)outputPixel[KisStrategyColorSpaceGrayscale::PIXEL_GRAY], 255);
	CHECK((int)outputPixel[KisStrategyColorSpaceGrayscale::PIXEL_GRAY_ALPHA], 255);

	weights[0] = 0;
	weights[1] = 255;

	cs -> mixColors(pixelPtrs, weights, 2, outputPixel);

	CHECK((int)outputPixel[KisStrategyColorSpaceGrayscale::PIXEL_GRAY], 0);
	CHECK((int)outputPixel[KisStrategyColorSpaceGrayscale::PIXEL_GRAY_ALPHA], 0);

	weights[0] = 128;
	weights[1] = 127;

	cs -> mixColors(pixelPtrs, weights, 2, outputPixel);
	 
	CHECK((int)outputPixel[KisStrategyColorSpaceGrayscale::PIXEL_GRAY], 255);
	CHECK((int)outputPixel[KisStrategyColorSpaceGrayscale::PIXEL_GRAY_ALPHA], 128);

	pixel1[KisStrategyColorSpaceGrayscale::PIXEL_GRAY] = 200;
	pixel1[KisStrategyColorSpaceGrayscale::PIXEL_GRAY_ALPHA] = 255;

	pixel2[KisStrategyColorSpaceGrayscale::PIXEL_GRAY] = 100;
	pixel2[KisStrategyColorSpaceGrayscale::PIXEL_GRAY_ALPHA] = 255;

	cs -> mixColors(pixelPtrs, weights, 2, outputPixel);

	CHECK((int)outputPixel[KisStrategyColorSpaceGrayscale::PIXEL_GRAY], 150);
	CHECK((int)outputPixel[KisStrategyColorSpaceGrayscale::PIXEL_GRAY_ALPHA], 255);

	pixel1[KisStrategyColorSpaceGrayscale::PIXEL_GRAY] = 0;
	pixel1[KisStrategyColorSpaceGrayscale::PIXEL_GRAY_ALPHA] = 0;

	pixel2[KisStrategyColorSpaceGrayscale::PIXEL_GRAY] = 255;
	pixel2[KisStrategyColorSpaceGrayscale::PIXEL_GRAY_ALPHA] = 254;

	weights[0] = 89;
	weights[1] = 166;

	cs -> mixColors(pixelPtrs, weights, 2, outputPixel);

	CHECK((int)outputPixel[KisStrategyColorSpaceGrayscale::PIXEL_GRAY], 255);
	CHECK((int)outputPixel[KisStrategyColorSpaceGrayscale::PIXEL_GRAY_ALPHA], 165);
}

