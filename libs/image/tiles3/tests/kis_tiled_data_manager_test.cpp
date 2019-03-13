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
#include <QTest>

#include "tiles3/kis_tiled_data_manager.h"

#include "tiles_test_utils.h"
#include "config-limit-long-tests.h"

bool KisTiledDataManagerTest::checkHole(quint8* buffer,
                                        quint8 holeColor, QRect holeRect,
                                        quint8 backgroundColor, QRect backgroundRect)
{
    for(qint32 y = backgroundRect.y(); y <= backgroundRect.bottom(); y++) {
        for(qint32 x = backgroundRect.x(); x <= backgroundRect.right(); x++) {
            quint8 expectedColor = holeRect.contains(x,y) ? holeColor : backgroundColor;

            if(*buffer != expectedColor) {
                qDebug() << "Expected" << expectedColor << "but found" << *buffer;
                return false;
            }

            buffer++;
        }
    }
    return true;
}

bool KisTiledDataManagerTest::checkTilesShared(KisTiledDataManager *srcDM,
                                               KisTiledDataManager *dstDM,
                                               bool takeOldSrc,
                                               bool takeOldDst,
                                               QRect tilesRect)
{
    for(qint32 row = tilesRect.y(); row <= tilesRect.bottom(); row++) {
        for(qint32 col = tilesRect.x(); col <= tilesRect.right(); col++) {
            KisTileSP srcTile = takeOldSrc ? srcDM->getOldTile(col, row)
                : srcDM->getTile(col, row, false);
            KisTileSP dstTile = takeOldDst ? dstDM->getOldTile(col, row)
                : dstDM->getTile(col, row, false);

            if(srcTile->tileData() != dstTile->tileData()) {
                qDebug() << "Expected tile data (" << col << row << ")"
                         << srcTile->extent()
                         << srcTile->tileData()
                         << "but found" << dstTile->tileData();
                qDebug() << "Expected" << srcTile->data()[0] << "but found" << dstTile->data()[0];
                return false;
            }
        }
    }
    return true;
}

bool KisTiledDataManagerTest::checkTilesNotShared(KisTiledDataManager *srcDM,
                                                  KisTiledDataManager *dstDM,
                                                  bool takeOldSrc,
                                                  bool takeOldDst,
                                                  QRect tilesRect)
{
    for(qint32 row = tilesRect.y(); row <= tilesRect.bottom(); row++) {
        for(qint32 col = tilesRect.x(); col <= tilesRect.right(); col++) {
            KisTileSP srcTile = takeOldSrc ? srcDM->getOldTile(col, row)
                : srcDM->getTile(col, row, false);
            KisTileSP dstTile = takeOldDst ? dstDM->getOldTile(col, row)
                : dstDM->getTile(col, row, false);

            if(srcTile->tileData() == dstTile->tileData()) {
                qDebug() << "Expected tiles not be shared:"<< srcTile->extent();
                return false;
            }
        }
    }
    return true;
}

void KisTiledDataManagerTest::testUndoingNewTiles()
{
    // "growing extent bug"

    const QRect nullRect(qint32_MAX,qint32_MAX,0,0);

    quint8 defaultPixel = 0;
    KisTiledDataManager srcDM(1, &defaultPixel);

    KisTileSP emptyTile = srcDM.getTile(0, 0, false);

    QCOMPARE(srcDM.extent(), nullRect);

    KisMementoSP memento0 = srcDM.getMemento();
    KisTileSP createdTile = srcDM.getTile(0, 0, true);
    srcDM.commit();

    QCOMPARE(srcDM.extent(), QRect(0,0,64,64));

    srcDM.rollback(memento0);
    QCOMPARE(srcDM.extent(), nullRect);
}

