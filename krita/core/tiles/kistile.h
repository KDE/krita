/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
#if !defined KISTILE_H_
#define KISTILE_H_

#include <qglobal.h>
#include <qmutex.h>
#include <qvaluevector.h>

#include <ksharedptr.h>

#include "kis_types.h"

class KisTileCacheInterface;
class KisTileSwapInterface;

/**
 * Provides abstraction to a tile.  A tile contains
 * a part of a layer.  Layers form an image.
 */
class KisTile : public KShared {
	typedef KShared super;

public:
	enum drawingHints { broken, opaque, transparent, mixed, outofrange, undef, unknown };

public:
	KisTile(Q_INT32 depth, KisTileCacheInterface *cache = 0, KisTileSwapInterface *swap = 0);
	KisTile(KisTile& rhs);
	virtual ~KisTile();

public:
	QMutex *mutex();
	void lock();
	void lockAsync();
	void release();
	void allocate();
	QUANTUM *data(Q_INT32 xoff = 0, Q_INT32 yoff = 0);
	Q_INT32 width() const;
	void width(Q_INT32 w);
	Q_INT32 height() const;
	void height(Q_INT32 h);
	Q_INT32 depth() const;
	Q_INT32 size() const;

        /** Comment from email dd. 12/10/03
	   > > > Dirty probably means, if a tile is modified or not, so that krita
	   > > > knows if a tile has to be repainted or not. But what does valid then
	   > > > mean.
	   > >
	   > > Ouch! You got me, I really don't remember, it's already been 9 months.
	   >
	   > I still don't understand it. Each tile has two bools: dirty and
	   > valid/invalid. I grep'ed for both and I think actually valid/invalid
	   > actually indicates the state of the tile. Maybe dirty has something to
	   > do with swapping?
	   >
	   > Now, the rendering code, that actually renders the tiles to the screen
	   > is in KisDoc (and in KisColorStrategyRGB which is called from
	   > KisDoc). But the rendering code is not making any use of the
	   > valid/invalid flag of the tiles. I thought the whole point about
	   > setting tiles valid/invalid is to know if they should be drawn to the
	   > screen or not? Or am I getting something terribly wrong here? What is
	   > the intention of valid/invalid?
	   
	   The dirty flag was meant for actual rendering, the valid flag meant that the 
	   tile pointed to valid memory, i.e. memory that is still in-core.
	*/
	bool dirty() const;
	void dirty(bool val);
	bool valid() const;
	void valid(bool valid);
	Q_INT32 rowHint(Q_INT32 row) const;
	void setRowHint(Q_INT32 row, drawingHints hint);
	Q_INT32 refCount() const;
	void ref();
	Q_INT32 shareCount() const;
	void shareRef();
	void shareRelease();
	Q_INT32 writeCount() const;
	void writeRef();
	void duplicate(KisTile *tile);

private:
	KisTile& operator=(const KisTile&);
	void init(Q_INT32 depth, KisTileCacheInterface *cache, KisTileSwapInterface *swap);
	void initRowHints();

private:
	bool m_dirty;
	bool m_valid;
	QUANTUM *m_data;
	Q_INT32 m_width;
	Q_INT32 m_height;
	Q_INT32 m_depth;
	QMutex m_mutex;
	QValueVector<drawingHints> m_hints;
	KisTileSwapInterface *m_swap;
	KisTileCacheInterface *m_cache;
	Q_INT32 m_swapNo;
	Q_INT32 m_nref;
	Q_INT32 m_nshare;
	Q_INT32 m_nwrite;
};

inline
Q_INT32 KisTile::width() const
{
	return m_width;
}

inline
void KisTile::width(Q_INT32 w)
{
	m_width = w;
}

inline
Q_INT32 KisTile::height() const
{
	return m_height;
}

inline
void KisTile::height(Q_INT32 h)
{
	m_height = h;
}

inline
Q_INT32 KisTile::depth() const
{
	return m_depth;
}

inline
Q_INT32 KisTile::size() const
{
	return m_width * m_height * m_depth;
}

inline
bool KisTile::valid() const
{
	return m_valid;
}

inline
void KisTile::valid(bool valid)
{
	m_valid = valid;
}

inline
bool KisTile::dirty() const
{
	return m_dirty;
}

inline
void KisTile::dirty(bool val)
{
	m_dirty = val;
}

#endif // KISTILE_H_

