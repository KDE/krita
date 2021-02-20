/*
 *  SPDX-FileCopyrightText: 2006-2007 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2016, 2017, 2020 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _KO_CMYK_COLORSPACE_TRAITS_H_
#define _KO_CMYK_COLORSPACE_TRAITS_H_

#include <KoCmykColorSpaceMaths.h>

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

    inline static QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) {
        if (channelIndex > parent::channels_nb) return QString("Error");
        channels_type c = nativeArray(pixel)[channelIndex];
        switch (channelIndex) {
        case c_pos:
        case m_pos:
        case y_pos:
        case k_pos:
            return QString().setNum(100.0 * qBound((qreal)0,
                                                   ((qreal)c) / KoCmykColorSpaceMathsTraits<channels_type>::unitValueCMYK,
                                                   (qreal)KoCmykColorSpaceMathsTraits<channels_type>::unitValueCMYK));
        case 4:
            return QString().setNum(100.0 * qBound((qreal)0,
                                                   ((qreal)c) / KoCmykColorSpaceMathsTraits<channels_type>::unitValue,
                                                   (qreal)KoCmykColorSpaceMathsTraits<channels_type>::unitValue));
        default:
            return QString("Error");
        }
    }

    inline static void normalisedChannelsValue(const quint8 *pixel, QVector<float> &channels) {
        Q_ASSERT((int)channels.count() == (int)parent::channels_nb);
        channels_type c;
        for (uint i = 0; i < parent::channels_nb; i++) {
            c = nativeArray(pixel)[i];
            switch (i) {
            case c_pos:
            case m_pos:
            case y_pos:
            case k_pos:
                channels[i] = qBound((qreal)0,
                                     ((qreal)c) / KoCmykColorSpaceMathsTraits<channels_type>::unitValueCMYK,
                                     (qreal)KoCmykColorSpaceMathsTraits<channels_type>::unitValueCMYK);
                break;
            // As per KoChannelInfo alpha channels are [0..1]
            case 4:
            default:
                channels[i] = qBound((qreal)0,
                                     ((qreal)c) / KoCmykColorSpaceMathsTraits<channels_type>::unitValue,
                                     (qreal)KoCmykColorSpaceMathsTraits<channels_type>::unitValue);
                break;
            }
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
                           (float)KoCmykColorSpaceMathsTraits<channels_type>::unitValueCMYK * values[i],
                           (float)KoCmykColorSpaceMathsTraits<channels_type>::unitValueCMYK);
                break;
            default:
                b = qBound((float)KoCmykColorSpaceMathsTraits<channels_type>::min,
                           (float)KoCmykColorSpaceMathsTraits<channels_type>::unitValue * values[i],
                           (float)KoCmykColorSpaceMathsTraits<channels_type>::max);
                break;
            }
            c = (channels_type)b;
            parent::nativeArray(pixel)[i] = c;
        }
    }
};

#endif

struct KoCmykF32Traits : public KoCmykTraits<float> {

    inline static QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) {
        if (channelIndex > parent::channels_nb) return QString("Error");
        channels_type c = nativeArray(pixel)[channelIndex];
        switch (channelIndex) {
        case c_pos:
        case m_pos:
        case y_pos:
        case k_pos:
            return QString().setNum(100.0 * qBound((qreal)0,
                                                   ((qreal)c) / KoCmykColorSpaceMathsTraits<channels_type>::unitValueCMYK,
                                                   (qreal)KoCmykColorSpaceMathsTraits<channels_type>::unitValueCMYK));
        case 4:
            return QString().setNum(100.0 * qBound((qreal)0,
                                                   ((qreal)c) / KoCmykColorSpaceMathsTraits<channels_type>::unitValue,
                                                   (qreal)KoCmykColorSpaceMathsTraits<channels_type>::unitValue));
        default:
            return QString("Error");
        }
    }

    inline static void normalisedChannelsValue(const quint8 *pixel, QVector<float> &channels) {
        Q_ASSERT((int)channels.count() == (int)parent::channels_nb);
        channels_type c;
        for (uint i = 0; i < parent::channels_nb; i++) {
            c = nativeArray(pixel)[i];
            switch (i) {
            case c_pos:
            case m_pos:
            case y_pos:
            case k_pos:
                channels[i] = qBound((qreal)0,
                                     ((qreal)c) / KoCmykColorSpaceMathsTraits<channels_type>::unitValueCMYK,
                                     (qreal)KoCmykColorSpaceMathsTraits<channels_type>::unitValueCMYK);
                break;
            // As per KoChannelInfo alpha channels are [0..1]
            case 4:
            default:
                channels[i] = qBound((qreal)0,
                                     ((qreal)c) / KoCmykColorSpaceMathsTraits<channels_type>::unitValue,
                                     (qreal)KoCmykColorSpaceMathsTraits<channels_type>::unitValue);
                break;
            }
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
                           (float)KoCmykColorSpaceMathsTraits<channels_type>::unitValueCMYK * values[i],
                           (float)KoCmykColorSpaceMathsTraits<channels_type>::unitValueCMYK);
                break;
            default:
                b = qBound((float)KoCmykColorSpaceMathsTraits<channels_type>::min,
                           (float)KoCmykColorSpaceMathsTraits<channels_type>::unitValue * values[i],
                           (float)KoCmykColorSpaceMathsTraits<channels_type>::max);
                break;
            }
            c = (channels_type)b;
            parent::nativeArray(pixel)[i] = c;
        }
    }
};

struct KoCmykF64Traits : public KoCmykTraits<double> {

    inline static QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) {
        if (channelIndex > parent::channels_nb) return QString("Error");
        channels_type c = nativeArray(pixel)[channelIndex];
        switch (channelIndex) {
        case c_pos:
        case m_pos:
        case y_pos:
        case k_pos:
            return QString().setNum(100.0 * qBound((qreal)0,
                                                   ((qreal)c) / KoCmykColorSpaceMathsTraits<channels_type>::unitValueCMYK,
                                                   (qreal)KoCmykColorSpaceMathsTraits<channels_type>::unitValueCMYK));
        case 4:
            return QString().setNum(100.0 * qBound((qreal)0,
                                                   ((qreal)c) / KoCmykColorSpaceMathsTraits<channels_type>::unitValue,
                                                   (qreal)KoCmykColorSpaceMathsTraits<channels_type>::unitValue));
        default:
            return QString("Error");
        }
    }

    inline static void normalisedChannelsValue(const quint8 *pixel, QVector<float> &channels) {
        Q_ASSERT((int)channels.count() == (int)parent::channels_nb);
        channels_type c;
        for (uint i = 0; i < parent::channels_nb; i++) {
            c = nativeArray(pixel)[i];
            switch (i) {
            case c_pos:
            case m_pos:
            case y_pos:
            case k_pos:
                channels[i] = qBound((qreal)0,
                                     ((qreal)c) / KoCmykColorSpaceMathsTraits<channels_type>::unitValueCMYK,
                                     (qreal)KoCmykColorSpaceMathsTraits<channels_type>::unitValueCMYK);
                break;
            // As per KoChannelInfo alpha channels are [0..1]
            case 4:
            default:
                channels[i] = qBound((qreal)0,
                                     ((qreal)c) / KoCmykColorSpaceMathsTraits<channels_type>::unitValue,
                                     (qreal)KoCmykColorSpaceMathsTraits<channels_type>::unitValue);
                break;
            }
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
                           (float)KoCmykColorSpaceMathsTraits<channels_type>::unitValueCMYK * values[i],
                           (float)KoCmykColorSpaceMathsTraits<channels_type>::unitValueCMYK);
                break;
            default:
                b = qBound((float)KoCmykColorSpaceMathsTraits<channels_type>::min,
                           (float)KoCmykColorSpaceMathsTraits<channels_type>::unitValue * values[i],
                           (float)KoCmykColorSpaceMathsTraits<channels_type>::max);
                break;
            }
            c = (channels_type)b;
            parent::nativeArray(pixel)[i] = c;
        }
    }
};



#endif
    
