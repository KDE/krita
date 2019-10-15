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

#include <QByteArray>

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

#define FLOAT_CLAMP( v ) *v = (*v < 0.0) ? 0.0 : ( (*v>1.0) ? 1.0 : *v )

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

#include "kis_global.h"

struct AddPolicy
{
    static inline float adjustValue(float v, float dv) {
        return v + dv;
    }
};


struct MultiplyPolicy
{
    static inline float adjustValue(float v, float dv) {
        if (dv < 0) {
            v *= dv + 1.0f;
        } else {
            v += dv * (1.0f - v);
        }
        return v;
    }
};

static inline void writeRGBSimple(float *r, float *g, float *b,
                                  int sextant,
                                  float x, float m, float M)
{
    switch (sextant) {
    case 0: *r = M; *g = x + m; *b = m; break;
    case 1: *r = x + m; *g = M; *b = m; break;
    case 2: *r = m; *g = M; *b = x + m; break;
    case 3: *r = m; *g = x + m; *b = M; break;
    case 4: *r = x + m; *g = m; *b = M; break;
    case 5: *r = M; *g = m; *b = x + m; break;
    }
}

struct HSVPolicy
{
    inline bool hasChroma(float v) {
        static const float EPSILON = 1e-9f;
        return v > EPSILON;
    }

    inline float valueFromRGB(float r, float g, float b, float m, float M) {
        Q_UNUSED(r);
        Q_UNUSED(g);
        Q_UNUSED(b);
        Q_UNUSED(m);
        return M;
    }

    inline float fixupValueAfterChroma(float c, float v) {
        return qMax(v, c);
    }

    inline float fixupChromaAfterValue(float c, float v) {
        return qMin(v, c);
    }

    inline void writeRGB(float *r, float *g, float *b,
                                int sextant,
                                float x, float c, float v) {

        const float m = v - c;
        writeRGBSimple(r, g, b, sextant, x, m, v);
    }
};

struct HSLPolicy
{
    inline bool hasChroma(float v) {
        static const float EPSILON = 1e-9f;
        return v > EPSILON && v < 1.0f - EPSILON;
    }

    inline float valueFromRGB(float r, float g, float b, float m, float M) {
        Q_UNUSED(r);
        Q_UNUSED(g);
        Q_UNUSED(b);
        return 0.5f * (M + m);
    }

    inline float fixupValueAfterChroma(float c, float v) {
        if (v >= 0.5f) {
            if (c > 2.0f - 2.0f * v) {
                v = 1.0f - 0.5f * c;
            }
        } else {
            if (c > 2.0f * v) {
                v = 0.5f * c;
            }
        }
        return v;
    }

    inline float fixupChromaAfterValue(float c, float v) {
        if (v >= 0.5f) {
            c = qMin(c, 2.0f - 2.0f * v);
        } else {
            c = qMin(c, 2.0f * v);
        }

        return c;
    }

    inline void writeRGB(float *r, float *g, float *b,
                                int sextant,
                                float x, float c, float v) {

        const float M = v + 0.5f * c;
        const float m = v - 0.5f * c;

        writeRGBSimple(r, g, b, sextant, x, m, M);
    }
};

struct HCIPolicy
{
    inline bool hasChroma(float v) {
        static const float EPSILON = 1e-9f;
        return v > EPSILON && v < 1.0f - EPSILON;
    }

    inline float valueFromRGB(float r, float g, float b, float m, float M) {
        Q_UNUSED(m);
        Q_UNUSED(M);
        return (r + g + b) / 3.0f;
    }

    inline float fixupValueAfterChroma(float c, float v) {
        static const float oneThird = 1.0f / 3.0f;
        static const float twoThirds = 2.0f / 3.0f;

        if (v >= oneThird) {
            v = qMin(v, 1.0f - twoThirds * c);
        } else {
            v = qMax(v, oneThird * c);
        }
        return v;
    }

    inline float fixupChromaAfterValue(float c, float v) {
        static const float oneThird = 1.0f / 3.0f;

        if (v >= oneThird) {
            c = qMin(c, 1.5f * (1.0f - v));
        } else {
            c = qMin(c, 3.0f * v);
        }

        return c;
    }

    inline void writeRGB(float *r, float *g, float *b,
                                int sextant,
                                float x, float c, float v) {

        static const float oneThird = 1.0f / 3.0f;

        const float m = v - oneThird * (c + x);
        const float M = c + m;

        writeRGBSimple(r, g, b, sextant, x, m, M);
    }
};

