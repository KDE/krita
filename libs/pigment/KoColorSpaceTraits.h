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

#include <QtCore/QVector>

#include "KoColorSpaceConstants.h"
#include "KoColorSpaceMaths.h"
#include "DebugPigment.h"

/**
 * This class is the base class to define the main characteristics of a colorspace
 * which inherits KoColorSpaceAbstract.
 *
 * For instance a colorspace of three color channels and alpha channel in 16bits,
 * will be defined as KoColorSpaceTrait\<quint16, 4, 3\>. The same without the alpha
 * channel is KoColorSpaceTrait\<quint16,3,-1\>
 *
 * So: if you do not have an alpha channel, the _alpha_pos is -1.
 * Other code will depend on that.
 *
 */
template<typename _channels_type_, int _channels_nb_, int _alpha_pos_>
struct KoColorSpaceTrait {
    /// the type of the value of the channels of this color space
    typedef _channels_type_ channels_type;
    /// the number of channels in this color space
    static const quint32 channels_nb = _channels_nb_;
    /// the position of the alpha channel in the channels of the pixel (or -1 if no alpha
    /// channel.
    static const qint32 alpha_pos = _alpha_pos_;
    /// the number of bit for each channel
    static const int depth = KoColorSpaceMathsTraits<_channels_type_>::bits;
    /**
     * @return the size in byte of one pixel
     */
    static const quint32 pixelSize = channels_nb * sizeof(channels_type);
    /**
     * @return the value of the alpha channel for this pixel in the 0..255 range
     */
    inline static quint8 alpha(const quint8 * U8_pixel) {
        if (alpha_pos < 0) return OPACITY_OPAQUE;
        channels_type c = nativeArray(U8_pixel)[alpha_pos];
        return  KoColorSpaceMaths<channels_type, quint8>::scaleToA(c);
    }
    /**
     * Set the alpha channel for this pixel from a value in the 0..255 range
     */
    inline static void setAlpha(quint8 * pixels, quint8 alpha, qint32 nPixels) {
        if (alpha_pos < 0) return;
        qint32 psize = pixelSize;
        channels_type valpha =  KoColorSpaceMaths<quint8, channels_type>::scaleToA(alpha);
        for (; nPixels > 0; --nPixels, pixels += psize) {
            nativeArray(pixels)[alpha_pos] = valpha;
        }
    }
    /**
     * Convenient function for transforming a quint8* array in a pointer of the native channels type
     */
    inline static const channels_type* nativeArray(const quint8 * a) {
        return reinterpret_cast<const channels_type*>(a);
    }
    /**
     * Convenient function for transforming a quint8* array in a pointer of the native channels type
     */
    inline static channels_type* nativeArray(quint8 * a) {
        return reinterpret_cast< channels_type*>(a);
    }
    /**
     * Allocate nPixels pixels for this colorspace.
     */
    inline static quint8* allocate(quint32 nPixels) {
        return new quint8[ nPixels * pixelSize ];
    }
    inline static void singleChannelPixel(quint8 *dstPixel, const quint8 *srcPixel, quint32 channelIndex) {
        const channels_type* src = nativeArray(srcPixel);
        channels_type* dst = nativeArray(dstPixel);
        for (uint i = 0; i < channels_nb;i++) {
            if (i != channelIndex) {
                dst[i] = 0;
            } else {
                dst[i] = src[i];
            }
        }
    }
    inline static QString channelValueText(const quint8 *pixel, quint32 channelIndex) {
        if (channelIndex > channels_nb) return QString("Error");
        channels_type c = nativeArray(pixel)[channelIndex];
        return QString().setNum(c);
    }

