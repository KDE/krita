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
	KisPaintDevice(const QString& name, uint width, uint height, uint bpp, const QRgb& defaultColor);
	virtual ~KisPaintDevice();

	inline void setName(const QString& name);
	inline QString name() const;

	virtual void setPixel(uint x, uint y, const uchar *pixel, KisImageCmd *cmd = 0);
	virtual bool pixel(uint x, uint y, uchar **val);
	virtual uchar *pixel(uint x, uint y);

	virtual void resize(uint width, uint height, uint bpp);
	
	inline KisTileSP getTile(unsigned int x, unsigned int y);

	inline uint xTiles() const;
	inline uint yTiles() const;
	inline uchar bpp() const;
	QRect tileExtents() const;

	void findTileNumberAndOffset(QPoint pt, int *tileNo, int *offset) const;
	void findTileNumberAndPos(QPoint pt, int *tileNo, int *x, int *y) const;
	KisTileSP swapTile(KisTileSP tile);

	inline uchar opacity() const;
	inline void setOpacity(uchar o);

	inline bool visible() const;
	inline void setVisible(bool v);

	virtual bool writeToStore(KoStore *store);
	virtual bool loadFromStore(KoStore *store);

protected:
	uchar m_opacity;
	bool m_visible;
	QRect m_tileRect;
	QString m_name;
	KisTiles m_tiles;
	cMode m_cMode;
};

uint KisPaintDevice::xTiles() const
{
	return m_tiles.xTiles();
}

uint KisPaintDevice::yTiles() const
{
	return m_tiles.yTiles();
}

uchar KisPaintDevice::bpp() const
{
	return m_tiles.bpp();
}

void KisPaintDevice::setName(const QString& name)
{
	m_name = name;
}

QString KisPaintDevice::name() const
{
	return m_name;
}

KisTileSP KisPaintDevice::getTile(unsigned int x, unsigned int y) 
{ 
	return m_tiles.getTile(x, y); 
}

uchar KisPaintDevice::opacity() const 
{ 
	return m_opacity; 
}

void KisPaintDevice::setOpacity(uchar o) 
{ 
	m_opacity = o; 
}

bool KisPaintDevice::visible() const 
{ 
	return m_visible; 
}

void KisPaintDevice::setVisible(bool v) 
{ 
	m_visible = v; 
}

#endif // KIS_PAINT_DEVICE_H_

