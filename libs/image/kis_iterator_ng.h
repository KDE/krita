/* This file is part of the KDE project
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_ITERATOR_NG_H_
#define _KIS_ITERATOR_NG_H_

#include "kis_base_accessor.h"

class KRITAIMAGE_EXPORT KisBaseConstIteratorNG : public KisBaseConstAccessor
{
    Q_DISABLE_COPY(KisBaseConstIteratorNG)
public:
    KisBaseConstIteratorNG() {}
    virtual ~KisBaseConstIteratorNG();
    /**
     * Move to the next pixel
     * @return false if there is no more pixel in the line
     */
    virtual bool nextPixel() = 0;
    /**
     * Move to the next pixels
     */
    virtual bool nextPixels(qint32 n) = 0;
    /**
     * @return return number of consequential numbers of pixels, useful for optimization
     */
    virtual qint32 nConseqPixels() const = 0;
};

//class KRITAIMAGE_EXPORT KisBaseIteratorNG : public virtual KisBaseConstIteratorNG, public virtual KisBaseAccessor
//{
//    Q_DISABLE_COPY(KisBaseIteratorNG)
//public:
//    KisBaseIteratorNG() {}
//    virtual ~KisBaseIteratorNG();
//};

/**
 * Iterates over the line of a paint device.
 */
class KRITAIMAGE_EXPORT KisHLineConstIteratorNG : public virtual KisBaseConstIteratorNG
{
    Q_DISABLE_COPY(KisHLineConstIteratorNG)
public:
    KisHLineConstIteratorNG() {}
    virtual ~KisHLineConstIteratorNG();
    /**
     * Move to the next row
     */
    virtual void nextRow() = 0;

    virtual void resetPixelPos() = 0;
    virtual void resetRowPos() = 0;
};

/**
 * Also support writing.
 */
class KRITAIMAGE_EXPORT KisHLineIteratorNG : public KisHLineConstIteratorNG, public KisBaseAccessor
{
    Q_DISABLE_COPY(KisHLineIteratorNG)
public:
    KisHLineIteratorNG() {}
    virtual ~KisHLineIteratorNG();
};

/**
 * Iterates over the column of a paint device.
 */
class KRITAIMAGE_EXPORT KisVLineConstIteratorNG : public virtual KisBaseConstIteratorNG
{
    Q_DISABLE_COPY(KisVLineConstIteratorNG)
public:
    KisVLineConstIteratorNG() {}
    virtual ~KisVLineConstIteratorNG();
    /**
     * Move to the next row
     */
    virtual void nextColumn() = 0;

    virtual void resetPixelPos() = 0;
    virtual void resetColumnPos() = 0;
};

/**
 * Also support writing.
 */
class KRITAIMAGE_EXPORT KisVLineIteratorNG : public KisVLineConstIteratorNG, public KisBaseAccessor
{
    Q_DISABLE_COPY(KisVLineIteratorNG)
public:
    KisVLineIteratorNG() {}
    virtual ~KisVLineIteratorNG();
};

#include "kis_sequential_iterator.h"

#endif
