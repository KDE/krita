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

KisTiledIterator::KisTiledIterator( KisTiledDataManager *ndevice)
{
	m_ktm = ndevice;
	m_x = 0;
	m_y = 0;
	m_row = 0;
	m_col = 0;
	m_pixelSize = m_ktm -> pixelSize();
}

KisTiledIterator::~KisTiledIterator( )
{
}

KisTiledIterator::operator Q_UINT8 * ()
{
	return m_data + m_offset;
}


Q_UINT8 * KisTiledIterator::oldValue ()
{
	return m_oldData + m_offset;
}

void KisTiledIterator::fetchTileData(Q_INT32 col, Q_INT32 row)
{
	KisTile *tile = m_ktm->getTile(col, row, m_writable);
	m_data = tile->data();

	// set old data but default to current value
	m_oldData = m_ktm->getOldTile(col, row, tile)->data();
};
