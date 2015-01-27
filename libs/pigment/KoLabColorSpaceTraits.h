/*
 *  Copyright (c) 2006-2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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

#ifndef _KO_LAB_COLORSPACE_TRAITS_H_
#define _KO_LAB_COLORSPACE_TRAITS_H_


/** 
 * LAB traits, it provides some convenient functions to
 * access LAB channels through an explicit API.
 *
 * Use this class in conjonction with KoColorSpace::toLabA16 and
 * KoColorSpace::fromLabA16 data.
 *
 * Example:
 * quint8* p = KoLabU16Traits::allocate(1);
 * oneKoColorSpace->toLabA16(somepointertodata, p, 1);
 * KoLabU16Traits::setL( p, KoLabU16Traits::L(p) / 10 );
 * oneKoColorSpace->fromLabA16(p, somepointertodata, 1);
 */
template<typename _channels_type_>
struct KoLabTraits : public KoColorSpaceTrait<_channels_type_, 4, 3> {
    typedef _channels_type_ channels_type;
    typedef KoColorSpaceTrait<_channels_type_, 4, 3> parent;
    static const qint32 L_pos = 0;
    static const qint32 a_pos = 1;
    static const qint32 b_pos = 2;

    /**
     * An Lab pixel
     */
    struct Pixel {
        channels_type L;
        channels_type a;
        channels_type b;
        channels_type alpha;
    };

    /// @return the L component
    inline static channels_type L(quint8* data) {
        channels_type* d = parent::nativeArray(data);
        return d[L_pos];
    }
    /// Set the L component
    inline static void setL(quint8* data, channels_type nv) {
        channels_type* d = parent::nativeArray(data);
        d[L_pos] = nv;
    }
    /// @return the a component
    inline static channels_type a(quint8* data) {
        channels_type* d = parent::nativeArray(data);
        return d[a_pos];
    }
    /// Set the a component
    inline static void setA(quint8* data, channels_type nv) {
        channels_type* d = parent::nativeArray(data);
        d[a_pos] = nv;
    }
    /// @return the b component
    inline static channels_type b(quint8* data) {
        channels_type* d = parent::nativeArray(data);
        return d[b_pos];
    }
    /// Set the a component
    inline static void setB(quint8* data, channels_type nv) {
        channels_type* d = parent::nativeArray(data);
        d[b_pos] = nv;
    }
};


struct KoLabU8Traits : public KoLabTraits<quint8> {
};

struct KoLabU16Traits : public KoLabTraits<quint16> {
};

#include <KoConfig.h>
#ifdef HAVE_OPENEXR
#include <half.h>

struct KoLabF16Traits : public KoLabTraits<half> {
};

#endif

struct KoLabF32Traits : public KoLabTraits<float> {
};

struct KoLabF64Traits : public KoLabTraits<double> {
};

#endif
