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
#include "kis_pixel.h"
#include "kis_strategy_colorspace.h"


/**
 
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
 class KisIteratorUnit;
 
class KisIteratorPixel : public KisIteratorUnit
{
public:
	KisIteratorPixel( KisPaintDeviceSP ndevice, KisTileCommand* command, Q_INT32 nypos = 0, Q_INT32 nxpos = 0);
public:
	inline operator KisPixel();
        inline KisPixelRO oldValue();
	inline KisQuantum operator[](int index);
	virtual ~KisIteratorPixel() {}
private:
};


/**
 * XXX: document
 */
class KisIteratorLinePixel : public KisIteratorLine<KisIteratorPixel>
{
public:
	KisIteratorLinePixel( KisPaintDeviceSP ndevice, 
			      KisTileCommand* command, 
			      Q_INT32 nypos = 0,
			      Q_INT32 nxstart = -1, 
			      Q_INT32 nxend = -1);
public:
	virtual KisIteratorPixel operator*();
	virtual operator KisIteratorPixel* ();
	virtual KisIteratorPixel begin();
	virtual KisIteratorPixel end();
};

inline KisPixelRO KisIteratorPixel::oldValue()
{
  return m_colorSpace -> toKisPixelRO( this->oldQuantumValue(), m_device -> profile());
}

/**
 * Return the current pixel
 */
inline KisIteratorPixel::operator KisPixel()
{
	return m_colorSpace -> toKisPixel((QUANTUM*)(*this), m_device -> profile());
}

/**
 * Return one channel from the current kispixel. Does not check whether
 * channel index actually exists in this colorspace.
 */
inline KisQuantum KisIteratorPixel::operator[](int index)
{
	return m_colorSpace -> toKisPixel((QUANTUM*)(*this), m_device -> profile())[index];
}


#endif
