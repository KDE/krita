/*
 *  kis_tile.h - part of KImageShop aka Krayon aka Krita
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

#if !defined KIS_TILE_
#define KIS_TILE_

#include <Magick++.h>

#include <qcolor.h>
#include <qpoint.h>
#include <qimage.h>
#include <qmutex.h>
#include <qpoint.h>
#include <qsize.h>
#include <qvaluevector.h>

#include <ksharedptr.h>

#include "kis_global.h"

class KisPixelPacket;
class KisTile;

typedef KSharedPtr<KisTile> KisTileSP;
typedef QValueVector<KisTileSP> KisTileSPLst;
typedef KisTileSPLst::iterator KisTileSPLstIterator;
typedef KisTileSPLst::const_iterator KisTileSPLstConstIterator;

typedef Magick::CompositeOperator TileCompositeOperator;

class KisTile : public KShared {
public:
	KisTile(int x, int y, bool alpha, int width = TILE_SIZE, int height = TILE_SIZE, int depth = 4, const QRgb& defaultColor = 0);
	~KisTile();

	const KisPixelPacket* getConstPixels(int x, int y, int width = TILE_SIZE, int height = TILE_SIZE) const;
	const KisPixelPacket* getConstPixels() const;
	KisPixelPacket* getPixels(int x, int y, int width = TILE_SIZE, int height = TILE_SIZE);
	KisPixelPacket* getPixels();
	void syncPixels();

	Magick::Image* getImage();
	const Magick::Image* getImage() const;

	void copy(const KisTile& tile);
	void clear();

	void move(int x, int y);
	void move(const QPoint& parentPos);

	QPoint topLeft() const;
	QPoint bottomRight() const;

	const QRect& geometry() const;

	void modifyImage();

	QSize size() const;
	int width() const;
	int height() const;
	int depth() const;

	QImage convertTileToImage();
	void convertTileFromImage(const QImage& img);

	void composite(const KisTileSP src, const TileCompositeOperator& op);
	
private:
	KisTile(const KisTile& tile);
	KisTile& operator=(const KisTile& tile);
	void setGeometry(int x, int y, int w, int h);
	void setGeometry(const QRect& rc);
	void initTile();

private:
	QRect m_geometry;
	bool m_alpha;
	int m_depth;
	Magick::Image *m_data;
	QRgb m_defaultColor;
	QMutex m_mutex;
};

#endif // KIS_TILE_

