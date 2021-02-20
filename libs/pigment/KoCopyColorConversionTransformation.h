/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef _KO_COPY_COLOR_CONVERSION_TRANSFORMATION_H_
#define _KO_COPY_COLOR_CONVERSION_TRANSFORMATION_H_

#include "KoColorConversionTransformation.h"
#include "KoColorConversionTransformationFactory.h"

class KoCopyColorConversionTransformation : public KoColorConversionTransformation
{
public:
    explicit KoCopyColorConversionTransformation(const KoColorSpace *cs);
    void transform(const quint8 *srcU8, quint8 *dstU8, qint32 nPixels) const override;
};

class KoCopyColorConversionTransformationFactory : public KoColorConversionTransformationFactory
{
public:
    KoCopyColorConversionTransformationFactory(const QString& _colorModelId, const QString& _depthId, const QString& _profileName);
    KoColorConversionTransformation* createColorTransformation(const KoColorSpace* srcColorSpace,
                                                                       const KoColorSpace* dstColorSpace,
                                                                       KoColorConversionTransformation::Intent renderingIntent,
                                                                       KoColorConversionTransformation::ConversionFlags conversionFlags) const override;
    bool conserveColorInformation() const override;
    bool conserveDynamicRange() const override;
};


#endif
