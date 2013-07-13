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

#include "kis_color_balance_highlights_adjustment.h"
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
class KisColorBalanceHighlightsAdjustment : public KoColorTransformation
{
    typedef KoBgrTraits<_channel_type_> RGBTrait;
    typedef typename RGBTrait::Pixel RGBPixel;

public:
    KisColorBalanceHighlightsAdjustment(){}

void transform(const quint8 *srcU8, quint8 *dstU8, qint32 nPixels) const
{
    const RGBPixel* src = reinterpret_cast<const RGBPixel*>(srcU8);
    RGBPixel* dst = reinterpret_cast<RGBPixel*>(dstU8);
    float value_red, value_green, value_blue, hue, saturation, lightness, a = 0.25, b = 0.333, scale = 0.7;

    while(nPixels > 0) {

        float red = SCALE_TO_FLOAT(src->red);
        float green = SCALE_TO_FLOAT(src->green);
        float blue = SCALE_TO_FLOAT(src->blue);
        RGBToHSL(red, green, blue, &hue, &saturation, &lightness);

        float midtones_red = m_cyan_red;
        midtones_red *= CLAMP ((lightness + b - 1) /  a + 0.5, 0, 1) * scale;

        float midtones_green = m_magenta_green;
        midtones_green *= CLAMP ((lightness + b - 1) /  a + 0.5, 0, 1) * scale;

        float midtones_blue = m_yellow_blue;
        midtones_blue *= CLAMP ((lightness + b - 1) /  a + 0.5, 0, 1) * scale;
\
        midtones_red += red;
        midtones_green += green;
        midtones_blue += blue;
        value_red = CLAMP(midtones_red,0.0 ,1.0);
        value_green = CLAMP(midtones_green,0.0, 1.0);
        value_blue = CLAMP(midtones_blue, 0.0, 1.0);

        if(m_preserve)
        {
            float h1, s1, l1, h2, s2, l2;
            RGBToHSL(SCALE_TO_FLOAT(src->red), SCALE_TO_FLOAT(src->green), SCALE_TO_FLOAT(src->blue), &h1, &s1, &l1);
            RGBToHSL(value_red, value_green, value_blue, &h2, &s2, &l2);
            HSLToRGB(h2, s2, l1, &value_red, &value_green, &value_blue);
        }
        dst->red = SCALE_FROM_FLOAT(value_red);
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
        m_cyan_red = parameter.toDouble();
        break;
    case 1:
        m_magenta_green = parameter.toDouble();
        break;
    case 2:
        m_yellow_blue = parameter.toDouble();
        break;
    case 3:
        m_preserve = parameter.toBool();
        break;
    default:
        ;
    }
}
private:

    double m_cyan_red, m_magenta_green, m_yellow_blue;
    bool m_preserve;
};

 KisColorBalanceHighlightsAdjustmentFactory::KisColorBalanceHighlightsAdjustmentFactory()
    : KoColorTransformationFactory("BalanceHighlights", i18n("ColorBalanceMidtones Adjustment"))
{
}

QList< QPair< KoID, KoID > > KisColorBalanceHighlightsAdjustmentFactory::supportedModels() const
{
    QList< QPair< KoID, KoID > > l;
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Integer8BitsColorDepthID));
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Integer16BitsColorDepthID));
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Float32BitsColorDepthID));
    return l;
}

KoColorTransformation* KisColorBalanceHighlightsAdjustmentFactory::createTransformation(const KoColorSpace* colorSpace, QHash<QString, QVariant> parameters) const
{
    KoColorTransformation * adj;
    if (colorSpace->colorModelId() != RGBAColorModelID) {
        kError() << "Unsupported color space " << colorSpace->id() << " in KisColorBalanceHighlightsAdjustment::createTransformation";
        return 0;
    }
    if (colorSpace->colorDepthId() == Float32BitsColorDepthID) {
        adj = new KisColorBalanceHighlightsAdjustment< float >();
    } else if (colorSpace->colorDepthId() == Integer16BitsColorDepthID) {
        adj = new KisColorBalanceHighlightsAdjustment< quint16 >();
    } else if (colorSpace->colorDepthId() == Integer8BitsColorDepthID) {
        adj = new KisColorBalanceHighlightsAdjustment< quint8 >();
    } else {
        kError() << "Unsupported color space " << colorSpace->id() << " in KisColorBalanceHighlightsAdjustment::createTransformation";
        return 0;
    }
    adj->setParameters(parameters);
    return adj;

}