void KisTiledDataManagerTest::testPurgedAndEmptyTransactions()
{
    quint8 defaultPixel = 0;
    KisTiledDataManager srcDM(1, &defaultPixel);

    quint8 oddPixel1 = 128;

    QRect rect(0,0,512,512);
    QRect clearRect1(50,50,100,100);
    QRect clearRect2(150,50,100,100);

    quint8 *buffer = new quint8[rect.width()*rect.height()];

    // purged transaction

    KisMementoSP memento0 = srcDM.getMemento();
    srcDM.clear(clearRect1, &oddPixel1);
    srcDM.purgeHistory(memento0);
    memento0 = 0;

    srcDM.readBytes(buffer, rect.x(), rect.y(), rect.width(), rect.height());
    QVERIFY(checkHole(buffer, oddPixel1, clearRect1,
                      defaultPixel, rect));

    // one more purged transaction

    KisMementoSP memento1 = srcDM.getMemento();
    srcDM.clear(clearRect2, &oddPixel1);

    srcDM.readBytes(buffer, rect.x(), rect.y(), rect.width(), rect.height());
    QVERIFY(checkHole(buffer, oddPixel1, clearRect1 | clearRect2,
                      defaultPixel, rect));

    srcDM.purgeHistory(memento1);
    memento1 = 0;

    srcDM.readBytes(buffer, rect.x(), rect.y(), rect.width(), rect.height());
    QVERIFY(checkHole(buffer, oddPixel1, clearRect1 | clearRect2,
                      defaultPixel, rect));

    // empty one

    KisMementoSP memento2 = srcDM.getMemento();
    srcDM.commit();
    srcDM.rollback(memento2);

    srcDM.readBytes(buffer, rect.x(), rect.y(), rect.width(), rect.height());
    QVERIFY(checkHole(buffer, oddPixel1, clearRect1 | clearRect2,
                      defaultPixel, rect));


    // now check that everything works still

    KisMementoSP memento3 = srcDM.getMemento();
    srcDM.setExtent(clearRect2);
    srcDM.commit();

    srcDM.readBytes(buffer, rect.x(), rect.y(), rect.width(), rect.height());
    QVERIFY(checkHole(buffer, oddPixel1, clearRect2,
                      defaultPixel, rect));

    srcDM.rollback(memento3);

    srcDM.readBytes(buffer, rect.x(), rect.y(), rect.width(), rect.height());
    QVERIFY(checkHole(buffer, oddPixel1, clearRect1 | clearRect2,
                      defaultPixel, rect));


}
void KisTiledDataManagerTest::testUnversionedBitBlt()
{
    quint8 defaultPixel = 0;
    KisTiledDataManager srcDM(1, &defaultPixel);
    KisTiledDataManager dstDM(1, &defaultPixel);

    quint8 oddPixel1 = 128;
    quint8 oddPixel2 = 129;

    QRect rect(0,0,512,512);
    QRect cloneRect(81,80,250,250);
    QRect tilesRect(2,2,3,3);

    srcDM.clear(rect, &oddPixel1);
    dstDM.clear(rect, &oddPixel2);

    dstDM.bitBlt(&srcDM, cloneRect);

    quint8 *buffer = new quint8[rect.width()*rect.height()];

    dstDM.readBytes(buffer, rect.x(), rect.y(), rect.width(), rect.height());

    QVERIFY(checkHole(buffer, oddPixel1, cloneRect,
                      oddPixel2, rect));

    delete[] buffer;

    // Test whether tiles became shared
    QVERIFY(checkTilesShared(&srcDM, &dstDM, false, false, tilesRect));
}