    inline static QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) {
        if (channelIndex > channels_nb) return QString("Error");
        channels_type c = nativeArray(pixel)[channelIndex];
        return QString().setNum(100. * ((qreal)c) / KoColorSpaceMathsTraits< channels_type>::unitValue);
    }

    inline static void normalisedChannelsValue(const quint8 *pixel, QVector<float> &channels) {
        Q_ASSERT((int)channels.count() == (int)channels_nb);
        channels_type c;
        for (uint i = 0; i < channels_nb; i++) {
            c = nativeArray(pixel)[i];
            channels[i] = ((qreal)c) / KoColorSpaceMathsTraits<channels_type>::unitValue;
        }
    }

    inline static void fromNormalisedChannelsValue(quint8 *pixel, const QVector<float> &values) {
        Q_ASSERT((int)values.count() == (int)channels_nb);
        channels_type c;
        for (uint i = 0; i < channels_nb; i++) {
            c = (channels_type)
                ((float)KoColorSpaceMathsTraits<channels_type>::unitValue * values[i]);
            nativeArray(pixel)[i] = c;
        }
    }
    inline static void multiplyAlpha(quint8 * pixels, quint8 alpha, qint32 nPixels) {
        if (alpha_pos < 0) return;

        channels_type valpha =  KoColorSpaceMaths<quint8, channels_type>::scaleToA(alpha);

        for (; nPixels > 0; --nPixels, pixels += pixelSize) {
            channels_type* alphapixel = nativeArray(pixels) + alpha_pos;
            *alphapixel = KoColorSpaceMaths<channels_type>::multiply(*alphapixel, valpha);
        }
    }

    inline static void applyAlphaU8Mask(quint8 * pixels, const quint8 * alpha, qint32 nPixels) {
        if (alpha_pos < 0) return;

        for (; nPixels > 0; --nPixels, pixels += pixelSize, ++alpha) {
            channels_type valpha =  KoColorSpaceMaths<quint8, channels_type>::scaleToA(*alpha);
            channels_type* alphapixel = nativeArray(pixels) + alpha_pos;
            *alphapixel = KoColorSpaceMaths<channels_type>::multiply(*alphapixel, valpha);
        }
    }

    inline static void applyInverseAlphaU8Mask(quint8 * pixels, const quint8 * alpha, qint32 nPixels) {
        if (alpha_pos < 0) return;

        for (; nPixels > 0; --nPixels, pixels += pixelSize, ++alpha) {
            channels_type valpha =  KoColorSpaceMaths<quint8, channels_type>::scaleToA(OPACITY_OPAQUE - *alpha);
            channels_type* alphapixel = nativeArray(pixels) + alpha_pos;
            *alphapixel = KoColorSpaceMaths<channels_type>::multiply(*alphapixel, valpha);
        }
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

/** Use this class in conjonction with KoColorSpace::toRgbA16 and
 * KoColorSpace::fromRgbA16 data.
 * @see KoLabU16Traits for an exemple of use.
 */
struct KoLabU16Traits : public KoLabTraits<quint16> {
};

/** Base class for rgb traits, it provides some convenient functions to
 * access RGB channels through an explicit API.
 */
template<typename _channels_type_>
struct KoRgbTraits : public KoColorSpaceTrait<_channels_type_, 4, 3> {
    typedef _channels_type_ channels_type;
    typedef KoColorSpaceTrait<_channels_type_, 4, 3> parent;
    static const qint32 red_pos = 2;
    static const qint32 green_pos = 1;
    static const qint32 blue_pos = 0;
    /**
     * An RGB pixel
     */
    struct Pixel {
        channels_type blue;
        channels_type green;
        channels_type red;
        channels_type alpha;
    };

    /// @return the red component
    inline static channels_type red(quint8* data) {
        channels_type* d = parent::nativeArray(data);
        return d[red_pos];
    }
    /// Set the red component
    inline static void setRed(quint8* data, channels_type nv) {
        channels_type* d = parent::nativeArray(data);
        d[red_pos] = nv;
    }
    /// @return the green component
    inline static channels_type green(quint8* data) {
        channels_type* d = parent::nativeArray(data);
        return d[green_pos];
    }
    /// Set the green component
    inline static void setGreen(quint8* data, channels_type nv) {
        channels_type* d = parent::nativeArray(data);
        d[green_pos] = nv;
    }
    /// @return the blue component
    inline static channels_type blue(quint8* data) {
        channels_type* d = parent::nativeArray(data);
        return d[blue_pos];
    }
    /// Set the blue component
    inline static void setBlue(quint8* data, channels_type nv) {
        channels_type* d = parent::nativeArray(data);
        d[blue_pos] = nv;
    }
};


/**
 * 8-bits rgba traits.
 */
struct KoRgbU8Traits : public KoRgbTraits<quint8> {
};

/**
 * Use this class together with KoColorSpace::toRgbA16 and
 * KoColorSpace::fromRgbA16 data.
 * @see KoLabU16Traits for an exemple of use.
 */
struct KoRgbU16Traits : public KoRgbTraits<quint16> {
};

/** Base class for XYZ traits, it provides some convenient functions to
 * access XYZ channels through an explicit API.
 */
template<typename _channels_type_>
struct KoXyzTraits : public KoColorSpaceTrait<_channels_type_, 4, 3> {
    typedef _channels_type_ channels_type;
    static const qint32 x_pos = 0;
    static const qint32 y_pos = 1;
    static const qint32 z_pos = 2;

    /**
     * An RGB pixel
     */
    struct Pixel {
        channels_type X;
        channels_type Y;
        channels_type Z;
        channels_type alpha;
    };
};

/** Base class for CMYK traits, it provides some convenient functions to
 * access CMYK channels through an explicit API.
 */
template<typename _channels_type_>
struct KoCmykTraits : public KoColorSpaceTrait<_channels_type_, 5, 4> {
    typedef _channels_type_ channels_type;
    static const qint32 c_pos = 0;
    static const qint32 m_pos = 1;
    static const qint32 y_pos = 2;
    static const qint32 k_pos = 3;

    /**
     * An CMYK pixel
     */
    struct Pixel {
        channels_type cyan;
        channels_type magenta;
        channels_type yellow;
        channels_type black;
        channels_type alpha;
    };
};




#endif
