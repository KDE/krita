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

class KisTile;

class KisTiles {
public:
	KisTiles();
	KisTiles(unsigned int width, unsigned int height, unsigned int bpp);
	~KisTiles();
	
	inline unsigned int xTiles() const;
	inline unsigned int yTiles() const;
	inline unsigned int bpp() const;

	KisTile* getTile(unsigned int x, unsigned int y) const;

	void markDirty(unsigned int x, unsigned int y);
	bool isDirty(unsigned int x, unsigned int y);

	void resize(unsigned int width, unsigned int height, unsigned int bpp);

private:
	KisTiles(const KisTiles&);
	KisTiles& operator=(const KisTiles&);

	void init(unsigned int width, unsigned int height, unsigned int bpp);
	void cleanup();

private:
	unsigned underrun[1024];
	KisTile **m_tiles;
	unsigned int m_xTiles;
	unsigned int m_yTiles;
	unsigned int m_bpp;
	unsigned overrun[1024];
};

unsigned int KisTiles::xTiles() const
{ 
	return m_xTiles; 
}

unsigned int KisTiles::yTiles() const 
{ 
	return m_yTiles; 
}

unsigned int KisTiles::bpp() const
{
	return m_bpp;
}

#endif // KIS_TILES_