void KisTiledDataManagerTest::testVersionedBitBlt()
{
    quint8 defaultPixel = 0;
    KisTiledDataManager srcDM1(1, &defaultPixel);
    KisTiledDataManager srcDM2(1, &defaultPixel);
    KisTiledDataManager dstDM(1, &defaultPixel);

    quint8 oddPixel1 = 128;
    quint8 oddPixel2 = 129;
    quint8 oddPixel3 = 130;

    quint8 oddPixel4 = 131;

    QRect rect(0,0,512,512);
    QRect cloneRect(81,80,250,250);
    QRect tilesRect(2,2,3,3);


    KisMementoSP memento1 = srcDM1.getMemento();
    srcDM1.clear(rect, &oddPixel1);

    srcDM2.clear(rect, &oddPixel2);
    dstDM.clear(rect, &oddPixel3);

    KisMementoSP memento2 = dstDM.getMemento();
    dstDM.bitBlt(&srcDM1, cloneRect);

    QVERIFY(checkTilesShared(&srcDM1, &dstDM, false, false, tilesRect));
    QVERIFY(checkTilesNotShared(&srcDM1, &srcDM1, true, false, tilesRect));
    QVERIFY(checkTilesNotShared(&dstDM, &dstDM, true, false, tilesRect));

    dstDM.commit();
    QVERIFY(checkTilesShared(&dstDM, &dstDM, true, false, tilesRect));

    KisMementoSP memento3 = srcDM2.getMemento();
    srcDM2.clear(rect, &oddPixel4);

    KisMementoSP memento4 = dstDM.getMemento();
    dstDM.bitBlt(&srcDM2, cloneRect);

    QVERIFY(checkTilesShared(&srcDM2, &dstDM, false, false, tilesRect));
    QVERIFY(checkTilesNotShared(&srcDM2, &srcDM2, true, false, tilesRect));
    QVERIFY(checkTilesNotShared(&dstDM, &dstDM, true, false, tilesRect));

    dstDM.commit();
    QVERIFY(checkTilesShared(&dstDM, &dstDM, true, false, tilesRect));

    dstDM.rollback(memento4);
    QVERIFY(checkTilesShared(&srcDM1, &dstDM, false, false, tilesRect));
    QVERIFY(checkTilesShared(&dstDM, &dstDM, true, false, tilesRect));
    QVERIFY(checkTilesNotShared(&srcDM1, &srcDM1, true, false, tilesRect));

    dstDM.rollforward(memento4);
    QVERIFY(checkTilesShared(&srcDM2, &dstDM, false, false, tilesRect));
    QVERIFY(checkTilesShared(&dstDM, &dstDM, true, false, tilesRect));
    QVERIFY(checkTilesNotShared(&srcDM1, &srcDM1, true, false, tilesRect));
}

void KisTiledDataManagerTest::testBitBltOldData()
{
    quint8 defaultPixel = 0;
    KisTiledDataManager srcDM(1, &defaultPixel);
    KisTiledDataManager dstDM(1, &defaultPixel);

    quint8 oddPixel1 = 128;
    quint8 oddPixel2 = 129;

    QRect rect(0,0,512,512);
    QRect cloneRect(81,80,250,250);
    QRect tilesRect(2,2,3,3);

    quint8 *buffer = new quint8[rect.width()*rect.height()];

    KisMementoSP memento1 = srcDM.getMemento();
    srcDM.clear(rect, &oddPixel1);
    srcDM.commit();

    dstDM.bitBltOldData(&srcDM, cloneRect);
    dstDM.readBytes(buffer, rect.x(), rect.y(), rect.width(), rect.height());
    QVERIFY(checkHole(buffer, oddPixel1, cloneRect,
                      defaultPixel, rect));

    KisMementoSP memento2 = srcDM.getMemento();
    srcDM.clear(rect, &oddPixel2);
    dstDM.bitBltOldData(&srcDM, cloneRect);
    srcDM.commit();

    dstDM.readBytes(buffer, rect.x(), rect.y(), rect.width(), rect.height());
    QVERIFY(checkHole(buffer, oddPixel1, cloneRect,
                      defaultPixel, rect));

    delete[] buffer;
}

