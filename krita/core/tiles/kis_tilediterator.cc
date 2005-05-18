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
	Q_ASSERT(ndevice != 0);
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

Q_UINT8 * KisTiledIterator::rawData() const
{
	return m_data + m_offset;
}


const Q_UINT8 * KisTiledIterator::oldRawData() const
{
#ifdef DEBUG
	// Warn if we're misusing oldRawData(). If there's no memento, oldRawData is the same
	// as rawData().
	kdWarning(!m_ktm -> hasCurrentMemento(), DBG_AREA_TILES) << "Accessing oldRawData() when no transaction is in progress.\n";
#endif
	return m_oldData + m_offset;
}

void KisTiledIterator::fetchTileData(Q_INT32 col, Q_INT32 row)
{
	KisTile *tile = m_ktm->getTile(col, row, m_writable);
	Q_ASSERT(tile != 0);

	m_data = tile->data();
	Q_ASSERT(m_data != 0);

	// set old data but default to current value
	m_oldData = m_ktm->getOldTile(col, row, tile)->data();
};
