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

#ifndef _KIS_EXPOSURE_CORRECTED_RGB_TO_RGB_COLOR_CONVERSION_TRANSFORMATION_H_
#define _KIS_EXPOSURE_CORRECTED_RGB_TO_RGB_COLOR_CONVERSION_TRANSFORMATION_H_

#include <KoColorConversionTransformation.h>
#include <KoColorConversionTransformationFactory.h>

template<typename _src_CSTraits_, typename _dst_CSTraits_>
class KisExposureCorrectedIntegerRgbToFloatRgbConversionTransformation : public KoColorConversionTransformation {
    public:
        KisExposureCorrectedIntegerRgbToFloatRgbConversionTransformation(const KoColorSpace* srcCs, const KoColorSpace* dstCs) : KoColorConversionTransformation(srcCs, dstCs)
        {
            Q_ASSERT(srcCs->colorModelId() == dstCs->colorModelId());
        }
        virtual void transform(const quint8 *srcU8, quint8 *dstU8, qint32 nPixels) const
        {
            const KoHdrColorProfile* hdrProfile = dynamic_cast<const KoHdrColorProfile*>(dstColorSpace()->profile());
            Q_ASSERT(hdrProfile);
            const typename _src_CSTraits_::channels_type* src = _src_CSTraits_::nativeArray(srcU8);
            typename _dst_CSTraits_::channels_type* dst = _dst_CSTraits_::nativeArray(dstU8);
            for(quint32 i = 0; i< nPixels;i++)
            {
                for(quint32 j = 0; j < 4; j++)
                {
                    if( j == _dst_CSTraits_::alpha_pos)
                    {
                        dst[j] = KoColorSpaceMaths<typename _src_CSTraits_::channels_type, typename _dst_CSTraits_::channels_type>::scaleToA(src[j]);
                    } else {
                        dst[j] = hdrProfile->displayToChannel( KoColorSpaceMaths<typename _src_CSTraits_::channels_type, quint16>::scaleToA(src[j]) );
                    }
                }
                dst += 4;
                src += 4;
            }
        }
};

template<typename _src_CSTraits_, typename _dst_CSTraits_>
class KisExposureCorrectedIntegerRgbToFloatRgbConversionTransformationFactory : public KoColorConversionTransformationFactory {
    public:
        KisExposureCorrectedIntegerRgbToFloatRgbConversionTransformationFactory(QString _srcDepthId, QString _dstDepthId) : KoColorConversionTransformationFactory(RGBAColorModelID.id(),  _srcDepthId, RGBAColorModelID.id(), _dstDepthId)
        {}
        virtual KoColorConversionTransformation* createColorTransformation(const KoColorSpace* srcColorSpace, const KoColorSpace* dstColorSpace, KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual) const
        {
            Q_UNUSED(renderingIntent);
            Q_ASSERT(canBeSource(srcColorSpace));
            Q_ASSERT(canBeDestination(dstColorSpace));
            return new KisExposureCorrectedIntegerRgbToFloatRgbConversionTransformation<_src_CSTraits_, _dst_CSTraits_>(srcColorSpace, dstColorSpace);
        }
        virtual bool conserveColorInformation() const
        {
            return true;
        }
        virtual bool conserveDynamicRange() const
        {
            return false;
        }
};


template<typename _src_CSTraits_, typename _dst_CSTraits_>
class KisExposureCorrectedFloatRgbToIntegerRgbConversionTransformation : public KoColorConversionTransformation {
    public:
        KisExposureCorrectedFloatRgbToIntegerRgbConversionTransformation(const KoColorSpace* srcCs, const KoColorSpace* dstCs) : KoColorConversionTransformation(srcCs, dstCs)
        {
            Q_ASSERT(srcCs->colorModelId() == dstCs->colorModelId());
        }
        virtual void transform(const quint8 *srcU8, quint8 *dstU8, qint32 nPixels) const
        {
            const KoHdrColorProfile* hdrProfile = dynamic_cast<const KoHdrColorProfile*>(srcColorSpace()->profile());
            Q_ASSERT(hdrProfile);
            const typename _src_CSTraits_::channels_type* src = _src_CSTraits_::nativeArray(srcU8);
            typename _dst_CSTraits_::channels_type* dst = _dst_CSTraits_::nativeArray(dstU8);
            for(quint32 i = 0; i< nPixels;i++)
            {
                for(quint32 j = 0; j < 4; j++)
                {
                    if( j == _dst_CSTraits_::alpha_pos)
                    {
                        dst[j] = KoColorSpaceMaths<typename _src_CSTraits_::channels_type, typename _dst_CSTraits_::channels_type >::scaleToA(src[j]);
                    } else {
                        dst[j] = KoColorSpaceMaths<quint16, typename _dst_CSTraits_::channels_type>::scaleToA(hdrProfile->channelToDisplay( src[j]));
                    }
                }
                dst += 4;
                src += 4;
            }
        }
};

template<typename _src_CSTraits_, typename _dst_CSTraits_>
class KisExposureCorrectedFloatRgbToIntegerRgbConversionTransformationFactory : public KoColorConversionTransformationFactory {
    public:
        KisExposureCorrectedFloatRgbToIntegerRgbConversionTransformationFactory(QString _srcDepthId, QString _dstDepthId) : KoColorConversionTransformationFactory(RGBAColorModelID.id(),  _srcDepthId, RGBAColorModelID.id(), _dstDepthId)
        {}
        virtual KoColorConversionTransformation* createColorTransformation(const KoColorSpace* srcColorSpace, const KoColorSpace* dstColorSpace, KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual) const
        {
            Q_UNUSED(renderingIntent);
            Q_ASSERT(canBeSource(srcColorSpace));
            Q_ASSERT(canBeDestination(dstColorSpace));
            return new KisExposureCorrectedFloatRgbToIntegerRgbConversionTransformation<_src_CSTraits_, _dst_CSTraits_>(srcColorSpace, dstColorSpace);
        }
        virtual bool conserveColorInformation() const
        {
            return true;
        }
        virtual bool conserveDynamicRange() const
        {
            return false;
        }
};

#endif
