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
	KisTiles(int width, int height, int depth, const QRgb& defaultColor);
	~KisTiles();
	
	int xTiles() const;
	int yTiles() const;
	int depth() const;
	KisTileSP setTile(int x, int y, KisTileSP tile);
	KisTileSP setTile(const QPoint& pt, KisTileSP tile);
	KisTileSP getTile(int x, int y);
	KisTileSP getTile(const QPoint& pt);
	const KisTileSP getTile(int x, int y) const;
	const KisTileSP getTile(const QPoint& pt) const;
	KisTileSP takeTile(int x, int y);
	bool swapTile(int x1, int y1, int x2, int y2);
	void markDirty(int x, int y);
	void resize(int width, int height, int depth);
	int getTileNo(int x, int y) const;
	bool intersects(int x, int y) const;

private:
	KisTiles(const KisTiles&);
	KisTiles& operator=(const KisTiles&);

	void init(int width, int height, int depth);
	void cleanup();

private:
	KisTileSPLst m_tiles;
	QRgb m_defaultColor;
	int m_xTiles;
	int m_yTiles;
	int m_depth;
};

#endif // KIS_TILES_

