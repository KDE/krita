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
#include <assert.h>
#include <string.h>

#include <qtl.h>

#include <kdebug.h>

#include "kis_types.h"
#include "kis_global.h"
#include "kistilecache.h"
#include "kistile.h"
#include "kistileswap.h"

#define DEBUG_TILES 0

#if DEBUG_TILES
static int numTiles = 0;
#endif

KisTile::KisTile(Q_INT32 depth, KisTileCacheInterface *cache, KisTileSwapInterface *swap)
{
#if DEBUG_TILES
	numTiles++;
	kdDebug(DBG_AREA_TILES) << "TILE CREATED total now = " << numTiles << endl;
#endif
	init(depth, cache, swap);
}

KisTile::KisTile(KisTile& rhs) : super(rhs)
{
#if DEBUG_TILES
	numTiles++;
	kdDebug(DBG_AREA_TILES) << "TILE CREATED total now = " << numTiles << endl;
#endif
	if (this != &rhs) {
		init(rhs.m_depth, rhs.m_cache, rhs.m_swap);
		m_width = rhs.m_width;
		m_height = rhs.m_height;
		m_valid = rhs.valid();
		m_hints = rhs.m_hints;
		allocate();
		rhs.shareRelease();

		if (rhs.m_data) {
			rhs.lock();
			memcpy(m_data, rhs.m_data, m_width * m_height * m_depth * sizeof(QUANTUM));
			rhs.release();
		}
	}
}
	
KisTile::~KisTile()
{
#if DEBUG_TILES
	numTiles--;
	kdDebug(DBG_AREA_TILES) << "TILE DESTROYED total now = " << numTiles << endl;
#endif
	if (m_data) {
		delete[] m_data;
		m_data = 0;
	}

	if (m_swap)
		m_swap -> remove(m_swapNo);
}

QMutex *KisTile::mutex()
{
	return &m_mutex;
}

void KisTile::lock()
{
#if 0
	m_mutex.lock();
	m_nref++;

	if (m_nref == 1) {
	}

	if (m_data == 0) {
		Q_ASSERT(m_swap);
		m_swap -> swapIn(this);
	}
		
	m_mutex.unlock();
#endif
}

void KisTile::lockAsync()
{
}

void KisTile::release()
{
#if 0
	m_mutex.lock();
	m_nref--;

	if (m_dirty) {
		m_nwrite--;
		qFill(m_hints.begin(), m_hints.end(), unknown);
	}

	if (m_nref && m_nshare && m_cache)
		m_cache -> insert(this);

	m_mutex.unlock();
#endif
}

void KisTile::allocate()
{
	if (m_data == 0) {
		m_data = new QUANTUM[size()];
	}
}

QUANTUM *KisTile::data(Q_INT32 xoff, Q_INT32 yoff)
{
	Q_INT32 offset = yoff * m_width + xoff;

	if (!m_data)
		allocate();

#if !defined (NDEBUG)
	if (yoff > height()) {
		kdDebug(DBG_AREA_TILES) << "yoff = " << yoff << endl;
		kdDebug(DBG_AREA_TILES) << "height() = " << height() << endl;
	}

	if (xoff > width()) {
		kdDebug(DBG_AREA_TILES) << "xoff = " << xoff << endl;
		kdDebug(DBG_AREA_TILES) << "width() = " << width() << endl;
	}
#endif

	assert(xoff >= 0);
	assert(yoff >= 0);
	assert(yoff <= height());
	assert(xoff <= width());
	return m_data + offset * m_depth;
}

Q_INT32 KisTile::rowHint(Q_INT32 row) const
{
	if (m_hints.empty())
		return unknown;

	if (row >= height() || row < 0)
		return outofrange;

	return m_hints[row];
}

void KisTile::setRowHint(Q_INT32 row, KisTile::drawingHints hint)
{
	initRowHints();

	if (row < height() || row >= 0)
		m_hints[row] = hint;
}

void KisTile::init(Q_INT32 depth, KisTileCacheInterface *cache, KisTileSwapInterface *swap)
{
	m_dirty = false;
	m_valid = false;
	m_width = TILE_WIDTH;
	m_height = TILE_HEIGHT;
	m_depth = depth;
	m_data = 0;
	m_swap = swap;
	m_swapNo = SWAP_IN_CORE;
	m_cache = cache;
	m_nref = 0;
	m_nshare = 0;
	m_nwrite = 0;
}

void KisTile::initRowHints()
{
	if (m_hints.empty()) {
		Q_INT32 height = m_height;

		m_hints.resize(height);
		qFill(m_hints.begin(), m_hints.end(), unknown);
	}
}

Q_INT32 KisTile::refCount() const
{
	return m_nref;
}

void KisTile::ref()
{
	m_nref++;
}

Q_INT32 KisTile::shareCount() const
{
	return m_nshare;
}

void KisTile::shareRef()
{
	m_nshare++;
}

void KisTile::shareRelease()
{
	if (m_nshare > 0)
		m_nshare--;
}

Q_INT32 KisTile::writeCount() const
{
	return m_nwrite;
}

void KisTile::writeRef()
{
	m_nwrite++;
}

void KisTile::duplicate(KisTile *tile)
{
	tile -> lock();

	if (tile -> width() == width() && tile -> height() == height()) {
		memcpy(m_data, tile -> m_data, m_width * m_height * m_depth * sizeof(QUANTUM));
	} else {
		Q_INT32 rows = QMIN(height(), tile -> height());
		Q_INT32 cols = QMIN(width(), tile -> width());
		QUANTUM *dst = data();
		QUANTUM *src = tile -> data();
		QUANTUM *d;
		QUANTUM *s;

		while (rows-- > 0) {
			d = dst;
			s = src;

			for (Q_INT32 i = cols; i > 0; i--, d += m_depth, s += m_depth)
				memcpy(d, s, m_depth * sizeof(QUANTUM));

			dst += width() * depth();
			src += tile -> width() * tile -> depth();
		}
	}

	tile -> release();
}

