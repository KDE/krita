/*
 *  SPDX-FileCopyrightText: 2013 Sahil Nagpal <nagpal.sahil01@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kis_color_balance_adjustment.h"
#include <KoConfig.h>
#ifdef HAVE_OPENEXR
#include <half.h>
#endif

#include <kis_debug.h>
#include <klocalizedstring.h>

#include <KoColorConversions.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpace.h>
#include <KoColorSpaceTraits.h>
#include <KoColorTransformation.h>
#include <KoID.h>
#include <kis_hsv_adjustment.h>


#define SCALE_TO_FLOAT( v ) KoColorSpaceMaths< _channel_type_, float>::scaleToA( v )
#define SCALE_FROM_FLOAT( v  ) KoColorSpaceMaths< float, _channel_type_>::scaleToA( v )

class KisColorBalanceMath;
template<typename _channel_type_, typename traits>
class KisColorBalanceAdjustment : public KoColorTransformation
{
    typedef traits RGBTrait;
    typedef typename RGBTrait::Pixel RGBPixel;

public:
    KisColorBalanceAdjustment(){}

void transform(const quint8 *srcU8, quint8 *dstU8, qint32 nPixels) const override
{
    KisColorBalanceMath bal;
    const RGBPixel* src = reinterpret_cast<const RGBPixel*>(srcU8);
    RGBPixel* dst = reinterpret_cast<RGBPixel*>(dstU8);
    float value_red, value_green, value_blue, hue, saturation, lightness;

    while(nPixels > 0) {

        float red = SCALE_TO_FLOAT(src->red);
        float green = SCALE_TO_FLOAT(src->green);
        float blue = SCALE_TO_FLOAT(src->blue);
        RGBToHSL(red, green, blue, &hue, &saturation, &lightness);

        value_red = bal.colorBalanceTransform(red, lightness, m_cyan_shadows, m_cyan_midtones, m_cyan_highlights);
        value_green = bal.colorBalanceTransform(green, lightness, m_magenta_shadows, m_magenta_midtones, m_magenta_highlights);
        value_blue = bal.colorBalanceTransform(blue, lightness, m_yellow_shadows, m_yellow_midtones, m_yellow_highlights);

        if(m_preserve_luminosity)
        {
            float h2, s2, l2;
            RGBToHSL(value_red, value_green, value_blue, &h2, &s2, &l2);
            HSLToRGB(h2, s2, lightness, &value_red, &value_green, &value_blue);
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


QList<QString> parameters() const override
{
    QList<QString> list;
    list << "cyan_red_midtones"   << "magenta_green_midtones"   << "yellow_blue_midtones"
         << "cyan_red_shadows"    << "magenta_green_shadows"    << "yellow_blue_shadows"
         << "cyan_red_highlights" << "magenta_green_highlights" << "yellow_blue_highlights" << "preserve_luminosity";
    return list;
}

int parameterId(const QString& name) const override
{
    if (name == "cyan_red_midtones")
        return 0;
    else if(name == "magenta_green_midtones")
        return 1;
    else if(name == "yellow_blue_midtones")
        return 2;
    else if (name == "cyan_red_shadows")
        return 3;
    else if(name == "magenta_green_shadows")
        return 4;
    else if(name == "yellow_blue_shadows")
        return 5;
    else if (name == "cyan_red_highlights")
        return 6;
    else if(name == "magenta_green_highlights")
        return 7;
    else if(name == "yellow_blue_highlights")
        return 8;
    else if(name == "preserve_luminosity")
        return 9;
    return -1;
}

void setParameter(int id, const QVariant& parameter) override
{
    switch(id)
    {
    case 0:
        m_cyan_midtones = parameter.toDouble();
        break;
    case 1:
        m_magenta_midtones = parameter.toDouble();
        break;
    case 2:
        m_yellow_midtones = parameter.toDouble();
        break;
    case 3:
        m_cyan_shadows = parameter.toDouble();
        break;
    case 4:
        m_magenta_shadows = parameter.toDouble();
        break;
    case 5:
        m_yellow_shadows = parameter.toDouble();
        break;
    case 6:
        m_cyan_highlights = parameter.toDouble();
        break;
    case 7:
        m_magenta_highlights = parameter.toDouble();
        break;
    case 8:
        m_yellow_highlights = parameter.toDouble();
        break;
    case 9:
        m_preserve_luminosity = parameter.toBool();
        break;
    default:
        ;
    }
}
private:

    double m_cyan_midtones {0.0};
    double m_magenta_midtones {0.0};
    double m_yellow_midtones {0.0};
    double m_cyan_shadows {0.0};
    double m_magenta_shadows {0.0};
    double m_yellow_shadows {0.0};
    double m_cyan_highlights {0.0};
    double m_magenta_highlights {0.0};
    double m_yellow_highlights {0.0};

    bool m_preserve_luminosity {true};
};

 KisColorBalanceAdjustmentFactory::KisColorBalanceAdjustmentFactory()
    : KoColorTransformationFactory("ColorBalance")
{
}

QList< QPair< KoID, KoID > > KisColorBalanceAdjustmentFactory::supportedModels() const
{
    QList< QPair< KoID, KoID > > l;
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Integer8BitsColorDepthID));
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Integer16BitsColorDepthID));
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Float16BitsColorDepthID));
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Float32BitsColorDepthID));
    return l;
}

KoColorTransformation* KisColorBalanceAdjustmentFactory::createTransformation(const KoColorSpace* colorSpace, QHash<QString, QVariant> parameters) const
{
    KoColorTransformation * adj;
    if (colorSpace->colorModelId() != RGBAColorModelID) {
        dbgKrita << "Unsupported color space " << colorSpace->id() << " in KisColorBalanceAdjustment::createTransformation";
        return 0;
    }
    if (colorSpace->colorDepthId() == Float32BitsColorDepthID) {
        adj = new KisColorBalanceAdjustment< float, KoRgbTraits < float > >();
    }
#ifdef HAVE_OPENEXR
    else if (colorSpace->colorDepthId() == Float16BitsColorDepthID) {
        adj = new KisColorBalanceAdjustment< half, KoRgbTraits < half > >();
    }
#endif
    else if (colorSpace->colorDepthId() == Integer16BitsColorDepthID) {
        adj = new KisColorBalanceAdjustment< quint16, KoBgrTraits < quint16 > >();
    } else if (colorSpace->colorDepthId() == Integer8BitsColorDepthID) {
        adj = new KisColorBalanceAdjustment< quint8, KoBgrTraits < quint8 > >();
    } else {
        dbgKrita << "Unsupported color space " << colorSpace->id() << " in KisColorBalanceAdjustment::createTransformation";
        return 0;
    }
    adj->setParameters(parameters);
    return adj;

}

KisColorBalanceMath::KisColorBalanceMath(){}


float KisColorBalanceMath::colorBalanceTransform(float value, float lightness, float shadows, float midtones, float highlights)
{
      static const float a = 0.25, b = 0.333, scale = 0.7;

      shadows *= CLAMP ((lightness - b) / -a + 0.5, 0, 1) * scale;
      midtones *= CLAMP ((lightness - b) /  a + 0.5, 0, 1) * CLAMP ((lightness + b - 1) / -a + 0.5, 0, 1) * scale;
      highlights *= CLAMP ((lightness + b - 1) /  a + 0.5, 0, 1) * scale;

      value += shadows;
      value += midtones;
      value += highlights;
      value = CLAMP (value, 0.0, 1.0);

      return value;
}

