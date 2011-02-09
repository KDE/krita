/*
 *  Copyright (c) 2006,2007,2010 Cyrille Berger <cberger@cberger.bet
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

#ifndef KOCOLORSPACEMATHS_H_
#define KOCOLORSPACEMATHS_H_

#include <cmath>
#include <limits>

#include "pigment_export.h"
#include <KoIntegerMaths.h>
#include "KoChannelInfo.h"
#include "KoLut.h"

#undef _T

/**
 * This is an empty shell that needs to be "specialized" for each possible
 * numerical type (quint8, quint16...).
 *
 * It needs to defines some static constant fields :
 * - zeroValue : the zero for this numerical type
 * - unitValue : the maximum value of the normal dynamic range
 * - max : the maximum value
 * - min : the minimum value
 * - epsilon : a value close to zero but different of zero
 * - bits : the bit depth
 *
 * And some types :
 * - compositetype the type used for composite operations (usually one with
 *   a higher bit depth)
 */
template<typename _T>
class KoColorSpaceMathsTraits
{
public:
};

template<>
class PIGMENTCMS_EXPORT KoColorSpaceMathsTraits<quint8>
{
public:
    typedef qint32 compositetype;
    static const quint8 zeroValue = 0;
    static const quint8 unitValue = 0x00FF;
    static const quint8 halfValue = 0x00FF / 2;
    static const quint8 max = 0x00FF;
    static const quint8 min = 0;
    static const quint8 epsilon = 1;
    static const qint8 bits = 8;
    static const KoChannelInfo::enumChannelValueType channelValueType;
};

template<>
class PIGMENTCMS_EXPORT KoColorSpaceMathsTraits<quint16>
{
public:
    typedef qint64 compositetype;
    static const quint16 zeroValue = 0;
    static const quint16 unitValue = 0xFFFF;
    static const quint16 halfValue = 0xFFFF / 2;
    static const quint16 max = 0xFFFF;
    static const quint16 min = 0;
    static const quint16 epsilon = 1;
    static const qint8 bits = 16;
    static const KoChannelInfo::enumChannelValueType channelValueType;
};

template<>
class PIGMENTCMS_EXPORT KoColorSpaceMathsTraits<qint16>
{
public:
    typedef qint64 compositetype;
    static const qint16 zeroValue = 0;
    static const qint16 unitValue = 32767;
    static const qint16 halfValue = 32767 / 2;
    static const qint16 max = 32767;
    static const qint16 min = -32768;
    static const qint16 epsilon = 1;
    static const qint8 bits = 16;
    static const KoChannelInfo::enumChannelValueType channelValueType;
};

template<>
class PIGMENTCMS_EXPORT KoColorSpaceMathsTraits<quint32>
{
public:
    typedef qint64 compositetype;
    static const quint32 zeroValue = 0;
    static const quint32 unitValue = 0xFFFFFFFF;
    static const quint32 halfValue = 0xFFFFFFFF / 2;
    static const quint32 max = 0xFFFFFFFF;
    static const quint32 min = 0;
    static const quint32 epsilon = 1;
    static const qint8 bits = 32;
    static const KoChannelInfo::enumChannelValueType channelValueType;
};

#include <KoConfig.h>
#ifdef HAVE_OPENEXR
#include <half.h>

template<>
class PIGMENTCMS_EXPORT KoColorSpaceMathsTraits<half>
{
public:
    typedef double compositetype;
    static const half zeroValue;
    static const half unitValue;
    static const half halfValue;
    static const half max;
    static const half min;
    static const half epsilon;
    static const qint8 bits = 16;
    static const KoChannelInfo::enumChannelValueType channelValueType;
};
#endif

template<>
class PIGMENTCMS_EXPORT KoColorSpaceMathsTraits<float>
{
public:
    typedef double compositetype;
    static const float zeroValue;
    static const float unitValue;
    static const float halfValue;
    static const float max;
    static const float min;
    static const float epsilon;
    static const qint8 bits = 32;
    static const KoChannelInfo::enumChannelValueType channelValueType;
};

template<>
class PIGMENTCMS_EXPORT KoColorSpaceMathsTraits<double>
{
public:
    typedef double compositetype;
    static const double zeroValue;
    static const double unitValue;
    static const double halfValue;
    static const double max;
    static const double min;
    static const double epsilon;
    static const qint8 bits = 64;
    static const KoChannelInfo::enumChannelValueType channelValueType;
};

