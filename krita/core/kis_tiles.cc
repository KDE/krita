/*
 *  kis_tiles.cc - part of KImageShop aka Krayon aka Krita
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

#include <assert.h>
#include <stdlib.h>

#include <kdebug.h>

#include "kis_global.h"
#include "kis_tiles.h"
#include "kis_tile.h"

KisTiles::KisTiles(unsigned int width, unsigned int height, unsigned int bpp, const QRgb& defaultColor)
{
	m_defaultColor = defaultColor;
	init(width, height, bpp);
}

KisTiles::~KisTiles()
{
	cleanup();
}

KisTile* KisTiles::getTile(unsigned int x, unsigned int y) const
{
	if (y > yTiles() || x > xTiles())
		return 0;
	
	return m_tiles[y * m_xTiles + x];
}

void KisTiles::markDirty(unsigned int x, unsigned int y)
{
	if (y > yTiles() || x > xTiles())
		return;
	
	m_tiles[y * m_xTiles + x] -> setDirty(true);
}

bool KisTiles::isDirty(unsigned int x, unsigned int y)
{
	if (y > yTiles() || x > xTiles())
		return false;

	return m_tiles[y * m_xTiles + x] -> dirty();
}

void KisTiles::resize(unsigned int width, unsigned int height, unsigned int bpp)
{
	cleanup();
	init(width, height, bpp);
}

void KisTiles::init(unsigned int width, unsigned int height, unsigned int bpp)
{
	m_xTiles = width;
	m_yTiles = height;
	m_bpp = bpp;
	m_tiles = new KisTile*[m_xTiles * m_yTiles];

	for (unsigned int y = 0; y < m_yTiles; y++)
		for (unsigned int x = 0; x < m_xTiles; x++)
			m_tiles[y * m_xTiles + x] = new KisTile(TILE_SIZE, TILE_SIZE, bpp, m_defaultColor);
}

void KisTiles::cleanup()
{
	for (unsigned int y = 0; y < m_yTiles; y++)
		for (unsigned int x = 0; x < m_xTiles; x++)
			delete m_tiles[y * m_xTiles + x];

	delete[] m_tiles;
	m_xTiles = 0;
	m_yTiles = 0;
}

