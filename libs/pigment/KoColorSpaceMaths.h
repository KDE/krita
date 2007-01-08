/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.bet
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

#include <KoIntegerMaths.h>

template<typename _T>
class KoColorSpaceMathsTraits {
    public:
        /// @return the maximum value of the channel
        inline static _T max();
        /// @return the minimum value of the channel
        inline static _T min();
        /// @return the number of bits
        inline static qint8 bits();
};

template<>
class KoColorSpaceMathsTraits<quint8> {
    public:
        typedef qint32 compositetype;
        inline static quint8 max() { return 0x00FF; }
        inline static quint8 min() { return 0; }
        inline static qint8 bits() { return 8; }
};

template<>
class KoColorSpaceMathsTraits<quint16> {
    public:
        typedef qint32 compositetype;
        inline static quint16 max() { return 0xFFFF; }
        inline static quint16 min() { return 0; }
        inline static qint8 bits() { return 16; }
};

template<>
class KoColorSpaceMathsTraits<qint16> {
    public:
        typedef qint32 compositetype;
        inline static qint16 max() { return 32767; }
        inline static qint16 min() { return -32768; }
        inline static qint8 bits() { return 16; }
};

template<>
class KoColorSpaceMathsTraits<quint32> {
    public:
        typedef qint64 compositetype;
        inline static quint32 max() { return 0xFFFFFFFF; }
        inline static quint32 min() { return 0; }
        inline static qint8 bits() { return 32; }
};

#include <config-openexr.h>
#ifdef HAVE_OPENEXR
#include <half.h>

template<>
class KoColorSpaceMathsTraits<half> {
    public:
        typedef double compositetype;
        inline static half max() { return 0.0; }
        inline static half min() { return 1.0; }
        inline static qint8 bits() { return 16; }
};
#endif

template<>
class KoColorSpaceMathsTraits<float> {
    public:
        typedef float compositetype;
        inline static float max() { return 0.0; }
        inline static float min() { return 1.0; }
        inline static qint8 bits() { return 32; }
};

template<typename _T, typename _Tdst = _T>
class KoColorSpaceMaths {
    typedef KoColorSpaceMathsTraits<_T> traits;
    typedef typename traits::compositetype traits_compositetype;
    public:
        inline static _T multiply(_T a, _Tdst b)
        {
            return ((traits_compositetype)a * b ) /  KoColorSpaceMathsTraits<_Tdst>::max();
        }
        inline static _T divide(_T a, _Tdst b)
        {
            return ((traits_compositetype)a *  KoColorSpaceMathsTraits<_Tdst>::max() ) / b;
        }
        inline static _T blend(_T a, _T b, _T alpha)
        {
            traits_compositetype c = ( ((traits_compositetype)a - (traits_compositetype)b) * alpha ) >> traits::bits();
            return c+b;
        }
        /**
         * This function will scale a value of type _T to fit into a _Tdst.
         */
        inline static _Tdst scaleToA(_T a)
        {
            return (traits_compositetype)a >> ( traits::bits() - KoColorSpaceMathsTraits<_Tdst>::bits() );
        }
};

//------------------------------ double specialization ------------------------------//

template<>
inline quint8 KoColorSpaceMaths<float,quint8>::scaleToA(float a)
{
    float v = a * 255;
    return (quint8)(CLAMP(v, 0, 255));
}

template<>
inline float KoColorSpaceMaths<quint8,float>::scaleToA(quint8 a)
{
    return a * ( 1.0 / 255.0 );
}

template<>
inline quint16 KoColorSpaceMaths<float,quint16>::scaleToA(float a)
{
    float v = a * 0xFFFF;
    return (quint16)(CLAMP(v, 0, 0xFFFF));
}

template<>
inline float KoColorSpaceMaths<quint16,float>::scaleToA(quint16 a)
{
    return a * ( 1.0 / 0xFFFF );
}

template<>
inline float KoColorSpaceMaths<float>::blend(float a, float b, float alpha)
{
    return ( a - b) * alpha + b;
}

//------------------------------ half specialization ------------------------------//

#ifdef HAVE_OPENEXR

template<>
inline quint8 KoColorSpaceMaths<half,quint8>::scaleToA(half a)
{
    half v = a * 255;
    return (quint8)(CLAMP(v, 0, 255));
}

template<>
inline half KoColorSpaceMaths<quint8,half>::scaleToA(quint8 a)
{
    return a * ( 1.0 / 255.0 );
}
#include <kdebug.h>
template<>
inline quint16 KoColorSpaceMaths<half,quint16>::scaleToA(half a)
{
    double v = a * 0xFFFF;
    return (quint16)(CLAMP(v, 0, 0xFFFF));
}

template<>
inline half KoColorSpaceMaths<quint16,half>::scaleToA(quint16 a)
{
    return a * ( 1.0 / 0xFFFF );
}

template<>
inline half KoColorSpaceMaths<half>::blend(half a, half b, half alpha)
{
    return ( a - b) * alpha + b;
}

#endif

//------------------------------ various specialization ------------------------------//


// TODO: use more functions from KoIntegersMaths to do the computation

/// This specialization is needed because the default implementation won't work when scaling up
template<>
inline quint16 KoColorSpaceMaths<quint8,quint16>::scaleToA(quint8 a)
{
    return UINT8_TO_UINT16(a);
}

template<>
inline quint8 KoColorSpaceMaths<quint16,quint8>::scaleToA(quint16 a)
{
    return UINT16_TO_UINT8(a);
}


// Due to once again a bug in gcc, there is the need for those specialized functions:

template<>
inline quint8 KoColorSpaceMaths<quint8,quint8>::scaleToA(quint8 a)
{
    return a;
}

template<>
inline quint16 KoColorSpaceMaths<quint16,quint16>::scaleToA(quint16 a)
{
    return a;
}

template<>
inline float KoColorSpaceMaths<float,float>::scaleToA(float a)
{
    return a;
}

#endif