void KisTiledDataManagerTest::testBitBltRough()
{
    quint8 defaultPixel = 0;
    KisTiledDataManager srcDM(1, &defaultPixel);
    KisTiledDataManager dstDM(1, &defaultPixel);

    quint8 oddPixel1 = 128;
    quint8 oddPixel2 = 129;
    quint8 oddPixel3 = 130;

    QRect rect(0,0,512,512);
    QRect cloneRect(81,80,250,250);
    QRect actualCloneRect(64,64,320,320);
    QRect tilesRect(1,1,4,4);

    srcDM.clear(rect, &oddPixel1);
    dstDM.clear(rect, &oddPixel2);

    dstDM.bitBltRough(&srcDM, cloneRect);

    quint8 *buffer = new quint8[rect.width()*rect.height()];

    dstDM.readBytes(buffer, rect.x(), rect.y(), rect.width(), rect.height());

    QVERIFY(checkHole(buffer, oddPixel1, actualCloneRect,
                      oddPixel2, rect));

    // Test whether tiles became shared
    QVERIFY(checkTilesShared(&srcDM, &dstDM, false, false, tilesRect));

    // check bitBltRoughOldData
    KisMementoSP memento1 = srcDM.getMemento();
    srcDM.clear(rect, &oddPixel3);
    dstDM.bitBltRoughOldData(&srcDM, cloneRect);
    srcDM.commit();
    dstDM.readBytes(buffer, rect.x(), rect.y(), rect.width(), rect.height());
    QVERIFY(checkHole(buffer, oddPixel1, actualCloneRect,
                      oddPixel2, rect));

    delete[] buffer;
}

void KisTiledDataManagerTest::testTransactions()
{
    quint8 defaultPixel = 0;
    KisTiledDataManager dm(1, &defaultPixel);

    quint8 oddPixel1 = 128;
    quint8 oddPixel2 = 129;
    quint8 oddPixel3 = 130;

    KisTileSP tile00;
    KisTileSP oldTile00;

    // Create a named transaction: versioning is enabled
    KisMementoSP memento1 = dm.getMemento();
    dm.clear(0, 0, 64, 64, &oddPixel1);

    tile00 = dm.getTile(0, 0, false);
    oldTile00 = dm.getOldTile(0, 0);
    QVERIFY(memoryIsFilled(oddPixel1, tile00->data(), TILESIZE));
    QVERIFY(memoryIsFilled(defaultPixel, oldTile00->data(), TILESIZE));
    tile00 = oldTile00 = 0;

    // Create an anonymous transaction: versioning is disabled
    dm.commit();
    tile00 = dm.getTile(0, 0, false);
    oldTile00 = dm.getOldTile(0, 0);
    QVERIFY(memoryIsFilled(oddPixel1, tile00->data(), TILESIZE));
    QVERIFY(memoryIsFilled(oddPixel1, oldTile00->data(), TILESIZE));
    tile00 = oldTile00 = 0;

    dm.clear(0, 0, 64, 64, &oddPixel2);

    // Versioning is disabled, i said! >:)
    tile00 = dm.getTile(0, 0, false);
    oldTile00 = dm.getOldTile(0, 0);
    QVERIFY(memoryIsFilled(oddPixel2, tile00->data(), TILESIZE));
    QVERIFY(memoryIsFilled(oddPixel2, oldTile00->data(), TILESIZE));
    tile00 = oldTile00 = 0;

    // And the last round: named transaction:
    KisMementoSP memento2 = dm.getMemento();
    dm.clear(0, 0, 64, 64, &oddPixel3);

    tile00 = dm.getTile(0, 0, false);
    oldTile00 = dm.getOldTile(0, 0);
    QVERIFY(memoryIsFilled(oddPixel3, tile00->data(), TILESIZE));
    QVERIFY(memoryIsFilled(oddPixel2, oldTile00->data(), TILESIZE));
    tile00 = oldTile00 = 0;

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
    dm.commit();

    KisMementoSP memento2 = dm.getMemento();
    dm.clear(0, 0, 64, 64, &oddPixel2);

    KisTileSP tile00;
    KisTileSP oldTile00;

    tile00 = dm.getTile(0, 0, false);
    oldTile00 = dm.getOldTile(0, 0);
    QVERIFY(memoryIsFilled(oddPixel2, tile00->data(), TILESIZE));
    QVERIFY(memoryIsFilled(oddPixel1, oldTile00->data(), TILESIZE));
    tile00 = oldTile00 = 0;

    dm.purgeHistory(memento1);

    /**
     * Nothing nas changed in the visible state of the data manager
     */

    tile00 = dm.getTile(0, 0, false);
    oldTile00 = dm.getOldTile(0, 0);
    QVERIFY(memoryIsFilled(oddPixel2, tile00->data(), TILESIZE));
    QVERIFY(memoryIsFilled(oddPixel1, oldTile00->data(), TILESIZE));
    tile00 = oldTile00 = 0;

    dm.commit();

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
    tile00 = oldTile00 = 0;

    /**
     * Just test we won't crash when the memento is not
     * present in history anymore
     */

    KisMementoSP memento3 = dm.getMemento();
    dm.clear(0, 0, 64, 64, &oddPixel3);
    dm.commit();

    KisMementoSP memento4 = dm.getMemento();
    dm.clear(0, 0, 64, 64, &oddPixel4);
    dm.commit();

    dm.rollback(memento4);

    dm.purgeHistory(memento3);
    dm.purgeHistory(memento4);
}

