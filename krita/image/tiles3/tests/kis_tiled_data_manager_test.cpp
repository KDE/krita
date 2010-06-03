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

#define NUM_CYCLES 10000
//#define NUM_CYCLES 100000
#define NUM_TYPES 13

#define TILE_DIMENSION 64

class KisStressJob : public QRunnable
{
public:
    KisStressJob(KisTiledDataManager &dataManager, QRect rect)
        : m_accessRect(rect), dm(dataManager)
    {
    }

    void run() {
        qsrand(QTime::currentTime().msec());
        for(qint32 i = 0; i < NUM_CYCLES; i++) {
            qint32 type = qrand() % NUM_TYPES;

            quint8 *buf;
            KisTileSP tile;
            QRect newRect;
//            bool b;

//            qDebug() << ppVar(QThread::currentThreadId()) << ppVar(type);
            switch(type) {
            case 0:
                buf = new quint8[dm.pixelSize()];
                memcpy(buf, dm.defaultPixel(), dm.pixelSize());
                dm.setDefaultPixel(buf);
                delete[] buf;
                break;
            case 1:
            case 2:
                tile = dm.getTile(m_accessRect.x() / TILE_DIMENSION,
                                  m_accessRect.y() / TILE_DIMENSION, false);
                tile->lockForRead();
                tile->unlock();
                tile = dm.getTile(m_accessRect.x() / TILE_DIMENSION,
                                  m_accessRect.y() / TILE_DIMENSION, true);
                tile->lockForWrite();
                tile->unlock();

                tile = dm.getOldTile(m_accessRect.x() / TILE_DIMENSION,
                                     m_accessRect.y() / TILE_DIMENSION);
                tile->lockForRead();
                tile->unlock();
                break;
            case 3:
            case 4:
//                m_memento = dm.getMemento();
                break;
            case 5:
//                if(m_memento) {
//                    dm.rollback(m_memento);
//                    m_memento = 0;
//                }
                break;
            case 6:
//                if(m_memento) {
//                    dm.purgeHistory(m_memento);
//                    m_memento = 0;
//                }
                break;
            case 7:
//                b = dm.hasCurrentMemento();
                break;
            case 8:
                newRect = dm.extent();
                break;
            case 9:
                dm.setExtent(m_accessRect);
                break;
            case 10:
                dm.clear(m_accessRect.x(), m_accessRect.y(),
                         m_accessRect.width(), m_accessRect.height(), 1);
                break;
            case 11:
                dm.clear();
                break;
            case 12:
                buf = new quint8[m_accessRect.width() * m_accessRect.height() *
                                 dm.pixelSize()];
                dm.readBytes(buf, m_accessRect.x(), m_accessRect.y(),
                             m_accessRect.width(), m_accessRect.height());
                dm.writeBytes(buf, m_accessRect.x(), m_accessRect.y(),
                              m_accessRect.width(), m_accessRect.height());
                break;
            }
        }
    }

private:
    KisMementoSP m_memento;
    QRect m_accessRect;
    KisTiledDataManager &dm;
};

void KisTiledDataManagerTest::stressTest()
{
    quint8 defaultPixel = 0;
    KisTiledDataManager dm(1, &defaultPixel);

    QThreadPool pool;
    pool.setMaxThreadCount(NUM_TYPES);

    QRect accessRect(0,0,100,100);
    for(qint32 i = 0; i < NUM_TYPES; i++) {
        KisStressJob *job = new KisStressJob(dm, accessRect);
        pool.start(job);
        accessRect.translate(1024, 0);
    }
}

QTEST_KDEMAIN(KisTiledDataManagerTest, NoGUI)
#include "kis_tiled_data_manager_test.moc"

