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
};

struct KoLabU16Traits : public KoColorSpaceTrait<quint16, 4,3> {
};

template<typename _channels_type_>
struct KoRgbTraits : public KoColorSpaceTrait<_channels_type_, 4,3> {
    static const qint32 red = 2;
    static const qint32 green = 1;
    static const qint32 blue = 0;
};

struct KoRgbU16Traits : public KoRgbTraits<quint16> {
};

#endif
