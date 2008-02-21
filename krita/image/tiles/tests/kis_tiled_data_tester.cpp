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

#include "kis_tiled_data_tester.h"
#include <qtest_kde.h>


#include "kis_datamanager.h"
#include "kis_global.h"

#define TEST_PIXEL_SIZE 4

static quint8 defaultPixel[TEST_PIXEL_SIZE] = {0, 0, 0, 0};

void KisTiledDataTester::allTests()
{
    KisDataManager *dm = new KisDataManager(TEST_PIXEL_SIZE, defaultPixel);

    qint32 extentX;
    qint32 extentY;
    qint32 extentWidth;
    qint32 extentHeight;

    dm->extent(extentX, extentY, extentWidth, extentHeight);
    QCOMPARE(extentWidth, 0);
    QCOMPARE(extentHeight, 0);

    dm->clear();
    dm->extent(extentX, extentY, extentWidth, extentHeight);
    QCOMPARE(extentWidth, 0);
    QCOMPARE(extentHeight, 0);

    delete dm;
}
QTEST_KDEMAIN(KisTiledDataTester, NoGUI)
#include "kis_tiled_data_tester.moc"

