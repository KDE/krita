/* This file is part of the KDE project
   Copyright (c) 2004 Casper Boemann <cbr@boemann.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
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
	// The hack with 16384 is to avoid negative division which is undefined in C++ and the most
	// common result is not like what is desired.
	// however the hack is not perfect either since for coords lower it gives the wrong result
	m_col = (m_x + 16384 * tileWidth()) / tileWidth() - 16384;
	m_topRow = (m_y + 16384 * tileHeight()) / tileHeight() - 16384;
	m_bottomRow = (m_bottom + 16384 * tileHeight()) / tileHeight() - 16384;
	m_row = m_topRow;
	
	// calc limits within the tile
	m_xInTile = m_x - m_col * tileWidth();
	m_topInTile = m_y - m_topRow * tileHeight();	
	
	if(m_row == m_bottomRow)
		m_bottomInTile = m_bottom - m_bottomRow * tileHeight();
	else
		m_bottomInTile = tileHeight() - 1;
	
	m_yInTile = m_topInTile;
	
	m_tile = getTile(m_col, m_row);
	m_data = m_tile -> data();
	m_offset = m_depth * (m_yInTile * tileWidth() + m_xInTile);
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
		m_tile = getTile(m_col, m_row);
		m_data = m_tile -> data();
		m_yInTile =m_topInTile;
		m_offset = m_depth * (m_yInTile * tileWidth() + m_xInTile);
	}
	else
	{
		m_yInTile++;
		m_offset += m_depth * tileWidth();
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
			m_bottomInTile = m_bottom - m_bottomRow * tileHeight();
		else
			m_bottomInTile = tileHeight() - 1;
	}
}


KisTiledVLineIterator & KisTiledVLineIterator::operator -- ()
{
	return *this;
}

bool KisTiledVLineIterator::isDone()
{
	return m_y > m_bottom;
}
