/* This file is part of the KDE project
   Copyright (c) 2004 Casper Boemann <cbr@boemann.dkt>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#if !defined KIS_TILED_ITERATOR_H_
#define KIS_TILED_ITERATOR_H_

#include <qglobal.h>
#include <kis_tile.h>
#include <kis_tileddatamanager.h>

/** 
 * The KisIterator class iterates through the pixels of a KisPaintDevice hiding the tile structure
 */
class KisTiledIterator {

protected:
	KisTiledDataManager *m_ktm;
	Q_INT32 m_depth;		// bytes per pixel	 
	Q_INT32 m_x;		// current x position
	Q_INT32 m_y;		// cirrent y position
	Q_INT32 m_row;	// row in tilemgr
	Q_INT32 m_col;	// col in tilemgr
	Q_UINT8 *m_data;
	Q_INT32 m_offset;
	KisTile *m_tile;
	bool m_writable;
	
protected:
	Q_UINT32 tileWidth() { return KisTile::WIDTH; };
	Q_UINT32 tileHeight() { return KisTile::HEIGHT; };
	KisTile *getTile(Q_INT32 col, Q_INT32 row) { return m_ktm->getTile(col, row, m_writable); };
	
public:
	KisTiledIterator( KisTiledDataManager *ktm);
	
	~KisTiledIterator();

public:
	// current x position
	Q_INT32 x() { return m_x; };
	
	// cirrent y position
	Q_INT32 y() { return m_x; };
	
	/// returns a pointer to the pixel data. Do NOT interpret the data - leave that to a colorstrategy
	operator Q_UINT8 * ();
};

/** 
 * The KisRectIterator class iterates through the pixels of a rect in a KisPaintDevice hiding the
 * tile structure
 */
class KisTiledRectIterator : public KisTiledIterator
{

public:
	/// do not call constructor directly use factory method in KisDataManager instead.
	KisTiledRectIterator( KisTiledDataManager *dm, Q_INT32  x, Q_INT32  y, Q_INT32  w, Q_INT32  h, bool writable);
	
	~KisTiledRectIterator();

public:	
	/// Advances one pixel. Going to the beginning of the next line when it reaches the end of a line
	KisTiledRectIterator & operator++(int);
	
	/// Goes back one pixel. Going to the end of the line above when it reaches the beginning of a line
	KisTiledRectIterator & operator--(int);
	
	/// returns true when the iterator has reached the end
	bool isDone();

protected:
	 Q_INT32 m_left;
	 Q_INT32 m_top;
	 Q_INT32 m_w;
	 Q_INT32 m_h;
	 Q_INT32 m_topRow;
	 Q_INT32 m_bottomRow;
	 Q_INT32 m_leftCol;
	 Q_INT32 m_rightCol;
	 Q_INT32 m_xInTile;
	 Q_INT32 m_yInTile;
	 Q_INT32 m_leftInTile;
	 Q_INT32 m_rightInTile;
	 Q_INT32 m_topInTile;
	 Q_INT32 m_bottomInTile;
	 bool m_beyondEnd;
	 
private:
	 void nextTile();
};

/** 
 * The KisHLineIterator class iterates through the pixels of a horizontal line in a KisPaintDevice hiding the
 * tile structure
 */
class KisTiledHLineIterator : public KisTiledIterator
{

public:
	/// do not call constructor directly use factory method in KisDataManager instead.
	KisTiledHLineIterator( KisTiledDataManager *dm, Q_INT32  x, Q_INT32  w, Q_INT32 y, bool writable);
	
	~KisTiledHLineIterator();

public:	
	/// Advances one pixel. Going to the beginning of the next line when it reaches the end of a line
	KisTiledHLineIterator & operator++();
	
	/// Goes back one pixel. Going to the end of the line above when it reaches the beginning of a line
	KisTiledHLineIterator & operator--();
	
	/// returns true when the iterator has reached the end
	bool isDone();

protected:
	 Q_INT32 m_right;
	 Q_INT32 m_leftCol;
	 Q_INT32 m_rightCol;
	 Q_INT32 m_xInTile;
	 Q_INT32 m_yInTile;
	 Q_INT32 m_leftInTile;
	 Q_INT32 m_rightInTile;
	 
private:
	 void nextTile();
};

/** 
 * The KisVLineIterator class iterates through the pixels of a vertical line in a KisPaintDevice hiding the
 * tile structure
 */
class KisTiledVLineIterator : public KisTiledIterator
{

public:
	/// do not call constructor directly use factory method in KisDataManager instead.
	KisTiledVLineIterator( KisTiledDataManager *dm, Q_INT32  x, Q_INT32 y, Q_INT32 h, bool writable);
	
	~KisTiledVLineIterator();

public:	
	/// Advances one pixel. Going to the beginning of the next line when it reaches the end of a line
	KisTiledVLineIterator & operator++();
	
	/// Goes back one pixel. Going to the end of the line above when it reaches the beginning of a line
	KisTiledVLineIterator & operator--();
	
	/// returns true when the iterator has reached the end
	bool isDone();

protected:
	 Q_INT32 m_bottom;
	 Q_INT32 m_topRow;
	 Q_INT32 m_bottomRow;
	 Q_INT32 m_xInTile;
	 Q_INT32 m_yInTile;
	 Q_INT32 m_topInTile;
	 Q_INT32 m_bottomInTile;
	 
private:
	 void nextTile();
};

#endif // KIS_TILED_ITERATOR_H_
