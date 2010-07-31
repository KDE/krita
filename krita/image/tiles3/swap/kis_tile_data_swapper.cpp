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

#include <QSemaphore>

#include "tiles3/swap/kis_tile_data_swapper.h"
#include "tiles3/swap/kis_tile_data_swapper_p.h"
#include "tiles3/kis_tile_data.h"
#include "tiles3/kis_tile_data_store.h"
#include "tiles3/kis_tile_data_store_iterators.h"
#include "kis_debug.h"

#define SEC 1000

const qint32 KisTileDataSwapper::TIMEOUT = -1;
const qint32 KisTileDataSwapper::DELAY = 0.7 * SEC;

//#define DEBUG_SWAPPER

#ifdef DEBUG_SWAPPER
#define DEBUG_ACTION(action) qDebug() << action
#define DEBUG_VALUE(value) qDebug() << "\t" << ppVar(value)
#else
#define DEBUG_ACTION(action)
#define DEBUG_VALUE(value)
#endif


class KisTileDataSwapper::Private
{
public:
    QSemaphore semaphore;
    QAtomicInt shouldExitFlag;
    KisTileDataStore *store;
    KisStoreLimits limits;
};

KisTileDataSwapper::KisTileDataSwapper(KisTileDataStore *store)
    : QThread(),
      m_d(new Private())
{
    m_d->shouldExitFlag = 0;
    m_d->store = store;
}

KisTileDataSwapper::~KisTileDataSwapper()
{
    delete m_d;
}

void KisTileDataSwapper::kick()
{
    m_d->semaphore.release();
}

void KisTileDataSwapper::terminateSwapper()
{
    m_d->shouldExitFlag = 1;
    kick();
    wait();
}

void KisTileDataSwapper::waitForWork()
{
    m_d->semaphore.tryAcquire(1, TIMEOUT);
}

void KisTileDataSwapper::run()
{
    while (1) {
        waitForWork();

        if (m_d->shouldExitFlag)
            return;

        QThread::msleep(DELAY);

        doJob();
    }
}

void KisTileDataSwapper::checkFreeMemory()
{
    if(m_d->store->numTilesInMemory() > m_d->limits.emergencyThreshold())
        doJob();
}

void KisTileDataSwapper::doJob()
{
    qint32 tilesInMemory = m_d->store->numTilesInMemory();

    DEBUG_ACTION("Started swap cycle");
    DEBUG_VALUE( tilesInMemory);

    DEBUG_VALUE(m_d->limits.softLimitThreshold());
    DEBUG_VALUE(m_d->limits.hardLimitThreshold());


    if(tilesInMemory > m_d->limits.softLimitThreshold()) {
        qint32 softFreeTiles =  tilesInMemory - m_d->limits.softLimit();
        DEBUG_VALUE(softFreeTiles);
        DEBUG_ACTION("\t pass0");
        tilesInMemory -= pass0(softFreeTiles);
        // by the end of pass0 the estimation of limits is updated...
        DEBUG_VALUE( tilesInMemory);


        if(tilesInMemory > m_d->limits.hardLimitThreshold()) {
            qint32 hardFreeTiles =  tilesInMemory - m_d->limits.hardLimit();
            tilesInMemory -= pass1(hardFreeTiles);
        }
    }
}

qint32 KisTileDataSwapper::pass0(qint32 tilesToFree)
{
    qint32 numCountedTiles = 0;
    quint64 pixelSizeSum = 0;

    qint32 tilesFreed = 0;
    QList<KisTileData*> additionalCandidates;


    KisTileDataStoreIterator *iter = m_d->store->beginIteration();
    KisTileData *item;

    while(iter->hasNext()) {
        item = iter->next();

        if(tilesFreed >= tilesToFree) break;

        numCountedTiles++;
        pixelSizeSum += item->pixelSize();

        // Now we are working with mementoed tiles only...
        if(!item->mementoed()) continue;
        if(item->numUsers() > 1) continue;

        if(item->age() > 0) {
            if(m_d->store->trySwapTileData(item)) {
                tilesFreed++;
            }
        }
        else {
            item->markOld();
            additionalCandidates.append(item);
        }

    }

    foreach(item, additionalCandidates) {
        if(tilesFreed >= tilesToFree) break;

        if(m_d->store->trySwapTileData(item)) {
            tilesFreed++;
        }
    }

    m_d->store->endIteration(iter);

    // Correction of limits...
    if(numCountedTiles > 0) {
        qreal pixelSize = qreal(pixelSizeSum) / numCountedTiles;
        m_d->limits.recalculateLimits(pixelSize);
    }

    return tilesFreed;
}

qint32 KisTileDataSwapper::pass1(qint32 tilesToFree)
{
    Q_UNUSED(tilesToFree);

    qint32 tilesFreed = 0;
    return tilesFreed;
}
