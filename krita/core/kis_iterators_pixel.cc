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

KisHLineIteratorPixel::KisHLineIteratorPixel( KisPaintDeviceSP ndevice, KisDataManager *dm, Q_INT32 x, Q_INT32 y, Q_INT32 w, bool writable) :
	KisIteratorPixelTrait <KisHLineIterator> ( ndevice, this ),
	KisHLineIterator(dm, x, y, w, writable)
{
}

KisVLineIteratorPixel::KisVLineIteratorPixel( KisPaintDeviceSP ndevice, KisDataManager *dm, Q_INT32 x, Q_INT32 y, Q_INT32 h, bool writable) :
	KisIteratorPixelTrait <KisVLineIterator> ( ndevice, this ),
	KisVLineIterator(dm, x, y, h, writable)
{
}

KisRectIteratorPixel::KisRectIteratorPixel( KisPaintDeviceSP ndevice, KisDataManager *dm, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h, bool writable) :
	KisIteratorPixelTrait <KisRectIterator> ( ndevice, this ),
	KisRectIterator(dm, x, y, w, h, writable)
{
}
