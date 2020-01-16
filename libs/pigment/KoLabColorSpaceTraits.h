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

//For quint* values must range from 0 to 1 - see KoColorSpaceMaths<double, quint*>

struct KoLabU8Traits : public KoLabTraits<quint8> {

    static const quint32 MAX_CHANNEL_L = 100;
    static const quint32 MAX_CHANNEL_AB = 255;
    static const quint32 CHANNEL_AB_ZERO_OFFSET = 128;

    inline static void normalisedChannelsValue(const quint8 *pixel, QVector<float> &channels) {
        Q_ASSERT((int)channels.count() >= (int)parent::channels_nb);
        channels_type c;
        for (uint i = 0; i < parent::channels_nb; i++) {
            c = nativeArray(pixel)[i];
            switch (i) {
            case L_pos:
                channels[i] = ((qreal)c) / MAX_CHANNEL_L;
                break;
            case a_pos:
                channels[i] = (((qreal)c) - CHANNEL_AB_ZERO_OFFSET) / MAX_CHANNEL_AB;
                break;
            case b_pos:
                channels[i] = (((qreal)c) - CHANNEL_AB_ZERO_OFFSET) / MAX_CHANNEL_AB;
                break;
            case 3:
                channels[i] = ((qreal)c) / UINT16_MAX;
                break;
            default:
                channels[i] = ((qreal)c) / KoColorSpaceMathsTraits<channels_type>::unitValue;
                break;
            }
        }
    }

    inline static QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) {
        if (channelIndex > parent::channels_nb) return QString("Error");
        channels_type c = nativeArray(pixel)[channelIndex];
        switch (channelIndex) {
        case L_pos:
            return QString().setNum(100.0 * ((qreal)c) / MAX_CHANNEL_L);
        case a_pos:
            return QString().setNum(100.0 * ((((qreal)c) - CHANNEL_AB_ZERO_OFFSET) / MAX_CHANNEL_AB));
        case b_pos:
            return QString().setNum(100.0 * ((((qreal)c) - CHANNEL_AB_ZERO_OFFSET) / MAX_CHANNEL_AB));
        case 3:
            return QString().setNum(100.0 * ((qreal)c) / UINT16_MAX);
        default:
            return QString("Error");
        }
    }

    inline static void fromNormalisedChannelsValue(quint8 *pixel, const QVector<float> &values) {
        Q_ASSERT((int)values.count() >= (int)parent::channels_nb);
        channels_type c;
        for (uint i = 0; i < channels_nb; i++) {
            float b = 0;

            switch (i) {
            case L_pos:
                b = qBound((float)0,
                           (float)MAX_CHANNEL_L * values[i],
                           (float)MAX_CHANNEL_L);
                break;
            case a_pos:
            case b_pos:
                b = qBound((float)0,
                           (float)MAX_CHANNEL_AB * values[i] + CHANNEL_AB_ZERO_OFFSET,
                           (float)MAX_CHANNEL_AB);
                break;
            default:
                b = qBound((float)KoColorSpaceMathsTraits<channels_type>::min,
                           (float)KoColorSpaceMathsTraits<channels_type>::unitValue * values[i],
                           (float)KoColorSpaceMathsTraits<channels_type>::max);
                break;
            }
            c = (channels_type)b;
            nativeArray(pixel)[i] = c;
        }
    }
};

struct KoLabU16Traits : public KoLabTraits<quint16> {
    // https://github.com/mm2/Little-CMS/blob/master/src/cmspcs.c
    static const quint32 MAX_CHANNEL_L = 0xFF00;
    static const quint32 MAX_CHANNEL_AB = 0xFFFF;
    static const quint32 CHANNEL_AB_ZERO_OFFSET = 0x8000;

