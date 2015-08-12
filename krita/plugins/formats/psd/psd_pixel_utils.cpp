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

#include "psd_pixel_utils.h"

#include <QtGlobal>
#include <QMap>
#include <QIODevice>


#include <KoColorSpace.h>
#include <KoColorSpaceMaths.h>
#include <KoColorSpaceTraits.h>
#include <KoCmykColorSpaceTraits.h>

#include <netinet/in.h> // htonl

#include <asl/kis_asl_reader_utils.h>

#include "psd_layer_record.h"
#include <asl/kis_offset_keeper.h>
#include "kis_iterator_ng.h"


namespace PsdPixelUtils {

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

void readRgbPixelCommon(int channelSize,
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

void readGrayPixelCommon(int channelSize,
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

void readCmykPixelCommon(int channelSize,
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

void readLabPixelCommon(int channelSize,
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

QMap<quint16, QByteArray> fetchChannelsBytes(QIODevice *io, QVector<ChannelInfo*> channelInfoRecords,
                                            int row, int width, int channelSize)
{
    const int uncompressedLength = width * channelSize;

    QMap<quint16, QByteArray> channelBytes;

    foreach(ChannelInfo *channelInfo, channelInfoRecords) {
        // user supplied masks are ignored here
        if (channelInfo->channelId < -1) continue;

        io->seek(channelInfo->channelDataStart + channelInfo->channelOffset);

        if (channelInfo->compressionType == Compression::Uncompressed) {
            channelBytes[channelInfo->channelId] = io->read(uncompressedLength);
            channelInfo->channelOffset += uncompressedLength;
        }
        else if (channelInfo->compressionType == Compression::RLE) {
            int rleLength = channelInfo->rleRowLengths[row];
            QByteArray compressedBytes = io->read(rleLength);
            QByteArray uncompressedBytes = Compression::uncompress(uncompressedLength, compressedBytes, channelInfo->compressionType);
            channelBytes.insert(channelInfo->channelId, uncompressedBytes);
            channelInfo->channelOffset += rleLength;
        }
        else {
            QString error = QString("Unsupported Compression mode: %1").arg(channelInfo->compressionType);
            dbgFile << "ERROR: fetchChannelsBytes:" << error;
            throw KisAslReaderUtils::ASLParseException(error);
        }
    }

    return channelBytes;
}

typedef boost::function<void(int, const QMap<quint16, QByteArray>&, int, quint8*)> PixelFunc;

void readCommon(KisPaintDeviceSP dev,
                QIODevice *io,
                const QRect &layerRect,
                QVector<ChannelInfo*> infoRecords,
                int channelSize,
                PixelFunc pixelFunc)
{
    KisOffsetKeeper keeper(io);

    if (layerRect.isEmpty()) {
        dbgFile << "Empty layer!";
        return;
    }

    KisHLineIteratorSP it = dev->createHLineIteratorNG(layerRect.left(), layerRect.top(), layerRect.width());

    for (int i = 0 ; i < layerRect.height(); i++) {
        QMap<quint16, QByteArray> channelBytes;

        channelBytes = fetchChannelsBytes(io, infoRecords,
                                          i, layerRect.width(), channelSize);

        for (qint64 col = 0; col < layerRect.width(); col++){
            pixelFunc(channelSize, channelBytes, col, it->rawData());
            it->nextPixel();
        }
        it->nextRow();
    }
}

void readChannels(QIODevice *io,
                  KisPaintDeviceSP device,
                  psd_color_mode colorMode,
                  int channelSize,
                  const QRect &layerRect,
                  QVector<ChannelInfo*> infoRecords)
{
    switch (colorMode) {
    case Grayscale:
        readCommon(device, io, layerRect, infoRecords, channelSize, &readGrayPixelCommon);
        break;
    case RGB:
        readCommon(device, io, layerRect, infoRecords, channelSize, &readRgbPixelCommon);
        break;
    case CMYK:
        readCommon(device, io, layerRect, infoRecords, channelSize, &readCmykPixelCommon);
        break;
    case Lab:
        readCommon(device, io, layerRect, infoRecords, channelSize, &readLabPixelCommon);
        break;
    case Bitmap:
    case Indexed:
    case MultiChannel:
    case DuoTone:
    case UNKNOWN:
    default:
        QString error = QString("Unsupported color mode: %1").arg(colorMode);
        throw KisAslReaderUtils::ASLParseException(error);
    }
}
}
