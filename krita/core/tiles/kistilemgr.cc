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
#include <stdlib.h>
#include <string.h>
#include <qpoint.h>
#include <kdebug.h>
#include "kis_types.h"
#include "kis_global.h"
#include "kisscopedlock.h"
#include "kispixeldata.h"
#include "kistilemediator.h"
#include "kistilemgr.h"

KisTileMgr::KisTileMgr(Q_UINT32 depth,const enumImgType& imgType, Q_UINT32 width, Q_UINT32 height) :
	m_width ( width ),
	m_height ( height ),
	m_depth ( depth ),
	m_ntileRows ( (height + TILE_HEIGHT - 1) / TILE_HEIGHT),
	m_ntileCols ( (width + TILE_WIDTH - 1) / TILE_WIDTH ),
	m_mediator ( new KisTileMediator ),
	m_imgType ( imgType )
{
}

KisTileMgr::KisTileMgr(KisTileMgr *tm, Q_UINT32 depth, const enumImgType& imgType, Q_UINT32 width, Q_UINT32 height) :
	m_width ( width ),
	m_height ( height ),
	m_depth ( depth ),
	m_ntileRows ( (height + TILE_HEIGHT - 1) / TILE_HEIGHT ),
	m_ntileCols ( (width + TILE_WIDTH - 1) / TILE_WIDTH ),
	m_mediator ( new KisTileMediator ),
	m_imgType ( imgType )
{
	Q_ASSERT(tm != this);
	duplicate(m_ntileRows * m_ntileCols, tm);
}

KisTileMgr::KisTileMgr(const KisTileMgr& rhs) : KShared(rhs)
{
	if (this != &rhs) {
		m_width = rhs.m_width;
		m_height = rhs.m_height;
		m_depth = rhs.m_depth;
		m_ntileRows = rhs.m_ntileRows;
		m_ntileCols = rhs.m_ntileCols;
		m_mediator = new KisTileMediator;
		m_tiles = rhs.m_tiles;
		m_imgType = rhs.m_imgType;

		for (vKisTileSP_it it = m_tiles.begin(); it != m_tiles.end(); it++)
			(*it) -> shareRef();
	}
}

KisTileMgr::~KisTileMgr()
{
	for (vKisTileSP_it it = m_tiles.begin(); it != m_tiles.end(); it++)
		(*it) -> shareRelease();

	//m_mediator -> detachAll(this);
	delete m_mediator;
}

void KisTileMgr::attach(KisTileSP tile, Q_INT32 tilenum, bool)
{
	if (m_tiles.empty())
		allocate(m_ntileRows * m_ntileCols);

	if (tile) {
		KisScopedLock l(tile -> mutex());

		if (tile -> shareCount() > 0 && !tile -> valid())
			tile -> valid(true);

		m_mediator -> attach(tile, this, tilenum);

#if !defined(NDEBUG)
		if (tilenum < 0 || static_cast<Q_UINT32>(tilenum) >= m_tiles.size()) {
			kdDebug(DBG_AREA_TILES) << "Attaching tile to out of range tile number.\n";
			kdDebug(DBG_AREA_TILES) << "m_tiles.size() = " << m_tiles.size() << endl;
			kdDebug(DBG_AREA_TILES) << "tilenum = " << tilenum << endl;
			abort();
		}
#endif
		
		m_tiles[tilenum] = tile;
	}
}

void KisTileMgr::detach(KisTileSP tile, Q_INT32 tilenum, bool)
{
	if (m_tiles.empty())
		allocate(m_ntileRows * m_ntileCols);

	if (tile) {
		KisScopedLock l(tile -> mutex());
		KisTileSP prev;

		m_mediator -> detach(tile, this, tilenum);
		prev = 0; // TODO Get the previous tile that was in this spot.  Right now, the tile is lost when we assign.

#if !defined(NDEBUG)
		if (tilenum < 0 || static_cast<Q_UINT32>(tilenum) >= m_tiles.size()) {
			kdDebug(DBG_AREA_TILES) << "Detaching tile to out of range tile number.\n";
			kdDebug(DBG_AREA_TILES) << "m_tiles.size() = " << m_tiles.size() << endl;
			kdDebug(DBG_AREA_TILES) << "tilenum = " << tilenum << endl;
			abort();
		}
#endif
		m_tiles[tilenum] = prev;
	}
}

KisTileSP KisTileMgr::tile(Q_INT32 xpix, Q_INT32 ypix, Q_INT32 mode)
{
	Q_INT32 n = tileNum(xpix, ypix);

	if (n < 0)
		return 0;

	return tile(n, mode);
}

