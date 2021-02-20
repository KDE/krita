/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _KO_COLOR_CONVERSION_ALPHA_TRANSFORMATION_H_
#define _KO_COLOR_CONVERSION_ALPHA_TRANSFORMATION_H_

#include "KoColorConversionTransformation.h"
#include "KoColorConversionTransformationFactory.h"

#include <KoConfig.h>
#ifdef HAVE_OPENEXR
#include <half.h>
#endif

/**
 * Create converter from the alpha color space to any color space
 * This class is for use by the KoColorConversionSystemn, no reason
 * to use it directly.
 */
template<typename alpha_channel_type>
class KoColorConversionFromAlphaTransformationFactoryImpl : public KoColorConversionTransformationFactory
{
public:
    KoColorConversionFromAlphaTransformationFactoryImpl(const QString& _dstModelId, const QString& _dstDepthId, const QString& _dstProfileName);
    KoColorConversionTransformation* createColorTransformation(const KoColorSpace* srcColorSpace,
                                                                       const KoColorSpace* dstColorSpace,
                                                                       KoColorConversionTransformation::Intent renderingIntent,
                                                                       KoColorConversionTransformation::ConversionFlags conversionFlags) const override;
    bool conserveColorInformation() const override;
    bool conserveDynamicRange() const override;
};

typedef KoColorConversionFromAlphaTransformationFactoryImpl<quint8> KoColorConversionFromAlphaTransformationFactory;
typedef KoColorConversionFromAlphaTransformationFactoryImpl<quint16> KoColorConversionFromAlphaU16TransformationFactory;
#ifdef HAVE_OPENEXR
typedef KoColorConversionFromAlphaTransformationFactoryImpl<half> KoColorConversionFromAlphaF16TransformationFactory;
#endif
typedef KoColorConversionFromAlphaTransformationFactoryImpl<float> KoColorConversionFromAlphaF32TransformationFactory;

/**
 * Create converter to the alpha color space to any color space
 * This class is for use by the KoColorConversionSystemn, no reason
 * to use it directly.
 */
template <typename alpha_channel_type>
class KoColorConversionToAlphaTransformationFactoryImpl : public KoColorConversionTransformationFactory
{
public:
    KoColorConversionToAlphaTransformationFactoryImpl(const QString& _dstModelId, const QString& _dstDepthId, const QString& _srcProfileName);
    KoColorConversionTransformation* createColorTransformation(const KoColorSpace* srcColorSpace,
                                                                       const KoColorSpace* dstColorSpace,
                                                                       KoColorConversionTransformation::Intent renderingIntent,
                                                                       KoColorConversionTransformation::ConversionFlags conversionFlags) const override;
    bool conserveColorInformation() const override;
    bool conserveDynamicRange() const override;
};

typedef KoColorConversionToAlphaTransformationFactoryImpl<quint8> KoColorConversionToAlphaTransformationFactory;
typedef KoColorConversionToAlphaTransformationFactoryImpl<quint16> KoColorConversionToAlphaU16TransformationFactory;
#ifdef HAVE_OPENEXR
typedef KoColorConversionToAlphaTransformationFactoryImpl<half> KoColorConversionToAlphaF16TransformationFactory;
#endif
typedef KoColorConversionToAlphaTransformationFactoryImpl<float> KoColorConversionToAlphaF32TransformationFactory;

#endif
