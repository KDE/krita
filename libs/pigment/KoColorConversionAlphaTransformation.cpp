/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "KoColorConversionAlphaTransformation.h"

#include "KoColorSpace.h"
#include "KoIntegerMaths.h"
#include "KoColorSpaceTraits.h"
#include "KoColorModelStandardIds.h"
#include "KoColorModelStandardIdsUtils.h"

/**
 * Converter from the alpha color space to any color space
 */
template <typename alpha_channel_type>
class KoColorConversionFromAlphaTransformation : public KoColorConversionTransformation
{
public:
    KoColorConversionFromAlphaTransformation(const KoColorSpace* srcCs, const KoColorSpace* dstCs,
                                             Intent renderingIntent,
                                             KoColorConversionTransformation::ConversionFlags conversionFlags)
        : KoColorConversionTransformation(srcCs, dstCs, renderingIntent, conversionFlags)
    {
    }

    void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const override
    {
        const alpha_channel_type *srcPtr = reinterpret_cast<const alpha_channel_type*>(src);

        quint16 data[4];
        const qint32 pixelSize = dstColorSpace()->pixelSize();

        data[1] = UINT16_MAX / 2;   // a
        data[2] = UINT16_MAX / 2;   // b
        data[3] = UINT16_MAX;       // A

        while (nPixels > 0) {
            data[0] = KoColorSpaceMaths<alpha_channel_type, quint16>::scaleToA(*srcPtr); // L
            dstColorSpace()->fromLabA16((quint8*)data, dst, 1);

            srcPtr++;
            dst += pixelSize;
            nPixels--;
        }
    }
};

template <typename alpha_channel_type>
class KoColorConversionAlphaToLab16Transformation : public KoColorConversionTransformation
{
public:
    KoColorConversionAlphaToLab16Transformation(const KoColorSpace* srcCs, const KoColorSpace* dstCs,
                                             Intent renderingIntent,
                                             KoColorConversionTransformation::ConversionFlags conversionFlags)
        : KoColorConversionTransformation(srcCs, dstCs, renderingIntent, conversionFlags)
    {
    }

    void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const override
    {
        const alpha_channel_type *srcPtr = reinterpret_cast<const alpha_channel_type*>(src);
        quint16 *dstPtr = reinterpret_cast<quint16*>(dst);

        while (nPixels > 0) {
            dstPtr[0] = KoColorSpaceMaths<alpha_channel_type, quint16>::scaleToA(*srcPtr); // L
            dstPtr[1] = UINT16_MAX / 2;   // a
            dstPtr[2] = UINT16_MAX / 2;   // b
            dstPtr[3] = UINT16_MAX;       // A

            srcPtr++;
            dstPtr += 4;
            nPixels--;
        }
    }
};


template <typename alpha_channel_type, typename gray_channel_type>
class KoColorConversionGrayAFromAlphaTransformation : public KoColorConversionTransformation
{
public:
    KoColorConversionGrayAFromAlphaTransformation(const KoColorSpace* srcCs,
                                                  const KoColorSpace* dstCs,
                                                  Intent renderingIntent,
                                                  KoColorConversionTransformation::ConversionFlags conversionFlags)
        : KoColorConversionTransformation(srcCs, dstCs, renderingIntent, conversionFlags)
    {
    }
    void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const override {
        const alpha_channel_type *srcPtr = reinterpret_cast<const alpha_channel_type*>(src);
        gray_channel_type *dstPtr = reinterpret_cast<gray_channel_type*>(dst);

        while (nPixels > 0) {
            dstPtr[0] = KoColorSpaceMaths<alpha_channel_type, gray_channel_type>::scaleToA(*srcPtr);
            dstPtr[1] = KoColorSpaceMathsTraits<gray_channel_type>::unitValue;

            srcPtr++;
            dstPtr += 2;
            nPixels--;
        }
    }
};

//------ KoColorConversionFromAlphaTransformationFactoryImpl ------//

template<typename alpha_channel_type>
KoColorConversionFromAlphaTransformationFactoryImpl<alpha_channel_type>::
    KoColorConversionFromAlphaTransformationFactoryImpl(const QString& _dstModelId, const QString& _dstDepthId, const QString& _dstProfileName)
        : KoColorConversionTransformationFactory(AlphaColorModelID.id(),
                                                 colorDepthIdForChannelType<alpha_channel_type>().id(),
                                                 "default",
                                                 _dstModelId, _dstDepthId, _dstProfileName)
{
}

