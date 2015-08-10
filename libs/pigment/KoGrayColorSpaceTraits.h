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

#ifndef _KO_GRAY_COLORSPACE_TRAITS_H_
#define _KO_GRAY_COLORSPACE_TRAITS_H_

/** 
 * Base class for graya traits, it provides some convenient functions to
 * access gray channels through an explicit API.
 */
template<typename _channels_type_>
struct KoGrayTraits : public KoColorSpaceTrait<_channels_type_, 2, 1> {
    
    typedef _channels_type_ channels_type;
    typedef KoColorSpaceTrait<_channels_type_, 2, 1> parent;
    
    static const qint32 gray_pos = 0;
    
    /**
     * An grayscale pixel
     */
    struct Pixel {
        channels_type gray;
        channels_type alpha;
    };

    /// @return the gray component
    inline static channels_type gray(quint8* data) {
        channels_type* d = parent::nativeArray(data);
        return d[gray_pos];
    }
    
    /// Set the gray component
    inline static void setGray(quint8* data, channels_type nv) {
        channels_type* d = parent::nativeArray(data);
        d[gray_pos] = nv;
    }
};


struct KoGrayU8Traits : public KoGrayTraits<quint8> {
};

struct KoGrayU16Traits : public KoGrayTraits<quint16> {
};

struct KoGrayU32Traits : public KoGrayTraits<quint32> {
};


#include <KoConfig.h>
#ifdef HAVE_OPENEXR
#include <half.h>

struct KoGrayF16Traits : public KoGrayTraits<half> {
};

#endif

struct KoGrayF32Traits : public KoGrayTraits<float> {
};

struct KoGrayF64Traits : public KoGrayTraits<double> {
};

#endif
