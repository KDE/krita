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
#ifndef KIS_DATAMANAGER_H_
#define KIS_DATAMANAGER_H_

#include <qglobal.h>
#include <qvaluevector.h>

class KoStore;

// Change the following two lines to switch (at compiletime) to another datamanager
#include "kis_tileddatamanager.h"
#define ACTUAL_DATAMGR KisTiledDataManager

typedef QValueVector<Q_UINT8> ImageBytesVector;

 
/**
 * KisDataManager defines the interface that modules responsible for
 * storing and retrieving data must inmplement. Data modules, like 
 * the tile manager, are responsible for:
 *
 * * Storing undo/redo data
 * * Offering ordererd and unordered iterators over rects of pixels
 * * (eventually) efficiently loading and saving data in a format 
 * that may allow deferred loading.
 *
 * A datamanager knows nothing about the type of pixel data except 
 * how many Q_UINT8's a single pixel takes.  
 */
class KisDataManager : private ACTUAL_DATAMGR {

public:
	KisDataManager(Q_UINT32 depth) : ACTUAL_DATAMGR(depth) {};
	KisDataManager(const KisDataManager& dm) : ACTUAL_DATAMGR(dm) { };
		
public:

	/**
	 * Reguest a memento from the data manager.
	 *
	 * Any write actions on the datamanger builds undo data into this memento
	 * necessary to rollback the transaction.
	 */ 
	KisMemento *getMemento() { return ACTUAL_DATAMGR::getMemento(); };

	/**
	 * Restores the image data to the state at the time of the getMemento() call.
	 *
	 * Note that rollback should be performed with mementos in the reverse order of
	 * their creation, as mementos only store incremental changes
	 */
	void rollback(KisMemento *memento) { ACTUAL_DATAMGR::rollback(memento); };

public:
	/**
	 * Reads and writes the tiles from/onto a KoStore (wich is simply a file within a zip file)
	 *
	 */
	bool write(KoStore *store) { return ACTUAL_DATAMGR::write(store); };
	bool read(KoStore *store) { return ACTUAL_DATAMGR::read(store); };

public:

	/**
	 * Return the size in bytes the image data takes.
	 */
	Q_UINT32 size() { return ACTUAL_DATAMGR::size(); };

	/**
	 * The number of Q_UINT8 that make up pixel.
	 */
	Q_UINT32 getDepth() { return ACTUAL_DATAMGR::getDepth(); };

	/**
	 * Return the extent of the data in x,y,w,h.
	 */
	void extent(Q_INT32 &x, Q_INT32 &y, Q_INT32 &w, Q_INT32 &h) const
						 { return ACTUAL_DATAMGR::extent(x, y, w, h); };

public:

	/**
	 * Clear the specified rect to the specified value.
	 */
	void clear(Q_INT32 x, Q_INT32 y,
		   Q_INT32 w, Q_INT32 h,
		   Q_UINT8 def) { ACTUAL_DATAMGR::clear(x, y, w, h, def); };

	/**
	 * Clear the specified rect to the specified pixel value.
	 */
	void clear(Q_INT32 x, Q_INT32 y,
		   Q_INT32 w, Q_INT32 h,
		   Q_UINT8 * def) { ACTUAL_DATAMGR::clear(x, y, w, h, def); };


public:

	/**
	 * Copy the specified rect from the specified data into this
	 * data.
	 */
	void paste(KisDataManager * data,  Q_INT32 sx, Q_INT32 sy, Q_INT32 dx, Q_INT32 dy,
		   Q_INT32 w, Q_INT32 h) { ACTUAL_DATAMGR::paste(data, sx, sy, dx, dy, w, h); };
	
// XXX: Something like this is need to avoid having memcpy's in the paint device; we also
// need block-wise iterators that return a pointer to the actual image data in the block,
// wrapped in a vector.
// public:

// 	/**
// 	 * Copy the bytes in the specified rect to a vector. The caller is responsible
// 	 * for managing the vector.
// 	 */
// 	ImageBytesVector * readBytes(Q_INT32 x, Q_INT32 y,
// 					  Q_INT32 w, Q_INT32 h);

// 	/**
// 	 * Copy the bytes in the vector to the specified rect. If there are bytes left
// 	 * in the vector after filling the rect, they will be ignored. If there are
// 	 * not enough bytes, the rest of the rect will be filled with the default value
// 	 * given (by default, 0);
// 	 */
// 	void writeBytes(ImageBytesVector bytes, 
// 			Q_INT32 x, Q_INT32 y,
// 			Q_INT32 w, Q_INT32 h,
// 			Q_UINT8 defaultvalue = 0);



protected:
	friend class KisRectIterator;
	friend class KisHLineIterator;
	friend class KisVLineIterator;
};

#endif // KIS_DATAMANAGER_H_

