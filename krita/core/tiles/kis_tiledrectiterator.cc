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

KisTiledRectIterator::KisTiledRectIterator( KisTiledDataManager *ndevice,  Q_INT32 nleft,
						Q_INT32 ntop, Q_INT32 nw, Q_INT32 nh, bool writable) :
	KisTiledIterator(ndevice),
	m_left(nleft),
	m_top(ntop),
	m_w(nw),
	m_h(nh)
{
	m_writable = writable;
	m_x = nleft;
	m_y  = ntop;
	m_beyondEnd = (m_w == 0) || (m_h == 0);
	
	// Find tile row,col matching x,y
	// The hack with 16384 is to avoid negative division which is undefined in C++ and the most
	// common result is not like what is desired.
	// however the hack is not perfect either since for coords lower it gives the wrong result
	m_topRow = (m_y + 16384 * tileHeight()) / tileHeight() - 16384;
	m_bottomRow = (m_y + m_h - 1  + 16384 * tileHeight()) / tileHeight() - 16384;
	m_leftCol = (m_x + 16384 * tileWidth()) / tileWidth() - 16384;
	m_rightCol = (m_x + m_w - 1 + 16384 * tileWidth()) / tileWidth() - 16384;
	m_row = m_topRow;
	m_col = m_leftCol;
	
	// calc limits within the tile
	m_topInTile = m_top - m_topRow * tileHeight();
	
	if(m_row == m_bottomRow)
		m_bottomInTile = m_top + m_h - 1 - m_bottomRow * tileHeight();
	else
		m_bottomInTile = tileHeight() - 1;

	m_leftInTile = m_left - m_leftCol * tileWidth();
	
	if(m_col == m_rightCol)
		m_rightInTile = m_left + m_w - 1 - m_rightCol * tileWidth();
	else
		m_rightInTile = tileWidth() - 1;
	
	m_xInTile = m_leftInTile;
	m_yInTile = m_topInTile;
	
	m_tile = getTile(m_col, m_row);
	m_data = m_tile -> data();
	m_offset = m_depth * (m_yInTile * tileWidth() + m_xInTile);
}

KisTiledRectIterator::~KisTiledRectIterator( )
{
}

KisTiledRectIterator & KisTiledRectIterator::operator ++ (int )
{
	// advance through rect completing each tile before moving on
	// as per excellent suggestion by Cyrille, avoiding excessive tile switching
	if(m_xInTile >= m_rightInTile)
	{
		if (m_yInTile >= m_bottomInTile)
		{
			nextTile();
			if(m_beyondEnd)
				return *this;
			m_yInTile = m_topInTile;
			m_x = m_col * tileWidth() + m_leftInTile;
			m_y = m_row * tileHeight() + m_topInTile;
			m_tile = getTile(m_col, m_row);
			m_data = m_tile -> data();
		}
		else
		{
			m_x -= m_rightInTile - m_leftInTile;
			m_y++;
			m_yInTile++;
		}
		m_xInTile =m_leftInTile;
		m_offset = m_depth * (m_yInTile * tileWidth() + m_xInTile);
	}
	else
	{
		m_x++;
		m_xInTile++;
		m_offset += m_depth;
	}
	return *this;
}

void KisTiledRectIterator::nextTile()
{
	if(m_col >= m_rightCol)
	{
		// needs to switch row
		if(m_row >= m_bottomRow)
			m_beyondEnd = true;
		else
		{
			m_col = m_leftCol;
			m_row++;
			// The row has now changed, so recalc vertical limits
			if(m_row == m_topRow)
				m_topInTile = m_top - m_topRow * tileHeight();
			else
				m_topInTile = 0;
	
			if(m_row == m_bottomRow)
				m_bottomInTile = m_top + m_h - 1 - m_bottomRow * tileHeight();
			else
				m_bottomInTile = tileHeight() - 1;
		}
	}
	else
		m_col++;
	
	// No matter what the column has now changed, so recalc horizontal limits
	if(m_col == m_leftCol)
		m_leftInTile = m_left - m_leftCol * tileWidth();
	else
		m_leftInTile = 0;

	if(m_col == m_rightCol)
		m_rightInTile = m_left + m_w - 1 - m_rightCol * tileWidth();
	else
		m_rightInTile = tileWidth() - 1;
}


KisTiledRectIterator & KisTiledRectIterator::operator -- (int)
{
	return *this;
}

bool KisTiledRectIterator::isDone()
{
	return m_beyondEnd;
}
