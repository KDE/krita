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

KisIteratorUnit::KisIteratorUnit( KisPaintDeviceSP ndevice, KisTileCommand* command, Q_INT32 nypos, Q_INT32 nxpos, Q_INT8 /*inc*/)
:	m_device(ndevice),
	m_command(command),
	m_colorSpace(ndevice->colorStrategy()),
	m_underlying_iterator(ndevice->createHLineIterator(nxpos,100,nypos,false)) //AUTOLAYER
{
}