struct HCYPolicy
{
    HCYPolicy(float _rCoeff = 0.299f, float _gCoeff = 0.587f, float _bCoeff = 0.114f)
        : rCoeff(_rCoeff),
          gCoeff(_gCoeff),
          bCoeff(_bCoeff)
    {
    }

    const float rCoeff = 0.299f;
    const float gCoeff = 0.587f;
    const float bCoeff = 0.114f;

    inline bool hasChroma(float v) {
        static const float EPSILON = 1e-9f;
        return v > EPSILON && v < 1.0f - EPSILON;
    }

    inline float valueFromRGB(float r, float g, float b, float m, float M) {
        Q_UNUSED(m);
        Q_UNUSED(M);
        return rCoeff * r + gCoeff * g + bCoeff * b;
    }

    inline float fixupValueAfterChroma(float c, float v) {
        // NOTE: no sliding in HCY, because the shape of the triangle
        //       depends on Hue, which complicated code a lot. And it
        //       seems to work fine without it :)
        return v;
    }

    inline float fixupChromaAfterValue(float c, float v) {
        // NOTE: no sliding in HCY, because the shape of the triangle
        //       depends on Hue, which complicated code a lot. And it
        //       seems to work fine without it :)
        return c;
    }

    inline void writeRGB(float *r, float *g, float *b,
                         int sextant,
                         float x, float c, float v) {


        switch (sextant) {
        case 0: *r = c; *g = x; *b = 0; break;
        case 1: *r = x; *g = c; *b = 0; break;
        case 2: *r = 0; *g = c; *b = x; break;
        case 3: *r = 0; *g = x; *b = c; break;
        case 4: *r = x; *g = 0; *b = c; break;
        case 5: *r = c; *g = 0; *b = x; break;
        }

        const float m = v - *r * rCoeff - *g * gCoeff - *b * bCoeff;
        *r += m;
        *g += m;
        *b += m;
    }
};