    inline static void normalisedChannelsValue(const quint8 *pixel, QVector<float> &channels) {
        Q_ASSERT((int)channels.count() >= (int)parent::channels_nb);
        channels_type c;
        for (uint i = 0; i < parent::channels_nb; i++) {
            c = nativeArray(pixel)[i];
            switch (i) {
            case L_pos:
                channels[i] = ((qreal)c) / MAX_CHANNEL_L;
                break;
            case a_pos:
                channels[i] = (((qreal)c) - CHANNEL_AB_ZERO_OFFSET) / MAX_CHANNEL_AB;
                break;
            case b_pos:
                channels[i] = (((qreal)c) - CHANNEL_AB_ZERO_OFFSET) / MAX_CHANNEL_AB;
                break;
            case 3:
                channels[i] = ((qreal)c) / UINT16_MAX;
                break;
            default:
                channels[i] = ((qreal)c) / KoColorSpaceMathsTraits<channels_type>::unitValue;
                break;
            }
        }
    }

    inline static QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) {
        if (channelIndex > parent::channels_nb) return QString("Error");
        channels_type c = nativeArray(pixel)[channelIndex];
        switch (channelIndex) {
        case L_pos:
            return QString().setNum(100.0 * ((qreal)c) / MAX_CHANNEL_L);
        case a_pos:
            return QString().setNum(100.0 * ((((qreal)c) - CHANNEL_AB_ZERO_OFFSET) / MAX_CHANNEL_AB));
        case b_pos:
            return QString().setNum(100.0 * ((((qreal)c) - CHANNEL_AB_ZERO_OFFSET) / MAX_CHANNEL_AB));
        case 3:
            return QString().setNum(100.0 * ((qreal)c) / UINT16_MAX);
        default:
            return QString("Error");
        }
    }

    inline static void fromNormalisedChannelsValue(quint8 *pixel, const QVector<float> &values) {
        Q_ASSERT((int)values.count() >= (int)parent::channels_nb);
        channels_type c;
        for (uint i = 0; i < channels_nb; i++) {
            float b = 0;

            switch (i) {
            case L_pos:
                b = qBound((float)0,
                           (float)MAX_CHANNEL_L * values[i],
                           (float)MAX_CHANNEL_L);
                break;
            case a_pos:
            case b_pos:
                b = qBound((float)0,
                           (float)MAX_CHANNEL_AB * values[i] + CHANNEL_AB_ZERO_OFFSET,
                           (float)MAX_CHANNEL_AB);
                break;
            default:
                b = qBound((float)KoColorSpaceMathsTraits<channels_type>::min,
                           (float)KoColorSpaceMathsTraits<channels_type>::unitValue * values[i],
                           (float)KoColorSpaceMathsTraits<channels_type>::max);
                break;
            }
            c = (channels_type)b;
            nativeArray(pixel)[i] = c;
        }
    }
};

// Float values are not normalised
// XXX: is it really necessary to bind them to these ranges?

#include <KoConfig.h>
#ifdef HAVE_OPENEXR
#include <half.h>

struct KoLabF16Traits : public KoLabTraits<half> {
    static constexpr float MIN_CHANNEL_L = 0;
    static constexpr float MAX_CHANNEL_L = 100;
    static constexpr float MIN_CHANNEL_AB = -128;
    static constexpr float MAX_CHANNEL_AB = +127;

    // Lab has some... particulars
    // For instance, float et al. are NOT normalised
    inline static QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) {
        return channelValueText(pixel, channelIndex);
    }
    inline static void normalisedChannelsValue(const quint8 *pixel, QVector<float> &channels) {
        Q_ASSERT((int)channels.count() >= (int)parent::channels_nb);
        channels_type c;
        for (uint i = 0; i < parent::channels_nb; i++) {
            c = parent::nativeArray(pixel)[i];
            channels[i] = (qreal)c;
        }
    }
    inline static void fromNormalisedChannelsValue(quint8 *pixel, const QVector<float> &values) {
        Q_ASSERT((int)values.count() >= (int)parent::channels_nb);
        channels_type c;
        for (uint i = 0; i < parent::channels_nb; i++) {
            float b = 0;
            switch(i) {
            case L_pos:
                b = qBound((float)MIN_CHANNEL_L,
                           (float)KoColorSpaceMathsTraits<half>::unitValue * values[i],
                           (float)MAX_CHANNEL_L);
                break;
            case a_pos:
            case b_pos:
                b = qBound((float)MIN_CHANNEL_AB,
                           (float)KoColorSpaceMathsTraits<half>::unitValue * values[i],
                           (float)MAX_CHANNEL_AB);
                break;
            case 3:
                b = qBound((float)KoColorSpaceMathsTraits<half>::min,
                           (float)KoColorSpaceMathsTraits<half>::unitValue * values[i],
                           (float)KoColorSpaceMathsTraits<half>::max);
            default:
                break;
            }
            c = (channels_type)b;
            parent::nativeArray(pixel)[i] = c;
        }
    }
};

