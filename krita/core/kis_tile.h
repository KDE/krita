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

class KisTile {
public:
	KisTile(unsigned int width, unsigned int height, unsigned int bpp, bool dirty = false);
	~KisTile();
				
	void setDirty(bool dirty);
	inline bool dirty() const;
	inline unsigned int bpp();
	unsigned int* data();
	const unsigned int* data() const;

private:
	KisTile(const KisTile&);
	KisTile& operator=(const KisTile&);

private:
	bool m_dirty;
	unsigned int m_width;
	unsigned int m_height;
	unsigned int m_bpp;
	unsigned int *m_data;
};

bool KisTile::dirty() const
{
	return m_dirty;
}

unsigned int KisTile::bpp()
{
	return m_bpp;
}

#endif // KIS_TILE_

