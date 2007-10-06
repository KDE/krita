/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_RGB_TO_RGB_COLOR_CONVERSION_TRANSFORMATION_H_
#define _KIS_RGB_TO_RGB_COLOR_CONVERSION_TRANSFORMATION_H_

#include <KoColorConversionTransformation.h>
#include <KoColorConversionTransformationFactory.h>
/**
 * This transformation allows to convert between two color spaces with the same
 * color model but different channel type.
 */
template<typename _src_CSTraits_, typename _dst_CSTraits_>
class KoScaleColorConversionTransformation : public KoColorConversionTransformation {
    public:
        KoScaleColorConversionTransformation(const KoColorSpace* srcCs, const KoColorSpace* dstCs) : KoColorConversionTransformation(srcCs, dstCs)
        {
            Q_ASSERT(srcCs->colorModelId() == dstCs->colorModelId());
        }
        virtual void transform(const quint8 *srcU8, quint8 *dstU8, qint32 nPixels) const
        {
            const typename _src_CSTraits_::channels_type* src = _src_CSTraits_::nativeArray(srcU8);
            typename _dst_CSTraits_::channels_type* dst = _dst_CSTraits_::nativeArray(dstU8);
            for(quint32 i = 0; i< _src_CSTraits_::channels_nb * nPixels;i++)
            {
                dst[i] = KoColorSpaceMaths<typename _src_CSTraits_::channels_type, typename _dst_CSTraits_::channels_type>::scaleToA(src[i]);
            }
        }
};

/**
 * Factory to create KoScaleColorConversionTransformation.
 */
template<typename _src_CSTraits_, typename _dst_CSTraits_>
class KoScaleColorConversionTransformationFactory : public KoColorConversionTransformationFactory {
    public:
        KoScaleColorConversionTransformationFactory(QString colorModelId, QString _srcDepthId, QString _dstDepthId) : KoColorConversionTransformationFactory(colorModelId,  _srcDepthId, colorModelId, _dstDepthId),
                hdr( ( (srcColorDepthId() == Float16BitsColorDepthID.id()) and 
                       (dstColorDepthId() == Float32BitsColorDepthID.id()) ) or
                     ( (srcColorDepthId() == Float32BitsColorDepthID.id()) and 
                       (dstColorDepthId() == Float16BitsColorDepthID.id()) ) )
        {}
        virtual KoColorConversionTransformation* createColorTransformation(const KoColorSpace* srcColorSpace, const KoColorSpace* dstColorSpace, KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual)
        {
            Q_UNUSED(renderingIntent);
            Q_ASSERT(canBeSource(srcColorSpace));
            Q_ASSERT(canBeDestination(dstColorSpace));
            return new KoScaleColorConversionTransformation<_src_CSTraits_, _dst_CSTraits_>(srcColorSpace, dstColorSpace);
        }
        virtual bool conserveColorInformation() const
        {
            return true;
        }
        virtual bool conserveDynamicRange() const
        {
            return hdr;
        }
        virtual int depthDecrease() const
        {
            return qMax(0, _src_CSTraits_::depth - _dst_CSTraits_::depth);
        }
    private:
        bool hdr;
};

#endif
