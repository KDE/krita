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

#include "kis_tiled_data_tester.h"
#include "kis_datamanager.h"
#include "kis_global.h"

using namespace KUnitTest;

KUNITTEST_MODULE( kunittest_kis_tiled_data_tester, "Tiled Data Tester" );
KUNITTEST_MODULE_REGISTER_TESTER( KisTiledDataTester );

#define TEST_PIXEL_SIZE 4

static quint8 defaultPixel[TEST_PIXEL_SIZE] = {0, 0, 0, OPACITY_TRANSPARENT};

void KisTiledDataTester::allTests()
{
    KisDataManager *dm = new KisDataManager(TEST_PIXEL_SIZE, defaultPixel);

    qint32 extentX;
    qint32 extentY;
    qint32 extentWidth;
    qint32 extentHeight;

    dm->extent(extentX, extentY, extentWidth, extentHeight);
    CHECK(extentWidth, 0);
    CHECK(extentHeight, 0);

    const quint8 *readOnlyPixel = dm->pixel(KisTile::WIDTH/2, KisTile::HEIGHT/2);
    dm->extent(extentX, extentY, extentWidth, extentHeight);
    CHECK(extentWidth, 0);
    CHECK(extentHeight, 0);

    quint8 *writablePixel = dm->writablePixel(KisTile::WIDTH/2, KisTile::HEIGHT/2);
    dm->extent(extentX, extentY, extentWidth, extentHeight);
    CHECK(extentX, 0);
    CHECK(extentY, 0);
    CHECK(extentWidth, KisTile::WIDTH);
    CHECK(extentHeight, KisTile::HEIGHT);

    writablePixel = dm->writablePixel(-KisTile::WIDTH, -KisTile::HEIGHT);
    dm->extent(extentX, extentY, extentWidth, extentHeight);
    CHECK(extentX, -KisTile::WIDTH);
    CHECK(extentY, -KisTile::HEIGHT);
    CHECK(extentWidth, 2*KisTile::WIDTH);
    CHECK(extentHeight, 2*KisTile::HEIGHT);

    dm->clear();
    dm->extent(extentX, extentY, extentWidth, extentHeight);
    CHECK(extentWidth, 0);
    CHECK(extentHeight, 0);

    delete dm;
}