void KisTiledDataManagerTest::testUndoSetDefaultPixel()
{
    quint8 defaultPixel = 0;
    KisTiledDataManager dm(1, &defaultPixel);

    quint8 oddPixel1 = 128;
    quint8 oddPixel2 = 129;

    QRect fillRect(0,0,64,64);

    KisTileSP tile00;
    KisTileSP tile10;

    tile00 = dm.getTile(0, 0, false);
    tile10 = dm.getTile(1, 0, false);
    QVERIFY(memoryIsFilled(defaultPixel, tile00->data(), TILESIZE));
    QVERIFY(memoryIsFilled(defaultPixel, tile10->data(), TILESIZE));

    KisMementoSP memento1 = dm.getMemento();
    dm.clear(fillRect, &oddPixel1);
    dm.commit();

    tile00 = dm.getTile(0, 0, false);
    tile10 = dm.getTile(1, 0, false);
    QVERIFY(memoryIsFilled(oddPixel1, tile00->data(), TILESIZE));
    QVERIFY(memoryIsFilled(defaultPixel, tile10->data(), TILESIZE));

    KisMementoSP memento2 = dm.getMemento();
    dm.setDefaultPixel(&oddPixel2);
    dm.commit();

    tile00 = dm.getTile(0, 0, false);
    tile10 = dm.getTile(1, 0, false);
    QVERIFY(memoryIsFilled(oddPixel1, tile00->data(), TILESIZE));
    QVERIFY(memoryIsFilled(oddPixel2, tile10->data(), TILESIZE));

    dm.rollback(memento2);

    tile00 = dm.getTile(0, 0, false);
    tile10 = dm.getTile(1, 0, false);
    QVERIFY(memoryIsFilled(oddPixel1, tile00->data(), TILESIZE));
    QVERIFY(memoryIsFilled(defaultPixel, tile10->data(), TILESIZE));

    dm.rollback(memento1);

    tile00 = dm.getTile(0, 0, false);
    tile10 = dm.getTile(1, 0, false);
    QVERIFY(memoryIsFilled(defaultPixel, tile00->data(), TILESIZE));
    QVERIFY(memoryIsFilled(defaultPixel, tile10->data(), TILESIZE));

    dm.rollforward(memento1);

    tile00 = dm.getTile(0, 0, false);
    tile10 = dm.getTile(1, 0, false);
    QVERIFY(memoryIsFilled(oddPixel1, tile00->data(), TILESIZE));
    QVERIFY(memoryIsFilled(defaultPixel, tile10->data(), TILESIZE));

    dm.rollforward(memento2);

    tile00 = dm.getTile(0, 0, false);
    tile10 = dm.getTile(1, 0, false);
    QVERIFY(memoryIsFilled(oddPixel1, tile00->data(), TILESIZE));
    QVERIFY(memoryIsFilled(oddPixel2, tile10->data(), TILESIZE));
}

