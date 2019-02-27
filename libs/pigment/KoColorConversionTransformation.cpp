/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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

struct Q_DECL_HIDDEN KoColorConversionTransformation::Private {
    const KoColorSpace* srcColorSpace;
    const KoColorSpace* dstColorSpace;
    Intent renderingIntent;
    ConversionFlags conversionFlags;
};

KoColorConversionTransformation::KoColorConversionTransformation(const KoColorSpace* srcCs,
                                                                 const KoColorSpace* dstCs,
                                                                 Intent renderingIntent,
                                                                 ConversionFlags conversionFlags)
    : d(new Private)
{
    Q_ASSERT(srcCs);
    Q_ASSERT(dstCs);

    d->srcColorSpace = srcCs;
    d->dstColorSpace = dstCs;
    d->renderingIntent = renderingIntent;
    d->conversionFlags = conversionFlags;
}

KoColorConversionTransformation::~KoColorConversionTransformation()
{
    delete d;
}

const KoColorSpace* KoColorConversionTransformation::srcColorSpace() const
{
    return d->srcColorSpace;
}

const KoColorSpace* KoColorConversionTransformation::dstColorSpace() const
{
    return d->dstColorSpace;
}

KoColorConversionTransformation::Intent KoColorConversionTransformation::renderingIntent() const
{
    return d->renderingIntent;
}

KoColorConversionTransformation::ConversionFlags KoColorConversionTransformation::conversionFlags() const
{
    return d->conversionFlags;
}

void KoColorConversionTransformation::transformInPlace(const quint8 *src, quint8 *dst, qint32 nPixels) const
{
    if (src != dst) {
        transform(src, dst, nPixels);
    } else {
        QByteArray buffer(srcColorSpace()->pixelSize() * nPixels, 0);
        transform(src, reinterpret_cast<quint8*>(buffer.data()), nPixels);
        memcpy(dst, buffer.data(), buffer.size());
    }
}

void KoColorConversionTransformation::setSrcColorSpace(const KoColorSpace* cs) const
{
    Q_ASSERT(*d->srcColorSpace == *cs);
    d->srcColorSpace = cs;
}

void KoColorConversionTransformation::setDstColorSpace(const KoColorSpace* cs) const
{
    Q_ASSERT(*d->dstColorSpace == *cs);
    d->dstColorSpace = cs;
}
