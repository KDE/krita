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

#include <qcolor.h>
#include <qpoint.h>
#include <qmutex.h>
#include <qvaluevector.h>

#include <ksharedptr.h>

class KisTile;

typedef KSharedPtr<KisTile> KisTileSP;
typedef QValueVector<KisTileSP> KisTileSPLst;
typedef KisTileSPLst::iterator KisTileSPLstIterator;
typedef KisTileSPLst::const_iterator KisTileSPLstConstIterator;

class KisTile : public KShared {
public:
	KisTile(int x, int y, uint width, uint height, uchar bpp, const QRgb& defaultColor, bool dirty = false);
	KisTile(const QPoint& parentPos, uint width, uint height, uchar bpp, const QRgb& defaultColor, bool dirty = false);
	KisTile(const KisTile& tile);
	KisTile& operator=(const KisTile& tile);
	virtual ~KisTile();

	void copyTile(const KisTile& tile);
	void setDirty(bool dirty);
	void move(int x, int y);
	void move(const QPoint& parentPos);
	uchar* data();
	uchar* data(int x, int y);

	inline bool dirty() const;
	inline uchar bpp() const;
	inline const uchar* data() const;
	const uchar* data(int x, int y) const;
	inline QPoint tileCoords();

	inline bool cow() const;
	inline void setCow(bool enable = true);

	inline uint size() const;
	inline uint width() const;
	inline uint height() const;
	
private:
	void initTile();

private:
	QPoint m_parentPos;
	bool m_cow;
	bool m_dirty;
	uint m_width;
	uint m_height;
	uchar m_bpp;
	uchar *m_data;
	QRgb m_defaultColor;
	QMutex m_mutex;
};

bool KisTile::dirty() const
{
	return m_dirty;
}

uchar KisTile::bpp() const
{
	return m_bpp;
}

const uchar* KisTile::data() const
{
	return m_data;
}

QPoint KisTile::tileCoords()
{
	return m_parentPos;
}

bool KisTile::cow() const
{
	return m_cow;
}

void KisTile::setCow(bool enable)
{
	m_cow = enable;
}

uint KisTile::size() const
{
	return m_width * m_height * m_bpp;
}

uint KisTile::width() const
{
	return m_width;
}

uint KisTile::height() const
{
	return m_height;
}

#endif // KIS_TILE_

