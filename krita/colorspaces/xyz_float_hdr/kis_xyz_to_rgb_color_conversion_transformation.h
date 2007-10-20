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

#ifndef _KIS_XYZ_TO_RGB_COLOR_CONVERSION_TRANSFORMATION_H_
#define _KIS_XYZ_TO_RGB_COLOR_CONVERSION_TRANSFORMATION_H_

#include <KoColorConversionTransformation.h>

template<typename _xyz_CSTraits_, typename _rgb_CSTraits_>
class KisXyzToRgbColorConversionTransformation : public KoColorConversionTransformation {
    public:
        KisXyzToRgbColorConversionTransformation(const KoColorSpace* srcCs, const KoColorSpace* dstCs) : KoColorConversionTransformation(srcCs, dstCs)
        {
        }
        virtual void transform(const quint8 *src8, quint8 *dst8, qint32 nPixels) const
        {
            typename _rgb_CSTraits_::Pixel* dst = reinterpret_cast<typename _rgb_CSTraits_::Pixel*>(dst8);
            const typename _xyz_CSTraits_::Pixel* src = reinterpret_cast<const typename _xyz_CSTraits_::Pixel*>(src8);
            while(nPixels > 0)
            {
                dst->red = 3.240708 * src->X -1.537259 * src->Y - 0.498570 * src->Z;
                dst->green = -0.969257 * src->X + 1.875995 * src->Y - 0.041555 * src->Z;
                dst->blue = 0.055636 * src->X - 0.203996 * src->Y + 1.057069 * src->Z;
                dst->alpha = src->alpha;
                
                nPixels--;
                dst++;
                src++;
            }
        }
};

template<typename _xyz_CSTraits_, typename _rgb_CSTraits_>
class KisXyzToRgbColorConversionTransformationFactory : public KoColorConversionTransformationFactory {
    public:
        KisXyzToRgbColorConversionTransformationFactory(QString _srcDepthId, QString _dstDepthId) : KoColorConversionTransformationFactory(XYZAColorModelID.id(),  _srcDepthId, RGBAColorModelID.id(), _dstDepthId)
        {}
        virtual KoColorConversionTransformation* createColorTransformation(const KoColorSpace* srcColorSpace, const KoColorSpace* dstColorSpace, KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual)
        {
            Q_UNUSED(renderingIntent);
            Q_ASSERT(canBeSource(srcColorSpace));
            Q_ASSERT(canBeDestination(dstColorSpace));
            return new KisXyzToRgbColorConversionTransformation<_xyz_CSTraits_, _rgb_CSTraits_>(srcColorSpace, dstColorSpace);
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
