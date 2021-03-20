/*
 *  SPDX-FileCopyrightText: 2006-2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _KO_YCbCr_COLORSPACE_TRAITS_H_
#define _KO_YCbCr_COLORSPACE_TRAITS_H_

/** 
 * YCbCr traits, it provides some convenient functions to
 * access YCbCr  channels through an explicit API.
 */
template<typename _channels_type_>
struct KoYCbCrTraits : public KoColorSpaceTrait<_channels_type_, 4, 3> {
    
    typedef _channels_type_ channels_type;
    typedef KoColorSpaceTrait<_channels_type_, 4, 3> parent;
    
    static const qint32 Y_pos = 0;
    static const qint32 Cb_pos = 1;
    static const qint32 Cr_pos = 2;
    
    /**
     * An YCbCr pixel
     */
    struct Pixel {
        channels_type Y;
        channels_type Cb;
        channels_type Cr;
        channels_type alpha;
    };

    /// @return the Y component
    inline static channels_type Y(quint8* data) {
        channels_type* d = parent::nativeArray(data);
        return d[Y_pos];
    }
    /// Set the Y component
    inline static void setY(quint8* data, channels_type nv) {
        channels_type* d = parent::nativeArray(data);
        d[Y_pos] = nv;
    }
    /// @return the Cb component
    inline static channels_type Cb(quint8* data) {
        channels_type* d = parent::nativeArray(data);
        return d[Cb_pos];
    }
    /// Set the Cb component
    inline static void setCb(quint8* data, channels_type nv) {
        channels_type* d = parent::nativeArray(data);
        d[Cb_pos] = nv;
    }
    /// @return the Cr component
    inline static channels_type Cr(quint8* data) {
        channels_type* d = parent::nativeArray(data);
        return d[Cr_pos];
    }
    /// Set the Cr component
    inline static void setCr(quint8* data, channels_type nv) {
        channels_type* d = parent::nativeArray(data);
        d[Cr_pos] = nv;
    }
};

struct KoYCbCrU8Traits : public KoYCbCrTraits<quint8> {
};

struct KoYCbCrU16Traits : public KoYCbCrTraits<quint16> {
};

#include <KoConfig.h>
#ifdef HAVE_OPENEXR
#include <half.h>

struct KoYCbCrF16Traits : public KoYCbCrTraits<half> {
};

#endif

struct KoYCbCrF32Traits : public KoYCbCrTraits<float> {
};

struct KoYCbCrF64Traits : public KoYCbCrTraits<double> {
};


#endif
