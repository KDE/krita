/*
 *  kis_tile.cc - part of KImageShop aka Krayon aka Krita
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

#include <string.h>
#include <stdlib.h>

#include <qtl.h>

#include <kdebug.h>

#include "kis_global.h"
#include "kis_tile.h"

KisTile::KisTile(unsigned int width, unsigned int height, unsigned int bpp, const QRgb& defaultColor, bool dirty)
{
	m_dirty = dirty;
	m_width = width;
	m_height = height;
	m_bpp = bpp;
	m_data = 0;
	m_defaultColor = defaultColor;

	if (qAlpha(defaultColor))
		initTile();
}

KisTile::KisTile(const KisTile& tile)
{
	if (this != &tile)
		copyTile(tile);
}

KisTile& KisTile::operator=(const KisTile& tile)
{
	if (this != &tile)
		copyTile(tile);

	return *this;
}

KisTile::~KisTile()
{
	delete[] m_data;
}
	
void KisTile::copyTile(const KisTile& tile)
{
	m_dirty = tile.m_dirty;
	m_width = tile.m_width;
	m_height = tile.m_height;
	m_bpp = tile.m_bpp;
	m_data = 0;
	m_defaultColor = tile.m_defaultColor;

	if (tile.m_data) {
		m_data = new unsigned int[m_width * m_height];
		memcpy(m_data, tile.m_data, m_width * m_height * sizeof(unsigned int));
	}
}

void KisTile::setDirty(bool dirty)
{
	m_dirty = dirty;
}

unsigned int* KisTile::data()
{
	if (!m_data)
		m_data = new unsigned int[m_width * m_height];

	return m_data;
}

void KisTile::initTile()
{
	m_data = new unsigned int[m_width * m_height];
	qFill(m_data, m_data + m_width * m_height, m_defaultColor);
	memset(m_data, rand() % 255, m_width * m_height * sizeof(unsigned int));
}

