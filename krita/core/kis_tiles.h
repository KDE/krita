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
	KisTiles();
	KisTiles(uint width, uint height, uint depth, const QRgb& defaultColor);
	~KisTiles();
	
	uint xTiles() const;
	uint yTiles() const;
	uint depth() const;
	KisTileSP setTile(uint x, uint y, KisTileSP tile);
	KisTileSP setTile(const QPoint& pt, KisTileSP tile);
	KisTileSP getTile(uint x, uint y);
	KisTileSP getTile(const QPoint& pt);
	KisTileSP takeTile(uint x, uint y);
	bool swapTile(uint x1, uint y1, uint x2, uint y2);
	void markDirty(uint x, uint y);
	void resize(uint width, uint height, uint depth);
	uint getTileNo(uint x, uint y);
	bool intersects(uint x, uint y);

private:
	KisTiles(const KisTiles&);
	KisTiles& operator=(const KisTiles&);

	void init(uint width, uint height, uint depth);
	void cleanup();

private:
	KisTileSPLst m_tiles;
	QRgb m_defaultColor;
	uint m_xTiles;
	uint m_yTiles;
	uint m_depth;
};

#endif // KIS_TILES_

