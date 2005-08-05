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

#include <kstaticdeleter.h>

#include "kis_config.h"

#include "kis_tileddatamanager.h"
#include "kis_tile.h"
#include "kis_tilemanager.h"

// Note: the cache file doesn't get deleted when we crash and so :(

KisTileManager* KisTileManager::m_singleton = 0;

static KStaticDeleter<KisTileManager> staticDeleter;

KisTileManager::KisTileManager() {
	KisConfig config;

	Q_ASSERT(KisTileManager::m_singleton == 0);
	KisTileManager::m_singleton = this;
	m_fileSize = 0;
	m_bytesInMem = 0;
	m_bytesTotal = 0;

	m_currentInMem = 0;
	m_maxInMem = config.maxTilesInMem();
	m_swappiness = config.swappiness();

	counter = 0;
}

KisTileManager::~KisTileManager() {
	kdDebug(DBG_AREA_TILES) << "Destructing TileManager: unmapping everything" << endl;

	if (!m_freeList.empty()) { // Let's see if there is an item in the freelist that fits
		FreeList::iterator it = m_freeList.begin();
		FreeList::iterator end = m_freeList.end();

		while (it != end) {
			// munmap it
			munmap((*it) -> pointer, (*it) -> size);
			delete *it;
			++it;
		}
		
		m_freeList.clear();
	}

	kdDebug(DBG_AREA_TILES) << "Destructing TileManager: deleting file" << endl;
	m_tempFile.close();
	m_tempFile.unlink();
}

KisTileManager* KisTileManager::instance() {
	if(KisTileManager::m_singleton == 0) {
		staticDeleter.setObject(KisTileManager::m_singleton, new KisTileManager());
		Q_CHECK_PTR(KisTileManager::m_singleton);
	}
	return KisTileManager::m_singleton;
}

void KisTileManager::registerTile(KisTile* tile) {
	TileInfo* info = new TileInfo();
	info -> tile = tile;
	info -> inMem = true;
	info -> filePos = -1;
	info -> size = tile -> WIDTH * tile -> HEIGHT * tile -> m_pixelSize;
	info -> fsize = 0; // the size in the file

	m_tileMap[tile] = info;
	m_swappableList.push_back(info);

	m_currentInMem++;
	m_bytesTotal += info -> size;
	m_bytesInMem += info -> size;

	doSwapping();

	if (++counter % 50 == 0)
		printInfo();
}

void KisTileManager::deregisterTile(KisTile* tile) {
	Q_ASSERT(m_tileMap.contains(tile));

	TileInfo* info = m_tileMap[tile];

	if (info -> filePos >= 0) { // It is mmapped
		// To freelist
		FreeInfo* freeInfo = new FreeInfo();
		freeInfo -> pointer = tile -> m_data;
		freeInfo -> filePos = info -> filePos;
		freeInfo -> size = info -> fsize;
		m_freeList.push_back(freeInfo);

		madvise(info -> tile -> m_data, info -> fsize, MADV_DONTNEED);

		// the KisTile will attempt to delete its data. This is of course silly when
		// it was mmapped. So change the m_data to NULL, which is safe to delete
		tile -> m_data = 0;
	} else {
		m_bytesInMem -= info -> size;
		m_currentInMem--;
	}

	m_swappableList.remove(info);

	m_bytesTotal -= info -> size;

	delete info;
	m_tileMap.erase(tile);

	
	doSwapping();
}

void KisTileManager::ensureTileLoaded(KisTile* tile) {
	TileInfo* info = m_tileMap[tile];
	m_swappableList.remove(info);

	if (!info -> inMem) {
		fromSwap(info);
	}
}

void KisTileManager::maySwapTile(KisTile* tile) {
	m_swappableList.push_back(m_tileMap[tile]);
	doSwapping();
}

void KisTileManager::fromSwap(TileInfo* info) {
	Q_ASSERT(!info -> inMem);

	doSwapping();

	// ### check return value!
	madvise(info -> tile -> m_data, info -> size, MADV_WILLNEED);

	info -> inMem = true;
	m_currentInMem++;
	m_bytesInMem += info -> size;
}

void KisTileManager::toSwap(TileInfo* info) {
	Q_ASSERT(info -> inMem);

	if (info -> filePos < 0) {
		// This tile is not yet in the file. Save it there
		// ### check return values of mmap functions!
		KisTile *tile = info -> tile;
		Q_UINT8* data = 0;

		if (!m_freeList.empty()) { // Let's see if there is an item in the freelist that fits
			FreeList::iterator it = m_freeList.begin();
			FreeList::iterator end = m_freeList.end();

			while (it != end) {
				if ( (*it) -> size >= info -> size ) {
					// found one
					data = (*it) -> pointer;
					info -> filePos = (*it) -> filePos;
					info -> fsize = (*it) -> size;

					delete *it;
					m_freeList.erase(it);

					kdDebug(DBG_AREA_TILES) << "found in freelist\n";

					break;
				} else {
					kdDebug(DBG_AREA_TILES) << (*it) -> size << " < " << info -> size << endl;
				}
				++it;
			}
		}

		if (data == 0) { // No position found or free, create a new
			long pagesize = sysconf(_SC_PAGESIZE);
			int newsize = m_fileSize + info -> size;
			newsize = newsize + newsize % pagesize;

			ftruncate(m_tempFile.handle(), newsize);

			data = (Q_UINT8*) mmap(0, info -> size,
								   PROT_READ | PROT_WRITE,
								   MAP_SHARED,
								   m_tempFile.handle(), m_fileSize);

			if (data == (Q_UINT8*)-1) {
				kdDebug(DBG_AREA_TILES) << "mmap failed: errno is " << errno << "; we're going to crash...\n";
			}

			info -> fsize = info -> size;
			info -> filePos = m_fileSize;
			m_fileSize = newsize;
		}

		madvise(data, info -> fsize, MADV_WILLNEED);
		memcpy(data, tile -> m_data, info -> size);
		madvise(data, info -> fsize, MADV_DONTNEED);

		delete[] tile -> m_data;

		tile -> m_data = data;
	} else {
		madvise(info -> tile -> m_data, info -> fsize, MADV_DONTNEED);
	}

	info -> inMem = false;
	m_currentInMem--;
	m_bytesInMem -= info -> size;
}

void KisTileManager::doSwapping() {
	if (m_currentInMem <= m_maxInMem)
		return;
#if 1 // enable this to enable swapping
	Q_INT32 count = QMIN(m_swappableList.size(), m_swappiness);

	for (Q_INT32 i = 0; i < count; i++) {
		toSwap(m_swappableList.front());
		m_swappableList.pop_front();
	}
#endif
}

void KisTileManager::printInfo() {
	kdDebug(DBG_AREA_TILES) << m_bytesInMem << " out of " << m_bytesTotal << " bytes in memory\n";
	kdDebug(DBG_AREA_TILES) << m_currentInMem << " out of " << m_tileMap.size() << " tiles in memory\n";
	kdDebug(DBG_AREA_TILES) << m_swappableList.size() << " elements in the swapable list\n";
	kdDebug(DBG_AREA_TILES) << m_freeList.size() << " elements in the freelist\n";
	kdDebug(DBG_AREA_TILES) << endl;
}

void KisTileManager::configChanged() {
	KisConfig config;
	m_maxInMem = config.maxTilesInMem();
	m_swappiness = config.swappiness();

	kdDebug(DBG_AREA_TILES) << "TileManager has new config: maxinmem: " << m_maxInMem
			<< " swappiness: " << m_swappiness << endl;
	
	doSwapping();
}
