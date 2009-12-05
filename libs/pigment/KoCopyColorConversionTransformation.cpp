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

#include "KoCopyColorConversionTransformation.h"

#include <KoColorSpace.h>

// --- KoCopyColorConversionTransformation ---
KoCopyColorConversionTransformation::KoCopyColorConversionTransformation(const KoColorSpace* cs)
    : KoColorConversionTransformation(cs, cs)
{
}
void KoCopyColorConversionTransformation::transform(const quint8 *srcU8, quint8 *dstU8, qint32 nPixels) const
{
    memcpy(dstU8, srcU8, nPixels * srcColorSpace()->pixelSize());
}

// --- KoCopyColorConversionTransformationFactory ---
KoCopyColorConversionTransformationFactory::KoCopyColorConversionTransformationFactory(const QString& _colorModelId, const QString& _depthId, const QString& _profileName) : KoColorConversionTransformationFactory(_colorModelId, _depthId, _profileName, _colorModelId, _depthId, _profileName)
{}
KoColorConversionTransformation* KoCopyColorConversionTransformationFactory::createColorTransformation(const KoColorSpace* srcColorSpace, const KoColorSpace* dstColorSpace, KoColorConversionTransformation::Intent renderingIntent) const
{
    Q_UNUSED(renderingIntent);
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
