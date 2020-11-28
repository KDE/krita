/*
 *  SPDX-FileCopyrightText: 2009 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_TILE_DATA_POOLER_H_
#define KIS_TILE_DATA_POOLER_H_

#include <QObject>
#include <QThread>
#include <QSemaphore>

#include "kritaimage_export.h"

class KisTileDataStore;
class KisTileData;


class KRITAIMAGE_EXPORT KisTileDataPooler : public QThread
{
    Q_OBJECT

public:

    KisTileDataPooler(KisTileDataStore *store, qint32 memoryLimit = -1);
    ~KisTileDataPooler() override;

    void kick();
    void terminatePooler();

    void testingRereadConfig();

    qint64 lastPoolMemoryMetric() const;
    qint64 lastRealMemoryMetric() const;
    qint64 lastHistoricalMemoryMetric() const;


    /**
     * Is case the pooler thread is not running, the user might force
     * recalculation of the memory statistics explicitly.
     */
    void forceUpdateMemoryStats();

protected:
    static const qint32 MAX_NUM_CLONES;
    static const qint32 MAX_TIMEOUT;
    static const qint32 MIN_TIMEOUT;
    static const qint32 TIMEOUT_FACTOR;

    void waitForWork();
    qint32 numClonesNeeded(KisTileData *td) const;
    void cloneTileData(KisTileData *td, qint32 numClones) const;
    void run() override;

    inline int clonesMetric(KisTileData *td, int numClones);
    inline int clonesMetric(KisTileData *td);

    inline void tryFreeOrphanedClones(KisTileData *td);
    inline qint32 needMemory(KisTileData *td);
    inline qint32 canDonorMemory(KisTileData *td);
    qint32 tryGetMemory(QList<KisTileData*> &donors, qint32 memoryMetric);

    template<class Iter>
        void getLists(Iter *iter, QList<KisTileData*> &beggers,
                      QList<KisTileData*> &donors,
                      qint32 &memoryOccupied,
                      qint32 &statRealMemory,
                      qint32 &statHistoricalMemory);

    bool processLists(QList<KisTileData*> &beggers,
                      QList<KisTileData*> &donors,
                      qint32 &memoryOccupied);

private:
    void debugTileStatistics();
protected:
    QSemaphore m_semaphore;
    QAtomicInt m_shouldExitFlag;
    KisTileDataStore *m_store;
    qint32 m_timeout;
    bool m_lastCycleHadWork;
    qint32 m_memoryLimit;
    qint32 m_lastPoolMemoryMetric;
    qint32 m_lastRealMemoryMetric;
    qint32 m_lastHistoricalMemoryMetric;
};



#endif /* KIS_TILE_DATA_POOLER_H_ */

