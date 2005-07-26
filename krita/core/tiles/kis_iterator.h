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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

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
 * The KisRectIterator iterators over a rectangular area in the most efficient order. That is,
 * there is no guarantee that the iterator will work scanline by scanline.
 */
class KisRectIterator : private ACTUAL_RECTITERATOR
{
public:
	/// Constructor, but use factory method in paint device instead.
	KisRectIterator ( KisDataManager *dm, Q_INT32  x, Q_INT32  y, Q_INT32  w, Q_INT32  h, bool writable) :
				ACTUAL_RECTITERATOR((ACTUAL_DATAMGR * )dm, x, y, w, h, writable) {};
	KisRectIterator(const KisRectIterator& rhs) : ACTUAL_RECTITERATOR(rhs) {}
	KisRectIterator& operator=(const KisRectIterator& rhs)
		{ ACTUAL_RECTITERATOR::operator=(rhs); return *this; }

public:	
	/// returns a pointer to the pixel data. Do NOT interpret the data - leave that to a colorstrategy
	inline Q_UINT8 * rawData() const { return ACTUAL_RECTITERATOR::rawData();};

	/// Returns a pointer to the pixel data as it was at the moment of the last memento creation.
	inline const Q_UINT8 * oldRawData() const { return ACTUAL_RECTITERATOR::oldRawData();};

	/// Advances one pixel going to the beginning of the next line when it reaches the end of a line
	inline KisRectIterator & operator++() { ACTUAL_RECTITERATOR::operator++(); return *this; };
	
	/// Goes back one pixel going to the end of the line above when it reaches the beginning of a line
	//inline KisRectIterator & operator--() { ACTUAL_RECTITERATOR::operator--(); return *this; };
	
	/// returns true when iterators has reached the end
	inline bool isDone()  const { return ACTUAL_RECTITERATOR::isDone(); };
	
	 // current x position
	 inline Q_INT32 x() const { return ACTUAL_RECTITERATOR::x(); };
	 
	 // current y position
	 inline Q_INT32 y() const { return ACTUAL_RECTITERATOR::y(); };
};

class KisHLineIterator : private ACTUAL_HLINEITERATOR
{
public:
	/// Constructor, but use factory method in paint device instead.
	KisHLineIterator ( KisDataManager *dm, Q_INT32  x, Q_INT32 y, Q_INT32 w, bool writable) :
				ACTUAL_HLINEITERATOR((ACTUAL_DATAMGR * )dm, x, y, w, writable) {};
	KisHLineIterator(const KisHLineIterator& rhs) : ACTUAL_HLINEITERATOR(rhs) {}
	KisHLineIterator& operator=(const KisHLineIterator& rhs)
		{ ACTUAL_HLINEITERATOR::operator=(rhs); return *this; }

public:	
	/// Returns a pointer to the pixel data. Do NOT interpret the data - leave that to a colorstrategy
	inline Q_UINT8 *rawData() const { return ACTUAL_HLINEITERATOR::rawData();};

	/// Returns a pointer to the pixel data as it was at the moment of the last memento creation.
	inline const Q_UINT8 *oldRawData() const { return ACTUAL_HLINEITERATOR::oldRawData();};

	/// Advances one pixel until it reaches the end of the line
	inline KisHLineIterator & operator++() { ACTUAL_HLINEITERATOR::operator++(); return *this; };
	
	/// Returns the number of consequtive horizontal pixels that we point at
	/// This is useful for optimizing
	inline Q_INT32 nConseqHPixels() const { return ACTUAL_HLINEITERATOR::nConseqHPixels(); };
	
	/// Advances a number of pixels until it reaches the end of the line
	inline KisHLineIterator & operator+=(int n) { ACTUAL_HLINEITERATOR::operator+=(n); return *this; };
	
	/// Goes back one pixel until it reaches the beginning of the line
	inline KisHLineIterator & operator--() { ACTUAL_HLINEITERATOR::operator--(); return *this; };
	
	/// returns true when iterators has reached the end
	inline bool isDone()  const { return ACTUAL_HLINEITERATOR::isDone(); };
	
	 // current x position
	 inline Q_INT32 x() const { return ACTUAL_HLINEITERATOR::x(); };
	 
	 // current y position
	 inline Q_INT32 y() const { return ACTUAL_HLINEITERATOR::y(); };

};

class KisVLineIterator : private ACTUAL_VLINEITERATOR
{

public:
	/// Constructor, but use factory method in paint device instead.
	KisVLineIterator ( KisDataManager *dm, Q_INT32  x, Q_INT32 y, Q_INT32  h, bool writable) :
				ACTUAL_VLINEITERATOR((ACTUAL_DATAMGR * )dm, x, y, h, writable) {};
	KisVLineIterator(const KisVLineIterator& rhs) : ACTUAL_VLINEITERATOR(rhs) {}
	KisVLineIterator& operator=(const KisVLineIterator& rhs)
		{ ACTUAL_VLINEITERATOR::operator=(rhs); return *this; }

public:
	/// returns a pointer to the pixel data. Do NOT interpret the data - leave that to a colorstrategy
	inline Q_UINT8 *rawData() const { return ACTUAL_VLINEITERATOR::rawData();};

	/// Returns a pointer to the pixel data as it was at the moment of the last memento creation.
	inline const Q_UINT8 * oldRawData() const { return ACTUAL_VLINEITERATOR::oldRawData();};

	/// Advances one pixel until it reaches the end of the line
	inline KisVLineIterator & operator++() { ACTUAL_VLINEITERATOR::operator++(); return *this; };
	
	/// Goes back one pixel until it reaches the beginning of the line
	//inline KisVLineIterator & operator--() { ACTUAL_VLINEITERATOR::operator--(); return *this; };
	
	/// returns true when iterators has reached the end
	inline bool isDone() const { return ACTUAL_VLINEITERATOR::isDone(); };
	
	// current x position
	inline Q_INT32 x() const { return ACTUAL_VLINEITERATOR::x(); };
	
	// current y position
	inline Q_INT32 y() const { return ACTUAL_VLINEITERATOR::y(); };

};

#endif
