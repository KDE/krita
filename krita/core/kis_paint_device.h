/*
 *  kis_paint_device.h - part of KImageShop aka Krayon aka Krita
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

#if !defined KIS_PAINT_DEVICE_H_
#define KIS_PAINT_DEVICE_H_

#include <qcolor.h>
#include <qobject.h>
#include <qrect.h>
#include <qstring.h>

#include <ksharedptr.h>

#include "kis_global.h"
#include "kis_pixel_packet.h"
#include "kis_tiles.h"
#include "kis_tile.h"

class KoStore;
class KisImageCmd;
class KisPaintDevice;

typedef KSharedPtr<KisPaintDevice> KisPaintDeviceSP;
typedef QValueVector<KisPaintDeviceSP> KisPaintDeviceSPLst;
typedef KisPaintDeviceSPLst::iterator KisPaintDeviceSPLstIterator;
typedef KisPaintDeviceSPLst::const_iterator KisPaintDeviceSPLstConstIterator;

class KisPaintDevice : public QObject, public KShared {
	typedef QObject super;

public:
	KisPaintDevice(const QString& name, int width, int height, uchar depth, const QRgb& defaultColor, bool alpha = true);
	virtual ~KisPaintDevice();

	void setName(const QString& name);
	QString name() const;

	const KisPixelPacket* getConstPixels(int x, int y, int width = TILE_SIZE, int height = TILE_SIZE) const;
	KisPixelPacket* getPixels(int x, int y, int width = TILE_SIZE, int height = TILE_SIZE);
	void syncPixels(KisPixelPacket *region);

	virtual void resize(int width, int height, uchar depth);
	
	KisTileSP getTile(unsigned int x, unsigned int y);
	const KisTileSP getTile(unsigned int x, unsigned int y) const;

	int xTiles() const;
	int yTiles() const;
	uchar depth() const;
	QRect tileExtents() const;

	void findTileNumberAndOffset(QPoint pt, int *tileNo, int *offset) const;
	void findTileNumberAndPos(QPoint pt, int *tileNo, int *x, int *y) const;

	uchar opacity() const;
	void setOpacity(uchar o);

	bool visible() const;
	void setVisible(bool v);

	virtual bool writeToStore(KoStore *store);
	virtual bool loadFromStore(KoStore *store);

	QRect imageExtents() const;
	void moveBy(int dx, int dy);
	void moveTo(int x, int y);
	void allocateRect(const QRect& rc, uchar depth);

	int width() const;
	int height() const;

	Magick::Image* getImage() { return m_tiles; }

protected:
	uchar m_depth;
	uchar m_opacity;
	bool m_visible;
	QRect m_tileRect;
	QRect m_imgRect;
	QString m_name;
	Magick::Image *m_tiles;
//	KisTiles m_tiles;
};

#endif // KIS_PAINT_DEVICE_H_

