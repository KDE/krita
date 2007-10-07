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

#ifndef _KO_COPY_COLOR_CONVERSION_TRANSFORMATION_H_
#define _KO_COPY_COLOR_CONVERSION_TRANSFORMATION_H_

#include "KoColorConversionTransformation.h"
#include "KoColorConversionTransformationFactory.h"

class KoCopyColorConversionTransformation : public KoColorConversionTransformation {
    public:
        KoCopyColorConversionTransformation(const KoColorSpace* cs) : KoColorConversionTransformation(cs, cs)
        {
        }
        virtual void transform(const quint8 *srcU8, quint8 *dstU8, qint32 nPixels) const
        {
            memcpy(dstU8, srcU8, nPixels * srcColorSpace()->pixelSize());
        }
};

class KoCopyColorConversionTransformationFactory : public KoColorConversionTransformationFactory {
    public:
        KoCopyColorConversionTransformationFactory(QString colorModelId, QString depthId) : KoColorConversionTransformationFactory(colorModelId, depthId, colorModelId, depthId)
        {}
        virtual KoColorConversionTransformation* createColorTransformation(const KoColorSpace* srcColorSpace, const KoColorSpace* dstColorSpace, KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual)
        {
            Q_UNUSED(renderingIntent);
            Q_ASSERT(canBeSource(srcColorSpace));
            Q_ASSERT(canBeDestination(dstColorSpace));
            Q_ASSERT(srcColorSpace == dstColorSpace);
            return new KoCopyColorConversionTransformation(srcColorSpace);
        }
        virtual bool conserveColorInformation() const
        {
            return true;
        }
        virtual bool conserveDynamicRange() const
        {
            return true;
        }
        virtual int depthDecrease() const
        {
            return 0;
        }
};


#endif
