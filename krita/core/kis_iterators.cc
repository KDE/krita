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

#include "kis_global.h"
#include "kis_iterators.h"
#include <kistile.h>
#include <kistilemgr.h>

KisIteratorPixelQuantum::KisIteratorPixelQuantum( KisPaintDeviceSP ndevice, KisTileCommand* command, Q_INT32 nypos) :
	KisIteratorPixel<QUANTUM, 1>( ndevice, command, nypos)
{

}
QUANTUM KisIteratorPixelQuantum::KisIteratorPixelQuantum::operator*() const
{
	KisTileMgrSP ktm = m_device->data();
	KisTileSP tile;
	int depth = ::imgTypeDepth( m_device->typeWithoutAlpha() ) +1;
	int x = xpos / depth;
	int num = ktm->tileNum(x, ypos);
	if( tile = ktm->tile( num, TILEMODE_READ) ) // Lent si on veut juste lire
	{
		m_command->addTile( num, tile);
	}
	int xt = x % tile->width();
	int yt = ypos % tile->height();
	QUANTUM* q = tile->data(xt, yt);
	q += xpos % depth;
	return *q;
}
QUANTUM* KisIteratorPixelQuantum::KisIteratorPixelQuantum::operator->() const
{
	KisTileMgrSP ktm = m_device->data();
	KisTileSP tile;
	int depth = ::imgTypeDepth( m_device->typeWithoutAlpha() ) +1;
	int x = xpos / depth;
	int num = ktm->tileNum(x, ypos);
	if( tile = ktm->tile( num, TILEMODE_RW) ) // Lent si on veut juste lire
	{
		m_command->addTile( num , tile);
	}
	int xt = x / tile->width();
	int yt = ypos / tile->height();
	QUANTUM* q = tile->data(xt, yt);
	q += xpos % depth;
	return q;
}

KisIteratorLineQuantum::KisIteratorLineQuantum( KisPaintDeviceSP ndevice, KisTileCommand* command) :
	KisIteratorLine<KisIteratorPixelQuantum>( ndevice, command)
{
}

KisIteratorPixelQuantum KisIteratorLineQuantum::operator*() const
{
	return KisIteratorPixelQuantum( m_device, m_command, ypos );
}
KisIteratorPixelQuantum* KisIteratorLineQuantum::operator->() const
{
	return new KisIteratorPixelQuantum( m_device, m_command, ypos );
}
