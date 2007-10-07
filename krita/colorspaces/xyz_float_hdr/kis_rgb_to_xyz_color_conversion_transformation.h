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

#ifndef _KIS_RGB_TO_XYZ_COLOR_CONVERSION_TRANSFORMATION_H_
#define _KIS_RGB_TO_XYZ_COLOR_CONVERSION_TRANSFORMATION_H_

#include <KoColorConversionTransformation.h>

template< typename _rgb_CSTraits_, typename _xyz_CSTraits_>
class KisRgbToXyzColorConversionTransformation : public KoColorConversionTransformation {
    public:
        KisRgbToXyzColorConversionTransformation(const KoColorSpace* srcCs, const KoColorSpace* dstCs) : KoColorConversionTransformation(srcCs, dstCs)
        {
        }
        virtual void transform(const quint8 *src8, quint8 *dst8, qint32 nPixels) const
        {
            const KoRgbU16Traits::Pixel* src = reinterpret_cast<const KoRgbU16Traits::Pixel*>(src8);
            typename _xyz_CSTraits_::Pixel* dst = reinterpret_cast<typename _xyz_CSTraits_::Pixel*>(dst8);
            while(nPixels > 0)
            {
                dst->X = 0.412424 * src->red + 0.212656 * src->green + 0.0193324 * src->blue;
                dst->Y = 0.357579 * src->red + 0.715158 * src->green + 0.119193 * src->blue;
                dst->Z = 0.180464 * src->red + 0.0721856 * src->green + 0.950444 * src->blue;
                dst->alpha = src->alpha;
                nPixels--;
                dst++;
                src++;
            }
        }
};

template<typename _rgb_CSTraits_, typename _xyz_CSTraits_>
class KisRgbToXyzColorConversionTransformationFactory : public KoColorConversionTransformationFactory {
    public:
        KisRgbToXyzColorConversionTransformationFactory(QString _srcDepthId, QString _dstDepthId) : KoColorConversionTransformationFactory(RGBAColorModelID.id(),  _srcDepthId, XYZAColorModelID.id(), _dstDepthId)
        {}
        virtual KoColorConversionTransformation* createColorTransformation(const KoColorSpace* srcColorSpace, const KoColorSpace* dstColorSpace, KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual)
        {
            Q_UNUSED(renderingIntent);
            Q_ASSERT(canBeSource(srcColorSpace));
            Q_ASSERT(canBeDestination(dstColorSpace));
            return new KisRgbToXyzColorConversionTransformation<_rgb_CSTraits_, _xyz_CSTraits_>(srcColorSpace, dstColorSpace);
        }
        virtual bool conserveColorInformation() const
        {
            return true;
        }
        virtual bool conserveDynamicRange() const
        {
            return true;
        }
    private:
        bool hdr;
};

#endif
