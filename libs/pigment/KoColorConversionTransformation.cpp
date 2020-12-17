/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
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
