/*
 *  Copyright (c) 2006-2007 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2005-2007 Adrian Page <adrian@pagenet.plus.com>
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

#include <klocale.h>
#include <math.h>

#include "KoIncompleteColorSpace.h"
#include "KoLcmsColorProfileContainer.h"
#include "KoColorSpaceRegistry.h"
#include "KoColorSpaceTraits.h"
#include "KoChannelInfo.h"

#include "compositeops/KoCompositeOpOver.h"
#include "compositeops/KoCompositeOpErase.h"

#define UINT8_TO_FLOAT(v) (KoColorSpaceMaths<quint8, typename _CSTraits::channels_type >::scaleToA(v))
#define FLOAT_TO_UINT8(v) (KoColorSpaceMaths<typename _CSTraits::channels_type, quint8>::scaleToA(v))
#define UINT16_TO_FLOAT(v) (KoColorSpaceMaths<quint16, typename _CSTraits::channels_type >::scaleToA(v))
#define FLOAT_TO_UINT16(v) (KoColorSpaceMaths<typename _CSTraits::channels_type, quint16>::scaleToA(v))

template <class _CSTraits>
class KisRgbFloatHDRColorSpace : public KoIncompleteColorSpace<_CSTraits>
{
    public:
        KisRgbFloatHDRColorSpace(const QString &id, const QString &name, KoColorSpaceRegistry * parent, KoColorProfile *profile)
          : KoIncompleteColorSpace<_CSTraits>(id, name, parent, parent->rgb16(""))
        {
            // We assume an alpha channel at the moment
            Q_ASSERT(_CSTraits::alpha_pos != -1);

            // We require a profile
            Q_ASSERT(profile);

            m_profile = 0;
            m_profileLcms = 0;

            if (profile) {
                m_profile = dynamic_cast<KoIccColorProfile *>(profile);
                Q_ASSERT(m_profile != 0);
                m_profileLcms = m_profile->asLcms();
            }

            // We use an RgbU16 colorspace to convert exposed pixels into
            // QImages.
            m_rgbU16ColorSpace = KoColorSpaceRegistry::instance()->rgb16(profile);
            Q_ASSERT(m_rgbU16ColorSpace);

            const KoChannelInfo::enumChannelValueType channelValueType = KoColorSpaceMathsTraits<typename _CSTraits::channels_type>::channelValueType;
            const int channelSize = sizeof(typename _CSTraits::channels_type);

            this->addChannel(new KoChannelInfo(i18n("Red"),
                                         _CSTraits::red_pos * channelSize,
                                         KoChannelInfo::COLOR,
                                         channelValueType,
                                         channelSize,
                                         QColor(255,0,0)));
            this->addChannel(new KoChannelInfo(i18n("Green"),
                                         _CSTraits::green_pos * channelSize,
                                         KoChannelInfo::COLOR,
                                         channelValueType,
                                         channelSize,
                                         QColor(0,255,0)));
            this->addChannel(new KoChannelInfo(i18n("Blue"),
                                         _CSTraits::blue_pos * channelSize,
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

        virtual bool hasHighDynamicRange() const { return true; }

        virtual KoColorProfile *profile() const { return m_profile; }

        virtual bool profileIsCompatible(KoColorProfile* profile) const
        {
            KoIccColorProfile *lcmsProfile = dynamic_cast<KoIccColorProfile *>(profile);
            if (lcmsProfile) {
                if (lcmsProfile->asLcms()->colorSpaceSignature() == icSigRgbData) {
                    return true;
                }
            }
            return false;
        }

        virtual bool willDegrade(ColorSpaceIndependence /*independence*/) const 
        {
            // Currently all levels of colorspace independence will degrade floating point
            // colorspaces images.
            return true;
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
                                       KoColorProfile *dstProfile,
                                       KoColorConversionTransformation::Intent renderingIntent, 
                                       float exposure) const
        {
            int numPixelsToConvert = width * height;
            KoRgbU16Traits::Pixel *u16Pixels = new KoRgbU16Traits::Pixel[numPixelsToConvert];
            KoRgbU16Traits::Pixel *dstPixel = u16Pixels;

            const Pixel *srcPixels = reinterpret_cast<const Pixel *>(dataU8);
            const Pixel *srcPixel = srcPixels;

            const float exposureFactor = pow(2, exposure + 2.47393);

            // Apply exposure and convert to u16.
            while (numPixelsToConvert > 0) {

                dstPixel->red = convertToDisplay(srcPixel->red, exposureFactor);
                dstPixel->green = convertToDisplay(srcPixel->green, exposureFactor);
                dstPixel->blue = convertToDisplay(srcPixel->blue, exposureFactor);
                dstPixel->alpha = KoColorSpaceMaths<typename _CSTraits::channels_type, quint16>::scaleToA(srcPixel->alpha);

                ++dstPixel;
                ++srcPixel;
                --numPixelsToConvert;
            }

            QImage image = m_rgbU16ColorSpace->convertToQImage(reinterpret_cast<quint8 *>(u16Pixels), 
                                                               width, 
                                                               height, 
                                                               dstProfile, 
                                                               renderingIntent);
            delete [] u16Pixels;

            return image;
        }

        virtual void invertColor(quint8 * srcU8, qint32 nPixels) const
        {
            typename _CSTraits::channels_type *src = reinterpret_cast<typename _CSTraits::channels_type *>(srcU8);
            while(nPixels--)
            {
                src[0] = KoColorSpaceMathsTraits<quint16>::max - src[0];
                src[1] = KoColorSpaceMathsTraits<quint16>::max - src[1];
                src[2] = KoColorSpaceMathsTraits<quint16>::max - src[2];
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
        struct Pixel {
            typename _CSTraits::channels_type blue;
            typename _CSTraits::channels_type green;
            typename _CSTraits::channels_type red;
            typename _CSTraits::channels_type alpha;
        };

        quint16 convertToDisplay(typename _CSTraits::channels_type value, float exposureFactor) const
        {
            value *= exposureFactor;

            // After adjusting by the exposure, map 1.0 to 3.5 f-stops below 1.0
            // I.e. scale by 1/(2^3.5).
            const float middleGreyScaleFactor = 0.0883883;
            value *= middleGreyScaleFactor;

            const int minU16 = 0;
            const int maxU16 = 65535;

            return (quint16)qBound(minU16, qRound(value * maxU16), maxU16);
        }

        KoIccColorProfile *m_profile;
        KoLcmsColorProfileContainer *m_profileLcms;
        KoColorSpace *m_rgbU16ColorSpace;

        friend class KisRgbFloatHDRColorSpaceTest;
};

class KisRgbFloatHDRColorSpaceFactory : public KoColorSpaceFactory
{
public:
    virtual bool profileIsCompatible(KoColorProfile* profile) const
    {
        KoIccColorProfile *lcmsProfile = dynamic_cast<KoIccColorProfile *>(profile);
        if (lcmsProfile) {
            if (lcmsProfile->asLcms()->colorSpaceSignature() == icSigRgbData) {
                return true;
            }
        }
        return false;
    }

    virtual bool isIcc() const { return false; }
    virtual bool isHdr() const { return true; }
    virtual QString defaultProfile() const { return "lcms virtual RGB profile - Rec. 709 Linear"; }
};

#endif

