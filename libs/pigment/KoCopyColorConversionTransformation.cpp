/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "KoCopyColorConversionTransformation.h"

#include <KoColorSpace.h>

// --- KoCopyColorConversionTransformation ---
KoCopyColorConversionTransformation::KoCopyColorConversionTransformation(const KoColorSpace* cs)
        : KoColorConversionTransformation(cs, cs, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags())
{
}
void KoCopyColorConversionTransformation::transform(const quint8 *srcU8, quint8 *dstU8, qint32 nPixels) const
{
    memcpy(dstU8, srcU8, nPixels * srcColorSpace()->pixelSize());
}

// --- KoCopyColorConversionTransformationFactory ---
KoCopyColorConversionTransformationFactory::KoCopyColorConversionTransformationFactory(const QString& _colorModelId, const QString& _depthId, const QString& _profileName) : KoColorConversionTransformationFactory(_colorModelId, _depthId, _profileName, _colorModelId, _depthId, _profileName)
{}
KoColorConversionTransformation* KoCopyColorConversionTransformationFactory::createColorTransformation(const KoColorSpace* srcColorSpace, const KoColorSpace* dstColorSpace, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags) const
{
    Q_UNUSED(renderingIntent);
    Q_UNUSED(conversionFlags);
#ifdef QT_NO_DEBUG
    Q_UNUSED(dstColorSpace);
#endif
    Q_UNUSED(dstColorSpace);
    Q_ASSERT(canBeSource(srcColorSpace));
    Q_ASSERT(canBeDestination(dstColorSpace));
    Q_ASSERT(srcColorSpace->id() == dstColorSpace->id());
    return new KoCopyColorConversionTransformation(srcColorSpace);
}
bool KoCopyColorConversionTransformationFactory::conserveColorInformation() const
{
    return true;
}
bool KoCopyColorConversionTransformationFactory::conserveDynamicRange() const
{
    return true;
}
