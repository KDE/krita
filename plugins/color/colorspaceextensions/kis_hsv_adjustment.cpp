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

#include "kis_hsv_adjustment.h"
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

#define SCALE_TO_FLOAT( v ) KoColorSpaceMaths< _channel_type_, float>::scaleToA( v )
#define SCALE_FROM_FLOAT( v  ) KoColorSpaceMaths< float, _channel_type_>::scaleToA( v )

template<typename _channel_type_>
void clamp(float* r, float* g, float* b);

#define FLOAT_CLAMP( v ) * v = (*v < 0.0) ? 0.0 : ( (*v>1.0) ? 1.0 : *v )

template<>
void clamp<quint8>(float* r, float* g, float* b)
{
    FLOAT_CLAMP(r);
    FLOAT_CLAMP(g);
    FLOAT_CLAMP(b);
}

template<>
void clamp<quint16>(float* r, float* g, float* b)
{
    FLOAT_CLAMP(r);
    FLOAT_CLAMP(g);
    FLOAT_CLAMP(b);
}

#ifdef HAVE_OPENEXR
template<>
void clamp<half>(float* r, float* g, float* b)
{
    Q_UNUSED(r);
    Q_UNUSED(g);
    Q_UNUSED(b);
}
#endif

template<>
void clamp<float>(float* r, float* g, float* b)
{
    Q_UNUSED(r);
    Q_UNUSED(g);
    Q_UNUSED(b);
}


template<typename _channel_type_,typename traits>
class KisHSVAdjustment : public KoColorTransformation
{
    typedef traits RGBTrait;
    typedef typename RGBTrait::Pixel RGBPixel;

public:
    KisHSVAdjustment() :
        m_adj_h(0.0),
        m_adj_s(0.0),
        m_adj_v(0.0),
        m_lumaRed(0.0),
        m_lumaGreen(0.0),
        m_lumaBlue(0.0),
        m_type(0),
        m_colorize(false)
    {
    }

public:

    void transform(const quint8 *srcU8, quint8 *dstU8, qint32 nPixels) const
    {

        //if (m_model="RGBA" || m_colorize) {
        /*It'd be nice to have LCH automatically selector for LAB in the future, but I don't know how to select LAB 
         * */
            const RGBPixel* src = reinterpret_cast<const RGBPixel*>(srcU8);
            RGBPixel* dst = reinterpret_cast<RGBPixel*>(dstU8);
            float h, s, v, r, g, b;
            qreal lumaR, lumaG, lumaB;
            //Default to rec 709 when there's no coefficients given//
            if (m_lumaRed<=0 || m_lumaGreen<=0 || m_lumaBlue<=0) {
                lumaR   = 0.2126;
                lumaG   = 0.7152;
                lumaB   = 0.0722;
            } else {
                lumaR   = m_lumaRed;
                lumaG   = m_lumaGreen;
                lumaB   = m_lumaBlue;
            }
            while (nPixels > 0) {

                if (m_colorize) {
                    h = m_adj_h * 360;
                    if (h >= 360.0) h = 0;

                    s = m_adj_s;

                    r = SCALE_TO_FLOAT(src->red);
                    g = SCALE_TO_FLOAT(src->green);
                    b = SCALE_TO_FLOAT(src->blue);

                    float luminance = r * lumaR + g * lumaG + b * lumaB;

                    if (m_adj_v > 0) {
                        luminance *= (1.0 - m_adj_v);
                        luminance += 1.0 - (1.0 - m_adj_v);
                    }
                    else if (m_adj_v < 0 ){
                        luminance *= (m_adj_v + 1.0);
                    }
                    v = luminance;
                    HSLToRGB(h, s, v, &r, &g, &b);

                }
                else {

                    if (m_type == 0) {
                        RGBToHSV(SCALE_TO_FLOAT(src->red), SCALE_TO_FLOAT(src->green), SCALE_TO_FLOAT(src->blue), &h, &s, &v);
                        h += m_adj_h * 180;
                        if (h > 360) h -= 360;
                        if (h < 0) h += 360;
                        s += m_adj_s;
                        v += m_adj_v;
                        HSVToRGB(h, s, v, &r, &g, &b);
                    } else if (m_type == 1) {

                        RGBToHSL(SCALE_TO_FLOAT(src->red), SCALE_TO_FLOAT(src->green), SCALE_TO_FLOAT(src->blue), &h, &s, &v);

                        h += m_adj_h * 180;
                        if (h > 360) h -= 360;
                        if (h < 0) h += 360;

                        s *= (m_adj_s + 1.0);
                        if (s < 0.0) s = 0.0;
                        if (s > 1.0) s = 1.0;

                        if (m_adj_v < 0)
                            v *= (m_adj_v + 1.0);
                        else
                            v += (m_adj_v * (1.0 - v));


                        HSLToRGB(h, s, v, &r, &g, &b);
                    } else if (m_type == 2){

                        qreal red = SCALE_TO_FLOAT(src->red);
                        qreal green = SCALE_TO_FLOAT(src->green);
                        qreal blue = SCALE_TO_FLOAT(src->blue);
                        qreal hue, sat, intensity;
                        RGBToHCI(red, green, blue, &hue, &sat, &intensity);

                        hue *=360.0;
                        hue += m_adj_h * 180;
                        //if (intensity+m_adj_v>1.0){hue+=180.0;}
                        if (hue < 0) hue += 360;
                        hue = fmod(hue, 360.0);

                        sat *= (m_adj_s + 1.0);
                        //sat = qBound(0.0, sat, 1.0);
                        
                        intensity += (m_adj_v);

                        HCIToRGB(hue/360.0, sat, intensity, &red, &green, &blue);

                        r = red;
                        g = green;
                        b = blue;
                    } else if (m_type == 3){

                        qreal red = SCALE_TO_FLOAT(src->red);
                        qreal green = SCALE_TO_FLOAT(src->green);
                        qreal blue = SCALE_TO_FLOAT(src->blue);
                        qreal hue, sat, luma;
                        RGBToHCY(red, green, blue, &hue, &sat, &luma, lumaR, lumaG, lumaB);

                        hue *=360.0;
                        hue += m_adj_h * 180;
                        //if (luma+m_adj_v>1.0){hue+=180.0;}
                        if (hue < 0) hue += 360;
                        hue = fmod(hue, 360.0);

                        sat *= (m_adj_s + 1.0);
                        //sat = qBound(0.0, sat, 1.0);

                        luma += m_adj_v;


                        HCYToRGB(hue/360.0, sat, luma, &red, &green, &blue, lumaR, lumaG, lumaB);
                        r = red;
                        g = green;
                        b = blue;
                        
                    } else if (m_type == 4){

                        qreal red = SCALE_TO_FLOAT(src->red);
                        qreal green = SCALE_TO_FLOAT(src->green);
                        qreal blue = SCALE_TO_FLOAT(src->blue);
                        qreal y, cb, cr;
                        RGBToYUV(red, green, blue, &y, &cb, &cr, lumaR, lumaG, lumaB);

                        cb *= (m_adj_h + 1.0);
                        //cb = qBound(0.0, cb, 1.0);

                        cr *= (m_adj_s + 1.0);
                        //cr = qBound(0.0, cr, 1.0);

                        
                        
                        y += (m_adj_v);


                        YUVToRGB(y, cb, cr, &red, &green, &blue, lumaR, lumaG, lumaB);
                        r = red;
                        g = green;
                        b = blue;
                    }
                }

                clamp< _channel_type_ >(&r, &g, &b);
                dst->red = SCALE_FROM_FLOAT(r);
                dst->green = SCALE_FROM_FLOAT(g);
                dst->blue = SCALE_FROM_FLOAT(b);
                dst->alpha = src->alpha;

                --nPixels;
                ++src;
                ++dst;
            }
        /*} else if (m_model="LABA"){
            const LABPixel* src = reinterpret_cast<const LABPixel*>(srcU8);
            LABPixel* dst = reinterpret_cast<LABPixel*>(dstU8);
            qreal lightness = SCALE_TO_FLOAT(src->L);
            qreal a = SCALE_TO_FLOAT(src->a);
            qreal b = SCALE_TO_FLOAT(src->b);
            qreal L, C, H;
            
            while (nPixels > 0) {
                if (m_type = 4) {
                    a *= (m_adj_h + 1.0);
                    a = qBound(0.0, a, 1.0);

                    b *= (m_adj_s + 1.0);
                    b = qBound(0.0, b, 1.0);

                    if (m_adj_v < 0)
                        lightness *= (m_adj_v + 1.0);
                    else
                        lightness += (m_adj_v * (1.0 - lightness));
                } else {//lch
                    LABToLCH(lightness, a, b, &L, &C, &H);
                    H *=360;
                    H += m_adj_h * 180;
                    if (H > 360) h -= 360;
                    if (H < 0) h += 360;
                    C += m_adj_s;
                    C = qBound(0.0,C,1.0);
                    L += m_adj_v;
                    L = qBound(0.0,L,1.0);
                    LCHToLAB(L, C, H/360.0, &lightness, &a, &b);
                }
                clamp< _channel_type_ >(&lightness, &a, &b);
                dst->L = SCALE_FROM_FLOAT(lightness);
                dst->a = SCALE_FROM_FLOAT(a);
                dst->b = SCALE_FROM_FLOAT(b);
                dst->alpha = src->alpha;

                --nPixels;
                ++src;
                ++dst;
                }
        }*/
    }

