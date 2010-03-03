/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#include "kis_iterators_pixel_test.h"

#include <qtest_kde.h>
#include "kis_iterators_pixel.h"

#include <kis_paint_device.h>
#include <KoColorSpaceRegistry.h>
#include <kis_iterator_ng.h>

void KisIteratorsPixelTest::testHLine(int width, int height)
{
    KisPaintDevice dev(KoColorSpaceRegistry::instance()->rgb8());

    KisHLineIteratorSP it = dev.createHLineIteratorNG(0, 0, width);
    quint8 data = 0;
    for (int y = 0; y < height; ++y) {
        do {
            for (int i = 0; i < 4; ++i) {
                it->rawData()[i] = ++data;
            }
        } while (it->nextPixel());
        it->nextRow();
    }
    KisHLineIteratorPixel it2 = dev.createHLineIterator(0, 0, width);
    data = 0;
    for (int y = 0; y < height; ++y) {
        while (!it2.isDone()) {
            for (int i = 0; i < 4; ++i) {
                quint8 d = ++data;
                QCOMPARE(it2.rawData()[i], d);
            }
            ++it2;
        }
        it2.nextRow();
    }
}

void KisIteratorsPixelTest::testHLineAlignedOnTile()
{
    testHLine(64, 64);
    testHLine(64, 128);
    testHLine(128, 64);
    testHLine(128, 128);
}

void KisIteratorsPixelTest::testHLineUnalignedOnTile()
{
    testHLine(200, 200);
    testHLine(20, 20);
    testHLine(20, 200);
    testHLine(200, 20);
}

void KisIteratorsPixelTest::testVLine(int width, int height)
{
    KisPaintDevice dev(KoColorSpaceRegistry::instance()->rgb8());

    KisVLineIteratorSP it = dev.createVLineIteratorNG(0, 0, height);
    quint8 data = 0;
    for (int y = 0; y < width; ++y) {
        do {
            for (int i = 0; i < 4; ++i) {
                it->rawData()[i] = ++data;
            }
        } while (it->nextPixel());
        it->nextColumn();
    }
    KisVLineIteratorPixel it2 = dev.createVLineIterator(0, 0, height);
    data = 0;
    for (int y = 0; y < width; ++y) {
        while (!it2.isDone()) {
            for (int i = 0; i < 4; ++i) {
                quint8 d = ++data;
                QCOMPARE(it2.rawData()[i], d);
            }
            ++it2;
        }
        it2.nextCol();
    }
}

void KisIteratorsPixelTest::testVLineAlignedOnTile()
{
    testVLine(64, 64);
    testVLine(64, 128);
    testVLine(128, 64);
    testVLine(128, 128);
}

void KisIteratorsPixelTest::testVLineUnalignedOnTile()
{
    testVLine(200, 200);
    testVLine(20, 20);
    testVLine(20, 200);
    testVLine(200, 20);
}


void KisIteratorsPixelTest::testRectAlignedOnTile()
{
    testRect(64, 64);
    testRect(64, 128);
    testRect(128, 64);
    testRect(128, 128);
}

void KisIteratorsPixelTest::testRectUnalignedOnTile()
{
    testRect(200, 200);
    testRect(20, 20);
    testRect(20, 200);
    testRect(200, 20);
}

void KisIteratorsPixelTest::testRect(int width, int height)
{
    KisPaintDevice dev(KoColorSpaceRegistry::instance()->rgb8());

    KisRectIteratorSP it = dev.createRectIteratorNG(0, 0, width, height);
    quint8 data = 0;
    do {
        for (int i = 0; i < 4; ++i) {
            it->rawData()[i] = ++data;
        }
    } while (it->nextPixel());

    KisRectIteratorPixel it2 = dev.createRectIterator(0, 0, width, height);
    data = 0;
    while (!it2.isDone()) {
        for (int i = 0; i < 4; ++i) {
            quint8 d = ++data;
            QCOMPARE(it2.rawData()[i], d);
        }
        ++it2;
    }
}

QTEST_KDEMAIN(KisIteratorsPixelTest, GUI)
#include "kis_iterators_pixel_test.moc"
