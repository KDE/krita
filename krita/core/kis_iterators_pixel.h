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

#include "tiles/kis_iterator.h"
#include "kis_iteratorpixeltrait.h"
#include "kis_pixel.h"
#include "kis_strategy_colorspace.h"


/**
 * The pixel iterators are high level iterarators. The lower level iterators merely return a pointer to some memory
 * where a pixel begins; these iterators return KisPixels -- high-level representations of a pixel together with
 * color model, profile and selectedness. You can access individual channels using the KisPixel [] operator, and .
 */ 


class KisHLineIteratorPixel : public KisHLineIterator, public KisIteratorPixelTrait <KisHLineIterator>
{
public:
	KisHLineIteratorPixel( KisPaintDeviceSP ndevice, KisDataManager *dm, Q_INT32 x , Q_INT32 y , Q_INT32 w, bool writable);
};

class KisVLineIteratorPixel : public KisIteratorPixelTrait <KisVLineIterator>, public KisVLineIterator
{
public:
	KisVLineIteratorPixel( KisPaintDeviceSP ndevice, KisDataManager *dm, Q_INT32 xpos , Q_INT32 ypos , Q_INT32 height, bool writable);
};

class KisRectIteratorPixel : public KisIteratorPixelTrait <KisRectIterator>, public KisRectIterator
{
public:
	KisRectIteratorPixel( KisPaintDeviceSP ndevice, KisDataManager *dm, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h, bool writable);
};

#endif
