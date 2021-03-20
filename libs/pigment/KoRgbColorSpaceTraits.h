/*
 *  SPDX-FileCopyrightText: 2006-2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _KO_RGB_COLORSPACE_TRAITS_H_
#define _KO_RGB_COLORSPACE_TRAITS_H_

/** 
 * Base class for rgb traits, it provides some convenient functions to
 * access RGB channels through an explicit API.
 */
template<typename _channels_type_>
struct KoRgbTraits : public KoColorSpaceTrait<_channels_type_, 4, 3> {
    typedef _channels_type_ channels_type;
    typedef KoColorSpaceTrait<_channels_type_, 4, 3> parent;
    static const qint32 red_pos = 0;
    static const qint32 green_pos = 1;
    static const qint32 blue_pos = 2;
    /**
     * An RGB pixel
     */
    struct Pixel {
        channels_type red;
        channels_type green;
        channels_type blue;
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

struct KoRgbU8Traits : public KoRgbTraits<quint8> {
};

struct KoRgbU16Traits : public KoRgbTraits<quint16> {
};

struct KoRgbU32Traits : public KoRgbTraits<quint32> {
};


#include <KoConfig.h>
#ifdef HAVE_OPENEXR
#include <half.h>

struct KoRgbF16Traits : public KoRgbTraits<half> {
};

#endif

struct KoRgbF32Traits : public KoRgbTraits<float> {
};

struct KoRgbF64Traits : public KoRgbTraits<double> {
};


#endif