template<typename alpha_channel_type>
KoColorConversionTransformation*
KoColorConversionFromAlphaTransformationFactoryImpl<alpha_channel_type>::
    createColorTransformation(const KoColorSpace* srcColorSpace,
                              const KoColorSpace* dstColorSpace,
                              KoColorConversionTransformation::Intent renderingIntent,
                              KoColorConversionTransformation::ConversionFlags conversionFlags) const
{
    Q_ASSERT(canBeSource(srcColorSpace));
    Q_ASSERT(canBeDestination(dstColorSpace));

    if (dstColorSpace->colorModelId() == GrayAColorModelID &&
        dstColorSpace->colorDepthId() == Integer8BitsColorDepthID) {
        return new KoColorConversionGrayAFromAlphaTransformation<alpha_channel_type, quint8>(srcColorSpace, dstColorSpace, renderingIntent, conversionFlags);

    } else if (dstColorSpace->colorModelId() == GrayAColorModelID &&
               dstColorSpace->colorDepthId() == Integer16BitsColorDepthID) {
            return new KoColorConversionGrayAFromAlphaTransformation<alpha_channel_type, quint16>(srcColorSpace, dstColorSpace, renderingIntent, conversionFlags);

    #ifdef HAVE_OPENEXR
    } else if (dstColorSpace->colorModelId() == GrayAColorModelID &&
               dstColorSpace->colorDepthId() == Float16BitsColorDepthID) {
            return new KoColorConversionGrayAFromAlphaTransformation<alpha_channel_type, half>(srcColorSpace, dstColorSpace, renderingIntent, conversionFlags);
    #endif

    } else if (dstColorSpace->colorModelId() == GrayAColorModelID &&
               dstColorSpace->colorDepthId() == Float32BitsColorDepthID) {
            return new KoColorConversionGrayAFromAlphaTransformation<alpha_channel_type, float>(srcColorSpace, dstColorSpace, renderingIntent, conversionFlags);

    } else if (dstColorSpace->colorModelId() == LABAColorModelID &&
               dstColorSpace->colorDepthId() == Integer16BitsColorDepthID) {
            return new KoColorConversionAlphaToLab16Transformation<alpha_channel_type>(srcColorSpace, dstColorSpace, renderingIntent, conversionFlags);

    } else {
        return new KoColorConversionFromAlphaTransformation<alpha_channel_type>(srcColorSpace, dstColorSpace, renderingIntent, conversionFlags);
    }
}

template<typename alpha_channel_type>
bool
KoColorConversionFromAlphaTransformationFactoryImpl<alpha_channel_type>::
    conserveColorInformation() const
{
    return false;
}

template<typename alpha_channel_type>
bool
KoColorConversionFromAlphaTransformationFactoryImpl<alpha_channel_type>::
    conserveDynamicRange() const
{
    return false;
}

template class KoColorConversionFromAlphaTransformationFactoryImpl<quint8>;
template class KoColorConversionFromAlphaTransformationFactoryImpl<quint16>;
#ifdef HAVE_OPENEXR
template class KoColorConversionFromAlphaTransformationFactoryImpl<half>;
#endif
template class KoColorConversionFromAlphaTransformationFactoryImpl<float>;

//------ KoColorConversionToAlphaTransformation ------//

/**
 * Converter to the alpha color space to any color space
 */
template <typename alpha_channel_type>
class KoColorConversionToAlphaTransformation : public KoColorConversionTransformation
{
public:
    KoColorConversionToAlphaTransformation(const KoColorSpace* srcCs, const KoColorSpace* dstCs, Intent renderingIntent, ConversionFlags conversionFlags)
        : KoColorConversionTransformation(srcCs, dstCs, renderingIntent, conversionFlags)
    {
    }

    void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const override {
        alpha_channel_type *dstPtr = reinterpret_cast<alpha_channel_type*>(dst);

        quint16 data[4];
        qint32 pixelSize = srcColorSpace()->pixelSize();

        while (nPixels > 0) {
            srcColorSpace()->toLabA16(src, (quint8*)data, 1);
            *dstPtr = KoColorSpaceMaths<quint16, alpha_channel_type>::scaleToA(UINT16_MULT(data[0], data[3])); // L * A

            src += pixelSize;
            dstPtr ++;
            nPixels --;

        }
    }
};

template <typename alpha_channel_type>
class KoColorConversionLab16ToAlphaTransformation : public KoColorConversionTransformation
{
public:
    KoColorConversionLab16ToAlphaTransformation(const KoColorSpace* srcCs, const KoColorSpace* dstCs,
                                                Intent renderingIntent,
                                                KoColorConversionTransformation::ConversionFlags conversionFlags)
        : KoColorConversionTransformation(srcCs, dstCs, renderingIntent, conversionFlags)
    {
    }

    void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const override
    {
        const quint16 *srcPtr = reinterpret_cast<const quint16*>(src);
        alpha_channel_type *dstPtr = reinterpret_cast<alpha_channel_type*>(dst);

        while (nPixels > 0) {
            *dstPtr = KoColorSpaceMaths<quint16, alpha_channel_type>::scaleToA(UINT16_MULT(srcPtr[0], srcPtr[3])); // L * A

            srcPtr += 4;
            dstPtr++;
            nPixels--;
        }
    }
};


