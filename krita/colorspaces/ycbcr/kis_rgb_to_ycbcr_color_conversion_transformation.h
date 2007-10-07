/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_RGB_TO_YCBCR_COLOR_CONVERSION_TRANSFORMATION_H_
#define _KIS_RGB_TO_YCBCR_COLOR_CONVERSION_TRANSFORMATION_H_

#include <KoColorConversionTransformation.h>

#define SRC_TO_DST(v) (KoColorSpaceMaths<typename _src_CSTraits_::channels_type, typename _dst_CSTraits_::channels_type >::scaleToA(v))

template<typename _src_CSTraits_, typename _dst_CSTraits_>
class KisRgbToYCbCrColorConversionTransformation : public KoColorConversionTransformation {
    public:
        KisRgbToYCbCrColorConversionTransformation(const KoColorSpace* srcCs, const KoColorSpace* dstCs) : KoColorConversionTransformation(srcCs, dstCs)
        {
        }
        virtual void transform(const quint8 *srcU8, quint8 *dstU8, qint32 nPixels) const
        {
            const typename _src_CSTraits_::Pixel* src = reinterpret_cast< const typename _src_CSTraits_::Pixel*>(srcU8);
            typename _dst_CSTraits_::Pixel* dst = reinterpret_cast<typename _dst_CSTraits_::Pixel*>(dstU8);
            while(nPixels > 0)
            {
                typename _dst_CSTraits_::channels_type red = SRC_TO_DST(src->red);
                typename _dst_CSTraits_::channels_type green = SRC_TO_DST(src->green);
                typename _dst_CSTraits_::channels_type blue = SRC_TO_DST(src->blue);
                dst->Y = _dst_CSTraits_::computeY( red, green, blue);
                dst->Cb = _dst_CSTraits_::computeCb( red, green, blue);
                dst->Cr = _dst_CSTraits_::computeCr( red, green, blue);
                dst->alpha = SRC_TO_DST(src->alpha);
                src ++;
                dst ++;
                nPixels--;
            }
        }
};

template<typename _src_CSTraits_, typename _dst_CSTraits_>
class KisRgbToYCbCrColorConversionTransformationFactory : public KoColorConversionTransformationFactory {
    public:
        KisRgbToYCbCrColorConversionTransformationFactory(QString _srcColorModelId, QString _srcDepthId, QString _dstColorModelId, QString _dstDepthId) : KoColorConversionTransformationFactory(_srcColorModelId,  _srcDepthId, _dstColorModelId, _dstDepthId)
        {}
        virtual KoColorConversionTransformation* createColorTransformation(const KoColorSpace* srcColorSpace, const KoColorSpace* dstColorSpace, KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual)
        {
            Q_UNUSED(renderingIntent);
            Q_ASSERT(canBeSource(srcColorSpace));
            Q_ASSERT(canBeDestination(dstColorSpace));
            return new KisRgbToYCbCrColorConversionTransformation<_src_CSTraits_, _dst_CSTraits_>(srcColorSpace, dstColorSpace);
        }
        virtual bool conserveColorInformation() const
        {
            return true;
        }
        virtual bool conserveDynamicRange() const
        {
            return false;
        }
    private:
        bool hdr;
};

#endif
