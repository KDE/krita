/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_TILEDDATAMANAGER_H_
#define KIS_TILEDDATAMANAGER_H_

#include <qglobal.h>
#include <qvaluevector.h>

class KisDataManager;
class KisTiledIterator;
class KisTile;
class KoStore;
class KisMemento;

typedef QValueVector<Q_UINT8> ImageBytesVector;

/**
 * KisTiledDataManager implements the interface that KisDataManager defines
 * 
 * The interface definition is enforced by KisDataManager calling all the methods
 * which must also be defined in KisTiledDataManager. It is not allowed to change the interface
 * as other datamangers may also rely on the same interface.
 *
 * * Storing undo/redo data
 * * Offering ordered and unordered iterators over rects of pixels
 * * (eventually) efficiently loading and saving data in a format 
 * that may allow deferred loading.
 *
 * A datamanager knows nothing about the type of pixel data except 
 * how many Q_UINT8's a single pixel takes.  
 */
 
class KisTiledDataManager {

protected:
	KisTiledDataManager(Q_UINT32 depth);
	~KisTiledDataManager();
	KisTiledDataManager(const KisTiledDataManager &dm);
	KisTiledDataManager & operator=(const KisTiledDataManager &dm);

	
public:
	// Allow the baseclass of iterators acces to the interior
	// derived classes must go through KisTiledIterator
	friend class KisTiledIterator;
	
public:

	KisMemento *getMemento();
	void rollback(KisMemento *memento);

public:
	/**
	 * Reads and writes the tiles from/onto a KoStore (wich is simply a file within a zip file)
	 *
	 */
	bool write(KoStore *store);
	bool read(KoStore *store);

public:

	// XXX: Refactor these names to conform to rest of Krita : depth() && setDepth()
	Q_UINT32 size();
	Q_UINT32 getDepth();
	
	void extent(Q_INT32 &x, Q_INT32 &y, Q_INT32 &w, Q_INT32 &h) const;


public:

	void clear(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h, Q_UINT8 def);
	void clear(Q_INT32 x, Q_INT32 y,  Q_INT32 w, Q_INT32 h, Q_UINT8 * def);


public:

	void paste(KisDataManager * data,  Q_INT32 sx, Q_INT32 sy, Q_INT32 dx, Q_INT32 dy,
			    Q_INT32 w, Q_INT32 h);


// public:

// 	/**
// 	 * Copy the bytes in the specified rect to a vector. The caller is responsible
// 	 * for managing the vector.
// 	 */
// 	ImageBytesVector * readBytes(Q_INT32 x, Q_INT32 y,
// 				     Q_INT32 w, Q_INT32 h);
// 	/**
// 	 * Copy the bytes in the vector to the specified rect. If there are bytes left
// 	 * in the vector after filling the rect, they will be ignored. If there are
// 	 * not enough bytes, the rest of the rect will be filled with the default value
// 	 * given (by default, 0);
// 	 */
// 	void writeBytes(ImageBytesVector * bytes, 
// 			Q_INT32 x, Q_INT32 y,
// 			Q_INT32 w, Q_INT32 h,
// 			Q_UINT8 defaultvalue = 0);


private:
	 
	Q_UINT32 m_depth;
	Q_UINT32 m_numTiles;
	KisTile *m_defaultTile;
	KisTile **m_hashTable;
	KisMemento *m_currentMemento;
	Q_UINT32 m_extentMinX;
	Q_UINT32 m_extentMinY;
	Q_UINT32 m_extentMaxX;
	Q_UINT32 m_extentMaxY;
private:

	void ensureTileMementoed(Q_INT32 col, Q_INT32 row, Q_UINT32 tileHash, KisTile *refTile);
	KisTile *getTile(Q_INT32 col, Q_INT32 row, bool writeAccess);
	Q_UINT32 calcTileHash(Q_INT32 col, Q_INT32 row);
	void updateExtent(Q_INT32 col, Q_INT32 row);
};

// during development the following line helps to check the interface is correct
// it should be safe to keep it here even during normal compilation
#include "kis_datamanager.h"

#endif // KIS_TILEDDATAMANAGER_H_

