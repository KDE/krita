/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_tiled_data_manager_test.h"
#include <qtest_kde.h>

#include "tiles3/kis_tiled_data_manager.h"

bool KisTiledDataManagerTest::memoryIsFilled(quint8 c, quint8 *mem, qint32 size)
{
    for(; size > 0; size--)
        if(*(mem++) != c) {
            qDebug() << "Expected" << c << "but found" << *(mem-1);
            return false;
        }

    return true;
}

#define TILESIZE 64*64

void KisTiledDataManagerTest::testMemento()
{
    quint8 defaultPixel = 0;
    KisTiledDataManager dm(1, &defaultPixel);

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

void KisTiledDataManagerTest::testPurgeHistory()
{
    quint8 defaultPixel = 0;
    KisTiledDataManager dm(1, &defaultPixel);

    quint8 oddPixel1 = 128;
    quint8 oddPixel2 = 129;
    quint8 oddPixel3 = 130;
    quint8 oddPixel4 = 131;

    KisMementoSP memento1 = dm.getMemento();
    dm.clear(0, 0, 64, 64, &oddPixel1);

    KisMementoSP memento2 = dm.getMemento();
    dm.clear(0, 0, 64, 64, &oddPixel2);


    KisTileSP tile00;
    KisTileSP oldTile00;

    tile00 = dm.getTile(0, 0, false);
    oldTile00 = dm.getOldTile(0, 0);
    QVERIFY(memoryIsFilled(oddPixel2, tile00->data(), TILESIZE));
    QVERIFY(memoryIsFilled(oddPixel1, oldTile00->data(), TILESIZE));

    dm.purgeHistory(memento1);

    /**
     * Nothing nas changed in the visible state of the data manager
     */

    tile00 = dm.getTile(0, 0, false);
    oldTile00 = dm.getOldTile(0, 0);
    QVERIFY(memoryIsFilled(oddPixel2, tile00->data(), TILESIZE));
    QVERIFY(memoryIsFilled(oddPixel1, oldTile00->data(), TILESIZE));

    dm.purgeHistory(memento2);

    /**
     * We've removed all the history of the device, so it
     * became "unversioned".
     * NOTE: the return value for getOldTile() when there is no
     * history present is a subject for change
     */

    tile00 = dm.getTile(0, 0, false);
    oldTile00 = dm.getOldTile(0, 0);
    QVERIFY(memoryIsFilled(oddPixel2, tile00->data(), TILESIZE));
    QVERIFY(memoryIsFilled(oddPixel2, oldTile00->data(), TILESIZE));

    /**
     * Just test we won't crash when the memento is not
     * present in history anymore
     */

    KisMementoSP memento3 = dm.getMemento();
    dm.clear(0, 0, 64, 64, &oddPixel3);

    KisMementoSP memento4 = dm.getMemento();
    dm.clear(0, 0, 64, 64, &oddPixel4);

    dm.rollback(memento4);

    dm.purgeHistory(memento3);
    dm.purgeHistory(memento4);
}

QTEST_KDEMAIN(KisTiledDataManagerTest, NoGUI)
#include "kis_tiled_data_manager_test.moc"

