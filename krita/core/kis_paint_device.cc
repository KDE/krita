/*
 *  kis_paint_device.cc - part of KImageShop aka Krayon aka Krita
 *
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

#include <qcstring.h>
#include <qdatastream.h>

#include <kdebug.h>

#include <koStore.h>

#include "kis_global.h"
#include "kis_image_cmd.h"
#include "kis_paint_device.h"
#include "kis_util.h"
#include "kis_tile.h"

const int TILE_BYTES = TILE_SIZE * TILE_SIZE * sizeof(unsigned int);
    
KisPaintDevice::KisPaintDevice(const QString& name, uint width, uint height, uchar bpp, const QRgb& defaultColor) :
	m_tiles(width / TILE_SIZE, height / TILE_SIZE, bpp, defaultColor)
{
	m_name = name;
	m_imgRect = QRect(0, 0, width, height);
	m_tileRect = KisUtil::findTileExtents(m_imgRect);
	resize(m_tileRect.width(), m_tileRect.height(), bpp);
	m_visible = true;
	m_opacity = 255;
}

KisPaintDevice::~KisPaintDevice()
{
}

void KisPaintDevice::setPixel(uint x, uint y, const uchar *src, KisImageCmd *cmd)
{
	int tileNoY = y / TILE_SIZE;
	int tileNoX = x / TILE_SIZE;
	KisTileSP tile = m_tiles.getTile(tileNoX, tileNoY);

	if (cmd) {
		if (tile && !cmd -> hasTile(tile)) {
			if (tile -> cow()) {
				KisTileSP tileCopy = new KisTile(*tile);

				tile -> setCow(false);	
				swapTile(tileCopy);
				tile = tileCopy;
			}

			cmd -> addTile(tile);
		}
	}

	uchar *dst = 0;

	if (pixel(x, y, &dst)) {
		dst = pixel(x, y);
		memcpy(dst, src, tile -> bpp());
	}
}

uchar* KisPaintDevice::pixel(uint x, uint y)
{
	uchar *pix = 0;

	pixel(x, y, &pix);
	return pix;
}

bool KisPaintDevice::pixel(uint x, uint y, uchar **val)
{
	int tileNoY = y / TILE_SIZE;
	int tileNoX = x / TILE_SIZE;
	KisTileSP tile = m_tiles.getTile(tileNoX, tileNoY);

	if (!tile)
		return false;

	*val = tile -> data(x % TILE_SIZE, y % TILE_SIZE);
	return true;
}

void KisPaintDevice::setPixel(uint x, uint y, const QRgb& rgb, KisImageCmd *cmd)
{
	// TODO
}

bool KisPaintDevice::pixel(uint x, uint y, QRgb *rgb)
{
	// TODO
	return false;
}

QRgb KisPaintDevice::rgb(uint x, uint y)
{
	// TODO
	return 0;
}

void KisPaintDevice::resize(uint width, uint height, uchar bpp)
{
	m_imgRect.setWidth(width);
      	m_imgRect.setHeight(height);
	m_tileRect = KisUtil::findTileExtents(m_imgRect);
	m_tiles.resize(m_tileRect.width() / TILE_SIZE, m_tileRect.height() / TILE_SIZE, bpp);
	m_tiles.resize(width / TILE_SIZE, height / TILE_SIZE, bpp);
}

void KisPaintDevice::findTileNumberAndOffset(QPoint pt, int *tileNo, int *offset) const
{
	pt = pt - tileExtents().topLeft();
	*tileNo = (pt.y() / TILE_SIZE) * xTiles() + pt.x() / TILE_SIZE;
	*offset = (pt.y() % TILE_SIZE) * TILE_SIZE + pt.x() % TILE_SIZE;
}

void KisPaintDevice::findTileNumberAndPos(QPoint pt, int *tileNo, int *x, int *y) const
{
	pt = pt - tileExtents().topLeft();
	*tileNo = (pt.y() / TILE_SIZE) * xTiles() + pt.x() / TILE_SIZE;
	*y = pt.y() % TILE_SIZE;
	*x = pt.x() % TILE_SIZE;
}

QRect KisPaintDevice::tileExtents() const
{
	return m_tileRect;
}

KisTileSP KisPaintDevice::swapTile(KisTileSP tile)
{
	return m_tiles.setTile(tile -> tileCoords(), tile);
}

bool KisPaintDevice::writeToStore(KoStore *store)
{
	for (uint ty = 0; ty < yTiles(); ty++) 
		for (uint tx = 0; tx < xTiles(); tx++) {
			KisTileSP src = m_tiles.getTile(tx, ty);
			const char *p = reinterpret_cast<const char*>(src -> data());

			if (store -> write(p, TILE_BYTES) != TILE_BYTES)
				return false;
		}

	return true;
}

bool KisPaintDevice::loadFromStore(KoStore *store)
{
	int nread;

	for (uint ty = 0; ty < yTiles(); ty++) {
		for (uint tx = 0; tx < xTiles(); tx++) {
			KisTileSP dst = m_tiles.getTile(tx, ty);
			char *p = reinterpret_cast<char*>(dst -> data());

			nread = store -> read(p, TILE_BYTES);

			if (nread != TILE_BYTES)
				return false;
		}
	}

	return true;
}

QRect KisPaintDevice::imageExtents() const
{
	return m_imgRect;
}

void KisPaintDevice::moveBy(int dx, int dy)
{
	m_imgRect.moveBy(dx, dy);
	m_tileRect.moveBy(dx, dy);
}

void KisPaintDevice::moveTo(int x, int y)
{
	int dx = x - m_imgRect.x();
	int dy = y - m_imgRect.y();

	m_imgRect.moveTopLeft(QPoint(x, y));
	m_tileRect.moveBy(dx,dy);
}

void KisPaintDevice::allocateRect(const QRect& rc, uchar bpp)
{
	resize(m_tileRect.width(), m_tileRect.height(), bpp);
	m_imgRect = rc;
}

