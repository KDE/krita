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

#ifndef KIS_ITERATORS_PIXEL_H_
#define KIS_ITERATORS_PIXEL_H_

#include "kis_iterators.h"
#include "kis_iterator_proxy.h"
#include "kis_pixel.h"
#include "kis_strategy_colorspace.h"

/** FIXME: update this comment to be compatible with the last API change
 
There are two functions to create an iterator : iteratorPixelBegin and
iteratorPixelSelectionBegin, use the first if you don't care about
whether pixels are selected, the second if you need to take selections
into account.

KisIteratorLinePixel lineIt = device->iteratorPixelBegin( command, 0, width(), y);
KisIteratorPixel pixelIt = *linetIt;
KisIteratorPixel endIt = linetIt.end();
while( pixelIt <= endIt )
{
 // your computing
 ++pixelIt;
}

If y goes from 0 to height(), you may do the following:

KisIteratorLinePixel lineIt = device->iteratorPixelBegin( command, 0, width(), 
0);
KisIteratorLinePixel endLineIt = device->iteratorPixelEnd( command, 0, 
width(), height);
while( lineIt <) endLineIt )
{
 KisIteratorPixel pixelIt = *linetIt;
 KisIteratorPixel endIt = linetIt.end();
 while( pixelIt <= endIt )
 {
   // your computing
   ++pixelIt;
 }
 ++lineIt;
}


 */

 
// Proxy

class KisIteratorLinePixel;

class KisIteratorPixel : public KisIteratorProxy< KisIteratorUnit, KisIteratorLinePixel>
{
	typedef KisIteratorProxy< KisIteratorUnit, KisIteratorLinePixel > parent;
	public:
		inline operator KisPixel ();
		inline KisPixelRO oldValue();
		inline KisQuantum operator[](int index);
		KisIteratorPixel( KisIteratorUnit* it) : parent(it) { }
};

class KisIteratorLinePixel : public KisIteratorProxy<KisIteratorLine< KisIteratorPixel >, KisIteratorPixel>
{
	typedef KisIteratorProxy<KisIteratorLine<KisIteratorPixel>, KisIteratorPixel > parent;
	public:
		KisIteratorLinePixel( KisIteratorLine<KisIteratorPixel>* it) : parent(it) { }
		inline operator KisIteratorPixel* () {  return (KisIteratorPixel*)(*m_proxyed); }
};

// Functions
inline KisPixelRO KisIteratorPixel::oldValue()
{
	return m_proxyed->oldValue();
// 	return m_colorSpace -> toKisPixelRO( this->oldQuantumValue(), m_device -> profile());
}
/**
 * Return the current pixel
 */
inline KisIteratorPixel::operator KisPixel()
{
	return m_proxyed->value();
// 	return m_colorSpace -> toKisPixel((QUANTUM*)(*this), m_device -> profile());
}

/**
 * Return one channel from the current kispixel. Does not check whether
 * channel index actually exists in this colorspace.
 */
inline KisQuantum KisIteratorPixel::operator[](int index)
{
	return (*m_proxyed)[index]; 
// 	return m_colorSpace -> toKisPixel((QUANTUM*)(*this), m_device -> profile())[index];
}


#endif
