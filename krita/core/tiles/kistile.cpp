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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <assert.h>
#include <string.h>
#include <qtl.h>
#include "kis_types.h"
#include "kis_global.h"
#include "kistilecache.h"
#include "kistile.h"
#include "kistileswap.h"

KisTile::KisTile(Q_INT32 depth, KisTileCacheInterface *cache, KisTileSwapInterface *swap)
{
	init(depth, cache, swap);
}

KisTile::KisTile(KisTile& rhs) : super(rhs)
{
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
			m_img = QImage(width(), height(), 32);
		}
	}
}
	
KisTile::~KisTile()
{
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
		m_img = QImage(width(), height(), 32);
	}
}

QUANTUM *KisTile::data(Q_INT32 xoff, Q_INT32 yoff)
{
	Q_INT32 offset = yoff * m_width + xoff;

	assert(xoff >= 0);
	assert(yoff >= 0);
	assert(yoff <= height());
	assert(xoff <= width());

	if (!m_data)
		allocate();

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

QImage KisTile::convertToImage()
{
	QUANTUM *pixel = data();

	Q_ASSERT(m_img.width() == width());
	Q_ASSERT(m_img.height() == height());

	// TODO : Get convertToImage out of here...
	// TODO : use some kind of proxy to access
	// TODO : color info.  Also this proxy would support
	// TODO : all image formats.  Only RGB/RGBA is supported
	// TODO : here.
	for (Q_INT32 j = 0; j < height(); j++) {
		for (Q_INT32 i = 0; i < width(); i++) {
			Q_UINT8 red = downscale(pixel[PIXEL_RED]);
			Q_UINT8 green = downscale(pixel[PIXEL_GREEN]);
			Q_UINT8 blue = downscale(pixel[PIXEL_BLUE]);

			m_img.setPixel(i, j, qRgb(red, green, blue));
			pixel += depth();
		}
	}

	return m_img;
}

