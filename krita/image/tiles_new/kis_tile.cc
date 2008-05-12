/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2007 Bart Coppens <kde@bartcoppens.be>
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

#include "kis_tile.h"
#include <assert.h>
#include <kis_debug.h>

#include "kis_tile_global.h"

#include "kis_tileddatamanager.h"
#include "kis_tilestore.h"
#include "kis_sharedtiledata.h"

const qint32 KisTile::WIDTH = 64;
const qint32 KisTile::HEIGHT = 64;

// TODO: Actually, m_tileData->tiles would only be needed if we'd do dirty signalling from the shared data to the tile...

#define USE_IMPLICIT_SHARING

KisTile::KisTile(KisTileStoreSP store, qint32 pixelSize, qint32 col, qint32 row, const quint8 *defPixel)
    : m_lock(QMutex::Recursive)
{
    m_pixelSize = pixelSize;
    m_nextTile = 0;
    m_col = col;
    m_row = row;

    // ### TODO: Maybe we could all share tiles with the same defPixel?

    allocate(store);

    setData(defPixel);

    m_tileData->lastUse = QTime::currentTime(); // Should this lock the shareData? ###

    // Not shared, so this should be valid unconditionally
    store->maySwapTile(m_tileData); // ### Possibly leaked! So try to keep this at the end at all times
    //trace = kBacktrace();
}

KisTile::KisTile(const KisTile& rhs, qint32 col, qint32 row)
    : m_lock(QMutex::Recursive)
{
    // Lock rhs?
    if (this != &rhs) {
        m_pixelSize = rhs.m_pixelSize;
        m_nextTile = 0;

        m_col = col;
        m_row = row;

#ifdef USE_IMPLICIT_SHARING
        rhs.m_tileData->lock.lock();

        m_tileData = rhs.m_tileData;
        m_tileData->references++;

        rhs.m_tileData->lock.unlock();
#else
        rhs.m_tileData->lock.lock();
        allocate(rhs.m_tileData->store);
        rhs.m_tileData->lock.unlock();

        rhs.addReader();
        memcpy(m_tileData->data, rhs.m_tileData->data, WIDTH * HEIGHT * m_pixelSize * sizeof(quint8));
        rhs.removeReader();

        m_tileData->lastUse = QTime::currentTime(); // Should this lock?
        m_tileData->store->maySwapTile(m_tileData); // ### Possibly leaked! So try to keep this at the end at all times
#endif
    } else {
        //m_data = 0;
        //m_nextTile = 0;
    }

    m_tileData->lastUse = QTime::currentTime(); // Should this lock? ###
    //trace = kBacktrace();
}

KisTile::KisTile(qint32 pixelSize, qint32 col, qint32 row, KisSharedTileData *data)
    : m_lock(QMutex::Recursive)
{
    m_pixelSize = pixelSize;
    m_nextTile = 0;
    m_col = col;
    m_row = row;

    QMutexLocker lock(&data->lock);
    m_tileData = data;
    m_tileData->references++;
    m_tileData->lastUse = QTime::currentTime(); // Should this lock the shareData? ###
}

KisTile::KisTile(const KisTile& rhs)
    : m_lock(QMutex::Recursive)
{
    // Lock rhs?
    if (this != &rhs) {
        m_pixelSize = rhs.m_pixelSize;
        m_col = rhs.m_col;
        m_row = rhs.m_row;
        m_nextTile = 0;

#ifdef USE_IMPLICIT_SHARING
        rhs.m_tileData->lock.lock();

        m_tileData = rhs.m_tileData;
        m_tileData->references++;

        rhs.m_tileData->lock.unlock();
#else
        rhs.m_tileData->lock.lock();
        allocate(rhs.m_tileData->store);
        rhs.m_tileData->lock.unlock();

        rhs.addReader();
        memcpy(m_tileData->data, rhs.m_tileData->data, WIDTH * HEIGHT * m_pixelSize * sizeof(quint8));
        rhs.removeReader();

        m_tileData->lastUse = QTime::currentTime(); // Lock?
        m_tileData->store->maySwapTile(m_tileData); // ### Possibly leaked! So try to keep this at the end at all times
#endif
    }
    else {
        ;
    }

    m_tileData->lastUse = QTime::currentTime(); // Lock?
    //trace = kBacktrace();
}

