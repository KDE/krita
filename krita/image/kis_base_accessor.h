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

#ifndef _KISBASEACCESSOR_H_
#define _KISBASEACCESSOR_H_

#include <krita_export.h>
#include <kis_shared.h>

class KRITAIMAGE_EXPORT KisBaseConstAccessor : public KisShared
{
    Q_DISABLE_COPY(KisBaseConstAccessor)
public:
    KisBaseConstAccessor() {}
    virtual ~KisBaseConstAccessor();
    /**
     * @return a pointer to the pixel data as it was at the moment of the last memento creation.
     */
    virtual const quint8 * oldRawData() const = 0;

    /**
     * @return a pointer to the most actual pixel data,
     * this points to te same data as rawData() method of
     * a writable accessor
     */
    virtual const quint8 * rawDataConst() const = 0;

    /**
     * @return return number of consequential numbers of pixels, useful for optimization
     */
    virtual qint32 nConseqPixels() const = 0;
    virtual qint32 x() const = 0;
    virtual qint32 y() const = 0;
};

class KRITAIMAGE_EXPORT KisBaseAccessor
{
    Q_DISABLE_COPY(KisBaseAccessor)
public:
    KisBaseAccessor() {}
    virtual ~KisBaseAccessor();
    /**
     * @return a pointer to the pixel data. Do NOT interpret the data - leave that to a colorspace
     */
    virtual quint8 * rawData() = 0;
};

#endif