//#include <valgrind/callgrind.h>

void KisTiledDataManagerTest::benchmarkReadOnlyTileLazy()
{
    quint8 defaultPixel = 0;
    KisTiledDataManager dm(1, &defaultPixel);

    /*
     * See KisTileHashTableTraits2 for more details
     */

    const qint32 numTilesToTest = 0x7fff;

    //CALLGRIND_START_INSTRUMENTATION;

    QBENCHMARK_ONCE {
        for(qint32 i = 0; i < numTilesToTest; i++) {
            KisTileSP tile = dm.getTile(i, i, false);
        }
    }

    //CALLGRIND_STOP_INSTRUMENTATION;
}

class KisSimpleClass : public KisShared
{
    qint64 m_int;
public:
    KisSimpleClass() {
        Q_UNUSED(m_int);
    }
};

typedef KisSharedPtr<KisSimpleClass> KisSimpleClassSP;
void KisTiledDataManagerTest::benchmarkSharedPointers()
{
    const qint32 numIterations = 2 * 1000000;

    //CALLGRIND_START_INSTRUMENTATION;

    QBENCHMARK_ONCE {
        for(qint32 i = 0; i < numIterations; i++) {
            KisSimpleClassSP pointer = new KisSimpleClass;
            pointer = 0;
        }
    }

    //CALLGRIND_STOP_INSTRUMENTATION;
}

void KisTiledDataManagerTest::benchmarkCOWImpl()
{
    const int pixelSize = 8;
    quint8 defaultPixel[pixelSize];
    memset(defaultPixel, 1, pixelSize);

    KisTiledDataManager dm(pixelSize, defaultPixel);


    KisMementoSP memento1 = dm.getMemento();

    /**
     * Imagine a regular image of 4096x2048 pixels
     * (64x32 tiles)
     */
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 64; j++) {
            KisTileSP tile = dm.getTile(j, i, true);
            tile->lockForWrite();
            tile->unlock();
        }
    }

    dm.commit();

    QTest::qSleep(200);

    KisMementoSP memento2 = dm.getMemento();
    QTest::qSleep(200);
    QBENCHMARK_ONCE {

        for (int i = 0; i < 32; i++) {
            for (int j = 0; j < 64; j++) {
                KisTileSP tile = dm.getTile(j, i, true);
                tile->lockForWrite();
                tile->unlock();
            }
        }

    }
    dm.commit();
}

void KisTiledDataManagerTest::benchmarkCOWNoPooler()
{
    KisTileDataStore::instance()->testingSuspendPooler();
    QTest::qSleep(200);

    benchmarkCOWImpl();

    KisTileDataStore::instance()->testingResumePooler();
    QTest::qSleep(200);
}

void KisTiledDataManagerTest::benchmarkCOWWithPooler()
{
    benchmarkCOWImpl();
}

/******************* Stress job ***********************/

#ifdef LIMIT_LONG_TESTS
#define NUM_CYCLES 10000
#else
#define NUM_CYCLES 100000
#endif

#define NUM_TYPES 12

#define TILE_DIMENSION 64


/**
 * The data manager has partial guarantees of reentrancy. That is
 * you can call any arbitrary number of methods concurrently as long
 * as their access areas do not intersect.
 *
 * Though the rule can be quite tricky -- some of the methods always
 * use entire image as their access area, so they cannot be called
 * concurrently in any circumstances.
 * The examples are: clear(), commit(), rollback() and etc...
 */