KisTileSP KisTileMgr::tile(Q_INT32 tilenum, Q_INT32 mode)
{
	KisTileSP tile;
	Q_INT32 ntiles;

	ntiles = m_ntileRows * m_ntileCols;

	if (tilenum < 0 || tilenum >= ntiles)
		return 0;

	if (m_tiles.empty())
		allocate(ntiles);

	tile = m_tiles[tilenum];

	if (!tile)
		return tile;

	if (mode & TILEMODE_WRITE) {
		if (tile -> shareCount() > 0) {
			KisTileSP tileNew = new KisTile(*tile);
#if 0
			kdDebug(DBG_AREA_TILES) << "Tile " << tilenum << " is shared.  Duplicating.\n";
#endif
			detach(tile, tilenum);
			attach(tileNew, tilenum);
			tile = tileNew;
		}

		tile -> writeRef();
		tile -> dirty(true);
	}

	return tile;
}

void KisTileMgr::tileMap(Q_INT32 xpix, Q_INT32 ypix, KisTileSP src)
{
	Q_INT32 tilenum;

	if (!src)
		return;

	tilenum = tileNum(xpix, ypix);
	tileMap(tilenum, src);
}

void KisTileMgr::tileMap(Q_INT32 tilenum, KisTileSP src)
{
	KisTileSP tile;
	Q_INT32 ntiles;

	if (!src)
		return;

	ntiles = m_ntileRows * m_ntileCols;

	if (tilenum < 0 || tilenum >= ntiles)
		return;

	if (m_tiles.empty())
		allocate(ntiles);

	tile = m_tiles[tilenum];
	detach(tile, tilenum);
	attach(src, tilenum);
}

KisTileSP KisTileMgr::invalidate(Q_INT32 tileno)
{
	KisTileSP t;

	if (tileno < 0)
		return 0;

	t = tile(tileno, TILEMODE_NONE);
	return invalidateTile(t, tileno);
}

KisTileSP KisTileMgr::invalidate(Q_INT32 xpix, Q_INT32 ypix)
{
	KisTileSP t;
	Q_INT32 tilenum = tileNum(xpix, ypix);

	if (tilenum < 0)
		return 0;

	t = tile(tilenum, TILEMODE_NONE);
	return invalidateTile(t, tilenum);
}

KisTileSP KisTileMgr::invalidate(KisTileSP tile, Q_INT32 xpix, Q_INT32 ypix)
{
	Q_INT32 tilenum;

	if (!tile)
		return tile;

	tilenum = tileNum(xpix, ypix);

	if (tilenum < 0)
		return tile;

	tile = invalidateTile(tile, tilenum);
	return tile;
}

void KisTileMgr::invalidateTiles(KisTileSP top)
{
	double x;
	double y;
	Q_INT32 row;
	Q_INT32 col;
	Q_INT32 num;
	Q_INT32 tilenum;

	if (!top || m_tiles.empty())
		return;

	tilenum = m_mediator -> tileNum(top, this);
	col = tilenum % m_ntileCols;
	row = tilenum / m_ntileCols;
	x = (col * TILE_WIDTH + top -> width() / 2.0) / static_cast<double>(width());
	y = (row * TILE_HEIGHT + top -> height() / 2.0) / static_cast<double>(height());
	col = static_cast<Q_INT32>(x * width() / TILE_WIDTH);
	row = static_cast<Q_INT32>(y * height() / TILE_HEIGHT);
	num = row * m_ntileCols + col;
	m_tiles[num] = invalidateTile(m_tiles[num], num);
}

Q_UINT32 KisTileMgr::memSize()
{
	Q_UINT32 n = 0;

	n = sizeof(KisTileMgr) + m_ntileRows * m_ntileCols * (sizeof(KisTile) + m_depth * TILE_WIDTH * TILE_HEIGHT);
	return n;
}

void KisTileMgr::tileCoord(const KisTileSP& tile, QPoint& coord)
{
	Q_INT32 tilenum = m_mediator -> tileNum(tile, this);

	if (tilenum == TILE_NOT_ATTACHED)
		return;

	coord.setX(TILE_WIDTH * (tilenum % m_ntileCols));
	coord.setY(TILE_HEIGHT * (tilenum / m_ntileCols));
}

void KisTileMgr::tileCoord(const KisTileSP& tile, Q_INT32 *x, Q_INT32 *y)
{
	if (x && y) {
		QPoint coord;

		tileCoord(tile, coord);
		*x = coord.x();
		*y = coord.y();
	}
}

