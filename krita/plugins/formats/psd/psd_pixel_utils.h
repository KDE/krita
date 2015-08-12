/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __PSD_PIXEL_UTILS_H
#define __PSD_PIXEL_UTILS_H

#include <QtGlobal>
#include <QMap>

#include <KoColorSpace.h>
#include <KoColorSpaceMaths.h>
#include <KoColorSpaceTraits.h>
#include <KoCmykColorSpaceTraits.h>

#include <netinet/in.h> // htonl


namespace PsdPixelUtils
{

template <class Traits>
typename Traits::channels_type convertByteOrder(typename Traits::channels_type value);
// default implementation is undefined for every color space should be added manually

template <>
inline quint8 convertByteOrder<KoGrayU8Traits>(quint8 value) {
    return value;
}

template <>
inline quint16 convertByteOrder<KoGrayU16Traits>(quint16 value) {
    return ntohs(value);
}

template <>
inline quint32 convertByteOrder<KoGrayU32Traits>(quint32 value) {
    return ntohs(value);
}

template <>
inline quint8 convertByteOrder<KoBgrU8Traits>(quint8 value) {
    return value;
}

template <>
inline quint16 convertByteOrder<KoBgrU16Traits>(quint16 value) {
    return ntohs(value);
}

template <>
inline quint32 convertByteOrder<KoBgrU32Traits>(quint32 value) {
    return ntohs(value);
}

template <>
inline quint8 convertByteOrder<KoCmykU8Traits>(quint8 value) {
    return value;
}

template <>
inline quint16 convertByteOrder<KoCmykU16Traits>(quint16 value) {
    return ntohs(value);
}

template <>
inline float convertByteOrder<KoCmykF32Traits>(float value) {
    return ntohs(value);
}

template <>
inline quint8 convertByteOrder<KoLabU8Traits>(quint8 value) {
    return value;
}

template <>
inline quint16 convertByteOrder<KoLabU16Traits>(quint16 value) {
    return ntohs(value);
}

template <>
inline float convertByteOrder<KoLabF32Traits>(float value) {
    return ntohs(value);
}

template <class Traits>
void readGrayPixel(const QMap<quint16, QByteArray> &channelBytes,
                  int col, quint8 *dstPtr)
{
    typedef typename Traits::Pixel Pixel;
    typedef typename Traits::channels_type channels_type;

    const channels_type unitValue = KoColorSpaceMathsTraits<channels_type>::unitValue;
    channels_type opacity = unitValue;
    if (channelBytes.contains(-1)) {
        opacity = channelBytes[-1].constData()[col];
    }

    Pixel *pixelPtr = reinterpret_cast<Pixel*>(dstPtr);

    channels_type gray = convertByteOrder<Traits>(reinterpret_cast<const channels_type *>(channelBytes[0].constData())[col]);

    pixelPtr->gray = gray;
    pixelPtr->alpha = opacity;
}

template <class Traits>
void readRgbPixel(const QMap<quint16, QByteArray> &channelBytes,
                  int col, quint8 *dstPtr)
{
    typedef typename Traits::Pixel Pixel;
    typedef typename Traits::channels_type channels_type;

    const channels_type unitValue = KoColorSpaceMathsTraits<channels_type>::unitValue;
    channels_type opacity = unitValue;
    if (channelBytes.contains(-1)) {
        opacity = channelBytes[-1].constData()[col];
    }

    Pixel *pixelPtr = reinterpret_cast<Pixel*>(dstPtr);

    channels_type blue = convertByteOrder<Traits>(reinterpret_cast<const channels_type *>(channelBytes[2].constData())[col]);
    channels_type green = convertByteOrder<Traits>(reinterpret_cast<const channels_type *>(channelBytes[1].constData())[col]);
    channels_type red = convertByteOrder<Traits>(reinterpret_cast<const channels_type *>(channelBytes[0].constData())[col]);

    pixelPtr->blue = blue;
    pixelPtr->green = green;
    pixelPtr->red = red;
    pixelPtr->alpha = opacity;
}

template <class Traits>
void readCmykPixel(const QMap<quint16, QByteArray> &channelBytes,
                       int col, quint8 *dstPtr)
{
    typedef typename Traits::Pixel Pixel;
    typedef typename Traits::channels_type channels_type;

    const channels_type unitValue = KoColorSpaceMathsTraits<channels_type>::unitValue;
    channels_type opacity = unitValue;
    if (channelBytes.contains(-1)) {
        opacity = channelBytes[-1].constData()[col];
    }

    Pixel *pixelPtr = reinterpret_cast<Pixel*>(dstPtr);

    channels_type cyan = unitValue - convertByteOrder<Traits>(reinterpret_cast<const channels_type *>(channelBytes[0].constData())[col]);
    channels_type magenta = unitValue - convertByteOrder<Traits>(reinterpret_cast<const channels_type *>(channelBytes[1].constData())[col]);
    channels_type yellow = unitValue - convertByteOrder<Traits>(reinterpret_cast<const channels_type *>(channelBytes[2].constData())[col]);
    channels_type black = unitValue - convertByteOrder<Traits>(reinterpret_cast<const channels_type *>(channelBytes[3].constData())[col]);

    pixelPtr->cyan = cyan;
    pixelPtr->magenta = magenta;
    pixelPtr->yellow = yellow;
    pixelPtr->black = black;
    pixelPtr->alpha = opacity;
}

template <class Traits>
void readLabPixel(const QMap<quint16, QByteArray> &channelBytes,
                  int col, quint8 *dstPtr)
{
    typedef typename Traits::Pixel Pixel;
    typedef typename Traits::channels_type channels_type;

    const channels_type unitValue = KoColorSpaceMathsTraits<channels_type>::unitValue;
    channels_type opacity = unitValue;
    if (channelBytes.contains(-1)) {
        opacity = channelBytes[-1].constData()[col];
    }

    Pixel *pixelPtr = reinterpret_cast<Pixel*>(dstPtr);

    channels_type L = convertByteOrder<Traits>(reinterpret_cast<const channels_type *>(channelBytes[0].constData())[col]);
    channels_type a = convertByteOrder<Traits>(reinterpret_cast<const channels_type *>(channelBytes[1].constData())[col]);
    channels_type b = convertByteOrder<Traits>(reinterpret_cast<const channels_type *>(channelBytes[2].constData())[col]);

    pixelPtr->L = L;
    pixelPtr->a = a;
    pixelPtr->b = b;
    pixelPtr->alpha = opacity;
}

template <class Traits8, class Traits16, class Traits32>
inline void readPixelCommon(int channelSize,
                            const QMap<quint16, QByteArray> &channelBytes,
                            int col, quint8 *dstPtr)
{
    if (channelSize == 1) {
        readRgbPixel<Traits8>(channelBytes, col, dstPtr);
    } else if (channelSize == 2) {
        readRgbPixel<Traits16>(channelBytes, col, dstPtr);
    } else if (channelSize == 4) {
        readRgbPixel<Traits32>(channelBytes, col, dstPtr);
    }
}

inline void readRgbPixelCommon(int channelSize,
                               const QMap<quint16, QByteArray> &channelBytes,
                               int col, quint8 *dstPtr)
{
    if (channelSize == 1) {
        readRgbPixel<KoBgrU8Traits>(channelBytes, col, dstPtr);
    } else if (channelSize == 2) {
        readRgbPixel<KoBgrU16Traits>(channelBytes, col, dstPtr);
    } else if (channelSize == 4) {
        readRgbPixel<KoBgrU16Traits>(channelBytes, col, dstPtr);
    }
}

inline void readGrayPixelCommon(int channelSize,
                                const QMap<quint16, QByteArray> &channelBytes,
                                int col, quint8 *dstPtr)
{
    if (channelSize == 1) {
        readGrayPixel<KoGrayU8Traits>(channelBytes, col, dstPtr);
    } else if (channelSize == 2) {
        readGrayPixel<KoGrayU16Traits>(channelBytes, col, dstPtr);
    } else if (channelSize == 4) {
        readGrayPixel<KoGrayU32Traits>(channelBytes, col, dstPtr);
    }
}

inline void readCmykPixelCommon(int channelSize,
                                const QMap<quint16, QByteArray> &channelBytes,
                                int col, quint8 *dstPtr)
{
    if (channelSize == 1) {
        readCmykPixel<KoCmykU8Traits>(channelBytes, col, dstPtr);
    } else if (channelSize == 2) {
        readCmykPixel<KoCmykU16Traits>(channelBytes, col, dstPtr);
    } else if (channelSize == 4) {
        readCmykPixel<KoCmykF32Traits>(channelBytes, col, dstPtr);
    }
}

inline void readLabPixelCommon(int channelSize,
                                const QMap<quint16, QByteArray> &channelBytes,
                                int col, quint8 *dstPtr)
{
    if (channelSize == 1) {
        readLabPixel<KoLabU8Traits>(channelBytes, col, dstPtr);
    } else if (channelSize == 2) {
        readLabPixel<KoLabU16Traits>(channelBytes, col, dstPtr);
    } else if (channelSize == 4) {
        readLabPixel<KoLabF32Traits>(channelBytes, col, dstPtr);
    }
}

}

#endif /* __PSD_PIXEL_UTILS_H */
