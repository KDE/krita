/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KO_CTL_COLOR_CONVERSION_TRANSFORMATION_H_
#define _KO_CTL_COLOR_CONVERSION_TRANSFORMATION_H_

#include <KoColorConversionTransformation.h>
#include <KoColorConversionTransformationFactory.h>
#include "pigment_export.h"

class KoCtlColorConversionTransformation : public KoColorConversionTransformation
{
public:
    KoCtlColorConversionTransformation(const KoColorSpace* srcCs, const KoColorSpace* dstCs);
    virtual ~KoCtlColorConversionTransformation();
    virtual void transform(const quint8 *src8, quint8 *dst8, qint32 nPixels) const;
private:
    struct Private;
    Private* const d;
};

class PIGMENTCMS_EXPORT KoCtlColorConversionTransformationFactory : public KoColorConversionTransformationFactory
{
public:
    KoCtlColorConversionTransformationFactory(QString _srcModelId, QString _srcDepthId, QString _srcProfile, QString _dstModelId, QString _dstDepthId, QString _dstProfile);
    virtual ~KoCtlColorConversionTransformationFactory();
    virtual KoColorConversionTransformation* createColorTransformation(const KoColorSpace* srcColorSpace, const KoColorSpace* dstColorSpace, KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual) const;
    virtual bool conserveColorInformation() const;
    virtual bool conserveDynamicRange() const;
private:
    struct Private;
    Private* const d;
};

#endif
