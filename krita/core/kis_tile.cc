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

#include <limits.h>
#include <string.h>

#include <kdebug.h>

#include "kis_global.h"
#include "kis_tile.h"

KisTile::KisTile(unsigned int width, unsigned int height, unsigned int bpp, bool dirty)
{
	m_dirty = dirty;
	m_width = width;
	m_height = height;
	m_bpp = bpp;
	m_data = 0;
}

KisTile::KisTile(const KisTile& rhs)
{
	m_dirty = rhs.m_dirty;
	m_width = rhs.m_width;
	m_height = rhs.m_height;
	m_data = new unsigned int[m_width * m_height];
	memcpy(m_data, rhs.m_data, m_width * m_height * sizeof(unsigned int));
}

KisTile& KisTile::operator=(const KisTile& rhs)
{
	if (this != &rhs) {
		m_dirty = rhs.m_dirty;
		m_width = rhs.m_width;
		m_height = rhs.m_height;
		m_data = new unsigned int[m_width * m_height];
		memcpy(m_data, rhs.m_data, m_width * m_height);
	}

	return *this;
}

KisTile::~KisTile()
{
	delete m_data;
}
	
void KisTile::setDirty(bool dirty)
{
	m_dirty = dirty;
}

unsigned int* KisTile::data()
{
	if (!m_data) {
		m_data = new unsigned int[m_width * m_height];
		memset(m_data, 255, m_width * m_height * sizeof(unsigned int));
	}

	return m_data;
}

const unsigned int* KisTile::data() const
{
	return m_data;
}

