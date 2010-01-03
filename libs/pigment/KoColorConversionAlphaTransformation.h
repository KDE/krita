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

#ifndef _KO_COLOR_CONVERSION_ALPHA_TRANSFORMATION_H_
#define _KO_COLOR_CONVERSION_ALPHA_TRANSFORMATION_H_

#include "KoColorConversionTransformation.h"
#include "KoColorConversionTransformationFactory.h"

/**
 * Create converter from the alpha color space to any color space
 * This class is for use by the KoColorConversionSystemn, no reason
 * to use it directly.
 */
class KoColorConversionFromAlphaTransformationFactory : public KoColorConversionTransformationFactory
{
public:
    KoColorConversionFromAlphaTransformationFactory(const QString& _dstModelId, const QString& _dstDepthId, const QString& _dstProfileName);
    virtual KoColorConversionTransformation* createColorTransformation(const KoColorSpace* srcColorSpace, const KoColorSpace* dstColorSpace, KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual) const;
    virtual bool conserveColorInformation() const;
    virtual bool conserveDynamicRange() const;
};

/**
 * Create converter to the alpha color space to any color space
 * This class is for use by the KoColorConversionSystemn, no reason
 * to use it directly.
 */
class KoColorConversionToAlphaTransformationFactory : public KoColorConversionTransformationFactory
{
public:
    KoColorConversionToAlphaTransformationFactory(const QString& _dstModelId, const QString& _dstDepthId, const QString& _srcProfileName);
    virtual KoColorConversionTransformation* createColorTransformation(const KoColorSpace* srcColorSpace, const KoColorSpace* dstColorSpace, KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual) const;
    virtual bool conserveColorInformation() const;
    virtual bool conserveDynamicRange() const;
};

#endif
