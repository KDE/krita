/*
 * This file is part of the KDE project
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

KisTiledHLineIterator::KisTiledHLineIterator( KisTiledDataManager *ndevice,  Q_INT32 x, Q_INT32 y, Q_INT32 w, bool writable) :
	KisTiledIterator(ndevice),
	m_right(x+w-1)
{
	m_writable = writable;
	m_x = x;
	m_y = y;

	// Find tile row,col matching x,y
	m_row = yToRow(m_y);
	m_leftCol = xToCol(m_x);
	m_rightCol = xToCol(m_right);
	m_col = m_leftCol;

	// calc limits within the tile
	m_yInTile = m_y - m_row * tileHeight();
	m_leftInTile = m_x - m_leftCol * tileWidth();

	if(m_col == m_rightCol)
		m_rightInTile = m_right - m_rightCol * tileWidth();
	else
		m_rightInTile = tileWidth() - 1;

	m_xInTile = m_leftInTile;

	fetchTileData(m_col, m_row);
	m_offset = m_pixelSize * (m_yInTile * tileWidth() + m_xInTile);
}
;
KisTiledHLineIterator::~KisTiledHLineIterator( )
{
}

KisTiledHLineIterator & KisTiledHLineIterator::operator ++ ()
{
	if(m_xInTile >= m_rightInTile)
	{
		nextTile();
		fetchTileData(m_col, m_row);
		m_xInTile =m_leftInTile;
		m_offset = m_pixelSize * (m_yInTile * tileWidth() + m_xInTile);
	}
	else
	{
		m_xInTile++;
		m_offset += m_pixelSize;
	}
	m_x++;

	return *this;
}

void KisTiledHLineIterator::nextTile()
{
	if(m_col < m_rightCol)
	{
		m_col++;
		m_leftInTile = 0;

		if(m_col == m_rightCol)
			m_rightInTile = m_right - m_rightCol * tileWidth();
		else
			m_rightInTile = tileWidth() - 1;
	}
}

Q_INT32 KisTiledHLineIterator::nConseqHPixels()
{
	return m_rightInTile - m_xInTile + 1;
}

KisTiledHLineIterator & KisTiledHLineIterator::operator+=(int n)
{
	if(m_xInTile + n > m_rightInTile)
	{
		nextTile();
		m_xInTile = m_leftInTile;
		m_x += n;
	}
	else
	{
		m_xInTile += n;
		m_x += n;
	}
	fetchTileData(m_col, m_row);
	m_offset = m_pixelSize * (m_yInTile * tileWidth() + m_xInTile);

	return *this;
}

KisTiledHLineIterator & KisTiledHLineIterator::operator -- ()
{
	return *this;
}

bool KisTiledHLineIterator::isDone()
{
	return m_x > m_right;
}