template <class ValueAdjustPolicy, class ValuePolicy>
void HSVTransform(float *r, float *g, float *b, float dh, float ds, float dv, ValuePolicy valuePolicy)
{
    static const float EPSILON = 1e-9f;

    float h;

    float M = qMax(*r, qMax(*g, *b));
    float m = qMin(*r, qMin(*g, *b));

    float chroma = M - m;

    float v = valuePolicy.valueFromRGB(*r, *g, *b, m, M);

    if (!valuePolicy.hasChroma(v)) {
        chroma = 0.0f;
        h = 0.0f;
        v = qBound(0.0f, ValueAdjustPolicy::adjustValue(v, dv), 1.0f);
    } else {
        if (chroma > EPSILON) {
            if (*r == M)
                h = (*g - *b) / chroma;
            else if (*g == M)
                h = 2 + (*b - *r) / chroma;
            else
                h = 4 + (*r - *g) / chroma;

            h *= 60;
            h += dh * 180;

            h = normalizeAngleDegrees(h);
            chroma = qBound(0.0f, chroma * (ds + 1.0f), 1.0f);
        } else {
            h = 0.0f;
        }

        v = valuePolicy.fixupValueAfterChroma(chroma, v);
        v = ValueAdjustPolicy::adjustValue(v, dv);
        v = qBound(0.0f, v, 1.0f);
        chroma = valuePolicy.fixupChromaAfterValue(chroma, v);
    }

    if (v <= EPSILON) {
        *r = *g = *b = 0.0;
    } else {
        h /= 60.0f;
        const int sextant = static_cast<int>(h);
        const float fract = h - sextant;

        const float x =
            sextant & 0x1 ?
            chroma - chroma * fract :
            chroma * fract;

        valuePolicy.writeRGB(r, g, b, sextant, x, chroma, v);
    }
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
        m_colorize(false),
        m_compatibilityMode(true)
    {
    }

public:

    void transform(const quint8 *srcU8, quint8 *dstU8, qint32 nPixels) const override
    {

        //if (m_model="RGBA" || m_colorize) {
        /*It'd be nice to have LCH automatically selector for LAB in the future, but I don't know how to select LAB
         * */
            const RGBPixel* src = reinterpret_cast<const RGBPixel*>(srcU8);
            RGBPixel* dst = reinterpret_cast<RGBPixel*>(dstU8);
            float h, s, v;
            float r = 0.0;
            float g = 0.0;
            float b = 0.0;
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

                } else {

                    if (m_type == 0) {
                        if (!m_compatibilityMode) {
                            r = SCALE_TO_FLOAT(src->red);
                            g = SCALE_TO_FLOAT(src->green);
                            b = SCALE_TO_FLOAT(src->blue);
                            HSVTransform<MultiplyPolicy>(&r, &g, &b, m_adj_h, m_adj_s, m_adj_v, HSVPolicy());
                        } else {
                            RGBToHSV(SCALE_TO_FLOAT(src->red), SCALE_TO_FLOAT(src->green), SCALE_TO_FLOAT(src->blue), &h, &s, &v);
                            h += m_adj_h * 180;
                            h = normalizeAngleDegrees(h);
                            s += m_adj_s;
                            v += m_adj_v;
                            HSVToRGB(h, s, v, &r, &g, &b);
                        }
                    } else if (m_type == 1) {

                        if (!m_compatibilityMode) {
                            r = SCALE_TO_FLOAT(src->red);
                            g = SCALE_TO_FLOAT(src->green);
                            b = SCALE_TO_FLOAT(src->blue);
                            HSVTransform<MultiplyPolicy>(&r, &g, &b, m_adj_h, m_adj_s, m_adj_v, HSLPolicy());
                        } else {
                            RGBToHSL(SCALE_TO_FLOAT(src->red), SCALE_TO_FLOAT(src->green), SCALE_TO_FLOAT(src->blue), &h, &s, &v);

                            h += m_adj_h * 180;
                            h = normalizeAngleDegrees(h);
                            s *= (m_adj_s + 1.0);
                            if (m_adj_v < 0) {
                                v *= (m_adj_v + 1.0);
                            } else {
                                v += (m_adj_v * (1.0 - v));
                            }
                            HSLToRGB(h, s, v, &r, &g, &b);
                        }
                    } else if (m_type == 2) {

                        if (!m_compatibilityMode) {
                            r = SCALE_TO_FLOAT(src->red);
                            g = SCALE_TO_FLOAT(src->green);
                            b = SCALE_TO_FLOAT(src->blue);
                            HSVTransform<MultiplyPolicy>(&r, &g, &b, m_adj_h, m_adj_s, m_adj_v, HCIPolicy());
                        } else {
                            qreal red = SCALE_TO_FLOAT(src->red);
                            qreal green = SCALE_TO_FLOAT(src->green);
                            qreal blue = SCALE_TO_FLOAT(src->blue);
                            qreal hue, sat, intensity;
                            RGBToHCI(red, green, blue, &hue, &sat, &intensity);

                            hue *= 360.0;
                            hue += m_adj_h * 180;
                            hue = normalizeAngleDegrees(hue);
                            sat *= (m_adj_s + 1.0);
                            intensity += m_adj_v;

                            HCIToRGB(hue/360.0, sat, intensity, &red, &green, &blue);

                            r = red;
                            g = green;
                            b = blue;
                        }
                    } else if (m_type == 3) {

                        if (!m_compatibilityMode) {
                            r = SCALE_TO_FLOAT(src->red);
                            g = SCALE_TO_FLOAT(src->green);
                            b = SCALE_TO_FLOAT(src->blue);
                            HSVTransform<MultiplyPolicy>(&r, &g, &b, m_adj_h, m_adj_s, m_adj_v, HCYPolicy(lumaR, lumaG, lumaB));
                        } else {
                            qreal red = SCALE_TO_FLOAT(src->red);
                            qreal green = SCALE_TO_FLOAT(src->green);
                            qreal blue = SCALE_TO_FLOAT(src->blue);
                            qreal hue, sat, luma;
                            RGBToHCY(red, green, blue, &hue, &sat, &luma, lumaR, lumaG, lumaB);

                            hue *= 360.0;
                            hue += m_adj_h * 180;
                            hue = normalizeAngleDegrees(hue);
                            sat *= (m_adj_s + 1.0);
                            luma += m_adj_v;

                            HCYToRGB(hue/360.0, sat, luma, &red, &green, &blue, lumaR, lumaG, lumaB);
                            r = red;
                            g = green;
                            b = blue;
                        }

                    } else if (m_type == 4) {

                        qreal red = SCALE_TO_FLOAT(src->red);
                        qreal green = SCALE_TO_FLOAT(src->green);
                        qreal blue = SCALE_TO_FLOAT(src->blue);
                        qreal y, cb, cr;
                        RGBToYUV(red, green, blue, &y, &cb, &cr, lumaR, lumaG, lumaB);

                        cb *= (m_adj_h + 1.0);
                        cr *= (m_adj_s + 1.0);
                        y += (m_adj_v);

                        YUVToRGB(y, cb, cr, &red, &green, &blue, lumaR, lumaG, lumaB);
                        r = red;
                        g = green;
                        b = blue;
                    } else {
                        Q_ASSERT_X(false, "", "invalid type");
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

    QList<QString> parameters() const override
    {
      QList<QString> list;
      list << "h" << "s" << "v" << "type" << "colorize" << "lumaRed" << "lumaGreen"<< "lumaBlue" << "compatibilityMode";
      return list;
    }

    int parameterId(const QString& name) const override
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
        } else if (name == "compatibilityMode") {
            return 8;
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
    void setParameter(int id, const QVariant& parameter) override
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
            m_type = parameter.toInt();
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
        case 8:
            m_compatibilityMode = parameter.toBool();
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
    bool m_compatibilityMode;
};

template<typename _channel_type_,typename traits>
class KisHSVCurveAdjustment : public KoColorTransformation
{
    typedef traits RGBTrait;
    typedef typename RGBTrait::Pixel RGBPixel;

public:
    KisHSVCurveAdjustment() :
        m_lumaRed(0.0),
        m_lumaGreen(0.0),
        m_lumaBlue(0.0)
    {}

    QList<QString> parameters() const override
    {
      QList<QString> list;
      list << "curve" << "channel" << "driverChannel" << "relative" << "lumaRed" << "lumaGreen"<< "lumaBlue";
      return list;
    }

    int parameterId(const QString& name) const override
    {
        if (name == "curve") {
            return PAR_CURVE;
        } else if (name == "channel") {
            return PAR_CHANNEL;
        } else if (name == "driverChannel") {
            return PAR_DRIVER_CHANNEL;
        } else if (name == "relative") {
            return PAR_RELATIVE;
        } else if (name == "lumaRed") {
            return PAR_LUMA_R;
        } else if (name == "lumaGreen") {
            return PAR_LUMA_G;
        } else if (name == "lumaBlue") {
            return PAR_LUMA_B;
        }
        return -1;
    }

    /**
    * curve: adjustment curve as QVector<quin16>
    * channel: which channel to adjust. See KisHSVCurve::ColorChannel.
    * driverChannel: which channel to use as source for adjustments.
    * relative:
    *   false: use curve for direct lookup.
    *   true: add adjustment to original. In this mode, the curve range is mapped to -1.0 to 1.0
    * luma Red/Green/Blue: Used for luma calculations.
    */
    void setParameter(int id, const QVariant& parameter) override
    {
        switch(id)
        {
        case PAR_CURVE:
            m_curve = parameter.value<QVector<quint16>>();
            break;
        case PAR_CHANNEL:
        case PAR_DRIVER_CHANNEL: {
            int channel = parameter.toInt();
            KIS_ASSERT_RECOVER_RETURN(0 <= channel && channel < KisHSVCurve::ChannelCount && "Invalid channel. Ignored!");

            if (id == PAR_CHANNEL) {
                m_channel = channel;
            } else {
                m_driverChannel = channel;
            }
            } break;
        case PAR_RELATIVE:
            m_relative = parameter.toBool();
            break;
        case PAR_LUMA_R:
            m_lumaRed = parameter.toDouble();
            break;
        case PAR_LUMA_G:
            m_lumaGreen = parameter.toDouble();
            break;
        case PAR_LUMA_B:
            m_lumaBlue = parameter.toDouble();
            break;
        default:
            KIS_ASSERT_RECOVER_NOOP(false && "Unknown parameter ID. Ignored!");
        }
    }

    const float SCALE_FROM_16BIT = 1.0f / 0xFFFF;
    void transform(const quint8 *srcU8, quint8 *dstU8, qint32 nPixels) const override
    {
        const RGBPixel* src = reinterpret_cast<const RGBPixel*>(srcU8);
        RGBPixel* dst = reinterpret_cast<RGBPixel*>(dstU8);
        float max = m_curve.size() - 1;

        int driverChannel = m_relative ? m_driverChannel : m_channel;

        float component[KisHSVCurve::ChannelCount];

        // Aliases for convenience
        float &h = component[KisHSVCurve::Hue];
        float &s = component[KisHSVCurve::Saturation];
        float &v = component[KisHSVCurve::Value];
        float &r = component[KisHSVCurve::Red];
        float &g = component[KisHSVCurve::Green];
        float &b = component[KisHSVCurve::Blue];
        float &a = component[KisHSVCurve::Alpha];

        while (nPixels > 0) {
            r = SCALE_TO_FLOAT(src->red);
            g = SCALE_TO_FLOAT(src->green);
            b = SCALE_TO_FLOAT(src->blue);
            a = SCALE_TO_FLOAT(src->alpha);

            RGBToHSV(r, g, b, &h, &s, &v);

            // Normalize hue to 0.0 to 1.0 range
            h /= 360.0f;

            float adjustment = lookupComponent(component[driverChannel], max) * SCALE_FROM_16BIT;

            if (m_relative) {
                // Curve uses range 0.0 to 1.0, but for adjustment we need -1.0 to 1.0
                adjustment = 2.0f * adjustment - 1.0f;

                if (m_channel == KisHSVCurve::AllColors) {
                    r += adjustment;
                    g += adjustment;
                    b += adjustment;
                } else {
                    component[m_channel] += adjustment;
                }
            } else {
                if (m_channel == KisHSVCurve::AllColors) {
                    r = b = g = adjustment;
                } else {
                    component[m_channel] = adjustment;
                }
            }

            h *= 360.0f;
            if (h > 360) h -= 360;
            if (h < 0) h += 360;

            if (m_channel >= KisHSVCurve::Hue) {
                HSVToRGB(h, s, v, &r, &g, &b);
            }

            clamp< _channel_type_ >(&r, &g, &b);
            FLOAT_CLAMP(&a);

            dst->red = SCALE_FROM_FLOAT(r);
            dst->green = SCALE_FROM_FLOAT(g);
            dst->blue = SCALE_FROM_FLOAT(b);
            dst->alpha = SCALE_FROM_FLOAT(a);

            --nPixels;
            ++src;
            ++dst;
        }
    }


    float lookupComponent(float x, float max) const
    {
        // No curve for this component? Pass through unmodified
        if (max < 2) return x;
        if (x < 0) return m_curve[0];

        float lookup = x * max;
        float base = floor(lookup);
        float offset = lookup - base;

        if (base >= max) {
            base = max - 1.0f;
            offset = 1.0f;
        }
        int index = (int)base;

        return (1.0f - offset) * m_curve[index]
                     + offset  * m_curve[index + 1];
    }


private:
    enum ParameterID
    {
        PAR_CURVE,
        PAR_CHANNEL,
        PAR_DRIVER_CHANNEL,
        PAR_RELATIVE,
        PAR_LUMA_R,
        PAR_LUMA_G,
        PAR_LUMA_B,
    };

    QVector<quint16> m_curve;
    int m_channel = 0;
    int m_driverChannel = 0;
    bool m_relative = false;

    /* Note: the filter currently only supports HSV, so these are
     * unused, but will be needed once HSL, etc.
     */
    qreal m_lumaRed, m_lumaGreen, m_lumaBlue;
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


KisHSVCurveAdjustmentFactory::KisHSVCurveAdjustmentFactory()
    : KoColorTransformationFactory("hsv_curve_adjustment")
{
}

QList< QPair< KoID, KoID > > KisHSVCurveAdjustmentFactory::supportedModels() const
{
    QList< QPair< KoID, KoID > > l;
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Integer8BitsColorDepthID));
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Integer16BitsColorDepthID));
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Float16BitsColorDepthID));
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Float32BitsColorDepthID));
    return l;
}

