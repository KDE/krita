/*
 *  Copyright (c) 2004 Casper Boemann <cbr@boemann.dk>
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
#if !defined KIS_TILE_H_
#define KIS_TILE_H_

#include <qglobal.h>

class KisTiledDataManager;
class KisTiledIterator;

/**
 * Provides abstraction to a tile.  A tile contains
 * a part of a PaintDevice, but only the individual pixels
 * are accesable and that only via iterators.
 */
class KisTile  {
public:
	KisTile(Q_INT32 pixelSize, Q_INT32 col, Q_INT32 row);
	KisTile(KisTile& rhs, Q_INT32 col, Q_INT32 row);
	KisTile(KisTile& rhs);
	~KisTile();

public:
	void release();
	void allocate();
	Q_UINT8 *data(Q_INT32 xoff = 0, Q_INT32 yoff = 0);
	Q_INT32 refCount() const;
	void ref();
	Q_INT32 getRow() {return m_row;};
	Q_INT32 getCol() {return m_col;};
	void setNext(KisTile *);
	KisTile *getNext();

	friend class KisTiledIterator;
	friend class KisTiledDataManager;
	friend class KisMemento;
private:
	KisTile& operator=(const KisTile&);

private:
	Q_UINT8 *m_data;
	Q_INT32 m_nReadlock;
	bool m_writeLock;
	Q_INT32 m_row;
	Q_INT32 m_col;
	Q_INT32 m_pixelSize;
	KisTile *m_nextTile;

public:
	static const Q_INT32 WIDTH;
	static const Q_INT32 HEIGHT;
};

#endif // KIS_TILE_H_

