/*
 *  kis_tiles.h - part of KImageShop aka Krayon aka Krita
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

#if !defined KIS_TILES_
#define KIS_TILES_

#include <qcolor.h>
#include <qpoint.h>

#include "kis_tile.h"

class KisTiles {
public:
	KisTiles(uint width, uint height, uint bpp, const QRgb& defaultColor);
	~KisTiles();
	
	inline uint xTiles() const;
	inline uint yTiles() const;
	inline uint bpp() const;

	KisTileSP setTile(uint x, uint y, KisTileSP tile);
	KisTileSP setTile(const QPoint& pt, KisTileSP tile);
	KisTileSP getTile(uint x, uint y);
	KisTileSP getTile(const QPoint& pt);
	KisTileSP takeTile(uint x, uint y);
	bool swapTile(uint x1, uint y1, uint x2, uint y2);

	void markDirty(uint x, uint y);

	void resize(uint width, uint height, uint bpp);

	inline uint getTileNo(uint x, uint y);
	inline bool intersects(uint x, uint y);

private:
	KisTiles(const KisTiles&);
	KisTiles& operator=(const KisTiles&);

	void init(uint width, uint height, uint bpp);
	void cleanup();

private:
	KisTileSPLst m_tiles;
	QRgb m_defaultColor;
	uint m_xTiles;
	uint m_yTiles;
	uint m_bpp;
};

uint KisTiles::xTiles() const
{ 
	return m_xTiles; 
}

uint KisTiles::yTiles() const 
{ 
	return m_yTiles; 
}

uint KisTiles::bpp() const
{
	return m_bpp;
}

uint KisTiles::getTileNo(uint x, uint y)
{
	return y * xTiles() + x;
}

bool KisTiles::intersects(uint x, uint y)
{
	return !(y >= yTiles() || x >= xTiles());
}

#endif // KIS_TILES_

