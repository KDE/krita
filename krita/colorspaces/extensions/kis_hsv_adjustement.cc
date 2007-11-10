/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; version 2
 * of the License.
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

#include "kis_hsv_adjustement.h"

#include <config-openexr.h>
#ifdef HAVE_OPENEXR
#include <half.h>
#endif

#include <klocale.h>

#include <KoColorConversions.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpace.h>
#include <KoColorSpaceTraits.h>
#include <KoColorTransformation.h>
#include <KoID.h>

#define SCALE_TO_FLOAT( v ) KoColorSpaceMaths< _channel_type_, float>::scaleToA( v )
#define SCALE_FROM_FLOAT( v  ) KoColorSpaceMaths< float, _channel_type_>::scaleToA( v )

template<typename _channel_type_>
class KisHSVAdjustement : public KoColorTransformation {
    typedef KoRgbTraits<_channel_type_> RGBTrait;
    typedef typename RGBTrait::Pixel RGBPixel;
    public:
        KisHSVAdjustement(double _adj_h, double _adj_s, double _adj_v) : m_adj_h(_adj_h), m_adj_s(_adj_s), m_adj_v(_adj_v)
        {
        }
    public:
        void transform(const quint8 *srcU8, quint8 *dstU8, qint32 nPixels) const
        {
            const RGBPixel* src = reinterpret_cast<const RGBPixel*>(srcU8);
            RGBPixel* dst = reinterpret_cast<RGBPixel*>(dstU8);
            float h,s,v,r,g,b;
            while( nPixels > 0)
            {
                RGBToHSV( SCALE_TO_FLOAT( src->red), SCALE_TO_FLOAT(src->green), SCALE_TO_FLOAT(src->blue), &h, &s,&v );
                
                HSVToRGB( h, s, v, &r, &g, &b);
                dst->red = SCALE_FROM_FLOAT( r );
                dst->green = SCALE_FROM_FLOAT( g );
                dst->blue = SCALE_FROM_FLOAT( b );
                dst->alpha = src->alpha;
                
                --nPixels;
                ++src;
                ++dst;
            }
        }
    private:
        double m_adj_h, m_adj_s, m_adj_v;
};


KisHSVAdjustementFactory::KisHSVAdjustementFactory() : KoColorTransformationFactory("hsv_adjustement", i18n("HSV Adjustement") )
{
    
}

QList< QPair< KoID, KoID > > KisHSVAdjustementFactory::supportedModels() const
{
    QList< QPair< KoID, KoID > > l;
    l.append( QPair< KoID, KoID >( RGBAColorModelID , Integer8BitsColorDepthID ) );
    l.append( QPair< KoID, KoID >( RGBAColorModelID , Integer16BitsColorDepthID ) );
    l.append( QPair< KoID, KoID >( RGBAColorModelID , Float16BitsColorDepthID ) );
    l.append( QPair< KoID, KoID >( RGBAColorModelID , Float32BitsColorDepthID ) );
}

KoColorTransformation* KisHSVAdjustementFactory::createTransformation(const KoColorSpace* colorSpace, QHash<QString, QVariant> parameters) const
{
    double h = 0.0, s = 0.0, v = 0.0;
    if(parameters.contains("h"))
    {
        h = parameters["h"].toDouble();
    }
    if(parameters.contains("s"))
    {
        s = parameters["s"].toDouble();
    }
    if(parameters.contains("v"))
    {
        v = parameters["v"].toDouble();
    }
    if( colorSpace->colorModelId() != RGBAColorModelID)
    {
        kDebug() << "Unsupported color space " << colorSpace->id() << " in KisHSVAdjustementFactory::createTransformation";
        return 0;
    }
    if( colorSpace->colorDepthId() == Integer8BitsColorDepthID )
    {
        return new KisHSVAdjustement< quint8 >(h,s,v);
    } else if( colorSpace->colorDepthId() == Integer16BitsColorDepthID )
    {
        return new KisHSVAdjustement< quint16 >(h,s,v);
    }
#ifdef HAVE_OPENEXR
    else if( colorSpace->colorDepthId() == Float16BitsColorDepthID )
    {
        return new KisHSVAdjustement< half >(h,s,v);
    }
#endif
    else if( colorSpace->colorDepthId() == Float32BitsColorDepthID )
    {
        return new KisHSVAdjustement< float >(h,s,v);
    } else {
        kDebug() << "Unsupported color space " << colorSpace->id() << " in KisHSVAdjustementFactory::createTransformation";
        return 0;
    }
}