KisPixelDataSP KisTileMgr::pixelData(Q_INT32 x1, Q_INT32 y1, Q_INT32 x2, Q_INT32 y2, Q_INT32 mode)
{
	KisPixelDataSP pd;
	Q_INT32 tilenum1;
	Q_INT32 tilenum2;

	if (m_tiles.empty())
		allocate(m_ntileRows * m_ntileCols);

	tilenum1 = tileNum(x1, y1);
	tilenum2 = tileNum(x2, y2);

	if (tilenum1 < 0 || tilenum2 < 0) {
		kdDebug(DBG_AREA_TILES) << "x1 = " << x1 << " y1 = " << y1 << endl;
		kdDebug(DBG_AREA_TILES) << "x2 = " << x2 << " y2 = " << y2 << endl;
		kdDebug(DBG_AREA_TILES) << "KisTileMgr::pixelData : Invalid tile number.\n";
		kdDebug(DBG_AREA_TILES) << "tilenum1 = " << tilenum1 << endl;
		kdDebug(DBG_AREA_TILES) << "tilenum2 = " << tilenum2 << endl;
		return 0;
	}

	pd = new KisPixelData;
	pd -> mgr = this;
	pd -> mode = mode;
	pd -> x1 = x1;
	pd -> y1 = y1;
	pd -> x2 = x2;
	pd -> y2 = y2;
	pd -> width = x2 - x1 + 1;
	pd -> height = y2 - y1 + 1;
	pd -> data = 0;
	pd -> depth = depth();

	if (tilenum1 == tilenum2 && (x1 % TILE_WIDTH) == 0 && (y1 % TILE_HEIGHT) == 0) {
		KisTileSP t = tile(tilenum1, mode);

		pd -> tile = t;
		pd -> data = t -> data();
		pd -> stride = t -> depth() * t -> width();
		pd -> owner = false;
	} else {
		pd -> tile = 0;
		pd -> data = new QUANTUM[pd -> width * pd -> height * depth()];
		pd -> stride = depth() * pd -> width;
		pd -> owner = true;

		if (mode & TILEMODE_READ)
			readPixelData(pd);
	}

	return pd;
}

void KisTileMgr::releasePixelData(KisPixelDataSP pd)
{
	if (pd -> owner) {
		if (pd -> mode & TILEMODE_WRITE)
			writePixelData(pd);
	} else {
		if (pd -> mode & TILEMODE_WRITE)
			pd -> tile -> valid(false);

		pd -> tile -> release();
	}
}

void KisTileMgr::readPixelData(Q_INT32 x1, Q_INT32 y1, Q_INT32 x2, Q_INT32 y2, QUANTUM *buffer, Q_UINT32 stride) 
{
	Q_INT32 x;
	Q_INT32 y;
	Q_INT32 rows;
	Q_INT32 cols;
	Q_UINT32 srcstride;
	KisTileSP t;
	QUANTUM *src;
	QUANTUM *dst;

	for (y = y1; y <= y2; y += TILE_HEIGHT - (y % TILE_HEIGHT)) {
		for (x = x1; x <= x2; x += TILE_WIDTH - (x % TILE_WIDTH)) {
			t = tile(x, y, TILEMODE_READ);
			t -> lock();
			src = t -> data(x % TILE_WIDTH, y % TILE_HEIGHT);
			dst = buffer + stride * (y - y1) + depth() * (x - x1);
			rows = t -> height() - y % TILE_HEIGHT;

			if (rows > y2 - y + 1)
				rows = y2 - y + 1;

			cols = t -> width() - x % TILE_WIDTH;

			if (cols > x2 - x + 1)
				cols = x2 - x + 1;

			srcstride = t -> width() * t -> depth();

			while (rows--) {
				memcpy(dst, src, cols * depth() * sizeof(QUANTUM));
				src += srcstride;
				dst += stride;
			}

			t -> dirty(false);
			t -> release();
		}
	} 
}

void KisTileMgr::readPixelData(KisPixelDataSP pd)
{
	readPixelData(pd -> x1, pd -> y1, pd -> x2, pd -> y2, pd -> data, pd -> stride);
}

