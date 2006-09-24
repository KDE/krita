/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.bet
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
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
        inline static qint64 max();
        /// @return the minimum value of the channel
        inline static qint64 min();
        /// @return the number of bits
        inline static qint8 bits();
};

template<>
class KoColorSpaceMathsTraits<quint8> {
    public:
        typedef qint32 compositetype;
        inline static qint64 max() { return 0x000F; }
        inline static qint64 min() { return 0; }
        inline static qint8 bits() { return 8; }
};

template<>
class KoColorSpaceMathsTraits<quint16> {
    public:
        typedef qint32 compositetype;
        inline static qint64 max() { return 0x00FF; }
        inline static qint64 min() { return 0; }
        inline static qint8 bits() { return 16; }
};

template<>
class KoColorSpaceMathsTraits<qint16> {
    public:
        typedef qint32 compositetype;
        inline static qint64 max() { return 32767; }
        inline static qint64 min() { return -32768; }
        inline static qint8 bits() { return 16; }
};

template<>
class KoColorSpaceMathsTraits<quint32> {
    public:
        typedef qint64 compositetype;
        inline static qint64 max() { return 0xFFFF; }
        inline static qint64 min() { return 0; }
        inline static qint8 bits() { return 32; }
};


template<typename _T, typename _Tdst = _T>
class KoColorSpaceMaths {
    typedef KoColorSpaceMathsTraits<_T> traits;
    typedef typename traits::compositetype traits_compositetype;
    public:
        inline static _T blend(_T a, _T b, qint64 alpha)
        {
            traits_compositetype c = ((traits_compositetype)a - (traits_compositetype)b) >> traits::bits();
            return c+b;
        }
        /**
         * This function will scale a value of type _T to fit into a _Tdst.
         */
        inline static _Tdst scaleToA(_T a)
        {
            return (traits_compositetype)a >> ( traits::bits() - KoColorSpaceMathsTraits<_Tdst>::bits() );
        }
        inline static _T multiply(_T a, _T b)
        {
            return a * b / traits::max();
        }
};


#endif
