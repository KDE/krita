	/*
 * This file is part of the Krita
 *
 * Copyright (c) 2004 Casper Boemann <cbr@boemann.dk>
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
#include <kdebug.h>

#include "kis_global.h"
#include "kis_iterator.h"

KisTiledVLineIterator::KisTiledVLineIterator( KisTiledDataManager *ndevice,  Q_INT32 x,  Q_INT32 y, Q_INT32 h, bool writable) :
	KisTiledIterator(ndevice),
	m_bottom(y + h - 1)
{
	m_writable = writable;
	m_x = x;
	m_y = y;

	// Find tile row,col matching x,y
	m_col = xToCol(m_x);
	m_topRow = yToRow(m_y);
	m_bottomRow = yToRow(m_bottom);
	m_row = m_topRow;

	// calc limits within the tile
	m_xInTile = m_x - m_col * KisTile::WIDTH;
	m_topInTile = m_y - m_topRow * KisTile::HEIGHT;

	if(m_row == m_bottomRow)
		m_bottomInTile = m_bottom - m_bottomRow * KisTile::HEIGHT;
	else
		m_bottomInTile = KisTile::HEIGHT - 1;

	m_yInTile = m_topInTile;

	fetchTileData(m_col, m_row);
	m_offset = m_pixelSize * (m_yInTile * KisTile::WIDTH + m_xInTile);
}
;
KisTiledVLineIterator::~KisTiledVLineIterator( )
{
}

KisTiledVLineIterator & KisTiledVLineIterator::operator ++ ()
{
	if(m_yInTile >= m_bottomInTile)
	{
		nextTile();
		fetchTileData(m_col, m_row);
		m_yInTile =m_topInTile;
		m_offset = m_pixelSize * (m_yInTile * KisTile::WIDTH + m_xInTile);
	}
	else
	{
		m_yInTile++;
		m_offset += m_pixelSize * KisTile::WIDTH;
	}
	m_y++;

	return *this;
}

void KisTiledVLineIterator::nextTile()
{
	if(m_row < m_bottomRow)
	{
		m_row++;
		m_topInTile = 0;

		if(m_row == m_bottomRow)
			m_bottomInTile = m_bottom - m_bottomRow * KisTile::HEIGHT;
		else
			m_bottomInTile = KisTile::HEIGHT - 1;
	}
}

/*
KisTiledVLineIterator & KisTiledVLineIterator::operator -- ()
{
	return *this;
}
*/