#define run_exclusive(lock, _i) for(_i = 0, (lock).lockForWrite(); _i < 1; _i++, (lock).unlock())
#define run_concurrent(lock, _i) for(_i = 0, (lock).lockForRead(); _i < 1; _i++, (lock).unlock())
//#define run_exclusive(lock, _i) while(0)
//#define run_concurrent(lock, _i) while(0)


class KisStressJob : public QRunnable
{
public:
    KisStressJob(KisTiledDataManager &dataManager, QRect rect, QReadWriteLock &_lock)
        : m_accessRect(rect), dm(dataManager), lock(_lock)
    {
    }

    void run() override {
        qsrand(QTime::currentTime().msec());
        for(qint32 i = 0; i < NUM_CYCLES; i++) {
            qint32 type = qrand() % NUM_TYPES;

            qint32 t;

            switch(type) {
            case 0:
                run_concurrent(lock,t) {
                    quint8 *buf;
                    buf = new quint8[dm.pixelSize()];
                    memcpy(buf, dm.defaultPixel(), dm.pixelSize());
                    dm.setDefaultPixel(buf);
                    delete[] buf;
                }
                break;
            case 1:
            case 2:
                run_concurrent(lock,t) {
                    KisTileSP tile;

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
                }
                break;
            case 3:
                run_concurrent(lock,t) {
                    QRect newRect = dm.extent();
		    Q_UNUSED(newRect);
                }
                break;
            case 4:
                run_concurrent(lock,t) {
                    dm.clear(m_accessRect.x(), m_accessRect.y(),
                             m_accessRect.width(), m_accessRect.height(), 4);
                }
                break;
            case 5:
                run_concurrent(lock,t) {
                    quint8 *buf;

                    buf = new quint8[m_accessRect.width() * m_accessRect.height() *
                                     dm.pixelSize()];
                    dm.readBytes(buf, m_accessRect.x(), m_accessRect.y(),
                                 m_accessRect.width(), m_accessRect.height());
                    dm.writeBytes(buf, m_accessRect.x(), m_accessRect.y(),
                                  m_accessRect.width(), m_accessRect.height());
                    delete[] buf;
                    }
                break;
            case 6:
                run_concurrent(lock,t) {
                    quint8 oddPixel = 13;
                    KisTiledDataManager srcDM(1, &oddPixel);
                    dm.bitBlt(&srcDM, m_accessRect);
                }
                break;
            case 7:
            case 8:
                run_exclusive(lock,t) {
                    m_memento = dm.getMemento();
                    dm.clear(m_accessRect.x(), m_accessRect.y(),
                             m_accessRect.width(), m_accessRect.height(), 2);
                    dm.commit();

                    dm.rollback(m_memento);
                    dm.rollforward(m_memento);

                    dm.purgeHistory(m_memento);
                    m_memento = 0;
                }
                break;
            case 9:
                run_exclusive(lock,t) {
                    bool b = dm.hasCurrentMemento();
                    Q_UNUSED(b);
                }
                break;
            case 10:
                run_exclusive(lock,t) {
                    dm.clear();
                }
                break;
            case 11:
                run_exclusive(lock,t) {
                    dm.setExtent(m_accessRect);
                }
                break;
            }
        }
    }

private:
    KisMementoSP m_memento;
    QRect m_accessRect;
    KisTiledDataManager &dm;
    QReadWriteLock &lock;
};

void KisTiledDataManagerTest::stressTest()
{
    quint8 defaultPixel = 0;
    KisTiledDataManager dm(1, &defaultPixel);
    QReadWriteLock lock;

#ifdef LIMIT_LONG_TESTS
    const int numThreads = 8;
    const int numWorkers = 8;
#else
    const int numThreads = 16;
    const int numWorkers = 48;
#endif

    QThreadPool pool;
    pool.setMaxThreadCount(numThreads);

    QRect accessRect(0,0,512,512);
    for(qint32 i = 0; i < numWorkers; i++) {
        KisStressJob *job = new KisStressJob(dm, accessRect, lock);
        pool.start(job);
        accessRect.translate(512, 0);
    }
    pool.waitForDone();
}

