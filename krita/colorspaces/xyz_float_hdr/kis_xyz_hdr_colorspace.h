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

#include <QDomElement>

#include <math.h>

#include "KoIncompleteColorSpace.h"
#include "KoColorSpaceRegistry.h"

#include "compositeops/KoCompositeOpOver.h"
#include "compositeops/KoCompositeOpErase.h"

// Observer= 2°, Illuminant= D65
#define X_r (95.047 / 100.0)
#define Y_r (100.000 / 100.0)
#define Z_r (108.883 / 100.0)

#define k 0.008856
#define delta (6.0 / 29.0)

template <class _CSTraits>
class KisXyzFloatHDRColorSpace : public KoIncompleteColorSpace<_CSTraits>
{
    public:
        KisXyzFloatHDRColorSpace(const QString &id, const QString &name,  KoColorProfile *profile)
          : KoIncompleteColorSpace<_CSTraits>(id, name, KoColorSpaceRegistry::instance()->lab16(""))
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
        virtual void colorToXML( const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const
        {
            const typename _CSTraits::Pixel* p = reinterpret_cast<const typename _CSTraits::Pixel*>( pixel );
            QDomElement labElt = doc.createElement( "XYZ" );
            labElt.setAttribute("x", KoColorSpaceMaths< typename _CSTraits::channels_type, double>::scaleToA( p->X) );
            labElt.setAttribute("y", KoColorSpaceMaths< typename _CSTraits::channels_type, double>::scaleToA( p->Y) );
            labElt.setAttribute("z", KoColorSpaceMaths< typename _CSTraits::channels_type, double>::scaleToA( p->Z) );
            labElt.setAttribute("space", "xyz" );
            colorElt.appendChild( labElt );
        }
        
        virtual void colorFromXML( quint8* pixel, const QDomElement& elt)
        {
            typename _CSTraits::Pixel* p = reinterpret_cast<typename _CSTraits::Pixel*>( pixel );
            p->X = KoColorSpaceMaths< double, typename _CSTraits::channels_type >::scaleToA(elt.attribute("x").toDouble());
            p->Y = KoColorSpaceMaths< double, typename _CSTraits::channels_type >::scaleToA(elt.attribute("y").toDouble());
            p->Z = KoColorSpaceMaths< double, typename _CSTraits::channels_type >::scaleToA(elt.attribute("z").toDouble());
        }
    public:
        virtual bool profileIsCompatible(const KoColorProfile* profile) const
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
