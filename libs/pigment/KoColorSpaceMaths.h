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


extern PIGMENTCMS_EXPORT const Ko::FullLut< KoIntegerToFloat<quint16>, float, quint16> KoUint16ToFloatLut;
extern PIGMENTCMS_EXPORT const Ko::FullLut< KoIntegerToFloat<quint8>, float, quint8> KoUint8ToFloatLut;


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

    /**
     * Division : (a * MAX ) / b
     * @param a
     * @param b
     */
    inline static _T divide(_T a, _Tdst b) {
        return ((traits_compositetype)a *  KoColorSpaceMathsTraits<_Tdst>::unitValue) / b;
    }

    /**
     * Blending : (a * alpha) + b * (1 - alpha)
     * @param a
     * @param b
     * @param alpha
     */
    inline static _T blend(_T a, _T b, _T alpha) {
        traits_compositetype c = (((traits_compositetype)a - (traits_compositetype)b) * alpha) >> traits::bits;
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
    return KoUint8ToFloatLut(a);
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
    return KoUint16ToFloatLut(a);
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
    return KoUint16ToFloatLut(a);
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
    return KoUint8ToFloatLut(a);
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
inline quint8 KoColorSpaceMaths<quint8>::divide(quint8 a, quint8 b)
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
inline quint16 KoColorSpaceMaths<quint16>::divide(quint16 a, quint16 b)
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

#endif
