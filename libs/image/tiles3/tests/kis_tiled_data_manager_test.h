/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_TILED_DATA_MANAGER_TEST_H
#define KIS_TILED_DATA_MANAGER_TEST_H

#include <simpletest.h>

class KisTiledDataManager;

class KisTiledDataManagerTest : public QObject
{
    Q_OBJECT

private:
    bool checkHole(quint8* buffer, quint8 holeColor, QRect holeRect,
                   quint8 backgroundColor, QRect backgroundRect);

    bool checkTilesShared(KisTiledDataManager *srcDM,
                          KisTiledDataManager *dstDM,
                          bool takeOldSrc, bool takeOldDst,
                          QRect tilesRect);

    bool checkTilesNotShared(KisTiledDataManager *srcDM,
                             KisTiledDataManager *dstDM,
                             bool takeOldSrc, bool takeOldDst,
                             QRect tilesRect);

    void benchmarkCOWImpl();

private Q_SLOTS:
    void testUndoingNewTiles();
    void testPurgedAndEmptyTransactions();
    void testUnversionedBitBlt();
    void testVersionedBitBlt();
    void testBitBltOldData();
    void testBitBltRough();
    void testTransactions();
    void testPurgeHistory();
    void testUndoSetDefaultPixel();

    void benchmarkReadOnlyTileLazy();
    void benchmarkSharedPointers();

    void benchmarkCOWNoPooler();
    void benchmarkCOWWithPooler();

    void stressTest();

    void stressTestLazyCopying();

    void stressTestExtentsColumn();

    void benchmaskQRegion();
    void benchmaskKisRegion();
    void benchmaskOverlappedKisRegion();
};

#endif /* KIS_TILED_DATA_MANAGER_TEST_H */