KoColorTransformation* KisHSVCurveAdjustmentFactory::createTransformation(const KoColorSpace* colorSpace, QHash<QString, QVariant> parameters) const
{
    KoColorTransformation * adj;
    if (colorSpace->colorModelId() != RGBAColorModelID) {
        dbgKrita << "Unsupported color space " << colorSpace->id() << " in KisHSVCurveAdjustmentFactory::createTransformation";
        return 0;
    }
    if (colorSpace->colorDepthId() == Integer8BitsColorDepthID) {
        adj = new KisHSVCurveAdjustment< quint8, KoBgrTraits < quint8 > >();
    } else if (colorSpace->colorDepthId() == Integer16BitsColorDepthID) {
        adj = new KisHSVCurveAdjustment< quint16, KoBgrTraits < quint16 > >();
    }
#ifdef HAVE_OPENEXR
    else if (colorSpace->colorDepthId() == Float16BitsColorDepthID) {
        adj = new KisHSVCurveAdjustment< half, KoRgbTraits < half > >();
    }
#endif
    else if (colorSpace->colorDepthId() == Float32BitsColorDepthID) {
        adj = new KisHSVCurveAdjustment< float, KoRgbTraits < float > >();
    } else {
        dbgKrita << "Unsupported color space " << colorSpace->id() << " in KisHSVCurveAdjustmentFactory::createTransformation";
        return 0;
    }
    adj->setParameters(parameters);
    return adj;

}
