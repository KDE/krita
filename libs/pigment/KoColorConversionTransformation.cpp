/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoColorConversionTransformation.h"

#include "KoColorSpace.h"

struct KoColorConversionTransformation::Private {
    const KoColorSpace* srcColorSpace;
    const KoColorSpace* dstColorSpace;
    Intent renderingIntent;
};

KoColorConversionTransformation::KoColorConversionTransformation(const KoColorSpace* srcCs, const KoColorSpace* dstCs, Intent renderingIntent) : d(new Private)
{
    d->srcColorSpace = srcCs;
    d->dstColorSpace = dstCs;
    d->renderingIntent = renderingIntent;
}

const KoColorSpace* KoColorConversionTransformation::srcColorSpace() const
{
    return d->srcColorSpace;
}

const KoColorSpace* KoColorConversionTransformation::dstColorSpace() const
{
    return d->dstColorSpace;
}

KoColorConversionTransformation::Intent KoColorConversionTransformation::renderingIntent()
{
    return d->renderingIntent;
}
