/*
 *  Copyright (c) 2006-2007 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KO_COLORSPACE_TRAITS_H_
#define _KO_COLORSPACE_TRAITS_H_

#include "KoColorSpaceConstants.h"
#include "KoColorSpaceMaths.h"

/**
 * This class is the base class to define the main characteristics of a colorspace
 * which inherits KoColorSpaceAbstract.
 *
 * For instance a colorspace of three color channels and alpha channel  in 16bits,
 * will be defined as KoColorSpaceTrait\<quint16, 4, 3\>. The same without the alpha
 * channel is KoColorSpaceTrait\<quint16,3,-1\>
 *
 */
template<typename _channels_type_, int _channels_nb_, int _alpha_pos_>
struct KoColorSpaceTrait {
    typedef _channels_type_ channels_type;
    static const quint32 channels_nb = _channels_nb_;
    static const qint32 alpha_pos = _alpha_pos_;
    /**
     * @return the size in byte of one pixel
     */
    inline static quint32 pixelSize()
    {
        return channels_nb * sizeof(channels_type);
    }
    inline static quint8 alpha(const quint8 * U8_pixel)
    {
        if (alpha_pos < 0) return OPACITY_OPAQUE;
        channels_type c = nativeArray(U8_pixel)[alpha_pos];
        return  KoColorSpaceMaths<channels_type,quint8>::scaleToA(c);
    }
    inline static void setAlpha(quint8 * pixels, quint8 alpha, qint32 nPixels)
    {
        if (alpha_pos < 0) return;
        qint32 psize = pixelSize();
        channels_type valpha =  KoColorSpaceMaths<quint8,channels_type>::scaleToA(alpha);
        for (; nPixels > 0; --nPixels, pixels += psize) {
            nativeArray(pixels)[alpha_pos] = valpha;
        }
    }
    /**
     * Convenient function for transforming a quint8* array in a pointer of the native channels type
     */
    inline static const channels_type* nativeArray(const quint8 * a)
    {
        return reinterpret_cast<const channels_type*>(a);
    }
    /**
     * Convenient function for transforming a quint8* array in a pointer of the native channels type
     */
    inline static channels_type* nativeArray(quint8 * a)
    {
        return reinterpret_cast< channels_type*>(a);
    }
    /**
     * Allocate nPixels pixels for this colorspace.
     */
    inline static quint8* allocate(quint32 nPixels)
    {
        return new quint8[ nPixels * pixelSize() ];
    }
};

/** LAB traits, it provides some convenient functions to
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
struct KoLabU16Traits : public KoColorSpaceTrait<quint16, 4,3> {
    static const qint32 L_pos = 0;
    static const qint32 a_pos = 1;
    static const qint32 b_pos = 2;
    /// @return the L component
    inline static channels_type L(quint8* data) {
        channels_type* d = nativeArray(data);
        return d[L_pos];
    };
    /// Set the L component
    inline static void setL(quint8* data, channels_type nv)
    {
        channels_type* d = nativeArray(data);
        d[L_pos] = nv;
    }
    /// @return the a component
    inline static channels_type a(quint8* data) {
        channels_type* d = nativeArray(data);
        return d[a_pos];
    };
    /// Set the a component
    inline static void setA(quint8* data, channels_type nv)
    {
        channels_type* d = nativeArray(data);
        d[a_pos] = nv;
    }
    /// @return the b component
    inline static channels_type b(quint8* data) {
        channels_type* d = nativeArray(data);
        return d[b_pos];
    };
    /// Set the a component
    inline static void setB(quint8* data, channels_type nv)
    {
        channels_type* d = nativeArray(data);
        d[b_pos] = nv;
    }
};

/** Base class for rgb traits, it provides some convenient functions to
 * access RGB channels through an explicit API.
 */
template<typename _channels_type_>
struct KoRgbTraits : public KoColorSpaceTrait<_channels_type_, 4,3> {
    typedef _channels_type_ channels_type; // /me wonders why gcc refuses to build without that line ?, which is pretty annoying as it's less clean
    typedef KoColorSpaceTrait<_channels_type_, 4,3> parent;
    static const qint32 red_pos = 2;
    static const qint32 green_pos = 1;
    static const qint32 blue_pos = 0;
    
    /// @return the red component
    inline static channels_type red(quint8* data) {
        channels_type* d = parent::nativeArray(data);
        return d[red_pos];
    };
    /// Set the red component
    inline static void setRed(quint8* data, channels_type nv)
    {
        channels_type* d = parent::nativeArray(data);
        d[red_pos] = nv;
    }
    /// @return the green component
    inline static channels_type green(quint8* data) {
        channels_type* d = parent::nativeArray(data);
        return d[green_pos];
    };
    /// Set the green component
    inline static void setGreen(quint8* data, channels_type nv)
    {
        channels_type* d = parent::nativeArray(data);
        d[green_pos] = nv;
    }
    /// @return the blue component
    inline static channels_type blue(quint8* data) {
        channels_type* d = parent::nativeArray(data);
        return d[blue_pos];
    };
    /// Set the blue component
    inline static void setBlue(quint8* data, channels_type nv)
    {
        channels_type* d = parent::nativeArray(data);
        d[blue_pos] = nv;
    }
};

/** Use this class in conjonction with KoColorSpace::toRgbA16 and
 * KoColorSpace::fromRgbA16 data.
 * @see KoLabU16Traits for an exemple of use.
 */
struct KoRgbU16Traits : public KoRgbTraits<quint16> {
};

#endif