#ifdef Q_CC_MSVC
// MSVC do not have lrint

const double _double2fixmagic = 68719476736.0*1.5;
const qint32 _shiftamt        = 16;                    //16.16 fixed point representation,

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
        #define iexp_                           0
        #define iman_                           1
#else
        #define iexp_                           1
        #define iman_                           0
#endif //BigEndian_

inline int float2int(double val)
{
    val = val + _double2fixmagic;
    return ((int*)&val)[iman_] >> _shiftamt; 
}

inline int float2int(float val)
{
    return float2int((double)val);
}

#else
#include <cmath>

inline int float2int(float x)
{
    return lrintf(x);
}

inline int float2int(double x)
{
    return lrint(x);
}

#endif

#include <KoLut.h>

template<typename _T_>
struct KoIntegerToFloat {
  inline float operator()(_T_ f) const
  {
    return f / float(KoColorSpaceMathsTraits<_T_>::max);
  }
};

struct KoLuts {

  static PIGMENTCMS_EXPORT const Ko::FullLut< KoIntegerToFloat<quint16>, float, quint16> Uint16ToFloat;
  static PIGMENTCMS_EXPORT const Ko::FullLut< KoIntegerToFloat<quint8>, float, quint8> Uint8ToFloat;
};

/**
 * This class defines some elementary operations used by various color
 * space. It's intended to be generic, but some specialization exists
 * either for optimization or just for being buildable.
 *
 * @param _T some numerical type with an existing trait
 * @param _Tdst some other numerical type with an existing trait, it is
 *              only needed if different of _T
 */
template < typename _T, typename _Tdst = _T >
class KoColorSpaceMaths
{
    typedef KoColorSpaceMathsTraits<_T> traits;
    typedef typename traits::compositetype traits_compositetype;
public:
    inline static traits_compositetype multiply(traits_compositetype a,
            typename  KoColorSpaceMathsTraits<_Tdst>::compositetype b) {
        return ((traits_compositetype)a * b) /  KoColorSpaceMathsTraits<_Tdst>::unitValue;
    }
    
    inline static traits_compositetype multiply(traits_compositetype a,
            typename  KoColorSpaceMathsTraits<_Tdst>::compositetype b,
            typename  KoColorSpaceMathsTraits<_Tdst>::compositetype c) {
        return ((traits_compositetype)a * b * c) / (KoColorSpaceMathsTraits<_Tdst>::unitValue * KoColorSpaceMathsTraits<_Tdst>::unitValue);
    }

    /**
     * Division : (a * MAX ) / b
     * @param a
     * @param b
     */
    inline static traits_compositetype divide(_T a, _Tdst b) {
        return ((traits_compositetype)a *  KoColorSpaceMathsTraits<_Tdst>::unitValue) / b;
    }

    /**
     * Blending : (a * alpha) + b * (1 - alpha)
     * @param a
     * @param b
     * @param alpha
     */
    inline static _T blend(_T a, _T b, _T alpha) {
        traits_compositetype c = (((traits_compositetype)a - (traits_compositetype)b) * alpha) / traits::unitValue;
        return c + b;
    }

    /**
     * This function will scale a value of type _T to fit into a _Tdst.
     */
    inline static _Tdst scaleToA(_T a) {
        return (traits_compositetype)a >> (traits::bits - KoColorSpaceMathsTraits<_Tdst>::bits);
    }

    inline static typename  KoColorSpaceMathsTraits<_Tdst>::compositetype clamp(typename  KoColorSpaceMathsTraits<_Tdst>::compositetype val) {
        return qBound((typename  KoColorSpaceMathsTraits<_Tdst>::compositetype) KoColorSpaceMathsTraits<_Tdst>::min,
                      val,
                      (typename  KoColorSpaceMathsTraits<_Tdst>::compositetype)KoColorSpaceMathsTraits<_Tdst>::max);
    }
};

//------------------------------ double specialization ------------------------------//
template<>
inline quint8 KoColorSpaceMaths<double, quint8>::scaleToA(double a)
{
    double v = a * 255;
    return float2int(CLAMP(v, 0, 255));
}