template <typename Func>
void applyToRect(const QRect &rc, Func func) {
    for (int y = rc.y(); y < rc.y() + rc.height(); y += KisTileData::HEIGHT) {
        for (int x = rc.x(); x < rc.x() + rc.width(); x += KisTileData::WIDTH) {
            const int col = x / KisTileData::WIDTH;
            const int row = y / KisTileData::HEIGHT;

            func(col, row);
        }
    }
}


class LazyCopyingStressJob : public QRunnable
{
public:
    LazyCopyingStressJob(KisTiledDataManager &dataManager,
                         const QRect &rect,
                         QReadWriteLock &dmExclusiveLock,
                         QReadWriteLock &tileExclusiveLock,
                         int numCycles,
                         bool isWriter)
        : m_accessRect(rect),
          dm(dataManager),
          m_dmExclusiveLock(dmExclusiveLock),
          m_tileExclusiveLock(tileExclusiveLock),
          m_numCycles(numCycles),
          m_isWriter(isWriter)
    {
    }

    void run() override {
        for(qint32 i = 0; i < m_numCycles; i++) {

            //const int epoch = i % 100;
            int t;

            if (m_isWriter && 0) {

            } else {
                const bool shouldClear = i % 5 <= 1; // 40% of requests are clears
                const bool shouldWrite = i % 5 <= 3; // other 40% of requests are writes

                run_concurrent(m_dmExclusiveLock, t) {
                    if (shouldClear) {
                        QWriteLocker locker(&m_tileExclusiveLock);
                        dm.clear(m_accessRect, 4);
                    } else {
                        auto readFunc = [this] (int col, int row) {
                            KisTileSP tile = dm.getTile(col, row, false);
                            tile->lockForRead();
                            tile->unlock();
                        };

                        auto writeFunc = [this] (int col, int row) {
                            KisTileSP tile = dm.getTile(col, row, true);
                            tile->lockForWrite();
                            tile->unlock();
                        };

                        auto readOldFunc = [this] (int col, int row) {
                            KisTileSP tile = dm.getOldTile(col, row);
                            tile->lockForRead();
                            tile->unlock();
                        };

                        applyToRect(m_accessRect, readFunc);
                        if (shouldWrite) {
                            QReadLocker locker(&m_tileExclusiveLock);
                            applyToRect(m_accessRect, writeFunc);
                        }
                        applyToRect(m_accessRect, readOldFunc);
                    }
                }
            }
        }
    }

private:
    KisMementoSP m_memento;
    QRect m_accessRect;
    KisTiledDataManager &dm;
    QReadWriteLock &m_dmExclusiveLock;
    QReadWriteLock &m_tileExclusiveLock;
    const int m_numCycles;
    const bool m_isWriter;
};

void KisTiledDataManagerTest::stressTestLazyCopying()
{
    quint8 defaultPixel = 0;
    KisTiledDataManager dm(1, &defaultPixel);
    QReadWriteLock dmLock;
    QReadWriteLock tileLock;

#ifdef LIMIT_LONG_TESTS
    const int numCycles = 10000;
    const int numThreads = 8;
    const int numWorkers = 8;
#else
    const int numThreads = 16;
    const int numWorkers = 32;
    const int numCycles = 100000;
#endif

    QThreadPool pool;
    pool.setMaxThreadCount(numThreads);

    const QRect accessRect(0,0,512,256);
    for(qint32 i = 0; i < numWorkers; i++) {
        const bool isWriter = i == 0;
        LazyCopyingStressJob *job = new LazyCopyingStressJob(dm, accessRect,
                                                             dmLock, tileLock,
                                                             numCycles, isWriter);
        pool.start(job);
    }
    pool.waitForDone();
}

QTEST_MAIN(KisTiledDataManagerTest)

