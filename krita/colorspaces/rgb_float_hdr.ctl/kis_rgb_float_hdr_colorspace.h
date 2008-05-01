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

#ifndef _KIS_RGB_HDR_COLORSPACE_H_
#define _KIS_RGB_HDR_COLORSPACE_H_

#include <QDomElement>

#include <math.h>

#include "KoColorSpaceRegistry.h"

#include "compositeops/KoCompositeOpOver.h"
#include "compositeops/KoCompositeOpErase.h"

#include "colorprofiles/KoCtlColorProfile.h"
#include "KoCtlMonoTypeColorSpace.h"

class KisRgbFloatHDRColorSpaceFactory : public KoCtlColorSpaceFactory
{
    public:
        virtual bool profileIsCompatible(const KoColorProfile* profile) const
        {
            return isRgbCtlColorProfile(profile);
        }
    
        virtual QString colorSpaceEngine() const { return ""; }
        virtual bool isHdr() const { return true; }
        virtual QString defaultProfile() const { return "Standard Linear RGB (scRGB/sRGB64)"; }
    public:
        static bool isRgbCtlColorProfile( const KoColorProfile* profile)
        {
            const KoCtlColorProfile* ctlp = dynamic_cast<const KoCtlColorProfile*>(profile);
            if(ctlp && ctlp->colorModel() == "RGBA" )
            {
                return true;
            }
            return false;
        }
};

template <class _CSTraits>
class KisRgbFloatHDRColorSpace : public KoCtlMonoTypeColorSpace<_CSTraits>
{
    public:
        KisRgbFloatHDRColorSpace(const QString &id, const QString &name, const KoCtlColorProfile *profile)
          : KoCtlMonoTypeColorSpace<_CSTraits>(id, name, KoColorSpaceRegistry::instance()->lab16(""), profile)
        {
            Q_UNUSED(profile);
            
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
        void colorToXML( const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const
        {
            const typename _CSTraits::Pixel* p = reinterpret_cast<const typename _CSTraits::Pixel*>( pixel );
            QDomElement labElt = doc.createElement( "RGB" );
            labElt.setAttribute("r", KoColorSpaceMaths< typename _CSTraits::channels_type, double>::scaleToA( p->red) );
            labElt.setAttribute("g", KoColorSpaceMaths< typename _CSTraits::channels_type, double>::scaleToA( p->green) );
            labElt.setAttribute("b", KoColorSpaceMaths< typename _CSTraits::channels_type, double>::scaleToA( p->blue) );
            labElt.setAttribute("space", this->profile()->name() );
            colorElt.appendChild( labElt );
        }
        
        void colorFromXML( quint8* pixel, const QDomElement& elt) const
        {
            typename _CSTraits::Pixel* p = reinterpret_cast<typename _CSTraits::Pixel*>( pixel );
            p->red = KoColorSpaceMaths< double, typename _CSTraits::channels_type >::scaleToA(elt.attribute("r").toDouble());
            p->green = KoColorSpaceMaths< double, typename _CSTraits::channels_type >::scaleToA(elt.attribute("g").toDouble());
            p->blue = KoColorSpaceMaths< double, typename _CSTraits::channels_type >::scaleToA(elt.attribute("b").toDouble());
        }
    public:
        virtual bool profileIsCompatible(const KoColorProfile* profile) const
        {
            return KisRgbFloatHDRColorSpaceFactory::isRgbCtlColorProfile( profile );
        }

        virtual bool willDegrade(ColorSpaceIndependence /*independence*/) const 
        {
            // Currently all levels of colorspace independence will degrade floating point
            // colorspaces images.
            return true;
        }

};

#endif
