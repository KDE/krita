/* This file is part of the KDE project
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

#ifndef KIS_ITERATORS_PIXEL_H_
#define KIS_ITERATORS_PIXEL_H_

#include "kis_iterator.h"
#include "kis_iteratorpixeltrait.h"

/**
 * This iterators will iterate over an horizontal line of the image, you can access to the data of the image using the
 * rawData function.
 * The function isSelected() and selectedness() gives you access to the selection of the current pixel.
 */
class KisHLineIteratorPixel : public KisHLineIterator, public KisIteratorPixelTrait <KisHLineIterator>
{

public:

    KisHLineIteratorPixel( KisPaintDevice *ndevice, KisDataManager *dm, KisDataManager *sel_dm,
                           qint32 x , qint32 y , qint32 w, qint32 offsetx, qint32 offsety,
                           bool writable);

    KisHLineIteratorPixel(const KisHLineIteratorPixel& rhs) : KisHLineIterator(rhs), KisIteratorPixelTrait<KisHLineIterator>(rhs)
        { m_offsetx = rhs.m_offsetx;  m_offsety = rhs.m_offsety; }

    KisHLineIteratorPixel& operator=(const KisHLineIteratorPixel& rhs)
        {
          KisHLineIterator::operator=(rhs);
          KisIteratorPixelTrait<KisHLineIterator>::operator=(rhs);
          m_offsetx = rhs.m_offsetx;  m_offsety = rhs.m_offsety;
          return *this;
        }

    inline KisHLineIteratorPixel & operator ++() { KisHLineIterator::operator++(); advance(1); return *this;}

    /// Advances a number of pixels until it reaches the end of the line
    KisHLineIteratorPixel & operator+=(int n) { KisHLineIterator::operator+=(n); advance(n); return *this; };

    qint32 x() const { return KisHLineIterator::x() + m_offsetx; }

    qint32 y() const { return KisHLineIterator::y() + m_offsety; }

protected:

    qint32 m_offsetx, m_offsety;
};

/**
 * This iterators will iterate over a vertical line of the image, you can access to the data of the image using the
 * rawData function.
 * The function isSelected() and selectedness() gives you access to the selection of the current pixel.
 */
class KisVLineIteratorPixel : public KisVLineIterator, public KisIteratorPixelTrait <KisVLineIterator>
{
public:
    KisVLineIteratorPixel( KisPaintDevice *ndevice, KisDataManager *dm, KisDataManager *sel_dm,
                           qint32 xpos , qint32 ypos , qint32 height, qint32 offsetx, qint32 offsety,
                           bool writable);

    KisVLineIteratorPixel(const KisVLineIteratorPixel& rhs) : KisVLineIterator(rhs), KisIteratorPixelTrait<KisVLineIterator>(rhs)
        { m_offsetx = rhs.m_offsetx;  m_offsety = rhs.m_offsety; }

    KisVLineIteratorPixel& operator=(const KisVLineIteratorPixel& rhs)
        {
          KisVLineIterator::operator=(rhs);
          KisIteratorPixelTrait<KisVLineIterator>::operator=(rhs);
          m_offsetx = rhs.m_offsetx;  m_offsety = rhs.m_offsety;
          return *this; }

    inline KisVLineIteratorPixel & operator ++() { KisVLineIterator::operator++(); advance(1); return *this;}

    qint32 x() const { return KisVLineIterator::x() + m_offsetx; }

    qint32 y() const { return KisVLineIterator::y() + m_offsety; }

protected:

    qint32 m_offsetx, m_offsety;
};

/**
 * This iterators will iterate over a rectangle area of the image, you can access to the data of the image using
 * the rawData function.
 * You must be carefull when using this iterator, while it is faster than the lines iterators, you can't predict the path
 * that will be followed by the iterator. Which means for instance that you should avoid it if you need to have a synchronized
 * iteration over two different paint device.
 * The function isSelected() and selectedness() gives you access to the selection of the current pixel.
 */
class KisRectIteratorPixel : public KisRectIterator, public KisIteratorPixelTrait <KisRectIterator>
{
public:
    KisRectIteratorPixel( KisPaintDevice *ndevice, KisDataManager *dm, KisDataManager *sel_dm,
                          qint32 x, qint32 y, qint32 w, qint32 h, qint32 offsetx, qint32 offsety,
                          bool writable);

    KisRectIteratorPixel(const KisRectIteratorPixel& rhs) : KisRectIterator(rhs), KisIteratorPixelTrait<KisRectIterator>(rhs)
        { m_offsetx = rhs.m_offsetx;  m_offsety = rhs.m_offsety; }

    KisRectIteratorPixel& operator=(const KisRectIteratorPixel& rhs)
        {
          KisRectIterator::operator=(rhs);
          KisIteratorPixelTrait<KisRectIterator>::operator=(rhs);
          m_offsetx = rhs.m_offsetx;  m_offsety = rhs.m_offsety;
          return *this; }

    inline KisRectIteratorPixel & operator ++() { KisRectIterator::operator++(); advance(1); return *this;}

    qint32 x() const { return KisRectIterator::x() + m_offsetx; }

    qint32 y() const { return KisRectIterator::y() + m_offsety; }

protected:

    qint32 m_offsetx, m_offsety;
};

#endif
