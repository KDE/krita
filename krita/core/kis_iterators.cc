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

#include "kis_iterators.h"

KisIteratorUnit::KisIteratorUnit( KisPaintDeviceSP ndevice, KisTileCommand* command, Q_INT32 nypos, Q_INT32 nxpos, Q_INT8 inc)
		: m_device (ndevice),
		  m_command (command), 
		  m_ktm( m_device->data()),
		  m_depth(::imgTypeDepth( m_device->typeWithoutAlpha() ) +1),
		  m_ypos(nypos), 
		  m_rownum(nypos / TILE_HEIGHT ), 
		  m_ypos_intile( nypos % TILE_HEIGHT ),
		  m_tilenum( m_ktm->ncols() * m_rownum + nxpos /  TILE_WIDTH ), 
		  m_xintile( (nxpos % TILE_WIDTH ) * m_depth),
			m_oldTileNeedRefresh (true),
		  m_tileNeedRefresh (true), 
		  m_tileNeedRefreshRW(true),
			m_inc(inc)
{
	m_tile = m_ktm->tile( m_tilenum, TILEMODE_READ);
}