KisTile::~KisTile()
{
    m_tileData->lock.lock();

    if ( (m_tileData->references == 1) && (m_tileData->timesLockedInMemory == 0) && (m_tileData->deleteable) ) { // We are the last tile refering to this shared tile data (timesLockedInMemory -> tileswapper
        m_tileData->lock.unlock();

        delete m_tileData; // Deregisters and deallocates itself
        m_tileData = 0;
    } else {
        if (m_tileData->references == 1) {
            // Otherwise it was the timesLockedInMemory, which is supposed to be 0 in that case
            assert(!m_tileData->deleteable);
            assert(m_tileData->timesLockedInMemory == 0);
        }
        m_tileData->references--;

        m_tileData->lock.unlock();
    }

    //assert( !readers() ); ### Hmmm, we should do sth similar with the shared data?
}

void KisTile::allocate(KisTileStoreSP store) const
{
    //assert (!readers()); // Cannot work anymore in this way, since we now can call this while detaching

    m_tileData = new KisSharedTileData(store, pixelSize() * KisTile::WIDTH * KisTile::HEIGHT, pixelSize());
    m_tileData->references++;
}

void KisTile::setNext(KisTile *n)
{
    m_lock.lock();
    m_nextTile = n;
    m_lock.unlock();
}

void KisTile::setData(const quint8 *pixel)
{
    addReader();
    quint8 *dst = m_tileData->data;
    for(int i=0; i <WIDTH * HEIGHT;i++)
    {
        memcpy(dst, pixel, m_pixelSize);
        dst+=m_pixelSize;
    }
    removeReader();
}

void KisTile::addReader() const
{
    m_tileData->addLockInMemory(); // TODO/FIXME: see below :( (also for all other stuff about this!!!)
}

void KisTile::removeReader() const
{
    m_tileData->removeLockInMemory(); // Actually, should we not lock the tile as well, since we access it's member var tileData? :( ###
}

// ### Check the locking here! (ESPECIALLY: returning m_data while we are locked!)
// This function should be called only in case this tile was not locked into memory by the caller...
void KisTile::detachShared() const
{
    QMutexLocker lock(&m_lock);

    // This assert is preventive: the code probably is expected to work when not using shared tiles, but once a shared tile is used: it'd fail!
    //Q_ASSERT(!readers()); // This should not really assert, but it is bad code nevertheless (meaning you'd have ptr data pointing to the old AND new data!)

    QMutexLocker lockData(&(m_tileData->lock));
    if (m_tileData->references == 1) {
        // We are the only ones using this data, don't detach!
        return;
    }

    //Q_ASSERT(m_tileData->timesLockedInMemory < m_tileData->references); // If equal, we'd also have a lock!
    Q_ASSERT(m_tileData->references > 1);

    addReader(); // Make sure we are in memory, use old tileData for it

    // Don't deregister the tiledata since we already asserted that we should not yet delete it!

    // Already registers the new tile data, ### (in case the store needs to be changed, like HTTP->Mem)
    KisSharedTileData* newTileData = m_tileData->store->degradedTileDataForSharing(m_tileData);

    if (newTileData == m_tileData) {
        removeReader();
        return;
    }

    m_tileData->references--;
    newTileData->references++;
    // Since we don't yet leak the pointer to the new tiledata, we don't need to lock it...

    memcpy(newTileData->data, m_tileData->data, WIDTH * HEIGHT * m_pixelSize * sizeof(quint8));

    removeReader();

    m_tileData = newTileData;
}
