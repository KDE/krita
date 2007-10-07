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

#ifndef _KIS_LAB_TO_XYZ_COLOR_CONVERSION_TRANSFORMATION_H_
#define _KIS_LAB_TO_XYZ_COLOR_CONVERSION_TRANSFORMATION_H_

#include <KoColorConversionTransformation.h>

#define UINT16_TO_FLOAT(v) (KoColorSpaceMaths<quint16, typename _xyz_CSTraits_::channels_type >::scaleToA(v))
#define L_TO_FLOAT(v) (UINT16_TO_FLOAT(v) * 100.0)
#define ab_TO_FLOAT(v) ((UINT16_TO_FLOAT(v) - 0.5) * 200.0)

template< typename _xyz_CSTraits_>
class KisLabToXyzColorConversionTransformation : public KoColorConversionTransformation {
    public:
        KisLabToXyzColorConversionTransformation(const KoColorSpace* srcCs, const KoColorSpace* dstCs) : KoColorConversionTransformation(srcCs, dstCs)
        {
        }
        virtual void transform(const quint8 *src8, quint8 *dst8, qint32 nPixels) const
        {
            const KoLabU16Traits::Pixel* src = reinterpret_cast<const KoLabU16Traits::Pixel*>(src8);
            typename _xyz_CSTraits_::Pixel* dst = reinterpret_cast<typename _xyz_CSTraits_::Pixel*>(dst8);
            while(nPixels > 0)
            {
                double fy = ( L_TO_FLOAT(src->L ) + 16.0) / 116.0;
                double fx = fy + ab_TO_FLOAT(src->a ) / 500.0;
                double fz = fy - ab_TO_FLOAT(src->b ) / 200.0;
                
                dst->X = f_lab_to_xyz(fx, X_r);
                dst->Y = f_lab_to_xyz(fy, Y_r);
                dst->Z = f_lab_to_xyz(fz, Z_r);
                dst->alpha = UINT16_TO_FLOAT(src->alpha);
                
                nPixels--;
                dst++;
                src++;
            }
        }
    private:
        inline double f_lab_to_xyz(double v, double r) const
        {
            if( v > delta)
            {
                return r * pow(v, 3.0);
            } else {
                return r * (v - 16.0 / 116.0) * 3.0 * delta *delta;
            }
        }
};

template<typename _xyz_CSTraits_>
class KisLabToXyzColorConversionTransformationFactory : public KoColorConversionTransformationFactory {
    public:
        KisLabToXyzColorConversionTransformationFactory(QString _dstDepthId) : KoColorConversionTransformationFactory(LABAColorModelID.id(),  Integer16BitsColorDepthID.id(), XYZAColorModelID.id(), _dstDepthId)
        {}
        virtual KoColorConversionTransformation* createColorTransformation(const KoColorSpace* srcColorSpace, const KoColorSpace* dstColorSpace, KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual)
        {
            Q_UNUSED(renderingIntent);
            Q_ASSERT(canBeSource(srcColorSpace));
            Q_ASSERT(canBeDestination(dstColorSpace));
            return new KisLabToXyzColorConversionTransformation<_xyz_CSTraits_>(srcColorSpace, dstColorSpace);
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
