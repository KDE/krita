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

#include <string.h>
#include <qpoint.h>
#include "kis_types.h"
#include "kis_global.h"
#include "kisscopedlock.h"
#include "kispixeldata.h"
#include "kistilemediator.h"
#include "kistilemgr.h"

KisTileMgr::KisTileMgr(Q_UINT32 depth, Q_UINT32 width, Q_UINT32 height)
{
	m_x = 0;
	m_y = 0;
	m_width = width;
	m_height = height;
	m_depth = depth;
	m_ntileRows = (height + TILE_HEIGHT - 1) / TILE_HEIGHT;
	m_ntileCols = (width + TILE_WIDTH - 1) / TILE_WIDTH;
	m_mediator = new KisTileMediator;
}

KisTileMgr::KisTileMgr(const KisTileMgr& rhs) : super(rhs)
{
	if (this != &rhs) {
	}
}

KisTileMgr::~KisTileMgr()
{
	m_mediator -> detachAll(this);
	delete m_mediator;
}

void KisTileMgr::attach(KisTileSP tile, Q_INT32 tilenum)
{
	KisScopedLock l(tile -> mutex());

	if (tile -> shareCount() > 1 && !tile -> valid())
		validate(tile);

	m_mediator -> attach(tile, this, tilenum);
}

void KisTileMgr::detach(KisTileSP tile, Q_INT32 tilenum)
{
	KisScopedLock l(tile -> mutex());

	m_mediator -> detach(tile, this, tilenum);
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

	if (mode & TILEMODE_READ) {
		KisScopedLock l(tile -> mutex());

		if (mode & TILEMODE_RW) {
			if (tile -> shareCount() > 1) {
				KisTileSP tileNew = new KisTile(*tile);

				detach(tile, tilenum);
				attach(tileNew, tilenum);
				tile = tileNew;
			}

			tile -> writeRef();
			tile -> dirty(true);
		}

		l.unlock();
		tile -> lock();
	}

	return tile;
}

void KisTileMgr::tileAsync(Q_INT32 xpix, Q_INT32 ypix)
{
	KisTileSP tile;
	Q_INT32 tilenum;
	Q_INT32 ntiles;

	tilenum = tileNum(xpix, ypix);
	ntiles = m_ntileRows * m_ntileCols;

	if (tilenum < 0 || tilenum >= ntiles)
		return;

	tile = m_tiles[tilenum];
	tile -> lockAsync();
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

bool KisTileMgr::completetlyValid() const
{
	return false;
}

void KisTileMgr::validate(KisTileSP tile)
{
//	tile -> valid(true);
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

Q_UINT32 KisTileMgr::width() const
{
	return m_width;
}

Q_UINT32 KisTileMgr::height() const
{
	return m_height;
}

Q_UINT32 KisTileMgr::depth() const
{
	return m_depth;
}

Q_UINT32 KisTileMgr::nrows() const
{
	return m_ntileRows;
}

Q_UINT32 KisTileMgr::ncols() const
{
	return m_ntileCols;
}

bool KisTileMgr::empty() const
{
	return m_tiles.empty();
}

void KisTileMgr::offset(QPoint& off) const
{
	off.setX(m_x);
	off.setY(m_y);
}

void KisTileMgr::offset(Q_INT32 *x, Q_INT32 *y) const
{
	if (x && y) {
		*x = m_x;
		*y = m_y;
	}
}

void KisTileMgr::setOffSet(const QPoint& off)
{
	m_x = off.x();
	m_y = off.y();
}

void KisTileMgr::setOffSet(Q_INT32 x, Q_INT32 y)
{
	m_x = x;
	m_y = y;
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

void KisTileMgr::mapOver(KisTileSP dst, KisTileSP src)
{
	Q_INT32 tilenum;

	if (!dst || !src)
		return;

	tilenum = m_mediator -> tileNum(dst, this);

	if (tilenum < 0)
		return;

	tileMap(tilenum, src);
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

	if (tilenum1 < 0 || tilenum2 < 0)
		return 0;

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

	if (tilenum1 == tilenum2) {
		KisTileSP t = tile(tilenum1, mode);

		pd -> tile = t;
		pd -> data = t -> data();
		pd -> stride = t -> depth() * t -> width();
		pd -> owner = false;
	} else {
		pd -> data = new QUANTUM[pd -> width * pd -> height * depth()];
		pd -> stride = depth() * pd -> width;
		pd -> owner = true;
		pd -> tile = 0;

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

			while (rows--) {
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

	m_tiles.resize(ntiles);
	nrows = m_ntileRows;
	ncols = m_ntileCols;
	rightTile = m_width - ((ncols - 1) * TILE_WIDTH);
	bottomTile = m_height - ((nrows -1) * TILE_HEIGHT);

	for (i = 0, k = 0; i < nrows; i++) {
		for (j = 0; j < ncols; j++, k++) {
			m_tiles[k] = new KisTile(depth(), 0, 0);
			attach(m_tiles[k], k);

			if (j == ncols - 1)
				m_tiles[k] -> width(rightTile);

			if (i == nrows - 1)
				m_tiles[k] -> height(bottomTile);
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

KisTileSP KisTileMgr::invalidateTile(KisTileSP tile, Q_INT32 tilenum)
{
	KisScopedLock l(tile -> mutex());

	if (tile -> valid())
		return 0;

	if (tile -> shareCount() > 1) {
		KisTileSP tileNew = new KisTile(*tile);

		l.unlock();
		detach(tile, tilenum);
		attach(tileNew, tilenum);
		tile = tileNew;
	}

	tile -> valid(false);
	return tile;
}

