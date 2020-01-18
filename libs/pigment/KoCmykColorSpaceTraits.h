/*
 *  Copyright (c) 2006-2007 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2016 L. E. Segovia <amy@amyspark.me>
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

#ifndef _KO_CMYK_COLORSPACE_TRAITS_H_
#define _KO_CMYK_COLORSPACE_TRAITS_H_

#include <QVector>

#include "KoColorSpaceConstants.h"
#include "KoColorSpaceMaths.h"
#include "DebugPigment.h"


/** 
 * Base class for CMYK traits, it provides some convenient functions to
 * access CMYK channels through an explicit API.
 */
template<typename _channels_type_>
struct KoCmykTraits : public KoColorSpaceTrait<_channels_type_, 5, 4> {
    typedef _channels_type_ channels_type;
    typedef KoColorSpaceTrait<_channels_type_, 5, 4> parent;

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
    /// @return the Cyan component
    inline static channels_type C(quint8* data) {
        channels_type* d = parent::nativeArray(data);
        return d[c_pos];
    }
    /// Set the Cyan component
    inline static void setC(quint8* data, channels_type nv) {
        channels_type* d = parent::nativeArray(data);
        d[c_pos] = nv;
    }
    /// @return the Magenta component
    inline static channels_type M(quint8* data) {
        channels_type* d = parent::nativeArray(data);
        return d[m_pos];
    }
    /// Set the Magenta component
    inline static void setM(quint8* data, channels_type nv) {
        channels_type* d = parent::nativeArray(data);
        d[m_pos] = nv;
    }
    /// @return the Yellow component
    inline static channels_type Y(quint8* data) {
        channels_type* d = parent::nativeArray(data);
        return d[y_pos];
    }
    /// Set the Yellow component
    inline static void setY(quint8* data, channels_type nv) {
        channels_type* d = parent::nativeArray(data);
        d[y_pos] = nv;
    }
    /// @return the Key component
    inline static channels_type k(quint8* data) {
        channels_type* d = parent::nativeArray(data);
        return d[k_pos];
    }
    /// Set the Key component
    inline static void setK(quint8* data, channels_type nv) {
        channels_type* d = parent::nativeArray(data);
        d[k_pos] = nv;
    }
};

struct KoCmykU8Traits : public KoCmykTraits<quint8> {
};

struct KoCmykU16Traits : public KoCmykTraits<quint16> {
};

#include <KoConfig.h>
#ifdef HAVE_OPENEXR
#include <half.h>

struct KoCmykF16Traits : public KoCmykTraits<half> {
    static constexpr float MAX_CHANNEL_CMYK = 100;

    inline static QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) {
        return channelValueText(pixel, channelIndex);
    }

    inline static void normalisedChannelsValue(const quint8 *pixel, QVector<float> &channels) {
        Q_ASSERT((int)channels.count() == (int)parent::channels_nb);
        channels_type c;
        for (uint i = 0; i < parent::channels_nb; i++) {
            c = parent::nativeArray(pixel)[i];
            channels[i] = (qreal)c;
        }
    }

    inline static void fromNormalisedChannelsValue(quint8 *pixel, const QVector<float> &values) {
        Q_ASSERT((int)values.count() == (int)parent::channels_nb);
        channels_type c;
        for (uint i = 0; i < parent::channels_nb; i++) {
            float b = 0;
            switch(i) {
            case c_pos:
            case m_pos:
            case y_pos:
            case k_pos:
                b = qBound((float)0,
                           (float)KoColorSpaceMathsTraits<float>::unitValue * values[i],
                           (float)MAX_CHANNEL_CMYK);
                break;
            default:
                b = qBound((float)KoColorSpaceMathsTraits<float>::min,
                           (float)KoColorSpaceMathsTraits<float>::unitValue * values[i],
                           (float)KoColorSpaceMathsTraits<float>::max);
                break;
            }
            c = (channels_type)b;
            parent::nativeArray(pixel)[i] = c;
        }
    }
};

#endif

struct KoCmykF32Traits : public KoCmykTraits<float> {
    static constexpr float MAX_CHANNEL_CMYK = 100;

    inline static QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) {
        return channelValueText(pixel, channelIndex);
    }

    inline static void normalisedChannelsValue(const quint8 *pixel, QVector<float> &channels) {
        Q_ASSERT((int)channels.count() == (int)parent::channels_nb);
        channels_type c;
        for (uint i = 0; i < parent::channels_nb; i++) {
            c = parent::nativeArray(pixel)[i];
            channels[i] = (qreal)c;
        }
    }

    inline static void fromNormalisedChannelsValue(quint8 *pixel, const QVector<float> &values) {
        Q_ASSERT((int)values.count() == (int)parent::channels_nb);
        channels_type c;
        for (uint i = 0; i < parent::channels_nb; i++) {
            float b = 0;
            switch(i) {
            case c_pos:
            case m_pos:
            case y_pos:
            case k_pos:
                b = qBound((float)0,
                           (float)KoColorSpaceMathsTraits<float>::unitValue * values[i],
                           (float)MAX_CHANNEL_CMYK);
                break;
            default:
                b = qBound((float)KoColorSpaceMathsTraits<float>::min,
                           (float)KoColorSpaceMathsTraits<float>::unitValue * values[i],
                           (float)KoColorSpaceMathsTraits<float>::max);
                break;
            }
            c = (channels_type)b;
            parent::nativeArray(pixel)[i] = c;
        }
    }
};

struct KoCmykF64Traits : public KoCmykTraits<double> {
    static constexpr double MAX_CHANNEL_CMYK = 100;

    inline static QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) {
        return channelValueText(pixel, channelIndex);
    }

    inline static void normalisedChannelsValue(const quint8 *pixel, QVector<float> &channels) {
        Q_ASSERT((int)channels.count() == (int)parent::channels_nb);
        channels_type c;
        for (uint i = 0; i < parent::channels_nb; i++) {
            c = parent::nativeArray(pixel)[i];
            channels[i] = (qreal)c;
        }
    }

    inline static void fromNormalisedChannelsValue(quint8 *pixel, const QVector<float> &values) {
        Q_ASSERT((int)values.count() == (int)parent::channels_nb);
        channels_type c;
        for (uint i = 0; i < parent::channels_nb; i++) {
            float b = 0;
            switch(i) {
            case c_pos:
            case m_pos:
            case y_pos:
            case k_pos:
                b = qBound((double)0,
                           (double)KoColorSpaceMathsTraits<double>::unitValue * values[i],
                           (double)MAX_CHANNEL_CMYK);
                break;
            default:
                b = qBound((double)KoColorSpaceMathsTraits<double>::min,
                           (double)KoColorSpaceMathsTraits<double>::unitValue * values[i],
                           (double)KoColorSpaceMathsTraits<double>::max);
                break;
            }
            c = (channels_type)b;
            parent::nativeArray(pixel)[i] = c;
        }
    }
};



#endif
    
