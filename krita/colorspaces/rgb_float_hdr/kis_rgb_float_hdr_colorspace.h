/*
 *  Copyright (c) 2006-2007 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
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
#ifndef KIS_RGB_FLOAT_HDR_COLORSPACE_H_
#define KIS_RGB_FLOAT_HDR_COLORSPACE_H_
#include "config-krita.h"

#include "klocale.h"
#include <KoIncompleteColorSpace.h>
#include <KoFallBack.h>

#define UINT8_TO_FLOAT(v) (KoColorSpaceMaths<quint8, typename _CSTraits::channels_type >::scaleToA(v))
#define FLOAT_TO_UINT8(v) (KoColorSpaceMaths<typename _CSTraits::channels_type, quint8>::scaleToA(v))
#define UINT16_TO_FLOAT(v) (KoColorSpaceMaths<quint16, typename _CSTraits::channels_type >::scaleToA(v))
#define FLOAT_TO_UINT16(v) (KoColorSpaceMaths<typename _CSTraits::channels_type, quint16>::scaleToA(v))

template <class _CSTraits>
class KisRgbFloatHDRColorSpace : public KoIncompleteColorSpace<_CSTraits, KoRGB16Fallback>
{
    public:
        KisRgbFloatHDRColorSpace(const QString &id, const QString &name, KoColorSpaceRegistry * parent, qint32 type)
          : KoIncompleteColorSpace<_CSTraits, KoRGB16Fallback>(id, name, parent, type, icSigRgbData)
        {
        
        }
        
        virtual void fromQColor(const QColor& c, quint8 *dstU8, KoColorProfile * /*profile*/) const
        {
            typename _CSTraits::channels_type* dst = _CSTraits::nativeArray(dstU8);
            dst[ _CSTraits::red_pos ] = UINT8_TO_FLOAT(c.red());
            dst[ _CSTraits::green_pos ] = UINT8_TO_FLOAT(c.green());
            dst[ _CSTraits::blue_pos ] = UINT8_TO_FLOAT(c.blue());
        }
        
        virtual void fromQColor(const QColor& c, quint8 opacity, quint8 *dstU8, KoColorProfile * /*profile*/) const
        {
            typename _CSTraits::channels_type* dst = _CSTraits::nativeArray(dstU8);
            dst[ _CSTraits::red_pos ] = UINT8_TO_FLOAT(c.red());
            dst[ _CSTraits::green_pos ] = UINT8_TO_FLOAT(c.green());
            dst[ _CSTraits::blue_pos ] = UINT8_TO_FLOAT(c.blue());
            dst[ _CSTraits::alpha_pos ] = UINT8_TO_FLOAT(opacity);
        }
        
        virtual void toQColor(const quint8 *srcU8, QColor *c, KoColorProfile * /*profile*/) const
        {
            const typename _CSTraits::channels_type* src = _CSTraits::nativeArray(srcU8);
            c->setRgb(FLOAT_TO_UINT8(src[_CSTraits::red_pos]), FLOAT_TO_UINT8(src[_CSTraits::green_pos]), FLOAT_TO_UINT8(src[_CSTraits::blue_pos]));
        }
        
        virtual void toQColor(const quint8 *srcU8, QColor *c, quint8 *opacity, KoColorProfile * /*profile*/) const
        {
            const typename _CSTraits::channels_type* src = _CSTraits::nativeArray(srcU8);
            c->setRgb(FLOAT_TO_UINT8(src[_CSTraits::red_pos]), FLOAT_TO_UINT8(src[_CSTraits::green_pos]), FLOAT_TO_UINT8(src[_CSTraits::blue_pos]));
            *opacity = FLOAT_TO_UINT8(src[_CSTraits::alpha_pos]);
        }
        
        quint8 difference(const quint8 *src1U8, const quint8 *src2U8)
        {
            const typename _CSTraits::channels_type* src1 = _CSTraits::nativeArray(src1U8);
            const typename _CSTraits::channels_type* src2 = _CSTraits::nativeArray(src2U8);
            return FLOAT_TO_UINT8(qMax(QABS(src2[_CSTraits::red_pos] - src1[_CSTraits::red_pos]),
                        qMax(QABS(src2[_CSTraits::green_pos] - src1[_CSTraits::green_pos]),
                            QABS(src2[_CSTraits::blue_pos] - src1[_CSTraits::blue_pos]))));
        }

        
        virtual QImage convertToQImage(const quint8 *dataU8, qint32 width, qint32 height,
                                                    KoColorProfile *  /*dstProfile*/,
                                                    qint32 /*renderingIntent*/, float exposure) const
        {
            const typename _CSTraits::channels_type *data = reinterpret_cast<const typename _CSTraits::channels_type *>(dataU8);
            
            QImage img = QImage(width, height, QImage::Format_ARGB32);
        
            qint32 i = 0;
            uchar *j = img.bits();
        
            // XXX: For now assume gamma 2.2.
            double gamma = 1 / 2.2;
            double exposureFactor = pow(2, exposure + 2.47393);
        
            while ( i < width * height * 4) {
                *( j + 3)  = KoColorSpaceMaths<float, quint8>::scaleToA(*( data + i + 3 ));
                *( j + 2 ) = convertToDisplay(*( data + i + 2 ), exposureFactor, gamma); //red_pos
                *( j + 1 ) = convertToDisplay(*( data + i + 1 ), exposureFactor, gamma); //green_pos
                *( j + 0 ) = convertToDisplay(*( data + i + 0 ), exposureFactor, gamma); //blue_pos
                i += 4;
                j += 4;
            }
        
            /*
            if (srcProfile != 0 && dstProfile != 0) {
                convertPixelsTo(img.bits(), srcProfile,
                        img.bits(), this, dstProfile,
                        width * height, renderingIntent);
            }
            */
            return img;
        }
        
        virtual void invertColor(quint8 * srcU8, qint32 nPixels) const
        {
            typename _CSTraits::channels_type *src = reinterpret_cast<typename _CSTraits::channels_type *>(srcU8);
            while(nPixels--)
            {
                src[0] = KoColorSpaceMathsTraits<quint16>::max() - src[0];
                src[1] = KoColorSpaceMathsTraits<quint16>::max() - src[1];
                src[2] = KoColorSpaceMathsTraits<quint16>::max() - src[2];
                src += this->pixelSize();
            }
        }
        virtual void fromRgbA16(const quint8 * srcU8, quint8 * dstU8, const quint32 nPixels) const
        {
            typename _CSTraits::channels_type* dst = _CSTraits::nativeArray(dstU8);
            const quint16* src = reinterpret_cast<const quint16*>(srcU8);
            for(quint32 i = 0; i< 4*nPixels;i++)
            {
                dst[i] = UINT16_TO_FLOAT(src[i]);
                kDebug() << "fromRgbA16 " << dst[i] << " " << src[i] << endl;
            }
        }
        virtual void toRgbA16(const quint8 * srcU8, quint8 * dstU8, const quint32 nPixels) const
        {
            const typename _CSTraits::channels_type* src = _CSTraits::nativeArray(srcU8);
            quint16* dst = reinterpret_cast<quint16*>(dstU8);
            for(quint32 i = 0; i< 4*nPixels;i++)
            {
                dst[i] = FLOAT_TO_UINT16(src[i]);
            }
        }
    private:
        quint8 convertToDisplay(double value, double exposureFactor, double gamma) const
        {
            //value *= pow(2, exposure + 2.47393);
            value *= exposureFactor;
        
            value = pow(value, gamma);
        
            // scale middle gray to the target framebuffer value
        
            value *= 84.66f;
        
            int valueInt = (int)(value + 0.5);
        
            return CLAMP(valueInt, 0, 255);
        }

};

#endif // KIS_STRATEGY_COLORSPACE_RGB_H_
