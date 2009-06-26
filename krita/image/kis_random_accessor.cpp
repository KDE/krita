/*
 * This file is part of the Krita project
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#include "kis_random_accessor.h"

#include <kis_shared_ptr.h>

#include <config-tiles.h> // For the next define
#include KIS_TILED_RANDOM_ACCESSOR_HEADER
#include "kis_datamanager.h"



typedef KisSharedPtr<KisTiledRandomAccessor> KisTiledRandomAccessorSP;

struct KisRandomConstAccessor::Private {
    KisTiledRandomAccessorSP accessor;
    qint32 offsetx, offsety;
};

KisRandomConstAccessor::KisRandomConstAccessor(KisDataManager *ktm, qint32 x, qint32 y, qint32 offsetx, qint32 offsety, bool writable) : d(new Private)
{
    d->offsetx = offsetx;
    d->offsety = offsety;
    d->accessor = new KisTiledRandomAccessor(ktm, x, y, writable);
}

KisRandomConstAccessor::KisRandomConstAccessor(KisDataManager *ktm, qint32 x, qint32 y, qint32 offsetx, qint32 offsety) : d(new Private)
{
    d->offsetx = offsetx;
    d->offsety = offsety;
    d->accessor = new KisTiledRandomAccessor(ktm, x, y, false);
}

KisRandomConstAccessor::KisRandomConstAccessor(const KisRandomConstAccessor& rhs) : d(new Private)
{
    d->accessor = rhs.d->accessor;
    d->offsetx = rhs.d->offsetx;
    d->offsety = rhs.d->offsety;
}

KisRandomConstAccessor::~KisRandomConstAccessor()
{
    delete d;
}

void KisRandomConstAccessor::moveTo(qint32 x, qint32 y)
{
    d->accessor->moveTo(x - d->offsetx, y  - d->offsety);
}

const quint8* KisRandomConstAccessor::rawData() const
{
    return d->accessor->rawData();
}

quint8* KisRandomAccessor::rawData() const
{
    return d->accessor->rawData();
}

const quint8* KisRandomConstAccessor::oldRawData() const
{
    return d->accessor->oldRawData();
}

