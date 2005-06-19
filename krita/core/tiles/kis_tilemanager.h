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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#if !defined KIS_TILEMANAGER_H_
#define KIS_TILEMANAGER_H_

#include <qglobal.h>
#include <qmap.h>
#include <qvaluelist.h>
#include <ktempfile.h>

class KisTile;
class KisTiledDataManager;

/**
 * Provides a way to store tiles on disk to a swap file, to reduce memory usage.
 */
class KisTileManager  {
public:
	~KisTileManager();
	static KisTileManager* instance();

public:
	void registerTile(KisTile* tile);
	void deregisterTile(KisTile* tile);
	void ensureTileLoaded(KisTile* tile);
	void maySwapTile(KisTile* tile);

private:
	KisTileManager();
	KisTileManager(KisTileManager& rhs) {}
	KisTileManager operator=(const KisTileManager&);

private:
	static KisTileManager *m_singleton;
	KTempFile m_tempFile;
	int m_fileSize;

	typedef struct { KisTile *tile; bool inMem; int filePos; int size; int fsize; } TileInfo;
	typedef struct { Q_UINT8 *pointer; int filePos; int size; } FreeInfo;
	typedef QMap<KisTile*, TileInfo*> TileMap;
	typedef QValueList<TileInfo*> TileList;
	typedef QValueList<FreeInfo*> FreeList;

	TileMap m_tileMap;
	TileList m_swappableList;
	FreeList m_freeList;
	Q_INT32 m_maxInMem;
	Q_INT32 m_currentInMem;
	Q_INT32 m_swappiness;
	unsigned long m_bytesInMem;
	unsigned long m_bytesTotal;

	// debug
	int counter;

private:
	void fromSwap(TileInfo* info);
	void toSwap(TileInfo* info);
	void doSwapping();
	void printInfo();
};

#endif // KIS_TILEMANAGER_H_
