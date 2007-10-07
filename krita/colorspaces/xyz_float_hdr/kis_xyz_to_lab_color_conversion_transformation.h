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

#ifndef _KIS_XYZ_TO_LAB_COLOR_CONVERSION_TRANSFORMATION_H_
#define _KIS_XYZ_TO_LAB_COLOR_CONVERSION_TRANSFORMATION_H_

#include <KoColorConversionTransformation.h>

#define FLOAT_TO_UINT16(v) (KoColorSpaceMaths<typename _xyz_CSTraits_::channels_type, quint16>::scaleToA(v))
#define FLOAT_TO_L(v) FLOAT_TO_UINT16( (v) / 100.0)
#define FLOAT_TO_ab(v) (FLOAT_TO_UINT16( (v)/200.0 + 0.5))

template<typename _xyz_CSTraits_>
class KisXyzToLabColorConversionTransformation : public KoColorConversionTransformation {
    public:
        KisXyzToLabColorConversionTransformation(const KoColorSpace* srcCs, const KoColorSpace* dstCs) : KoColorConversionTransformation(srcCs, dstCs)
        {
        }
        virtual void transform(const quint8 *src8, quint8 *dst8, qint32 nPixels) const
        {
            KoLabU16Traits::Pixel* dst = reinterpret_cast<KoLabU16Traits::Pixel*>(dst8);
            const typename _xyz_CSTraits_::Pixel* src = reinterpret_cast<const typename _xyz_CSTraits_::Pixel*>(src8);
            while(nPixels > 0)
            {
                double fx = f_xyz_to_lab( src->X / X_r );
                double fy = f_xyz_to_lab( src->Y / Y_r );
                double fz = f_xyz_to_lab( src->Z / Z_r );
                
                dst->L = FLOAT_TO_L(116 * fy - 16);
                dst->a = FLOAT_TO_ab(500 * ( fx - fy ));
                dst->b = FLOAT_TO_ab(200 * ( fy - fz ));
                dst->alpha = FLOAT_TO_UINT16(src->alpha);
                
                nPixels--;
                dst++;
                src++;
            }
        }
    private:
        inline double f_xyz_to_lab(double v) const
        {
            if(v > k)
            {
                return powf(v, 1.0/3.0);
            } else {
                return 7.787 * v + 16.0 / 116.0;
            }
        }
};

template<typename _xyz_CSTraits_>
class KisXyzToLabColorConversionTransformationFactory : public KoColorConversionTransformationFactory {
    public:
        KisXyzToLabColorConversionTransformationFactory(QString _srcDepthId) : KoColorConversionTransformationFactory(XYZAColorModelID.id(),  _srcDepthId, LABAColorModelID.id(), Integer16BitsColorDepthID.id())
        {}
        virtual KoColorConversionTransformation* createColorTransformation(const KoColorSpace* srcColorSpace, const KoColorSpace* dstColorSpace, KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual)
        {
            Q_UNUSED(renderingIntent);
            Q_ASSERT(canBeSource(srcColorSpace));
            Q_ASSERT(canBeDestination(dstColorSpace));
            return new KisXyzToLabColorConversionTransformation<_xyz_CSTraits_>(srcColorSpace, dstColorSpace);
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
