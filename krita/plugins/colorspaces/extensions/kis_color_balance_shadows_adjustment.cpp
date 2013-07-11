/*
 *  Copyright (c) 2013 Sahil Nagpal <nagpal.sahil01@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; version 2
 * of the License, or (at your option) any later version.
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

#include "kis_color_balance_shadows_adjustment.h"
#include <KoConfig.h>

#include <kis_debug.h>
#include <klocale.h>

#include <KoColorConversions.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpace.h>
#include <KoColorSpaceTraits.h>
#include <KoColorTransformation.h>
#include <KoID.h>
#include <kis_hsv_adjustment.h>


#define SCALE_TO_FLOAT( v ) KoColorSpaceMaths< _channel_type_, float>::scaleToA( v )
#define SCALE_FROM_FLOAT( v  ) KoColorSpaceMaths< float, _channel_type_>::scaleToA( v )

template<typename _channel_type_>
class KisColorBalanceShadowsAdjustment : public KoColorTransformation
{
    typedef KoBgrTraits<_channel_type_> RGBTrait;
    typedef typename RGBTrait::Pixel RGBPixel;

public:
    KisColorBalanceShadowsAdjustment(){};

void transform(const quint8 *srcU8, quint8 *dstU8, qint32 nPixels) const
{
    const RGBPixel* src = reinterpret_cast<const RGBPixel*>(srcU8);
    RGBPixel* dst = reinterpret_cast<RGBPixel*>(dstU8);
    float value_red, value_green, value_blue, a = 0.25, b = 0.333, scale = 0.7, h, s, v;

    while(nPixels > 0) {

        RGBToHSL(SCALE_TO_FLOAT(src->red), SCALE_TO_FLOAT(src->green), SCALE_TO_FLOAT(src->blue), &h, &s, &v);
        float shadows_red = cyan_red, shadows_green = magenta_green, shadows_blue = yellow_blue;

        shadows_red *= CLAMP ((v - b) / -a + 0.5, 0, 1) * scale;
        shadows_green *= CLAMP ((v - b) / -a + 0.5, 0, 1) * scale;
        shadows_blue *= CLAMP ((v - b) / -a + 0.5, 0, 1) * scale;

        value_red = SCALE_TO_FLOAT(src->red);
        value_red += CLAMP(shadows_red, 0, 1.0);
        value_green = SCALE_TO_FLOAT(src->green);
        value_green += CLAMP(shadows_green,0.0 ,1.0);
        value_blue = SCALE_TO_FLOAT(src->blue);
        value_blue += CLAMP(shadows_blue, 0.0, 1.0);

        if(m_preserve)
        {
            float h1, s1, l1, h2, s2, l2, r, g, bl;
            RGBToHSL(SCALE_TO_FLOAT(src->red), SCALE_TO_FLOAT(src->green), SCALE_TO_FLOAT(src->blue), &h1, &s1, &l1);
            RGBToHSL(value_red, value_green, value_blue, &h2, &s2, &l2);
            l2 = l1;

            HSLToRGB(h2, s2, l2, &r, &g, &bl);
            value_red = r;
            value_green = g;
            value_blue = bl;
        }

        dst->red = SCALE_FROM_FLOAT (value_red);
        dst->green = SCALE_FROM_FLOAT(value_green);
        dst->blue = SCALE_FROM_FLOAT(value_blue);
        dst->alpha = src->alpha;

        --nPixels;
        ++src;
        ++dst;
    }
}


virtual QList<QString> parameters() const
{
    QList<QString> list;
    list << "cyan_red" << "magenta_green" << "yellow_blue" << "preserve_luminosity";
    return list;
}

virtual int parameterId(const QString& name) const
{
    if (name == "cyan_red")
        return 0;
    else if(name == "magenta_green")
        return 1;
    else if(name == "yellow_blue")
        return 2;
    else if(name == "preserve_luminosity")
        return 3;
    return -1;
}

virtual void setParameter(int id, const QVariant& parameter)
{
    switch(id)
    {
    case 0:
        cyan_red = parameter.toDouble();
        break;
    case 1:
        magenta_green = parameter.toDouble();
        break;
    case 2:
        yellow_blue = parameter.toDouble();
        break;
    case 3:
        m_preserve = parameter.toBool();
        break;
    default:
        ;
    }
}
private:

    double cyan_red, magenta_green, yellow_blue;
    bool m_preserve;
};

 KisColorBalanceShadowsAdjustmentFactory::KisColorBalanceShadowsAdjustmentFactory()
    : KoColorTransformationFactory("BalanceShadows", i18n("ColorBalanceShadows Adjustment"))
{
}

QList< QPair< KoID, KoID > > KisColorBalanceShadowsAdjustmentFactory::supportedModels() const
{
    QList< QPair< KoID, KoID > > l;
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Integer8BitsColorDepthID));
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Integer16BitsColorDepthID));
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Float32BitsColorDepthID));
    return l;
}

KoColorTransformation* KisColorBalanceShadowsAdjustmentFactory::createTransformation(const KoColorSpace* colorSpace, QHash<QString, QVariant> parameters) const
{
    KoColorTransformation * adj;
    if (colorSpace->colorModelId() != RGBAColorModelID) {
        kError() << "Unsupported color space " << colorSpace->id() << " in KisColorBalanceShadowsAdjustment::createTransformation";
        return 0;
    }
    if (colorSpace->colorDepthId() == Float32BitsColorDepthID) {
        adj = new KisColorBalanceShadowsAdjustment< float >();
    } else if (colorSpace->colorDepthId() == Integer16BitsColorDepthID) {
        adj = new KisColorBalanceShadowsAdjustment< quint16 >();
    } else if (colorSpace->colorDepthId() == Integer8BitsColorDepthID) {
        adj = new KisColorBalanceShadowsAdjustment< quint8 >();
    } else {
        kError() << "Unsupported color space " << colorSpace->id() << " in KisColorBalanceShadowsAdjustment::createTransformation";
        return 0;
    }
    adj->setParameters(parameters);
    return adj;

}
