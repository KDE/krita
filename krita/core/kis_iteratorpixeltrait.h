/* This file is part of the KDE project
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>, the original iteratorpixel
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>, made it into a trait
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

#ifndef KIS_ITERATORPIXELTRAIT_H_
#define KIS_ITERATORPIXELTRAIT_H_

#include "tiles/kis_iterator.h"
#include "kis_pixel.h"
#include "kis_strategy_colorspace.h"
#include <kis_paint_device.h>
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
	inline operator KisPixel() { return m_device -> toPixel((QUANTUM*)(*this)); }; 
	// XXX: Isn't this the same as the above?
 	inline KisPixel value() { return m_device -> toPixel((QUANTUM*)(*this)); };
        inline KisPixelRO oldPixelValue() { return m_device -> toPixelRO( this -> oldQuantumValue()); };

	/**
	 * Return one channel from the current kispixel. Does not check whether
	 * channel index actually exists in this colorspace.
	 */
	inline KisQuantum operator[](int index) { return m_device -> toPixel((QUANTUM*)(*this))[index]; };

        inline operator QUANTUM*() { return (Q_UINT8 *)m_underlyingIterator; };

protected:
	inline QUANTUM* oldQuantumValue() { return m_underlyingIterator -> oldValue(); };
	
protected:
	KisPaintDeviceSP m_device;
	KisStrategyColorSpaceSP m_colorSpace;

	_iTp *m_underlyingIterator;
};

#endif
