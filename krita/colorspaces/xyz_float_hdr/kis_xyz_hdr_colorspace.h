/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2 of the License.
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

#ifndef _KIS_XYZ_HDR_COLORSPACE_H_
#define _KIS_XYZ_HDR_COLORSPACE_H_

#include <math.h>

#include "KoIncompleteColorSpace.h"

#include "compositeops/KoCompositeOpOver.h"
#include "compositeops/KoCompositeOpErase.h"

// Observer= 2°, Illuminant= D65
#define X_r 95.047
#define Y_r 100.000
#define Z_r 108.883

#define k 0.008856
#define delta (6.0 / 29.0)

#define UINT16_TO_FLOAT(v) (KoColorSpaceMaths<quint16, typename _CSTraits::channels_type >::scaleToA(v))
#define FLOAT_TO_UINT16(v) (KoColorSpaceMaths<typename _CSTraits::channels_type, quint16>::scaleToA(v))

template <class _CSTraits>
class KisXyzFloatHDRColorSpace : public KoIncompleteColorSpace<_CSTraits, KoLAB16Fallback>
{
    public:
        KisXyzFloatHDRColorSpace(const QString &id, const QString &name, KoColorSpaceRegistry * parent, KoColorProfile *profile)
          : KoIncompleteColorSpace<_CSTraits, KoLAB16Fallback>(id, name, parent)
        {
            Q_UNUSED(profile);
            
            const KoChannelInfo::enumChannelValueType channelValueType = KoColorSpaceMathsTraits<typename _CSTraits::channels_type>::channelValueType;
            const int channelSize = sizeof(typename _CSTraits::channels_type);
            this->addChannel(new KoChannelInfo(i18n("X"),
                                         _CSTraits::x_pos * channelSize,
                                         KoChannelInfo::COLOR,
                                         channelValueType,
                                         channelSize,
                                         QColor(255,0,0)));
            this->addChannel(new KoChannelInfo(i18n("Y"),
                                         _CSTraits::y_pos * channelSize,
                                         KoChannelInfo::COLOR,
                                         channelValueType,
                                         channelSize,
                                         QColor(0,255,0)));
            this->addChannel(new KoChannelInfo(i18n("Z"),
                                         _CSTraits::z_pos * channelSize,
                                         KoChannelInfo::COLOR,
                                         channelValueType,
                                         channelSize,
                                         QColor(0,0,255)));
            this->addChannel(new KoChannelInfo(i18n("Alpha"),
                                         _CSTraits::alpha_pos * channelSize,
                                         KoChannelInfo::ALPHA,
                                         channelValueType,
                                         channelSize));

            addCompositeOp( new KoCompositeOpOver<_CSTraits>( this ) );
            addCompositeOp( new KoCompositeOpErase<_CSTraits>( this ) );

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
    public:
        virtual void toLabA16(const quint8 * src8, quint8 * dst8, quint32 nPixels) const
        {
            KoLabU16Traits::Pixel* dst = reinterpret_cast<KoLabU16Traits::Pixel*>(dst8);
            const typename _CSTraits::Pixel* src = reinterpret_cast<const typename _CSTraits::Pixel*>(src8);
            while(nPixels > 0)
            {
                double fx = f_xyz_to_lab( src->X / X_r );
                double fy = f_xyz_to_lab( src->Y / Y_r );
                double fz = f_xyz_to_lab( src->Z / Z_r );
                
                dst->L = FLOAT_TO_UINT16((116 * fy - 16) );
                dst->a = FLOAT_TO_UINT16((500 * ( fx - fy )) );
                dst->b = FLOAT_TO_UINT16((200 * ( fy - fz )) );
                dst->alpha = FLOAT_TO_UINT16(src->alpha);
                
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
    public:
        virtual void fromLabA16(const quint8 * src8, quint8 * dst8, quint32 nPixels) const
        {
            const KoLabU16Traits::Pixel* src = reinterpret_cast<const KoLabU16Traits::Pixel*>(src8);
            typename _CSTraits::Pixel* dst = reinterpret_cast<typename _CSTraits::Pixel*>(dst8);
            while(nPixels > 0)
            {
                double fy = (UINT16_TO_FLOAT(src->L ) + 16.0) / 116.0;
                double fx = fy + UINT16_TO_FLOAT(src->a ) / 500.0;
                double fz = fy - UINT16_TO_FLOAT(src->b ) / 200.0;
                
                dst->X = f_lab_to_xyz(fx, X_r);
                dst->Y = f_lab_to_xyz(fy, Y_r);
                dst->Z = f_lab_to_xyz(fz, Z_r);
                dst->alpha = UINT16_TO_FLOAT(src->alpha);
                kDebug() << src->L << " " << src->a << " " << src->b << " " << dst->X << " " << dst->Y << " " << dst->Z;
                nPixels--;
                dst++;
                src++;
            }
            
        }
        virtual bool profileIsCompatible(KoColorProfile* profile) const
        {
            return profile == 0;
        }

        virtual bool willDegrade(ColorSpaceIndependence /*independence*/) const 
        {
            // Currently all levels of colorspace independence will degrade floating point
            // colorspaces images.
            return true;
        }

};

#endif
