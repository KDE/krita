/* This file is part of the KDE project
   Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>

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
#include "kis_iterators.h"

KisIteratorQuantum::KisIteratorQuantum( KisPaintDeviceSP ndevice, KisTileCommand* command, Q_INT32 nypos, Q_INT32 nxpos) :
	KisIteratorUnit<QUANTUM, KisIteratorQuantum, 1>( ndevice, command, nypos, nxpos)
{

}

QUANTUM KisIteratorQuantum::operator*()
{
	if( m_tileNeedRefresh && m_tileNeedRefreshRW )
	{
		if( !(m_tile = (m_ktm->tile( m_tilenum, TILEMODE_READ) ) ) )
		{
			return 0;
		}
		m_data =  m_tile->data(0, m_ypos_intile);
		m_tileNeedRefresh = false;
	}
	return *(m_data + m_xintile);
}

KisIteratorQuantum::operator QUANTUM* ()
{
	if( m_tileNeedRefreshRW )
	{
		if( (m_tile = m_ktm->tile( m_tilenum , TILEMODE_NONE)) )
		{
			m_command->addTile( m_tilenum , m_tile);
		}
		if (!(m_tile = m_ktm->tile( m_tilenum, TILEMODE_RW)))
			return 0;
		m_data =  m_tile->data(0, m_ypos_intile);
		m_tileNeedRefreshRW = false;
	}
	return m_data + m_xintile;
}


KisIteratorLineQuantum::KisIteratorLineQuantum( KisPaintDeviceSP ndevice, KisTileCommand* command, int nypos) :
	KisIteratorLine<KisIteratorQuantum>( ndevice, command, nypos)
{
}

KisIteratorQuantum KisIteratorLineQuantum::operator*()
{
	return KisIteratorQuantum( m_device, m_command, ypos, 0 );
}
KisIteratorLineQuantum::operator KisIteratorQuantum* ()
{
	return new KisIteratorQuantum( m_device, m_command, ypos, 0 );
}

KisIteratorQuantum KisIteratorLineQuantum::begin()
{
	return KisIteratorQuantum( m_device, m_command, ypos, 0 );
}
KisIteratorQuantum KisIteratorLineQuantum::end()
{
	return KisIteratorQuantum( m_device, m_command, ypos, m_device->width() - 1 );
}