template<>
inline double KoColorSpaceMaths<quint8, double>::scaleToA(quint8 a)
{
    return KoLuts::Uint8ToFloat(a);
}

template<>
inline quint16 KoColorSpaceMaths<double, quint16>::scaleToA(double a)
{
    double v = a * 0xFFFF;
    return float2int(CLAMP(v, 0, 0xFFFF));
}

template<>
inline double KoColorSpaceMaths<quint16, double>::scaleToA(quint16 a)
{
    return KoLuts::Uint16ToFloat(a);
}

template<>
inline double KoColorSpaceMaths<double>::clamp(double a)
{
    return a;
}

//------------------------------ float specialization ------------------------------//

template<>
inline float KoColorSpaceMaths<double, float>::scaleToA(double a)
{
    return (float)a;
}

template<>
inline double KoColorSpaceMaths<float, double>::scaleToA(float a)
{
    return a;
}

template<>
inline quint16 KoColorSpaceMaths<float, quint16>::scaleToA(float a)
{
    float v = a * 0xFFFF;
    return (quint16)float2int(CLAMP(v, 0, 0xFFFF));
}

template<>
inline float KoColorSpaceMaths<quint16, float>::scaleToA(quint16 a)
{
    return KoLuts::Uint16ToFloat(a);
}

template<>
inline quint8 KoColorSpaceMaths<float, quint8>::scaleToA(float a)
{
    float v = a * 255;
    return (quint8)float2int(CLAMP(v, 0, 255));
}

template<>
inline float KoColorSpaceMaths<quint8, float>::scaleToA(quint8 a)
{
    return KoLuts::Uint8ToFloat(a);
}

template<>
inline float KoColorSpaceMaths<float>::blend(float a, float b, float alpha)
{
    return (a - b) * alpha + b;
}

template<>
inline double KoColorSpaceMaths<float>::clamp(double a)
{
    return a;
}

//------------------------------ half specialization ------------------------------//

#ifdef HAVE_OPENEXR

template<>
inline half KoColorSpaceMaths<double, half>::scaleToA(double a)
{
    return (half)a;
}

template<>
inline double KoColorSpaceMaths<half, double>::scaleToA(half a)
{
    return a;
}

template<>
inline float KoColorSpaceMaths<half, float>::scaleToA(half a)
{
    return a;
}

template<>
inline half KoColorSpaceMaths<float, half>::scaleToA(float a)
{
    return (half) a;
}

template<>
inline quint8 KoColorSpaceMaths<half, quint8>::scaleToA(half a)
{
    half v = a * 255;
    return (quint8)(CLAMP(v, 0, 255));
}

template<>
inline half KoColorSpaceMaths<quint8, half>::scaleToA(quint8 a)
{
    return a *(1.0 / 255.0);
}
template<>
inline quint16 KoColorSpaceMaths<half, quint16>::scaleToA(half a)
{
    double v = a * 0xFFFF;
    return (quint16)(CLAMP(v, 0, 0xFFFF));
}

template<>
inline half KoColorSpaceMaths<quint16, half>::scaleToA(quint16 a)
{
    return a *(1.0 / 0xFFFF);
}

template<>
inline half KoColorSpaceMaths<half, half>::scaleToA(half a)
{
    return a;
}

template<>
inline half KoColorSpaceMaths<half>::blend(half a, half b, half alpha)
{
    return (a - b) * alpha + b;
}

template<>
inline double KoColorSpaceMaths<half>::clamp(double a)
{
    return a;
}


#endif

//------------------------------ quint8 specialization ------------------------------//

template<>
inline qint32 KoColorSpaceMaths<quint8>::multiply(qint32 a, qint32 b)
{
    return UINT8_MULT(a, b);
}


template<>
inline qint32 KoColorSpaceMaths<quint8>::multiply(qint32 a, qint32 b, qint32 c)
{
    return UINT8_MULT3(a, b, c);
}

template<>
inline KoColorSpaceMathsTraits<quint8>::compositetype
KoColorSpaceMaths<quint8>::divide(quint8 a, quint8 b)
{
    return UINT8_DIVIDE(a, b);
}

template<>
inline quint8 KoColorSpaceMaths<quint8>::blend(quint8 a, quint8 b, quint8 c)
{
    return UINT8_BLEND(a, b, c);
}

//------------------------------ quint16 specialization ------------------------------//

