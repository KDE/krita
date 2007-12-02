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

#include <QDomElement>

#include <klocale.h>
#include <math.h>

#include "KoIncompleteColorSpace.h"
#include "KoHdrColorProfile.h"
#include "KoLcmsColorProfileContainer.h"
#include "KoColorSpaceRegistry.h"
#include "KoColorSpaceTraits.h"
#include "KoChannelInfo.h"

#include "compositeops/KoCompositeOpOver.h"
#include "compositeops/KoCompositeOpErase.h"

#define UINT8_TO_FLOAT(v) (KoColorSpaceMaths<quint8, typename _CSTraits::channels_type >::scaleToA(v))
#define FLOAT_TO_UINT8(v) (KoColorSpaceMaths<typename _CSTraits::channels_type, quint8>::scaleToA(v))
#define EXPOSURE_CORRECTED_UINT8_TO_FLOAT(v) ( m_profile->displayToChannel(  KoColorSpaceMaths<quint8, quint16 >::scaleToA(v)) )
#define EXPOSURE_CORRECTED_FLOAT_TO_UINT8(v) (KoColorSpaceMaths<quint16, quint8>::scaleToA(m_profile->channelToDisplay(v)))

class KisRgbFloatHDRColorSpaceFactory : public KoColorSpaceFactory
{
    public:
        virtual bool profileIsCompatible(const KoColorProfile* profile) const
        {
            return isHdrRgbColorProfile(profile);
        }
    
        virtual bool isIcc() const { return false; }
        virtual bool isHdr() const { return true; }
        virtual QString defaultProfile() const { return "lcms virtual RGB profile - Rec. 709 Linear"; }
    public:
        static bool isHdrRgbColorProfile( const KoColorProfile* profile)
        {
            const KoHdrColorProfile* hdrProfile = dynamic_cast<const KoHdrColorProfile *>(profile);
            if( hdrProfile )
            {
                if ( hdrProfile->iccProfile()->asLcms()->colorSpaceSignature() == icSigRgbData)
                {
                    return true;
                }
            }
            return false;
        }
};

template <class _CSTraits>
class KisRgbFloatHDRColorSpace : public KoIncompleteColorSpace<_CSTraits>
{
    public:

        using KoIncompleteColorSpace<_CSTraits>::difference;
    
