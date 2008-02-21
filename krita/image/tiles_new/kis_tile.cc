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

const qint32 KisTile::WIDTH = 64;
const qint32 KisTile::HEIGHT = 64;

#define USE_IMPLICIT_SHARING

// ### WARNING: readers() != SharedTileData->timesLockedInMemory!!!!! FIXME

KisTile::KisTile(KisTileStore* store, qint32 pixelSize, qint32 col, qint32 row, const quint8 *defPixel)
    : m_lock(QMutex::Recursive)
{
    m_pixelSize = pixelSize;
    m_nextTile = 0;
    m_col = col;
    m_row = row;
    m_nReadlock = 0;
    m_store = store;

    // ### TODO: Maybe we could all share tiles with the same defPixel?

    allocate();

    setData(defPixel);

    m_tileData->lastUse = QTime::currentTime(); // Should this lock the shareData? ###

    // Not shared, so this should be valid unconditionally
    m_store->maySwapTile(m_tileData); // ### Possibly leaked! So try to keep this at the end at all times
}

KisTile::KisTile(const KisTile& rhs, qint32 col, qint32 row)
    : m_lock(QMutex::Recursive)
{
    if (this != &rhs) {
        m_pixelSize = rhs.m_pixelSize;
        m_nextTile = 0;
        m_nReadlock = 0;
        m_store = rhs.m_store;

        m_col = col;
        m_row = row;

#ifdef USE_IMPLICIT_SHARING
        rhs.m_tileData->lock.lock();

        m_tileData = rhs.m_tileData;
        m_tileData->tiles.push_back(this);

        rhs.m_tileData->lock.unlock();
#else
        allocate();

        rhs.addReader();
        memcpy(m_tileData->data, rhs.m_tileData->data, WIDTH * HEIGHT * m_pixelSize * sizeof(quint8));
        rhs.removeReader();

        m_tileData->lastUse = QTime::currentTime(); // Should this lock?
        m_store->maySwapTile(m_tileData); // ### Possibly leaked! So try to keep this at the end at all times
#endif
    } else {
        //m_data = 0;
        //m_nextTile = 0;
        m_nReadlock = 0; // ### WHY
    }

    m_tileData->lastUse = QTime::currentTime(); // Should this lock? ###
}

KisTile::KisTile(const KisTile& rhs)
    : m_lock(QMutex::Recursive)
{
    if (this != &rhs) {
        m_pixelSize = rhs.m_pixelSize;
        m_col = rhs.m_col;
        m_row = rhs.m_row;
        m_nextTile = 0;
        m_nReadlock = 0;
        m_store = rhs.m_store;

#ifdef USE_IMPLICIT_SHARING
        rhs.m_tileData->lock.lock();

        m_tileData = rhs.m_tileData;
        m_tileData->tiles.push_back(this);

        rhs.m_tileData->lock.unlock();
#else
        allocate();

        rhs.addReader();
        memcpy(m_tileData->data, rhs.m_tileData->data, WIDTH * HEIGHT * m_pixelSize * sizeof(quint8));
        rhs.removeReader();

        m_tileData->lastUse = QTime::currentTime(); // Lock?
        m_store->maySwapTile(m_tileData); // ### Possibly leaked! So try to keep this at the end at all times
#endif
    }
    else {
        m_nReadlock = 0; // ### WHY
    }

    m_tileData->lastUse = QTime::currentTime(); // Lock?
}

KisTile::~KisTile()
{
    kDebug() << "Deregistering" << this;

    m_tileData->lock.lock();

    if ( (m_tileData->tiles.size() == 1) && (m_tileData->timesLockedInMemory == 0) ) { // We are the last tile refering to this shared tile data
        Q_ASSERT(m_tileData->timesLockedInMemory == 0);

        m_tileData->lock.unlock();

        m_store->deregisterTileData(m_tileData); // goes before the deleting of m_data!
        m_tileData = 0;
    } else {
        m_tileData->tiles.remove(m_tileData->tiles.indexOf(this));

        m_tileData->lock.unlock();
    }

    assert( !readers() );
}

void KisTile::allocate() const
{
    //assert (!readers()); // Cannot work anymore in this way, since we now can call this while detaching

    m_tileData = new SharedTileData;
    m_tileData->timesLockedInMemory = 0;
    m_tileData->data = 0;
    m_tileData->tiles.push_back(this);
    m_tileData->tileSize = pixelSize() * KisTile::WIDTH * KisTile::HEIGHT;

    m_tileData->data = m_store->requestTileData(m_pixelSize);
    Q_CHECK_PTR(m_tileData->data);

    m_tileData->storeData = m_store->registerTileData(m_tileData);
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
    m_lock.lock();
    if (m_nReadlock++ == 0) {
        m_tileData->lock.lock();
        m_tileData->timesLockedInMemory++;

        m_store->ensureTileLoaded(m_tileData); // Needs to come after the locking of the tile in memory!

        m_tileData->lock.unlock(); // ### Here, or before?
    } else if (m_nReadlock < 0) {
        kDebug(41000) << m_nReadlock;
        assert(0);
    }
    assert(m_tileData->data);
    m_lock.unlock();
}

void KisTile::removeReader() const
{
    m_lock.lock();
    assert(m_nReadlock >= 0);
    if (--m_nReadlock == 0) {
        m_tileData->lock.lock();

        m_tileData->timesLockedInMemory--;
        m_tileData->lastUse = QTime::currentTime(); // Lock?

        if (m_tileData->timesLockedInMemory == 0) {
            m_store->maySwapTile(m_tileData);
        }

        m_tileData->lock.unlock();
    }
    m_lock.unlock();
}

// ### Check the locking here! (ESPECIALLY: returning m_data while we are locked!)
void KisTile::detachShared() const
{
    QMutexLocker lock(&m_lock);

    // This assert is preventive: the code probably is expected to work when not using shared tiles, but once a shared tile is used: it'd fail!
    //Q_ASSERT(!readers()); // This should not really assert, but it is bad code nevertheless (meaning you'd have ptr data pointing to the old AND new data!)

    QMutexLocker lockData(&(m_tileData->lock));
    if (m_tileData->tiles.size() == 1) {
        // We are the only ones using this data, don't detach!
        return;
    }

    Q_ASSERT(m_tileData->tiles.size() > 1);

    m_tileData->tiles.remove(m_tileData->tiles.indexOf(this));

    addReader(); // Make sure we are in memory, use old tileData for it
    
    SharedTileData* oldTileData = m_tileData; // Keep around to copy the data from...

    // Don't deregister the tiledata since we already asserted that we should not yet delete it!
    m_tileData = 0;

    m_store->degradeTileForSharing(const_cast<KisTile*>(this)); // ### IEW (in case the store needs to be changed, like HTTP->Mem)

    allocate(); // Will allocate a new m_tileData, etc. (we ourselves are locked) (already registers)

    memcpy(m_tileData->data, oldTileData->data, WIDTH * HEIGHT * m_pixelSize * sizeof(quint8));

    // HACK (iew!)
    SharedTileData* newTileData = m_tileData;
    m_tileData = oldTileData;
    removeReader();
    m_tileData = newTileData;
}

KisTile::SharedTileData::~SharedTileData() {
    delete storeData;
    storeData = 0;
    data = 0;
}