template<>
inline qint64 KoColorSpaceMaths<quint16>::multiply(qint64 a, qint64 b)
{
    return UINT16_MULT(a, b);
}

template<>
inline KoColorSpaceMathsTraits<quint16>::compositetype
KoColorSpaceMaths<quint16>::divide(quint16 a, quint16 b)
{
    return UINT16_DIVIDE(a, b);
}

//------------------------------ various specialization ------------------------------//


// TODO: use more functions from KoIntegersMaths to do the computation

/// This specialization is needed because the default implementation won't work when scaling up
template<>
inline quint16 KoColorSpaceMaths<quint8, quint16>::scaleToA(quint8 a)
{
    return UINT8_TO_UINT16(a);
}

template<>
inline quint8 KoColorSpaceMaths<quint16, quint8>::scaleToA(quint16 a)
{
    return UINT16_TO_UINT8(a);
}


// Due to once again a bug in gcc, there is the need for those specialized functions:

template<>
inline quint8 KoColorSpaceMaths<quint8, quint8>::scaleToA(quint8 a)
{
    return a;
}

template<>
inline quint16 KoColorSpaceMaths<quint16, quint16>::scaleToA(quint16 a)
{
    return a;
}

template<>
inline float KoColorSpaceMaths<float, float>::scaleToA(float a)
{
    return a;
}

namespace Arithmetic
{
    const static qreal pi = 3.14159265358979323846;
    
    template<class T>
    inline T mul(T a, T b) { return T(KoColorSpaceMaths<T>::multiply(a, b)); }
    
    template<class T>
    inline T mul(T a, T b, T c) { return T(KoColorSpaceMaths<T>::multiply(a, b, c)); }
    
    template<class T>
    inline T inv(T a) { return KoColorSpaceMathsTraits<T>::unitValue - a; }
    
    template<class T>
    inline T lerp(T a, T b, T alpha) { return KoColorSpaceMaths<T>::blend(b, a, alpha); }
    
    template<class TRet, class T>
    inline TRet scale(T a) { return KoColorSpaceMaths<T,TRet>::scaleToA(a); }
    
    template<class T>
    inline typename KoColorSpaceMathsTraits<T>::compositetype
    div(T a, T b) { return KoColorSpaceMaths<T>::divide(a, b); }
    
    template<class T>
    inline T clamp(typename KoColorSpaceMathsTraits<T>::compositetype a) {
        return KoColorSpaceMaths<T>::clamp(a);
    }
    
    template<class T>
    inline T min(T a, T b, T c) {
        b = (a < b) ? a : b;
        return (b < c) ? b : c;
    }
    
    template<class T>
    inline T max(T a, T b, T c) {
        b = (a > b) ? a : b;
        return (b > c) ? b : c;
    }
    
    template<class T>
    inline T unionShapeOpacy(T a, T b) {
        typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
        return T(composite_type(a) + b - mul(a,b));
    }
    
    template<class T>
    inline T blend(T src, T srcAlpha, T dst, T dstAlpha, T cfValue) {
        return mul(inv(srcAlpha), dstAlpha, dst) + mul(inv(dstAlpha), srcAlpha, src) + mul(dstAlpha, srcAlpha, cfValue);
    }
}

struct HSYType
{
    template<class TReal>
    inline static TReal getLightness(TReal r, TReal g, TReal b) {
        return TReal(0.299)*r + TReal(0.587)*g + TReal(0.114)*b;
    }
    
    template<class TReal>
    inline static TReal getSaturation(TReal r, TReal g, TReal b) {
        return Arithmetic::max(r,g,b) - Arithmetic::min(r,g,b);
    }
};

struct HSIType
{
    template<class TReal>
    inline static TReal getLightness(TReal r, TReal g, TReal b) {
        return (r + g + b) * TReal(0.33333333333333333333); // (r + g + b) / 3.0
    }
    
    template<class TReal>
    inline static TReal getSaturation(TReal r, TReal g, TReal b) {
        TReal max    = Arithmetic::max(r, g, b);
        TReal min    = Arithmetic::min(r, g, b);
        TReal chroma = max - min;
        
        return (chroma > std::numeric_limits<TReal>::epsilon()) ?
            (TReal(1.0) - min / getLightness(r, g, b)) : TReal(0.0);
    }
};

