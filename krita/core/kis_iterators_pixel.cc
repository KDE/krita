/*
 * This file is part of the Krita project
 *
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_iterators_pixel.h"
#include "kis_global.h"
#include "kis_paint_device.h"

KisHLineIteratorPixel::KisHLineIteratorPixel( KisPaintDevice *ndevice, KisDataManager *dm, KisDataManager *sel_dm, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 offsetx, Q_INT32 offsety, bool writable) :
    KisHLineIterator(dm, x - offsetx, y - offsety, w, writable),
    KisIteratorPixelTrait <KisHLineIterator> ( ndevice, this ),
    m_offsetx(offsetx), m_offsety(offsety)
{
    if(sel_dm) {
        KisHLineIterator * i = new KisHLineIterator(sel_dm, x - offsetx, y - offsety, w, false);
        Q_CHECK_PTR(i);
        KisIteratorPixelTrait <KisHLineIterator>::setSelectionIterator(i);
    }
}

KisVLineIteratorPixel::KisVLineIteratorPixel( KisPaintDevice *ndevice, KisDataManager *dm, KisDataManager *sel_dm, Q_INT32 x, Q_INT32 y, Q_INT32 h, Q_INT32 offsetx, Q_INT32 offsety, bool writable) :
    KisVLineIterator(dm, x - offsetx, y - offsety, h, writable),
    KisIteratorPixelTrait <KisVLineIterator> ( ndevice, this ),
    m_offsetx(offsetx), m_offsety(offsety)
{
    if(sel_dm) {
        KisVLineIterator * i = new KisVLineIterator(sel_dm, x - offsetx, y - offsety, h, false);
        Q_CHECK_PTR(i);
        KisIteratorPixelTrait <KisVLineIterator>::setSelectionIterator(i);
    }
}

KisRectIteratorPixel::KisRectIteratorPixel( KisPaintDevice *ndevice, KisDataManager *dm, KisDataManager *sel_dm, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h, Q_INT32 offsetx, Q_INT32 offsety, bool writable) :
    KisRectIterator(dm, x - offsetx, y - offsety, w, h, writable),
    KisIteratorPixelTrait <KisRectIterator> ( ndevice, this ),
    m_offsetx(offsetx), m_offsety(offsety)
{
    if(sel_dm) {
        KisRectIterator * i = new KisRectIterator(sel_dm, x - offsetx, y - offsety, w, h, false);
        Q_CHECK_PTR(i);
        KisIteratorPixelTrait <KisRectIterator>::setSelectionIterator(i);
    }
}
