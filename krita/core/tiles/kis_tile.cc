/*
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <assert.h>
#include <kdebug.h>
#include "kis_types.h"
#include "kis_global.h"
#include "kis_tile.h"
#include "kis_tileddatamanager.h"

const Q_INT32 KisTile::WIDTH = 64;
const Q_INT32 KisTile::HEIGHT = 64;


KisTile::KisTile(Q_INT32 pixelSize, Q_INT32 col, Q_INT32 row)
{
	m_pixelSize = pixelSize;
	m_data = 0;
	m_nextTile = 0;
	m_col = col;
	m_row = row;
	allocate();

	//HACK: This should use a settable default pixel.
	memset(m_data, 0, WIDTH * HEIGHT * m_pixelSize);

}

KisTile::KisTile(KisTile& rhs, Q_INT32 col, Q_INT32 row)
{
	if (this != &rhs) {
		m_pixelSize = rhs.m_pixelSize;
		m_data = 0;
		m_nextTile = 0;
		allocate();

		if (rhs.m_data) {
			memcpy(m_data, rhs.m_data, WIDTH * HEIGHT * m_pixelSize * sizeof(Q_UINT8));
		}

		m_col = col;
		m_row = row;
	}
}

KisTile::KisTile(KisTile& rhs)
{
	if (this != &rhs) {
		m_pixelSize = rhs.m_pixelSize;
		m_col = rhs.m_col;
		m_row = rhs.m_row;
		m_data = 0;
		m_nextTile = 0;
		allocate();

		if (rhs.m_data) {
			memcpy(m_data, rhs.m_data, WIDTH * HEIGHT * m_pixelSize * sizeof(Q_UINT8));
		}
	}
}


KisTile::~KisTile()
{
	if (m_data) {
		delete[] m_data;
		m_data = 0;
	}
}

void KisTile::allocate()
{
	if (m_data == 0) {
		m_data = new Q_UINT8[WIDTH * HEIGHT * m_pixelSize];
		Q_CHECK_PTR(m_data);
	}
}

KisTile * KisTile::getNext()
{
	return m_nextTile;
}

void KisTile::setNext(KisTile *n)
{
	m_nextTile = n;
}

Q_UINT8 *KisTile::data(Q_INT32 x, Q_INT32 y )
{
	Q_ASSERT(m_data != 0);
	return m_data + m_pixelSize * ( y * WIDTH + x );
}
