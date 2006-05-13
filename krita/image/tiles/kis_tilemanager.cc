/*
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
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

#include <kdebug.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include <QMutex>
#include <QThread>

#include <kstaticdeleter.h>
#include <kglobal.h>
#include <kconfig.h>

#include "kis_tileddatamanager.h"
#include "kis_tile.h"
#include "kis_tilemanager.h"

// Note: the cache file doesn't get deleted when we crash and so :(

KisTileManager* KisTileManager::m_singleton = 0;

static KStaticDeleter<KisTileManager> staticDeleter;

KisTileManager::KisTileManager() {

    Q_ASSERT(KisTileManager::m_singleton == 0);
    KisTileManager::m_singleton = this;
    m_fileSize = 0;
    m_bytesInMem = 0;
    m_bytesTotal = 0;
    m_swapForbidden = false;

    // Hardcoded (at the moment only?): 4 pools of 1000 tiles each
    m_tilesPerPool = 1000;

    m_pools = new quint8*[4];
    m_poolPixelSizes = new qint32[4];
    m_poolFreeList = new PoolFreeList[4];
    for (int i = 0; i < 4; i++) {
        m_pools[i] = 0;
        m_poolPixelSizes[i] = 0;
        m_poolFreeList[i] = PoolFreeList();
    }
    m_currentInMem = 0;

    KConfig * cfg = KGlobal::config();
    cfg->setGroup("");
    m_maxInMem = cfg->readEntry("maxtilesinmem",  500);
    m_swappiness = cfg->readEntry("swappiness", 100);

    m_tileSize = KisTile::WIDTH * KisTile::HEIGHT;
    m_freeLists.reserve(8);

    counter = 0;

    m_poolMutex = new QMutex(QMutex::Recursive);
    m_swapMutex = new QMutex(QMutex::Recursive);
}

KisTileManager::~KisTileManager() {
    if (!m_freeLists.empty()) { // See if there are any nonempty freelists
        FreeListList::iterator listsIt = m_freeLists.begin();
        FreeListList::iterator listsEnd = m_freeLists.end();

        while(listsIt != listsEnd) {
            if ( ! (*listsIt).empty() ) {
                FreeList::iterator it = (*listsIt).begin();
                FreeList::iterator end = (*listsIt).end();

                while (it != end) {
                    // munmap it
                    munmap((*it)->pointer, (*it)->size);
                    delete *it;
                    ++it;
                }
                (*listsIt).clear();
            }
            ++listsIt;
        }
        m_freeLists.clear();
    }

    m_tempFile.close();
    m_tempFile.unlink();

    delete [] m_poolPixelSizes;
    delete [] m_pools;

    m_poolMutex->unlock();
    delete m_poolMutex;

    m_swapMutex->unlock();
    delete m_swapMutex;

}

KisTileManager* KisTileManager::instance()
{
    if(KisTileManager::m_singleton == 0) {
        staticDeleter.setObject(KisTileManager::m_singleton, new KisTileManager());
        Q_CHECK_PTR(KisTileManager::m_singleton);
    }
    return KisTileManager::m_singleton;
}

void KisTileManager::registerTile(KisTile* tile)
{

    m_swapMutex->lock();

    TileInfo* info = new TileInfo();
    info->tile = tile;
    info->inMem = true;
    info->filePos = -1;
    info->size = tile->WIDTH * tile->HEIGHT * tile->m_pixelSize;
    info->fsize = 0; // the size in the file
    info->validNode = true;

    m_tileMap[tile] = info;
    m_swappableList.push_back(info);
    info->node = -- m_swappableList.end();

    m_currentInMem++;
    m_bytesTotal += info->size;
    m_bytesInMem += info->size;

    doSwapping();

    if (++counter % 50 == 0)
        printInfo();

    m_swapMutex->unlock();
}

void KisTileManager::deregisterTile(KisTile* tile) {

    m_swapMutex->lock();

    if (!m_tileMap.contains(tile)) return;
    // Q_ASSERT(m_tileMap.contains(tile));

    TileInfo* info = m_tileMap[tile];

    if (info->filePos >= 0) { // It is mmapped
        // To freelist
        FreeInfo* freeInfo = new FreeInfo();
        freeInfo->pointer = tile->m_data;
        freeInfo->filePos = info->filePos;
        freeInfo->size = info->fsize;
        int pixelSize = (info->size / m_tileSize);
        if (m_freeLists.capacity() <= pixelSize)
            m_freeLists.resize(pixelSize + 1);
        m_freeLists[pixelSize].push_back(freeInfo);

        madvise(info->tile->m_data, info->fsize, MADV_DONTNEED);

        // the KisTile will attempt to delete its data. This is of course silly when
        // it was mmapped. So change the m_data to NULL, which is safe to delete
        tile->m_data = 0;
    } else {
        m_bytesInMem -= info->size;
        m_currentInMem--;
    }

    if (info->validNode) {
        m_swappableList.erase(info->node);
        info->validNode = false;
    }

    m_bytesTotal -= info->size;

    delete info;
    m_tileMap.remove(tile);

    doSwapping();

    m_swapMutex->unlock();
}

void KisTileManager::ensureTileLoaded(KisTile* tile)
{

    m_swapMutex->lock();

    TileInfo* info = m_tileMap[tile];
    if (info->validNode) {
        m_swappableList.erase(info->node);
        info->validNode = false;
    }

    if (!info->inMem) {
        fromSwap(info);
    }

    m_swapMutex->unlock();
}

void KisTileManager::maySwapTile(KisTile* tile)
{

    m_swapMutex->lock();

    TileInfo* info = m_tileMap[tile];
    m_swappableList.push_back(info);
    info->validNode = true;
    info->node = -- m_swappableList.end();

    doSwapping();

    m_swapMutex->unlock();
}

void KisTileManager::fromSwap(TileInfo* info)
{
    m_swapMutex->lock();

    //Q_ASSERT(!info->inMem);
    if (info->inMem) return;

    doSwapping();

    // ### check return value!
    madvise(info->tile->m_data, info->size, MADV_WILLNEED);

    info->inMem = true;
    m_currentInMem++;
    m_bytesInMem += info->size;

    m_swapMutex->unlock();
}

void KisTileManager::toSwap(TileInfo* info) {

    m_swapMutex->lock();

    //Q_ASSERT(info->inMem);
    if (!info || !info->inMem) return;

    if (info->filePos < 0) {
        // This tile is not yet in the file. Save it there
        // ### check return values of mmap functions!
        KisTile *tile = info->tile;
        quint8* data = 0;
        int pixelSize = (info->size / m_tileSize);
        if (m_freeLists.capacity() > pixelSize) {
            if (!m_freeLists[pixelSize].empty()) {
                // found one
                FreeList::iterator it = m_freeLists[pixelSize].begin();
                data = (*it)->pointer;
                info->filePos = (*it)->filePos;
                info->fsize = (*it)->size;

                delete *it;
                m_freeLists[pixelSize].erase(it);
            }
        }

        if (data == 0) { // No position found or free, create a new
            long pagesize = sysconf(_SC_PAGESIZE);
            int newsize = m_fileSize + info->size;
            newsize = newsize + newsize % pagesize;

            if (ftruncate(m_tempFile.handle(), newsize)) {
                // XXX make these maybe i18n()able and in an error box, but then through
                // some kind of proxy such that we don't pollute this with GUI code
                kWarning(DBG_AREA_TILES) << "Resizing the temporary swapfile failed!" << endl;
                // Be somewhat pollite and try to figure out why it failed
                switch (errno) {
                    case EIO: kWarning(DBG_AREA_TILES) << "Error was E IO, "
                            << "possible reason is a disk error!" << endl; break;
                    case EINVAL: kWarning(DBG_AREA_TILES) << "Error was E INVAL, "
                            << "possible reason is that you are using more memory than "
                            << "the filesystem or disk can handle" << endl; break;
                    default: kWarning(DBG_AREA_TILES) << "Errno was: " << errno << endl;
                }
                kWarning(DBG_AREA_TILES) << "The swapfile is: " << m_tempFile.name() << endl;
                kWarning(DBG_AREA_TILES) << "Will try to avoid using the swap any further" << endl;

                kDebug(DBG_AREA_TILES) << "Failed ftruncate info: "
                        << "tried mapping " << info->size << " bytes"
                        << "to a " << m_fileSize << " bytes file" << endl;
                printInfo();

                m_swapForbidden = true;
                m_swapMutex->unlock();
                return;
            }

            data = (quint8*) mmap(0, info->size,
                                   PROT_READ | PROT_WRITE,
                                   MAP_SHARED,
                                   m_tempFile.handle(), m_fileSize);

            // Same here for warning and GUI
            if (data == (quint8*)-1) {
                kWarning(DBG_AREA_TILES) << "mmap failed: errno is " << errno << "; we're probably going to crash very soon now...\n";

                // Try to ignore what happened and carry on, but unlikely that we'll get
                // much further, since the file resizing went OK and this is memory-related...
                if (errno == ENOMEM) {
                    kWarning(DBG_AREA_TILES) << "mmap failed with E NOMEM! This means that "
                            << "either there are no more memory mappings available for Krita, "
                            << "or that there is no more memory available!" << endl;
                }

                kWarning(DBG_AREA_TILES) << "Trying to continue anyway (no guarantees)" << endl;
                kWarning(DBG_AREA_TILES) << "Will try to avoid using the swap any further" << endl;
                kDebug(DBG_AREA_TILES) << "Failed mmap info: "
                        << "tried mapping " << info->size << " bytes"
                        << "to a " << m_fileSize << " bytes file" << endl;
                printInfo();

                m_swapForbidden = true;
                m_swapMutex->unlock();
                return;
            }

            info->fsize = info->size;
            info->filePos = m_fileSize;
            m_fileSize = newsize;
        }

        madvise(data, info->fsize, MADV_WILLNEED);
        memcpy(data, tile->m_data, info->size);
        madvise(data, info->fsize, MADV_DONTNEED);

        m_poolMutex->lock();
        if (isPoolTile(tile->m_data, tile->m_pixelSize))
            reclaimTileToPool(tile->m_data, tile->m_pixelSize);
        else
            delete[] tile->m_data;
        m_poolMutex->unlock();

        tile->m_data = data;
    } else {
        madvise(info->tile->m_data, info->fsize, MADV_DONTNEED);
    }

    info->inMem = false;
    m_currentInMem--;
    m_bytesInMem -= info->size;

    m_swapMutex->unlock();
}

void KisTileManager::doSwapping()
{
    m_swapMutex->lock();

    if (m_swapForbidden || m_currentInMem <= m_maxInMem) {
        m_swapMutex->unlock();
        return;
    }

#if 1 // enable this to enable swapping

    quint32 count = qMin(m_swappableList.size(), (int)m_swappiness);

    for (quint32 i = 0; i < count; i++) {
        toSwap(m_swappableList.front());
        m_swappableList.front()->validNode = false;
        m_swappableList.pop_front();
    }

#endif

    m_swapMutex->unlock();
}

void KisTileManager::printInfo()
{
    kDebug(DBG_AREA_TILES) << m_bytesInMem << " out of " << m_bytesTotal << " bytes in memory\n";
    kDebug(DBG_AREA_TILES) << m_currentInMem << " out of " << m_tileMap.size() << " tiles in memory\n";
    kDebug(DBG_AREA_TILES) << m_swappableList.size() << " elements in the swapable list\n";
    kDebug(DBG_AREA_TILES) << "Freelists information\n";
    for (int i = 0; i < m_freeLists.count(); i++) {
        if ( ! m_freeLists[i].empty() ) {
            kDebug(DBG_AREA_TILES) << m_freeLists[i].size()
                    << " elements in the freelist for pixelsize " << i << "\n";
        }
    }
    kDebug(DBG_AREA_TILES) << "Pool stats (" <<  m_tilesPerPool << " tiles per pool)" << endl;
    for (int i = 0; i < 4; i++) {
        if (m_pools[i]) {
            kDebug(DBG_AREA_TILES) << "Pool " << i << ": Freelist count: " << m_poolFreeList[i].count()
                    << ", pixelSize: " << m_poolPixelSizes[i] << endl;
        }
    }
    kDebug(DBG_AREA_TILES) << endl;
}

quint8* KisTileManager::requestTileData(qint32 pixelSize)
{
    m_swapMutex->lock();

    quint8* data = findTileFor(pixelSize);
    if ( data ) {
        m_swapMutex->unlock();
        return data;
    }
    m_swapMutex->unlock();
    return new quint8[m_tileSize * pixelSize];
}

void KisTileManager::dontNeedTileData(quint8* data, qint32 pixelSize)
{
    m_poolMutex->lock();
    if (isPoolTile(data, pixelSize)) {
        reclaimTileToPool(data, pixelSize);
    } else
        delete[] data;
    m_poolMutex->unlock();
}

quint8* KisTileManager::findTileFor(qint32 pixelSize)
{
    m_poolMutex->lock();
    for (int i = 0; i < 4; i++) {
        if (m_poolPixelSizes[i] == pixelSize) {
            if (!m_poolFreeList[i].isEmpty()) {
                quint8* data = m_poolFreeList[i].front();
                m_poolFreeList[i].pop_front();
                m_poolMutex->unlock();
                return data;
            }
        }
        if (m_pools[i] == 0) {
            // allocate new pool
            m_poolPixelSizes[i] = pixelSize;
            m_pools[i] = new quint8[pixelSize * m_tileSize * m_tilesPerPool];
            // j = 1 because we return the first element, so no need to add it to the freelist
            for (int j = 1; j < m_tilesPerPool; j++)
                m_poolFreeList[i].append(&m_pools[i][j * pixelSize * m_tileSize]);
            m_poolMutex->unlock();
            return m_pools[i];
        }
    }
    m_poolMutex->unlock();
    return 0;
}

bool KisTileManager::isPoolTile(quint8* data, qint32 pixelSize) {

    if (data == 0)
        return false;

    m_poolMutex->lock();
    for (int i = 0; i < 4; i++) {
        if (m_poolPixelSizes[i] == pixelSize) {
            bool b = data >= m_pools[i]
                     && data < m_pools[i] + pixelSize * m_tileSize * m_tilesPerPool;
            if (b) {
                m_poolMutex->unlock();
                return true;
            }
        }
    }
    m_poolMutex->unlock();
    return false;
}

void KisTileManager::reclaimTileToPool(quint8* data, qint32 pixelSize) {
    m_poolMutex->lock();
    for (int i = 0; i < 4; i++) {
        if (m_poolPixelSizes[i] == pixelSize)
            if (data >= m_pools[i] && data < m_pools[i] + pixelSize * m_tileSize * m_tilesPerPool) {
                m_poolFreeList[i].append(data);
            }
    }
    m_poolMutex->unlock();
}

void KisTileManager::configChanged() {
    KConfig * cfg = KGlobal::config();
    cfg->setGroup("");
    m_maxInMem = cfg->readEntry("maxtilesinmem",  4000);
    m_swappiness = cfg->readEntry("swappiness", 100);

    m_swapMutex->lock();
    doSwapping();
    m_swapMutex->unlock();
}
