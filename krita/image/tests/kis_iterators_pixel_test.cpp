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

void KisIteratorsPixelTest::testCreation()
{
    KisPaintDevice dev(KoColorSpaceRegistry::instance()->rgb8());

    KisHLineIteratorNG* it = dev.createHLineIterator2(0, 0, 200);
    quint8 data = 0;
    for(int y = 0; y < 200; ++y)
    {
        do {
            for(int i = 0; i < 4; ++i)
            {
                it->rawData()[i] = ++data;
            }
        } while(it->nextPixel());
        it->nextRow();
    }
    delete it;
    it = 0;
    KisHLineIteratorPixel it2 = dev.createHLineIterator(0,0,200);
    data = 0;
    for(int y = 0; y < 200; ++y)
    {
        while(!it2.isDone())
        {
            for(int i = 0; i < 4; ++i)
            {
                quint8 d = ++data;
                QCOMPARE(it2.rawData()[i], d);
            }
            ++it2;
        }
        it2.nextRow();
    }
}

QTEST_KDEMAIN(KisIteratorsPixelTest, GUI)
#include "kis_iterators_pixel_test.moc"
