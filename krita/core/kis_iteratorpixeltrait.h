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
#include <kis_paint_device.h>

template< typename _iTp> 
class KisIteratorPixelTrait
{
public:
	KisIteratorPixelTrait(KisPaintDevice * ndevice, _iTp *underlyingIterator)
	:	m_device(ndevice),
		m_underlyingIterator(underlyingIterator)
	{
		m_selectionIterator = NULL;
	};

	
public:
	/**
	 * Return the current pixel
	 */
 	inline KisPixel pixel() const { return m_device->toPixel(m_underlyingIterator->rawData()); };
        inline KisPixelRO oldPixel() const { return m_device->toPixelRO( m_underlyingIterator->oldRawData()); };

	/**
	 * Return one channel from the current kispixel. Does not check whether
	 * channel index actually exists in this colorspace.
	 */
	inline KisQuantum operator[](int index) const
			{ return m_device -> toPixel(m_underlyingIterator->rawData())[index]; };
			
	/**
	 * Returns if the pixel is selected or not. This is much faster than first building a KisPixel
	 */
	inline bool isSelected() const {if(m_selectionIterator) return*(m_selectionIterator->rawData()); else return true;};
	
protected:
	KisPaintDevice *m_device;

	//KisStrategyColorSpaceSP m_colorSpace;

	// XXX: Is this fix correct? BSAR
	//inline void advance(int n){if(m_selectionIterator)(*m_selectionIterator)++;};
	inline void advance(int n){if (m_selectionIterator) for(int i=0; i< n; i++) (*m_selectionIterator)++;};

	void setSelectionIterator(_iTp *si){m_selectionIterator = si;};

	_iTp *m_underlyingIterator;
	_iTp *m_selectionIterator;
};

#endif
