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

#include "KoColorProofingConversionTransformation.h"

#include "KoColorSpace.h"

struct Q_DECL_HIDDEN KoColorProofingConversionTransformation::Private {
    const KoColorSpace* srcColorSpace;
    const KoColorSpace* dstColorSpace;
    const KoColorSpace* proofingSpace;
    Intent renderingIntent;
    ConversionFlags conversionFlags;
};

KoColorProofingConversionTransformation::KoColorProofingConversionTransformation(const KoColorSpace* srcCs,
                                                                 const KoColorSpace* dstCs,
                                                                 const KoColorSpace* proofingSpace,
                                                                 Intent renderingIntent,
                                                                 ConversionFlags conversionFlags)
    : d(new Private)
{
    Q_ASSERT(srcCs);
    Q_ASSERT(dstCs);
    Q_ASSERT(proofingSpace);

    d->srcColorSpace = srcCs;
    d->dstColorSpace = dstCs;
    d->proofingSpace = proofingSpace;
    d->renderingIntent = renderingIntent;
    d->conversionFlags = conversionFlags;
}

KoColorProofingConversionTransformation::~KoColorProofingConversionTransformation()
{
    delete d;
}

const KoColorSpace* KoColorProofingConversionTransformation::srcColorSpace() const
{
    return d->srcColorSpace;
}

const KoColorSpace* KoColorProofingConversionTransformation::dstColorSpace() const
{
    return d->dstColorSpace;
}

const KoColorSpace* KoColorProofingConversionTransformation::proofingSpace() const
{
    return d->proofingSpace;
}

KoColorProofingConversionTransformation::Intent KoColorProofingConversionTransformation::renderingIntent() const
{
    return d->renderingIntent;
}

KoColorProofingConversionTransformation::ConversionFlags KoColorProofingConversionTransformation::conversionFlags() const
{
    return d->conversionFlags;
}

void KoColorProofingConversionTransformation::setSrcColorSpace(const KoColorSpace* cs) const
{
    Q_ASSERT(*d->srcColorSpace == *cs);
    d->srcColorSpace = cs;
}

void KoColorProofingConversionTransformation::setDstColorSpace(const KoColorSpace* cs) const
{
    Q_ASSERT(*d->dstColorSpace == *cs);
    d->dstColorSpace = cs;
}

void KoColorProofingConversionTransformation::setProofingSpace(const KoColorSpace* cs) const
{
    Q_ASSERT(*d->proofingSpace == *cs);
    d->proofingSpace = cs;
}

void KoColorProofingConversionTransformation::setIntent(Intent renderingIntent) const
{
    d->renderingIntent = renderingIntent;
}
void KoColorProofingConversionTransformation::setConversionFlags(ConversionFlags conversionFlags) const
{
    d->conversionFlags = conversionFlags;
}
