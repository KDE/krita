/*
 *  Copyright (c) 2007 Boudewijn Rempt boud@valdyas.org
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

#include "kis_datamanager_test.h"

#include <qtest_kde.h>
#include "kis_datamanager.h"

void KisDataManagerTest::testCreation()
{
    quint8 c = 0;
    KisDataManager test(1, &c);
}


void KisDataManagerTest::testDefaultPixel()
{
    {
        quint8 c = 0;
        KisDataManager dm(1, &c);
        QVERIFY(dm.pixelSize() == 1);
        QVERIFY(*dm.defaultPixel() == 0);
    }
    {
        quint8 * p = new quint8[3];
        memset(p, 0, 3);

        // The default pixel is copied, we still own the pointer
        KisDataManager dm(3, p);
        QVERIFY(dm.pixelSize() == 3);

        // A pointer, not a copy is returned. Changing it changes the
        // default pixel in the data manager.
        const quint8 * defaultPixelC = dm.defaultPixel();
        QVERIFY(defaultPixelC[0] == 0);
        QVERIFY(defaultPixelC[1] == 0);
        QVERIFY(defaultPixelC[2] == 0);

        // unconst it, so we can change it.
        quint8 * defaultPixel = const_cast<quint8*>(defaultPixelC);
        defaultPixel[0] = 50;
        defaultPixel[1] = 150;
        defaultPixel[2] = 200;

        // Check that our original pixel isn't changed
        QVERIFY(p[0] == 0);
        QVERIFY(p[1] == 0);
        QVERIFY(p[2] == 0);

        // Reset the default pixel
        dm.setDefaultPixel(p);
        defaultPixelC = dm.defaultPixel();
        QVERIFY(defaultPixelC[0] == 0);
        QVERIFY(defaultPixelC[1] == 0);
        QVERIFY(defaultPixelC[2] == 0);

        delete[]p;

    }
}

bool KisDataManagerTest::memoryIsFilled(quint8 c, quint8 *mem, qint32 size)
{
    for(; size > 0; size--)
        if(*(mem++) != c)
            return false;

    return true;
}

#define TILESIZE 64*64

void KisDataManagerTest::testMemento()
{
    quint8 defaultPixel = 0;
    KisDataManager dm(1, &defaultPixel);

    quint8 oddPixel = 128;

    dm.clear(0, 0, 64, 64, &oddPixel);
    KisMementoSP memento1 = dm.getMemento();

    dm.clear(64, 0, 64, 64, &oddPixel);


    KisTileSP tile00 = dm.getTile(0, 0, false);
    KisTileSP tile10 = dm.getTile(1, 0, false);
    KisTileSP oldTile00 = dm.getOldTile(0, 0);
    KisTileSP oldTile10 = dm.getOldTile(1, 0);

    qDebug()<<ppVar(tile10)<<ppVar(oldTile10);

    QVERIFY(memoryIsFilled(oddPixel, tile00->data(), TILESIZE));
    QVERIFY(memoryIsFilled(oddPixel, tile10->data(), TILESIZE));
    QVERIFY(memoryIsFilled(oddPixel, oldTile00->data(), TILESIZE));
    QVERIFY(memoryIsFilled(defaultPixel, oldTile10->data(), TILESIZE));


    KisMementoSP memento2 = dm.getMemento();

    tile00 = dm.getTile(0, 0, false);
    tile10 = dm.getTile(1, 0, false);
    oldTile00 = dm.getOldTile(0, 0);
    oldTile10 = dm.getOldTile(1, 0);

    QVERIFY(memoryIsFilled(oddPixel, tile00->data(), TILESIZE));
    QVERIFY(memoryIsFilled(oddPixel, tile10->data(), TILESIZE));
    QVERIFY(memoryIsFilled(oddPixel, oldTile00->data(), TILESIZE));
    QVERIFY(memoryIsFilled(oddPixel, oldTile10->data(), TILESIZE));

}

//void KisDataManagerTest::testReadWrite() {}
//
//void KisDataManagerTest::testExtent() {}
//
//void KisDataManagerTest::testClear() {}
//
//void KisDataManagerTest::testSetPixel() {}
//
//void KisDataManagerTest::testReadBytes() {}
//
//void KisDataManagerTest::testWriteBytes() {}
//
//void KisDataManagerTest::testPlanarBytes() {}
//
//void KisDataManagerTest::testContiguousColumns() {}
//
//void KisDataManagerTest::testRowStride() {}
//
//void KisDataManagerTest::testThreadedReadAccess() {}
//
//void KisDataManagerTest::testThreadedWriteAccess() {}
//
//void KisDataManagerTest::testThreadedReadWriteAccess() {}

QTEST_KDEMAIN(KisDataManagerTest, GUI)
#include "kis_datamanager_test.moc"