        KisRgbFloatHDRColorSpace(const QString &id, const QString &name, KoColorProfile *profile)
          : KoIncompleteColorSpace<_CSTraits>(id, name, KoColorSpaceRegistry::instance()->rgb16(""))
        {
            // We assume an alpha channel at the moment
            Q_ASSERT(_CSTraits::alpha_pos != -1);

            // We require a profile
            Q_ASSERT(profile);

            m_profile = 0;
            m_profileLcms = 0;

            if (profile) {
                m_profile = dynamic_cast<KoHdrColorProfile *>(profile);
                Q_ASSERT(m_profile != 0);
                m_profileLcms = m_profile->iccProfile()->asLcms();
            }

            // We use an RgbU16 colorspace to convert exposed pixels into
            // QImages.
            
            m_rgbU16ColorSpace = KoColorSpaceRegistry::instance()->rgb16( m_profile->iccProfile() );
            
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

        virtual const KoColorProfile *profile() const { return m_profile; }
        virtual KoColorProfile * profile() { return m_profile; }

        virtual bool profileIsCompatible(const KoColorProfile* profile) const
        {
            return KisRgbFloatHDRColorSpaceFactory::isHdrRgbColorProfile(profile);
        }

        virtual bool willDegrade(ColorSpaceIndependence /*independence*/) const 
        {
            // Currently all levels of colorspace independence will degrade floating point
            // colorspaces images.
            return true;
        }

        virtual void fromQColor(const QColor& c, quint8 *dstU8, const KoColorProfile * /*profile*/) const
        {
            typename _CSTraits::channels_type* dst = _CSTraits::nativeArray(dstU8);
            dst[ _CSTraits::red_pos ] = EXPOSURE_CORRECTED_UINT8_TO_FLOAT(c.red());
            dst[ _CSTraits::green_pos ] = EXPOSURE_CORRECTED_UINT8_TO_FLOAT(c.green());
            dst[ _CSTraits::blue_pos ] = EXPOSURE_CORRECTED_UINT8_TO_FLOAT(c.blue());
        }

        virtual void fromQColor(const QColor& c, quint8 opacity, quint8 *dstU8, const KoColorProfile * /*profile*/) const
        {
            typename _CSTraits::channels_type* dst = _CSTraits::nativeArray(dstU8);
            dst[ _CSTraits::red_pos ] = EXPOSURE_CORRECTED_UINT8_TO_FLOAT(c.red());
            dst[ _CSTraits::green_pos ] = EXPOSURE_CORRECTED_UINT8_TO_FLOAT(c.green());
            dst[ _CSTraits::blue_pos ] = EXPOSURE_CORRECTED_UINT8_TO_FLOAT(c.blue());
            dst[ _CSTraits::alpha_pos ] = UINT8_TO_FLOAT(opacity);
        }

        virtual void toQColor(const quint8 *srcU8, QColor *c, const KoColorProfile * /*profile*/) const
        {
            const typename _CSTraits::channels_type* src = _CSTraits::nativeArray(srcU8);
            c->setRgb(EXPOSURE_CORRECTED_FLOAT_TO_UINT8(src[_CSTraits::red_pos]), EXPOSURE_CORRECTED_FLOAT_TO_UINT8(src[_CSTraits::green_pos]), EXPOSURE_CORRECTED_FLOAT_TO_UINT8(src[_CSTraits::blue_pos]));
        }

        virtual void toQColor(const quint8 *srcU8, QColor *c, quint8 *opacity, const KoColorProfile * /*profile*/) const
        {
            const typename _CSTraits::channels_type* src = _CSTraits::nativeArray(srcU8);
            c->setRgb(EXPOSURE_CORRECTED_FLOAT_TO_UINT8(src[_CSTraits::red_pos]), EXPOSURE_CORRECTED_FLOAT_TO_UINT8(src[_CSTraits::green_pos]), EXPOSURE_CORRECTED_FLOAT_TO_UINT8(src[_CSTraits::blue_pos]));
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
                                       const KoColorProfile *dstProfile,
                                       KoColorConversionTransformation::Intent renderingIntent) const
        {
            double exposure = m_profile->hdrExposure();
            Q_UNUSED(exposure); // XXX: Shouldn't we use exposure here?
            int numPixelsToConvert = width * height;
            KoRgbU16Traits::Pixel *u16Pixels = new KoRgbU16Traits::Pixel[numPixelsToConvert];
            KoRgbU16Traits::Pixel *dstPixel = u16Pixels;

            const Pixel *srcPixels = reinterpret_cast<const Pixel *>(dataU8);
            const Pixel *srcPixel = srcPixels;

            // XXX: Apply exposure and convert to u16.
            while (numPixelsToConvert > 0) {

                dstPixel->red = m_profile->channelToDisplay( srcPixel->red);
                dstPixel->green = m_profile->channelToDisplay(srcPixel->green);
                dstPixel->blue = m_profile->channelToDisplay(srcPixel->blue);
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
        
        void colorToXML( const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const
        {
            const typename _CSTraits::Pixel* p = reinterpret_cast<const typename _CSTraits::Pixel*>( pixel );
            QDomElement labElt = doc.createElement( "RGB" );
            labElt.setAttribute("r", KoColorSpaceMaths< typename _CSTraits::channels_type, double>::scaleToA( p->red) );
            labElt.setAttribute("g", KoColorSpaceMaths< typename _CSTraits::channels_type, double>::scaleToA( p->green) );
            labElt.setAttribute("b", KoColorSpaceMaths< typename _CSTraits::channels_type, double>::scaleToA( p->blue) );
            labElt.setAttribute("space", profile()->name() );
            colorElt.appendChild( labElt );
        }
        
        void colorFromXML( quint8* pixel, const QDomElement& elt) const
        {
            typename _CSTraits::Pixel* p = reinterpret_cast<typename _CSTraits::Pixel*>( pixel );
            p->red = KoColorSpaceMaths< double, typename _CSTraits::channels_type >::scaleToA(elt.attribute("r").toDouble());
            p->green = KoColorSpaceMaths< double, typename _CSTraits::channels_type >::scaleToA(elt.attribute("g").toDouble());
            p->blue = KoColorSpaceMaths< double, typename _CSTraits::channels_type >::scaleToA(elt.attribute("b").toDouble());
        }


    private:
        struct Pixel {
            typename _CSTraits::channels_type blue;
            typename _CSTraits::channels_type green;
            typename _CSTraits::channels_type red;
            typename _CSTraits::channels_type alpha;
        };

        KoHdrColorProfile *m_profile;
        KoLcmsColorProfileContainer *m_profileLcms;
        const KoColorSpace *m_rgbU16ColorSpace;

        friend class KisRgbFloatHDRColorSpaceTest;
};

#endif

