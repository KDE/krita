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

#include "kis_iterators_pixel.h"
#include "kis_global.h"
#include "kis_paint_device.h"

KisIteratorPixel::KisIteratorPixel( KisPaintDeviceSP ndevice, KisTileCommand* command, Q_INT32 nypos, Q_INT32 nxpos) :
	KisIteratorUnit( ndevice, command, nypos, nxpos, ndevice->depth() )
{

}


KisIteratorLinePixel::KisIteratorLinePixel( KisPaintDeviceSP ndevice, KisTileCommand* command, Q_INT32 nypos,
						Q_INT32 nxstart, Q_INT32 nxend) :
	KisIteratorLine<KisIteratorPixel>( ndevice, command, nypos, nxstart, nxend)
{
}

KisIteratorPixel KisIteratorLinePixel::operator*()
{
	return KisIteratorPixel( m_device, m_command, m_ypos, m_xstart );
}
KisIteratorLinePixel::operator KisIteratorPixel* ()
{
	return new KisIteratorPixel( m_device, m_command, m_ypos, m_xstart );
}

KisIteratorPixel KisIteratorLinePixel::begin()
{
	return KisIteratorPixel( m_device, m_command, m_ypos, m_xstart );
}
KisIteratorPixel KisIteratorLinePixel::end()
{
	return KisIteratorPixel( m_device, m_command, m_ypos, m_xend );
}
