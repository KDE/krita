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

#include "KoColorConversionAlphaTransformation.h"

#include "KoColorSpace.h"
#include "KoColorModelStandardIds.h"

/**
 * Converter from the alpha color space to any color space
 */
class KoColorConversionFromAlphaTransformation : public KoColorConversionTransformation
{
public:
    KoColorConversionFromAlphaTransformation(const KoColorSpace* srcCs, const KoColorSpace* dstCs, Intent renderingIntent = IntentPerceptual);
    virtual void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const;
};

//------ KoColorConversionFromAlphaTransformation ------//

KoColorConversionFromAlphaTransformation::KoColorConversionFromAlphaTransformation(const KoColorSpace* srcCs, const KoColorSpace* dstCs, Intent renderingIntent) : KoColorConversionTransformation(srcCs, dstCs, renderingIntent)
{

}

void KoColorConversionFromAlphaTransformation::transform(const quint8 *src, quint8 *dst, qint32 nPixels) const
{
    qint32 size = dstColorSpace()->pixelSize();

    while (nPixels > 0) {

        dstColorSpace()->setAlpha(dst, *src, 1);

        src ++;
        dst += size;
        nPixels--;

    }
}

//------ KoColorConversionFromAlphaTransformationFactory ------//

KoColorConversionFromAlphaTransformationFactory::KoColorConversionFromAlphaTransformationFactory(const QString& _dstModelId, const QString& _dstDepthId, const QString& _dstProfileName) : KoColorConversionTransformationFactory(AlphaColorModelID.id(), Integer8BitsColorDepthID.id(), "", _dstModelId, _dstDepthId, _dstProfileName)
{
}

KoColorConversionTransformation* KoColorConversionFromAlphaTransformationFactory::createColorTransformation(const KoColorSpace* srcColorSpace, const KoColorSpace* dstColorSpace, KoColorConversionTransformation::Intent renderingIntent) const
{
    Q_ASSERT(canBeSource(srcColorSpace));
    Q_ASSERT(canBeDestination(dstColorSpace));
    return new KoColorConversionFromAlphaTransformation(srcColorSpace, dstColorSpace, renderingIntent);
}

bool KoColorConversionFromAlphaTransformationFactory::conserveColorInformation() const
{
    return true;
}

bool KoColorConversionFromAlphaTransformationFactory::conserveDynamicRange() const
{
    return true;
}

//------ KoColorConversionToAlphaTransformation ------//

/**
 * Converter to the alpha color space to any color space
 */
class KoColorConversionToAlphaTransformation : public KoColorConversionTransformation
{
public:
    KoColorConversionToAlphaTransformation(const KoColorSpace* srcCs, const KoColorSpace* dstCs, Intent renderingIntent = IntentPerceptual);
    virtual void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const;
};


KoColorConversionToAlphaTransformation::KoColorConversionToAlphaTransformation(const KoColorSpace* srcCs, const KoColorSpace* dstCs, Intent renderingIntent) : KoColorConversionTransformation(srcCs, dstCs, renderingIntent)
{

}

void KoColorConversionToAlphaTransformation::transform(const quint8 *src, quint8 *dst, qint32 nPixels) const
{
    qint32 size = srcColorSpace()->pixelSize();

    while (nPixels > 0) {

        *dst = srcColorSpace()->alpha(src);

        src += size;
        dst ++;
        nPixels --;

    }
}


//------ KoColorConversionToAlphaTransformationFactory ------//

KoColorConversionToAlphaTransformationFactory::KoColorConversionToAlphaTransformationFactory(const QString& _srcModelId, const QString& _srcDepthId, const QString& _srcProfileName)
        : KoColorConversionTransformationFactory(_srcModelId, _srcDepthId, _srcProfileName, AlphaColorModelID.id(), Integer8BitsColorDepthID.id(), "")
{
}

KoColorConversionTransformation* KoColorConversionToAlphaTransformationFactory::createColorTransformation(const KoColorSpace* srcColorSpace, const KoColorSpace* dstColorSpace, KoColorConversionTransformation::Intent renderingIntent) const
{
    Q_ASSERT(canBeSource(srcColorSpace));
    Q_ASSERT(canBeDestination(dstColorSpace));
    return new KoColorConversionToAlphaTransformation(srcColorSpace, dstColorSpace, renderingIntent);
}

bool KoColorConversionToAlphaTransformationFactory::conserveColorInformation() const
{
    return false;
}

bool KoColorConversionToAlphaTransformationFactory::conserveDynamicRange() const
{
    return false;
}

