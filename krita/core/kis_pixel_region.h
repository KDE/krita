/*
 *  kis_pixel_region.h - part of KImageShop aka Krayon aka Krita
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

#if !defined KIS_PIXEL_REGION_H_
#define KIS_PIXEL_REGION_H_

#include <qmemarray.h>
#include <ksharedptr.h>
#include "kis_tile.h"

class QRect;
class KisPixelPacket;
class KisPixelRegion;

typedef KSharedPtr<KisPixelRegion> KisPixelRegionSP;

class KisPixelRegion : public KShared {
public:
	KisPixelRegion(const QRect& viewGeometry, int xTile, int yTile, int wTile, int hTile);
	~KisPixelRegion();

	int xTile() const;
	int yTile() const;
	int wTile() const;
	int hTile() const;
	
	void placeTile(int xTile, int yTile, KisTileSP tile);
	void placeTile(int xTile, int yTile, const KisTileSP tile) const;

	KisTileSP getModifiedTile(int xTile, int yTile);
	bool dirtyTile(int xTile, int yTile);

	// TODO more methods to fetch pixels
	const KisPixelPacket* getConstPixel(int x, int y) const;
	KisPixelPacket* getPixel(int x, int y);
	void syncPixels();

private:
	KisPixelRegion(const KisPixelRegion&);
	KisPixelRegion& operator=(const KisPixelRegion&);
	int tileNo(int xTile, int yTile) const;

private:
	QRect m_viewGeometry; 
	int m_xTile; 
	int m_yTile; 
	int m_wTile; 
	int m_hTile;
	mutable KisPixelPacket *m_internalBuffer;
	mutable KisTileSPLst m_tiles;
	mutable QMemArray<bool> m_dirty;
};

#endif // KIS_PIXEL_REGION_H_

