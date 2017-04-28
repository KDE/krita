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
    virtual KoColorConversionTransformation* createColorTransformation(const KoColorSpace* srcColorSpace,
                                                                       const KoColorSpace* dstColorSpace,
                                                                       KoColorConversionTransformation::Intent renderingIntent,
                                                                       KoColorConversionTransformation::ConversionFlags conversionFlags) const;
    virtual bool conserveColorInformation() const;
    virtual bool conserveDynamicRange() const;
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
    virtual KoColorConversionTransformation* createColorTransformation(const KoColorSpace* srcColorSpace,
                                                                       const KoColorSpace* dstColorSpace,
                                                                       KoColorConversionTransformation::Intent renderingIntent,
                                                                       KoColorConversionTransformation::ConversionFlags conversionFlags) const;
    virtual bool conserveColorInformation() const;
    virtual bool conserveDynamicRange() const;
};

typedef KoColorConversionToAlphaTransformationFactoryImpl<quint8> KoColorConversionToAlphaTransformationFactory;
typedef KoColorConversionToAlphaTransformationFactoryImpl<quint16> KoColorConversionToAlphaU16TransformationFactory;
#ifdef HAVE_OPENEXR
typedef KoColorConversionToAlphaTransformationFactoryImpl<half> KoColorConversionToAlphaF16TransformationFactory;
#endif
typedef KoColorConversionToAlphaTransformationFactoryImpl<float> KoColorConversionToAlphaF32TransformationFactory;

#endif