struct HSLType
{
    template<class TReal>
    inline static TReal getLightness(TReal r, TReal g, TReal b) {
        TReal max = Arithmetic::max(r, g, b);
        TReal min = Arithmetic::min(r, g, b);
        return (max + min) * TReal(0.5);
    }
    
    template<class TReal>
    inline static TReal getSaturation(TReal r, TReal g, TReal b) {
        TReal max    = Arithmetic::max(r, g, b);
        TReal min    = Arithmetic::min(r, g, b);
        TReal chroma = max - min;
        TReal light  = (max + min) * TReal(0.5);
        TReal div    = TReal(1.0) - std::abs(TReal(2.0)*light - TReal(1.0));
        
        if(div > std::numeric_limits<TReal>::epsilon())
            return chroma / div;
        
        return TReal(1.0);
    }
};

struct HSVType
{
    template<class TReal>
    inline static TReal getLightness(TReal r, TReal g, TReal b) {
        return Arithmetic::max(r,g,b);
    }
    
    template<class TReal>
    inline static TReal getSaturation(TReal r, TReal g, TReal b) {
        TReal max = Arithmetic::max(r, g, b);
        TReal min = Arithmetic::min(r, g, b);
        return (max == TReal(0.0)) ? TReal(0.0) : (max - min) / max;
    }
};

template<class TReal>
TReal getHue(TReal r, TReal g, TReal b) {
    TReal min    = Arithmetic::min(r, g, b);
    TReal max    = Arithmetic::max(r, g, b);
    TReal chroma = max - min;
    
    TReal hue = TReal(0.0);
    
    if(chroma > std::numeric_limits<TReal>::epsilon()) {
        if(max == r) // between yellow and magenta
            hue = (g - b) / chroma;
        else if(max == g) // between cyan and yellow
            hue = TReal(2.0) + (b - r) / chroma;
        else if(max == b) // between magenta and cyan
            hue = TReal(4.0) + (r - g) / chroma;
    }
    
    return hue;
}

template<class HSXType, class TReal>
inline static TReal getLightness(TReal r, TReal g, TReal b) {
    return HSXType::getLightness(r, g, b);
}

template<class HSXType, class TReal>
inline void setLightness(TReal& r, TReal& g, TReal& b, TReal light)
{
    using namespace Arithmetic;
    
    TReal d = light - HSXType::getLightness(r, g, b);
    
    r += d;
    g += d;
    b += d;
    
    TReal l = HSXType::getLightness(r, g, b);
    TReal n = min(r, g, b);
    TReal x = max(r, g, b);
    
    if(n < TReal(0.0)) {
        r = l + ((r-l) * l) / (l-n);
        g = l + ((g-l) * l) / (l-n);
        b = l + ((b-l) * l) / (l-n);
    }
    
    if(x > TReal(1.0)) {
        r = l + ((r-l) * (TReal(1.0)-l)) / (x-l);
        g = l + ((g-l) * (TReal(1.0)-l)) / (x-l);
        b = l + ((b-l) * (TReal(1.0)-l)) / (x-l);
    }
}

template<class HSXType, class TReal>
inline static TReal getSaturation(TReal r, TReal g, TReal b) {
    return HSXType::getSaturation(r, g, b);
}

template<class HSXType, class TReal>
inline void setSaturation(TReal& r, TReal& g, TReal& b, TReal sat)
{
    int   min    = 0;
    int   mid    = 1;
    int   max    = 2;
    TReal rgb[3] = {r, g, b};
    
    if(rgb[mid] < rgb[min]) {
        int tmp = min;
        min = mid;
        mid = tmp;
    }
    
    if(rgb[max] < rgb[mid]) {
        int tmp = mid;
        mid = max;
        max = tmp;
    }
    
    if(rgb[mid] < rgb[min]) {
        int tmp = min;
        min = mid;
        mid = tmp;
    }
    
    if((rgb[max] - rgb[min]) > TReal(0.0)) {
        rgb[mid] = ((rgb[mid]-rgb[min]) * sat) / (rgb[max]-rgb[min]);
        rgb[max] = sat;
        rgb[min] = TReal(0.0);
        
        r = rgb[0];
        g = rgb[1];
        b = rgb[2];
    }
    else r = g = b = TReal(0.0);
}

#endif