#endif

struct KoLabF32Traits : public KoLabTraits<float> {
    static constexpr float MIN_CHANNEL_L = 0;
    static constexpr float MAX_CHANNEL_L = 100;
    static constexpr float MIN_CHANNEL_AB = -128;
    static constexpr float MAX_CHANNEL_AB = +127;

    // Lab has some... particulars
    inline static QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) {
        return channelValueText(pixel, channelIndex);
    }
    inline static void normalisedChannelsValue(const quint8 *pixel, QVector<float> &channels) {
        Q_ASSERT((int)channels.count() >= (int)parent::channels_nb);
        channels_type c;
        for (uint i = 0; i < parent::channels_nb; i++) {
            c = parent::nativeArray(pixel)[i];
            channels[i] = (qreal)c;
        }
    }
    inline static void fromNormalisedChannelsValue(quint8 *pixel, const QVector<float> &values) {
        Q_ASSERT((int)values.count() >= (int)parent::channels_nb);
        channels_type c;
        for (uint i = 0; i < parent::channels_nb; i++) {
            float b = 0;
            switch(i) {
            case L_pos:
                b = qBound((float)MIN_CHANNEL_L,
                           (float)KoColorSpaceMathsTraits<float>::unitValue * values[i],
                           (float)MAX_CHANNEL_L);
                break;
            case a_pos:
            case b_pos:
                b = qBound((float)MIN_CHANNEL_AB,
                           (float)KoColorSpaceMathsTraits<float>::unitValue * values[i],
                           (float)MAX_CHANNEL_AB);
                break;
            case 3:
                b = qBound((float)KoColorSpaceMathsTraits<float>::min,
                           (float)KoColorSpaceMathsTraits<float>::unitValue * values[i],
                           (float)KoColorSpaceMathsTraits<float>::max);
            default:
                break;
            }
            c = (channels_type)b;
            parent::nativeArray(pixel)[i] = c;
        }
    }
};

struct KoLabF64Traits : public KoLabTraits<double> {
    static constexpr double MIN_CHANNEL_L = 0;
    static constexpr double MAX_CHANNEL_L = 100;
    static constexpr double MIN_CHANNEL_AB = -128;
    static constexpr double MAX_CHANNEL_AB = +127;

    // Lab has some... particulars
    inline static QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) {
        return channelValueText(pixel, channelIndex);
    }
    inline static void normalisedChannelsValue(const quint8 *pixel, QVector<float> &channels) {
        Q_ASSERT((int)channels.count() >= (int)parent::channels_nb);
        channels_type c;
        for (uint i = 0; i < parent::channels_nb; i++) {
            c = parent::nativeArray(pixel)[i];
            channels[i] = (qreal)c;
        }
    }
    inline static void fromNormalisedChannelsValue(quint8 *pixel, const QVector<float> &values) {
        Q_ASSERT((int)values.count() >= (int)parent::channels_nb);
        channels_type c;
        for (uint i = 0; i < parent::channels_nb; i++) {
            float b = 0;
            switch(i) {
            case L_pos:
                b = qBound((float)MIN_CHANNEL_L,
                           (float)KoColorSpaceMathsTraits<double>::unitValue * values[i],
                           (float)MAX_CHANNEL_L);
                break;
            case a_pos:
            case b_pos:
                b = qBound((float)MIN_CHANNEL_AB,
                           (float)KoColorSpaceMathsTraits<double>::unitValue * values[i],
                           (float)MAX_CHANNEL_AB);
                break;
            case 3:
                b = qBound((float)KoColorSpaceMathsTraits<double>::min,
                           (float)KoColorSpaceMathsTraits<double>::unitValue * values[i],
                           (float)KoColorSpaceMathsTraits<double>::max);
            default:
                break;
            }
            c = (channels_type)b;
            parent::nativeArray(pixel)[i] = c;
        }
    }
};

#endif