    virtual QList<QString> parameters() const
    {
      QList<QString> list;
      list << "h" << "s" << "v" << "type" << "colorize" << "lumaRed" << "lumaGreen"<< "lumaBlue";
      return list;
    }

    virtual int parameterId(const QString& name) const
    {
        if (name == "h") {
            return 0;
        } else if (name == "s") {
            return 1;
        } else if (name == "v") {
            return 2;
        } else if (name == "type") {
            return 3;
        } else if (name == "colorize") {
            return 4;
        } else if (name == "lumaRed") {
            return 5;
        } else if (name == "lumaGreen") {
            return 6;
        } else if (name == "lumaBlue") {
            return 7;
        }
        return -1;
    }
    
    /**
    * name - "h", "s" or "v"
    * (h)ue in range <-1.0, 1.0> ( for user, show as -180, 180 or 0, 360 for colorize)
    * (s)aturation in range <-1.0, 1.0> ( for user, show -100, 100, or 0, 100 for colorize)
    * (v)alue in range <-1.0, 1.0> (for user, show -100, 100)
    * type: 0:HSV, 1:HSL, 2:HSI, 3:HSY, 4:YUV
    * m_colorize: Use colorize formula instead
    * luma Red/Green/Blue: Used for luma calculations.
    */
    virtual void setParameter(int id, const QVariant& parameter)
    {
        switch(id)
        {
        case 0:
            m_adj_h = parameter.toDouble();
            break;
        case 1:
            m_adj_s = parameter.toDouble();
            break;
        case 2:
            m_adj_v = parameter.toDouble();
            break;
        case 3:
            m_type = parameter.toDouble();
            break;
        case 4:
            m_colorize = parameter.toBool();
            break;
        case 5:
            m_lumaRed = parameter.toDouble();
            break;
        case 6:
            m_lumaGreen = parameter.toDouble();
            break;
        case 7:
            m_lumaBlue = parameter.toDouble();
            break;
        default:
            KIS_ASSERT_RECOVER_NOOP(false && "Unknown parameter ID. Ignored!");
            ;
        }
    }

private:

    double m_adj_h, m_adj_s, m_adj_v;
    qreal m_lumaRed, m_lumaGreen, m_lumaBlue;
    int m_type;
    bool m_colorize;
};


KisHSVAdjustmentFactory::KisHSVAdjustmentFactory()
    : KoColorTransformationFactory("hsv_adjustment")
{
}

QList< QPair< KoID, KoID > > KisHSVAdjustmentFactory::supportedModels() const
{
    QList< QPair< KoID, KoID > > l;
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Integer8BitsColorDepthID));
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Integer16BitsColorDepthID));
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Float16BitsColorDepthID));
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Float32BitsColorDepthID));
    return l;
}

KoColorTransformation* KisHSVAdjustmentFactory::createTransformation(const KoColorSpace* colorSpace, QHash<QString, QVariant> parameters) const
{
    KoColorTransformation * adj;
    if (colorSpace->colorModelId() != RGBAColorModelID) {
        dbgKrita << "Unsupported color space " << colorSpace->id() << " in KisHSVAdjustmentFactory::createTransformation";
        return 0;
    }
    if (colorSpace->colorDepthId() == Integer8BitsColorDepthID) {
        adj = new KisHSVAdjustment< quint8, KoBgrTraits < quint8 > >();
    } else if (colorSpace->colorDepthId() == Integer16BitsColorDepthID) {
        adj = new KisHSVAdjustment< quint16, KoBgrTraits < quint16 > >();
    }
#ifdef HAVE_OPENEXR
    else if (colorSpace->colorDepthId() == Float16BitsColorDepthID) {
        adj = new KisHSVAdjustment< half, KoRgbTraits < half > >();
    }
#endif
    else if (colorSpace->colorDepthId() == Float32BitsColorDepthID) {
        adj = new KisHSVAdjustment< float, KoRgbTraits < float > >();
    } else {
        dbgKrita << "Unsupported color space " << colorSpace->id() << " in KisHSVAdjustmentFactory::createTransformation";
        return 0;
    }
    adj->setParameters(parameters);
    return adj;

}
