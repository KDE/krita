/* This file is part of the KDE project
 *   Copyright (c) 2004 Casper Boemann <cbr@boemann.dkt>
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

#if !defined KIS_ITERATOR_H_
#define KIS_ITERATOR_H_

#include <qglobal.h>

#include "kis_datamanager.h"

// Change the following two lines to switch (at compiletime) to another datamanager
#include "kis_tilediterator.h"
#define ACTUAL_RECTITERATOR KisTiledRectIterator
#define ACTUAL_HLINEITERATOR KisTiledHLineIterator
#define ACTUAL_VLINEITERATOR KisTiledVLineIterator


/** 
 * The KisHLinetIterator class iterates through the pixels of a horizontal line in a KisPaintDevice hiding the
 * tile structure
 */
class KisRectIterator : private ACTUAL_RECTITERATOR
{
public:
	/// Constructor, but use factory method in paint device instead.
	KisRectIterator ( KisDataManager *dm, Q_INT32  x, Q_INT32  y, Q_INT32  w, Q_INT32  h, bool writable) :
				ACTUAL_RECTITERATOR((ACTUAL_DATAMGR * )dm, x, y, w, h, writable) {};

public:	
	/// returns a pointer to the pixel data. Do NOT interpret the data - leave that to a colorstrategy
	Q_UINT8 * rawData() { return ACTUAL_RECTITERATOR::rawData();};

	/// Returns a pointer to the pixel data as it was at the moment of the last memento creation.
	Q_UINT8 * oldRawData() { return ACTUAL_RECTITERATOR::oldRawData();};

	/// Advances one pixel going to the beginning of the next line when it reaches the end of a line
	KisRectIterator & operator++(int n) { ACTUAL_RECTITERATOR::operator++(n); return *this; };
	
	/// Goes back one pixel going to the end of the line above when it reaches the beginning of a line
	KisRectIterator & operator--(int n) { ACTUAL_RECTITERATOR::operator--(n); return *this; };
	
	/// returns true when iterators has reached the end
	bool isDone()  { return ACTUAL_RECTITERATOR::isDone(); };
	
	 // current x position
	 Q_INT32 x() { return ACTUAL_RECTITERATOR::x(); };
	 
	 // current y position
	 Q_INT32 y() { return ACTUAL_RECTITERATOR::y(); };
};

class KisHLineIterator : private ACTUAL_HLINEITERATOR
{
public:
	/// Constructor, but use factory method in paint device instead.
	KisHLineIterator ( KisDataManager *dm, Q_INT32  x, Q_INT32 y, Q_INT32 w, bool writable) :
				ACTUAL_HLINEITERATOR((ACTUAL_DATAMGR * )dm, x, y, w, writable) {};

public:	
	/// Returns a pointer to the pixel data. Do NOT interpret the data - leave that to a colorstrategy
	Q_UINT8 *rawData() { return ACTUAL_HLINEITERATOR::rawData();};

	/// Returns a pointer to the pixel data as it was at the moment of the last memento creation.
	Q_UINT8 *oldRawData() { return ACTUAL_HLINEITERATOR::oldRawData();};

	/// Advances one pixel until it reaches the end of the line
	KisHLineIterator & operator++(int) { ACTUAL_HLINEITERATOR::operator++(); return *this; };
	
	/// Returns the number of consequtive horizontal pixels that we point at
	/// This is useful for optimizing
	Q_INT32 nConseqHPixels() { return ACTUAL_HLINEITERATOR::nConseqHPixels(); };
	
	/// Advances a number of pixels until it reaches the end of the line
	KisHLineIterator & operator+=(int n) { ACTUAL_HLINEITERATOR::operator+=(n); return *this; };
	
	/// Goes back one pixel until it reaches the beginning of the line
	KisHLineIterator & operator--(int) { ACTUAL_HLINEITERATOR::operator--(); return *this; };
	
	/// returns true when iterators has reached the end
	bool isDone()  { return ACTUAL_HLINEITERATOR::isDone(); };
	
	 // current x position
	 Q_INT32 x() { return ACTUAL_HLINEITERATOR::x(); };
	 
	 // current y position
	 Q_INT32 y() { return ACTUAL_HLINEITERATOR::y(); };

};

class KisVLineIterator : private ACTUAL_VLINEITERATOR
{

public:
	/// Constructor, but use factory method in paint device instead.
	KisVLineIterator ( KisDataManager *dm, Q_INT32  x, Q_INT32 y, Q_INT32  h, bool writable) :
				ACTUAL_VLINEITERATOR((ACTUAL_DATAMGR * )dm, x, y, h, writable) {};

public:
	/// returns a pointer to the pixel data. Do NOT interpret the data - leave that to a colorstrategy
	Q_UINT8 *rawData() { return ACTUAL_VLINEITERATOR::rawData();};

	/// Returns a pointer to the pixel data as it was at the moment of the last memento creation.
	Q_UINT8 * oldRawData() { return ACTUAL_VLINEITERATOR::oldRawData();};

	/// Advances one pixel until it reaches the end of the line
	KisVLineIterator & operator++(int) { ACTUAL_VLINEITERATOR::operator++(); return *this; };
	
	/// Goes back one pixel until it reaches the beginning of the line
	KisVLineIterator & operator--(int) { ACTUAL_VLINEITERATOR::operator--(); return *this; };
	
	/// returns true when iterators has reached the end
	bool isDone()  { return ACTUAL_VLINEITERATOR::isDone(); };
	
	// current x position
	Q_INT32 x() { return ACTUAL_VLINEITERATOR::x(); };
	
	// current y position
	Q_INT32 y() { return ACTUAL_VLINEITERATOR::y(); };

};

#endif
