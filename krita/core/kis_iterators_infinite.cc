/* This file is part of the KDE project
   Copyright (c) 2004 Bart Coppens <kde@bartcoppens.be>

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

#include "kis_layer.h"
#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_iterators_infinite.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_iterators.h"
#include "kis_iterators_pixel.h"
#include "kis_pixel_representation.h"
#include "color_strategy/kis_strategy_colorspace.h"

KisIteratorInfinitePixel::KisIteratorInfinitePixel(KisPaintDeviceSP plane , KisTileCommand* command, Q_INT32 nypos, Q_INT32 nxpos)
 : KisIteratorPixel ( plane, command, nypos % plane->height(), nxpos % plane->width() ),
   m_isPixel(false)
{ ; }

KisIteratorInfinitePixel::KisIteratorInfinitePixel(KisStrategyColorSpaceSP s, KisPixelRepresentation p)
 : KisIteratorPixel ( constructPixel(s, p) , 0, 0, 0 ),
   m_isPixel(true)
{ ; }

KisIteratorInfinitePixel::~KisIteratorInfinitePixel() {
	if (m_isPixel) // we created a layer; destroy it
		delete m_device;
}

KisPaintDeviceSP KisIteratorInfinitePixel::constructPixel(KisStrategyColorSpaceSP s, KisPixelRepresentation p) {
	KisLayerSP l = new KisLayer(1, 1, s, "InfinitePixelIterator pixel");
	KisIteratorPixel it((KisPaintDeviceSP)l, 0, 0, 0);
	KisPixelRepresentation data = it;
	for (int i = 0; i < l->depth(); i++)
		data[i] = (QUANTUM) p[i]; // explicit (QUANTUM) cast to prevent weirdness
	return (KisPaintDeviceSP)l;
}

inline KisIteratorInfinitePixel& KisIteratorInfinitePixel::inc() {
	Q_ASSERT( m_tile != 0 );
	if (m_isPixel)
		return *this;
	m_xintile+= m_inc;
	if( m_xintile >= m_tile->width() * m_inc )
	{
		m_xintile =  0;
		if ( (m_tilenum + 1) % m_ktm->ncols() == 0)
			m_tilenum = m_ktm->ncols() * m_rownum;
		else
			m_tilenum++;
		m_tileNeedRefresh = true;
		m_tileNeedRefreshRW = true;
		m_oldTileNeedRefresh = true;
	}
	return *this;
}

inline KisIteratorInfinitePixel& KisIteratorInfinitePixel::dec() {
	Q_ASSERT( m_tile != 0 );
	if (m_isPixel)
		return *this;
	m_xintile-=m_inc;
	if( m_xintile < 0 )
	{
		/* This produces something awful when this jumps from the most left tile to
		   the right. This is because the most right tile might be smaller than
		   the others, and that tile's size is only loaded on casting to a form of
		   QUANTUM. */
		if (m_tilenum % m_ktm->ncols() == 0)
			m_tilenum = m_ktm->ncols() * (m_rownum + 1) - 1;
		else
			m_tilenum--;
		m_xintile =  m_tile->width() * m_inc - m_inc;
		m_tileNeedRefresh = true;
		m_tileNeedRefreshRW = true;
		m_oldTileNeedRefresh = true;
	}
	return *this;
}

KisIteratorInfiniteLinePixel::KisIteratorInfiniteLinePixel(KisPaintDeviceSP plane, KisTileCommand* command, Q_INT32 nypos, Q_INT32 nxstart) :
 KisIteratorLinePixel(plane, command, nypos % plane->height(),
		(nxstart == -1) ? -1 : nxstart % plane->width(), -1) , m_plane(plane)
{ ; }

KisIteratorPixel KisIteratorInfiniteLinePixel::operator*() {
	return KisIteratorInfinitePixel( m_plane, m_command, m_ypos, m_xstart );
}

KisIteratorInfiniteLinePixel::operator KisIteratorPixel*() {
	return new KisIteratorInfinitePixel( m_plane, m_command, m_ypos, m_xstart );
}

KisIteratorInfiniteLinePixel& KisIteratorInfiniteLinePixel::inc() {
	m_ypos++;
	if (m_ypos % m_plane->height() == 0)
		m_ypos = 0;
	return *this;
}

KisIteratorInfiniteLinePixel& KisIteratorInfiniteLinePixel::dec() {
	m_ypos--;
	if (m_ypos < 0)
		m_ypos = m_plane->height() - 1;
	return *this;
}

KisIteratorPixel KisIteratorInfiniteLinePixel::begin() {
	return KisIteratorInfinitePixel( m_plane, m_command, m_ypos, m_xstart);
}