void KisTileMgr::writePixelData(Q_INT32 x1, Q_INT32 y1, Q_INT32 x2, Q_INT32 y2, QUANTUM *buffer, Q_UINT32 stride)
{
	Q_INT32 x;
	Q_INT32 y;
	Q_INT32 rows;
	Q_INT32 cols;
	Q_UINT32 dststride;
	KisTileSP t;
	QUANTUM *src;
	QUANTUM *dst;

	for (y = y1; y <= y2; y += TILE_HEIGHT - (y % TILE_HEIGHT)) {
		for (x = x1; x <= x2; x += TILE_WIDTH - (x % TILE_WIDTH)) {
			t = tile(x, y, TILEMODE_RW);
			src = buffer + stride * (y - y1) + depth() * (x - x1);
			dst = t -> data(x % TILE_WIDTH, y % TILE_HEIGHT);
			rows = t -> height() - y % TILE_HEIGHT;

			if (rows > y2 - y + 1)
				rows = y2 - y + 1;

			cols = t -> width() - x % TILE_WIDTH;
			
			if (cols > x2 - x + 1)
				cols = x2 - x + 1;

			dststride = t -> width() * t -> depth();

			while (rows-- > 0) {
				memcpy(dst, src, cols * depth() * sizeof(QUANTUM));
				dst += dststride;
				src += stride;
			}

			t -> valid(false);
			t -> release();
		}
	}
}

void KisTileMgr::writePixelData(KisPixelDataSP pd)
{
	writePixelData(pd -> x1, pd -> y1, pd -> x2, pd -> y2, pd -> data, pd -> stride);
}

void KisTileMgr::allocate(Q_INT32 ntiles)
{
	Q_INT32 nrows;
	Q_INT32 ncols;
	Q_INT32 rightTile;
	Q_INT32 bottomTile;
	Q_INT32 i;
	Q_INT32 j;
	Q_INT32 k;
	KisTileSP t;

	m_tiles.resize(ntiles);
	nrows = m_ntileRows;
	ncols = m_ntileCols;
	rightTile = m_width - ((ncols - 1) * TILE_WIDTH);
	bottomTile = m_height - ((nrows - 1) * TILE_HEIGHT);

	for (i = 0, k = 0; i < nrows; i++) {
		for (j = 0; j < ncols; j++, k++) {
			t = new KisTile(depth(), 0, 0);
			attach(t, k);

			if (j == ncols - 1)
				t -> width(rightTile);

			if (i == nrows - 1)
				t -> height(bottomTile);
		}
	}
}

void KisTileMgr::duplicate(Q_INT32 ntiles, KisTileMgr *tm)
{
	Q_INT32 nrows;
	Q_INT32 ncols;
	Q_INT32 rightTile;
	Q_INT32 bottomTile;
	Q_INT32 i;
	Q_INT32 j;
	Q_INT32 k;
	KisTileSP t;
	Q_INT32 w;
	Q_INT32 h;

	m_tiles.resize(ntiles);
	nrows = m_ntileRows;
	ncols = m_ntileCols;
	rightTile = m_width - ((ncols - 1) * TILE_WIDTH);
	bottomTile = m_height - ((nrows - 1) * TILE_HEIGHT);

	for (i = 0, k = 0; i < nrows; i++) {
		h = i == nrows - 1 ? bottomTile : TILE_HEIGHT;

		for (j = 0; j < ncols; j++, k++) {
			t = tm -> tile(j * TILE_WIDTH, i * TILE_HEIGHT, TILEMODE_READ);
			w = j == ncols - 1 ? rightTile : TILE_WIDTH;

			if (t) {
				if (t -> width() != w || t -> height() != h) {
					KisTileSP t2 = new KisTile(t -> depth(), 0, 0);

					t2 -> width(w);
					t2 -> height(h);
					t2 -> lock();
					t2 -> duplicate(t);
					t2 -> release();
					t = t2;
				} else {
					t -> shareRef();
				}
			} else {
				t = new KisTile(depth(), 0, 0);
				t -> width(w);
				t -> height(h);
				t -> lock();
				memset(t -> data(), 0, t -> size());
				t -> release();
			}

			t -> valid(false);
			attach(t, k);
		}
	}
}

Q_INT32 KisTileMgr::tileNum(Q_UINT32 xpix, Q_UINT32 ypix) const
{
	Q_INT32 row;
	Q_INT32 col;
	Q_INT32 num;

	if (xpix >= m_width || ypix >= m_height)
		return -1;

	row = ypix / TILE_HEIGHT;
	col = xpix / TILE_WIDTH;
	num = row * m_ntileCols + col;
	return num;
}

KisTileSP KisTileMgr::invalidateTile(KisTileSP tile, Q_INT32)
{
	KisScopedLock l(tile -> mutex());

	if (!tile -> valid())
		return 0;

	tile -> valid(false);
	return tile;
}

