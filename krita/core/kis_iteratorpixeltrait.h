/* This file is part of the KDE project
   Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>, the original iteratorpixel
   Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>, made it into a trait

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

#ifndef KIS_ITERATORPIXELTRAIT_H_
#define KIS_ITERATORPIXELTRAIT_H_

#include "tiles/kis_iterator.h"
#include "kis_pixel.h"
#include "kis_strategy_colorspace.h"
 
template< typename _iTp> 
class KisIteratorPixelTrait
{
public:
	KisIteratorPixelTrait(KisPaintDeviceSP ndevice, _iTp *underlyingIterator)
	:	m_device(ndevice),
		m_colorSpace(ndevice->colorStrategy()),
		m_underlyingIterator(underlyingIterator)
	{
	};

	
public:
	/**
	 * Return the current pixel
	 */
	inline operator KisPixel() { return m_colorSpace -> toKisPixel((QUANTUM*)(*this), m_device -> profile()); }; 
	inline KisPixel value() { return m_colorSpace -> toKisPixel((QUANTUM*)(*this), m_device -> profile()); };
        inline KisPixelRO oldValue() { return m_colorSpace -> toKisPixelRO( this->oldQuantumValue(), m_device -> profile()); };

	/**
	 * Return one channel from the current kispixel. Does not check whether
	 * channel index actually exists in this colorspace.
	 */
	inline KisQuantum operator[](int index) { return m_colorSpace->toKisPixel((QUANTUM*)(*this), m_device->profile())[index]; };

        inline operator QUANTUM*() { return (Q_UINT8 *)m_underlyingIterator; };

protected:
	inline QUANTUM* oldQuantumValue()
{
#if 0 // AUTOLAYER
	if( m_oldTileNeedRefresh )
	{
		m_oldTile = 0;
		if( m_command)
		{
			m_oldTile = (m_command->tile(m_tilenum));
		}
		if( m_oldTile == 0)
		{
			if( !(m_oldTile = (m_ktm->tile( m_tilenum, TILEMODE_READ) ) ) )
				return 0;
		}
		m_oldData =  m_oldTile->data(0, m_ypos_intile);
		m_oldTileNeedRefresh = false;
	}
	return (m_oldData + m_xintile);
#endif  // AUTOLAYER
return 0;
};
	
protected:
	KisPaintDeviceSP m_device;
	KisStrategyColorSpaceSP m_colorSpace;
	QUANTUM* m_data;
	QUANTUM* m_oldData;
	_iTp *m_underlyingIterator;
};

#endif
