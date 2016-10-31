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

#include "KoColorConversionAlphaTransformation.h"

#include "KoColorSpace.h"
#include "KoIntegerMaths.h"
#include "KoColorSpaceTraits.h"
#include "KoColorModelStandardIds.h"

/**
 * Converter from the alpha color space to any color space
 */
class KoColorConversionFromAlphaTransformation : public KoColorConversionTransformation
{
public:
    KoColorConversionFromAlphaTransformation(const KoColorSpace* srcCs, const KoColorSpace* dstCs,
                                             Intent renderingIntent,
                                             KoColorConversionTransformation::ConversionFlags conversionFlags);
    void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const override;
};

//------ KoColorConversionFromAlphaTransformation ------//

KoColorConversionFromAlphaTransformation::KoColorConversionFromAlphaTransformation(const KoColorSpace* srcCs,
                                                                                   const KoColorSpace* dstCs,
                                                                                   Intent renderingIntent,
                                                                                   ConversionFlags conversionFlags)
    : KoColorConversionTransformation(srcCs, dstCs, renderingIntent, conversionFlags)
{

}

void KoColorConversionFromAlphaTransformation::transform(const quint8 *src, quint8 *dst, qint32 nPixels) const
{
    quint16 data[4];
    qint32 size = dstColorSpace()->pixelSize();

    data[1] = UINT16_MAX / 2;   // a
    data[2] = UINT16_MAX / 2;   // b
    data[3] = UINT16_MAX;       // A

    while (nPixels > 0) {

        data[0] = UINT8_TO_UINT16(*src); // L
        dstColorSpace()->fromLabA16((quint8*)data, dst, 1);

        src ++;
        dst += size;
        nPixels--;

    }
}

class KoColorConversionGrayAU8FromAlphaTransformation : public KoColorConversionTransformation
{
public:
    KoColorConversionGrayAU8FromAlphaTransformation(const KoColorSpace* srcCs,
                                                    const KoColorSpace* dstCs,
                                                    Intent renderingIntent,
                                                    KoColorConversionTransformation::ConversionFlags conversionFlags)
        : KoColorConversionTransformation(srcCs, dstCs, renderingIntent, conversionFlags)
    {
    }
    void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const override {
        while (nPixels > 0) {

            dst[0] = *src;
            dst[1] = 255;

            src ++;
            dst += 2;
            nPixels--;
        }
    }
};

//------ KoColorConversionFromAlphaTransformationFactory ------//

KoColorConversionFromAlphaTransformationFactory::KoColorConversionFromAlphaTransformationFactory(const QString& _dstModelId, const QString& _dstDepthId, const QString& _dstProfileName)
    : KoColorConversionTransformationFactory(AlphaColorModelID.id(), Integer8BitsColorDepthID.id(), "", _dstModelId, _dstDepthId, _dstProfileName)
{
}

KoColorConversionTransformation* KoColorConversionFromAlphaTransformationFactory::createColorTransformation(const KoColorSpace* srcColorSpace,
                                                                                                            const KoColorSpace* dstColorSpace,
                                                                                                            KoColorConversionTransformation::Intent renderingIntent,
                                                                                                            KoColorConversionTransformation::ConversionFlags conversionFlags) const
{
    Q_ASSERT(canBeSource(srcColorSpace));
    Q_ASSERT(canBeDestination(dstColorSpace));

    if (dstColorSpace->id() == "GRAYA") {
        return new KoColorConversionGrayAU8FromAlphaTransformation(srcColorSpace, dstColorSpace, renderingIntent, conversionFlags);
    } else {
        return new KoColorConversionFromAlphaTransformation(srcColorSpace, dstColorSpace, renderingIntent, conversionFlags);
    }
}

bool KoColorConversionFromAlphaTransformationFactory::conserveColorInformation() const
{
    return false;
}

bool KoColorConversionFromAlphaTransformationFactory::conserveDynamicRange() const
{
    return false;
}

//------ KoColorConversionToAlphaTransformation ------//

/**
 * Converter to the alpha color space to any color space
 */
class KoColorConversionToAlphaTransformation : public KoColorConversionTransformation
{
public:
    KoColorConversionToAlphaTransformation(const KoColorSpace* srcCs, const KoColorSpace* dstCs, Intent renderingIntent, ConversionFlags conversionFlags);
    void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const override;
};


KoColorConversionToAlphaTransformation::KoColorConversionToAlphaTransformation(const KoColorSpace* srcCs,
                                                                               const KoColorSpace* dstCs,
                                                                               Intent renderingIntent,
                                                                               ConversionFlags conversionFlags)
    : KoColorConversionTransformation(srcCs, dstCs, renderingIntent, conversionFlags)
{

}

void KoColorConversionToAlphaTransformation::transform(const quint8 *src, quint8 *dst, qint32 nPixels) const
{
    quint16 data[4];
    qint32 size = srcColorSpace()->pixelSize();

    while (nPixels > 0) {

        srcColorSpace()->toLabA16(src, (quint8*)data, 1);
        *dst = UINT16_TO_UINT8(UINT16_MULT(data[0], data[3])); // L * A

        src += size;
        dst ++;
        nPixels --;

    }
}

//------ KoColorConversionGrayAU8ToAlphaTransformation ------//

class KoColorConversionGrayAU8ToAlphaTransformation : public KoColorConversionTransformation
{
public:
    KoColorConversionGrayAU8ToAlphaTransformation(const KoColorSpace* srcCs,
                                                  const KoColorSpace* dstCs,
                                                  Intent renderingIntent,
                                                  ConversionFlags conversionFlags)
        : KoColorConversionTransformation(srcCs, dstCs, renderingIntent, conversionFlags)
    {
    }

    void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const override
    {
        while (nPixels > 0) {

            *dst = UINT8_MULT(src[0], src[1]);

            src += 2;
            dst ++;
            nPixels --;
        }
    }
};

//------ KoColorConversionToAlphaTransformationFactory ------//

KoColorConversionToAlphaTransformationFactory::KoColorConversionToAlphaTransformationFactory(const QString& _srcModelId, const QString& _srcDepthId, const QString& _srcProfileName)
        : KoColorConversionTransformationFactory(_srcModelId, _srcDepthId, _srcProfileName, AlphaColorModelID.id(), Integer8BitsColorDepthID.id(), "")
{
}

KoColorConversionTransformation* KoColorConversionToAlphaTransformationFactory::createColorTransformation(const KoColorSpace* srcColorSpace,
                                                                                                          const KoColorSpace* dstColorSpace,
                                                                                                          KoColorConversionTransformation::Intent renderingIntent,
                                                                                                          KoColorConversionTransformation::ConversionFlags conversionFlags) const
{
    Q_ASSERT(canBeSource(srcColorSpace));
    Q_ASSERT(canBeDestination(dstColorSpace));

    if (srcColorSpace->id() == "GRAYA") {
        return new KoColorConversionGrayAU8ToAlphaTransformation(srcColorSpace, dstColorSpace, renderingIntent, conversionFlags);
    } else {
        return new KoColorConversionToAlphaTransformation(srcColorSpace, dstColorSpace, renderingIntent, conversionFlags);
    }
}

bool KoColorConversionToAlphaTransformationFactory::conserveColorInformation() const
{
    return false;
}

bool KoColorConversionToAlphaTransformationFactory::conserveDynamicRange() const
{
    return false;
}