//------ KoColorConversionGrayAU8ToAlphaTransformation ------//

template <typename gray_channel_type, typename alpha_channel_type>
class KoColorConversionGrayAToAlphaTransformation : public KoColorConversionTransformation
{
public:
    KoColorConversionGrayAToAlphaTransformation(const KoColorSpace* srcCs,
                                                  const KoColorSpace* dstCs,
                                                  Intent renderingIntent,
                                                  ConversionFlags conversionFlags)
        : KoColorConversionTransformation(srcCs, dstCs, renderingIntent, conversionFlags)
    {
    }

    void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const override
    {
        const gray_channel_type *srcPtr = reinterpret_cast<const gray_channel_type*>(src);
        alpha_channel_type *dstPtr = reinterpret_cast<alpha_channel_type*>(dst);

        while (nPixels > 0) {
            *dstPtr = KoColorSpaceMaths<gray_channel_type, alpha_channel_type>::scaleToA(
                        KoColorSpaceMaths<gray_channel_type>::multiply(srcPtr[0], srcPtr[1]));

            srcPtr += 2;
            dstPtr++;
            nPixels --;
        }
    }
};

//------ KoColorConversionToAlphaTransformationFactoryImpl ------//

template <typename alpha_channel_type>
KoColorConversionToAlphaTransformationFactoryImpl<alpha_channel_type>::
    KoColorConversionToAlphaTransformationFactoryImpl(const QString& _srcModelId, const QString& _srcDepthId, const QString& _srcProfileName)
        : KoColorConversionTransformationFactory(_srcModelId, _srcDepthId, _srcProfileName,
                                                 AlphaColorModelID.id(), colorDepthIdForChannelType<alpha_channel_type>().id(), "default")
{
}

template <typename alpha_channel_type>
KoColorConversionTransformation*
KoColorConversionToAlphaTransformationFactoryImpl<alpha_channel_type>::
    createColorTransformation(const KoColorSpace* srcColorSpace,
                              const KoColorSpace* dstColorSpace,
                              KoColorConversionTransformation::Intent renderingIntent,
                              KoColorConversionTransformation::ConversionFlags conversionFlags) const
{
    Q_ASSERT(canBeSource(srcColorSpace));
    Q_ASSERT(canBeDestination(dstColorSpace));

    if (srcColorSpace->colorModelId() == GrayAColorModelID &&
        srcColorSpace->colorDepthId() == Integer8BitsColorDepthID) {
        return new KoColorConversionGrayAToAlphaTransformation<quint8, alpha_channel_type>(srcColorSpace, dstColorSpace, renderingIntent, conversionFlags);

    } else if (srcColorSpace->colorModelId() == GrayAColorModelID &&
               srcColorSpace->colorDepthId() == Integer16BitsColorDepthID) {
            return new KoColorConversionGrayAToAlphaTransformation<quint16, alpha_channel_type>(srcColorSpace, dstColorSpace, renderingIntent, conversionFlags);

#ifdef HAVE_OPENEXR
    } else if (srcColorSpace->colorModelId() == GrayAColorModelID &&
               srcColorSpace->colorDepthId() == Float16BitsColorDepthID) {
            return new KoColorConversionGrayAToAlphaTransformation<half, alpha_channel_type>(srcColorSpace, dstColorSpace, renderingIntent, conversionFlags);
#endif

    } else if (srcColorSpace->colorModelId() == GrayAColorModelID &&
               srcColorSpace->colorDepthId() == Float32BitsColorDepthID) {
            return new KoColorConversionGrayAToAlphaTransformation<float, alpha_channel_type>(srcColorSpace, dstColorSpace, renderingIntent, conversionFlags);

    } else if (srcColorSpace->colorModelId() == LABAColorModelID &&
               srcColorSpace->colorDepthId() == Integer16BitsColorDepthID) {
            return new KoColorConversionLab16ToAlphaTransformation<alpha_channel_type>(srcColorSpace, dstColorSpace, renderingIntent, conversionFlags);

    } else {
        return new KoColorConversionToAlphaTransformation<alpha_channel_type>(srcColorSpace, dstColorSpace, renderingIntent, conversionFlags);
    }
}

template <typename alpha_channel_type>
bool
KoColorConversionToAlphaTransformationFactoryImpl<alpha_channel_type>::
    conserveColorInformation() const
{
    return false;
}

template <typename alpha_channel_type>
bool
KoColorConversionToAlphaTransformationFactoryImpl<alpha_channel_type>::
    conserveDynamicRange() const
{
    return false;
}

template class KoColorConversionToAlphaTransformationFactoryImpl<quint8>;
template class KoColorConversionToAlphaTransformationFactoryImpl<quint16>;
#ifdef HAVE_OPENEXR
template class KoColorConversionToAlphaTransformationFactoryImpl<half>;
#endif
template class KoColorConversionToAlphaTransformationFactoryImpl<float>;
