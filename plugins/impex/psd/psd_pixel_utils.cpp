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
#include <colorspaces/KoAlphaColorSpace.h>

#include <QtEndian>

#include "kis_global.h"
#include <asl/kis_asl_writer_utils.h>
#include <asl/kis_asl_reader_utils.h>

#include "psd_layer_record.h"
#include <asl/kis_offset_keeper.h>
#include "kis_iterator_ng.h"

#include "config_psd.h"
#ifdef HAVE_ZLIB
#include "zlib.h"
#endif

namespace PsdPixelUtils {

template <class Traits>
typename Traits::channels_type convertByteOrder(typename Traits::channels_type value);
// default implementation is undefined for every color space should be added manually

template <>
inline quint8 convertByteOrder<AlphaU8Traits>(quint8 value) {
    return value;
}

template <>
inline quint16 convertByteOrder<AlphaU16Traits>(quint16 value) {
    return qFromBigEndian((quint16)value);
}

template <>
inline float convertByteOrder<AlphaF32Traits>(float value) {
    return qFromBigEndian((quint32)value);
}


template <>
inline quint8 convertByteOrder<KoGrayU8Traits>(quint8 value) {
    return value;
}

template <>
inline quint16 convertByteOrder<KoGrayU16Traits>(quint16 value) {
    return qFromBigEndian((quint16)value);
}

template <>
inline quint32 convertByteOrder<KoGrayU32Traits>(quint32 value) {
    return qFromBigEndian((quint32)value);
}

template <>
inline quint8 convertByteOrder<KoBgrU8Traits>(quint8 value) {
    return value;
}

template <>
inline quint16 convertByteOrder<KoBgrU16Traits>(quint16 value) {
    return qFromBigEndian((quint16)value);
}

template <>
inline quint32 convertByteOrder<KoBgrU32Traits>(quint32 value) {
    return qFromBigEndian((quint32)value);
}

template <>
inline quint8 convertByteOrder<KoCmykU8Traits>(quint8 value) {
    return value;
}

template <>
inline quint16 convertByteOrder<KoCmykU16Traits>(quint16 value) {
    return qFromBigEndian((quint16)value);
}

template <>
inline float convertByteOrder<KoCmykF32Traits>(float value) {
    return qFromBigEndian((quint32)value);
}

template <>
inline quint8 convertByteOrder<KoLabU8Traits>(quint8 value) {
    return value;
}

template <>
inline quint16 convertByteOrder<KoLabU16Traits>(quint16 value) {
    return qFromBigEndian((quint16)value);
}

template <>
inline float convertByteOrder<KoLabF32Traits>(float value) {
    return qFromBigEndian((quint32)value);
}

template <class Traits>
void readAlphaMaskPixel(const QMap<quint16, QByteArray> &channelBytes,
                        int col, quint8 *dstPtr);

template <>
void readAlphaMaskPixel<AlphaU8Traits>(const QMap<quint16, QByteArray> &channelBytes,
                                       int col, quint8 *dstPtr)
{
    *dstPtr = reinterpret_cast<const quint8*>(channelBytes.first().constData())[col];
}

template <>
void readAlphaMaskPixel<AlphaU16Traits>(const QMap<quint16, QByteArray> &channelBytes,
                                       int col, quint8 *dstPtr)
{
    *dstPtr = reinterpret_cast<const quint16*>(channelBytes.first().constData())[col] >> 8;
}

template <>
void readAlphaMaskPixel<AlphaF32Traits>(const QMap<quint16, QByteArray> &channelBytes,
                                        int col, quint8 *dstPtr)
{
    *dstPtr = reinterpret_cast<const float*>(channelBytes.first().constData())[col] * 255;
}

template <class Traits>
inline typename Traits::channels_type readChannelValue(const QMap<quint16, QByteArray> &channelBytes,
                                       quint16 channelId, int col, typename Traits::channels_type defaultValue)
{
    typedef typename Traits::channels_type channels_type;

    if (channelBytes.contains(channelId)) {
        const QByteArray &bytes = channelBytes[channelId];
        if (col < bytes.size()) {
            return convertByteOrder<Traits>(reinterpret_cast<const channels_type *>(bytes.constData())[col]);
        }

        dbgFile << "col index out of range channelId: "<< channelId << " col:" << col;
    }

    return defaultValue;
}

template <class Traits>
void readGrayPixel(const QMap<quint16, QByteArray> &channelBytes,
                  int col, quint8 *dstPtr)
{
    typedef typename Traits::Pixel Pixel;
    typedef typename Traits::channels_type channels_type;

    const channels_type unitValue = KoColorSpaceMathsTraits<channels_type>::unitValue;
    Pixel *pixelPtr = reinterpret_cast<Pixel*>(dstPtr);

    pixelPtr->gray  = readChannelValue<Traits>(channelBytes, 0, col, unitValue);
    pixelPtr->alpha = readChannelValue<Traits>(channelBytes, -1, col, unitValue);
}

template <class Traits>
void readRgbPixel(const QMap<quint16, QByteArray> &channelBytes,
                  int col, quint8 *dstPtr)
{
    typedef typename Traits::Pixel Pixel;
    typedef typename Traits::channels_type channels_type;

    const channels_type unitValue = KoColorSpaceMathsTraits<channels_type>::unitValue;
    Pixel *pixelPtr = reinterpret_cast<Pixel*>(dstPtr);

    pixelPtr->blue  = readChannelValue<Traits>(channelBytes, 2, col, unitValue);
    pixelPtr->green = readChannelValue<Traits>(channelBytes, 1, col, unitValue);
    pixelPtr->red   = readChannelValue<Traits>(channelBytes, 0, col, unitValue);
    pixelPtr->alpha = readChannelValue<Traits>(channelBytes, -1, col, unitValue);

}

template <class Traits>
void readCmykPixel(const QMap<quint16, QByteArray> &channelBytes,
                       int col, quint8 *dstPtr)
{
    typedef typename Traits::Pixel Pixel;
    typedef typename Traits::channels_type channels_type;

    const channels_type unitValue = KoColorSpaceMathsTraits<channels_type>::unitValue;
    Pixel *pixelPtr = reinterpret_cast<Pixel*>(dstPtr);

    pixelPtr->cyan    = unitValue - readChannelValue<Traits>(channelBytes, 0, col, unitValue);
    pixelPtr->magenta = unitValue - readChannelValue<Traits>(channelBytes, 1, col, unitValue);
    pixelPtr->yellow  = unitValue - readChannelValue<Traits>(channelBytes, 2, col, unitValue);
    pixelPtr->black   = unitValue - readChannelValue<Traits>(channelBytes, 3, col, unitValue);
    pixelPtr->alpha   = readChannelValue<Traits>(channelBytes, -1, col, unitValue);
}

template <class Traits>
void readLabPixel(const QMap<quint16, QByteArray> &channelBytes,
                  int col, quint8 *dstPtr)
{
    typedef typename Traits::Pixel Pixel;
    typedef typename Traits::channels_type channels_type;

    const channels_type unitValue = KoColorSpaceMathsTraits<channels_type>::unitValue;
    Pixel *pixelPtr = reinterpret_cast<Pixel*>(dstPtr);

    pixelPtr->L = readChannelValue<Traits>(channelBytes, 0, col, unitValue);
    pixelPtr->a = readChannelValue<Traits>(channelBytes, 1, col, unitValue);
    pixelPtr->b = readChannelValue<Traits>(channelBytes, 2, col, unitValue);
    pixelPtr->alpha = readChannelValue<Traits>(channelBytes, -1, col, unitValue);
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

void readAlphaMaskPixelCommon(int channelSize,
                                const QMap<quint16, QByteArray> &channelBytes,
                                int col, quint8 *dstPtr)
{
    if (channelSize == 1) {
        readAlphaMaskPixel<AlphaU8Traits>(channelBytes, col, dstPtr);
    } else if (channelSize == 2) {
        readAlphaMaskPixel<AlphaU16Traits>(channelBytes, col, dstPtr);
    } else if (channelSize == 4) {
        readAlphaMaskPixel<AlphaF32Traits>(channelBytes, col, dstPtr);
    }
}

/**********************************************************************/
/* Two functions copied from the abandoned PSDParse library (GPL)     */
/* See: http://www.telegraphics.com.au/svn/psdparse/trunk/psd_zip.c   */
/* Created by Patrick in 2007.02.02, libpsd@graphest.com              */
/* Modifications by Toby Thain <toby@telegraphics.com.au>             */
/**********************************************************************/

typedef bool psd_status;
typedef quint8 psd_uchar;
typedef int psd_int;
typedef quint8 Bytef;

psd_status psd_unzip_without_prediction(psd_uchar *src_buf, psd_int src_len,
                                        psd_uchar *dst_buf, psd_int dst_len)
{
#ifdef HAVE_ZLIB
    z_stream stream;
    psd_int state;

    memset(&stream, 0, sizeof(z_stream));
    stream.data_type = Z_BINARY;

    stream.next_in = (Bytef *)src_buf;
    stream.avail_in = src_len;
    stream.next_out = (Bytef *)dst_buf;
    stream.avail_out = dst_len;

    if(inflateInit(&stream) != Z_OK)
        return 0;

    do {
        state = inflate(&stream, Z_PARTIAL_FLUSH);
        if(state == Z_STREAM_END)
            break;
        if(state != Z_OK)
            break;
    }  while (stream.avail_out > 0);

    if (state != Z_STREAM_END && state != Z_OK)
        return 0;

    return 1;

#endif /* HAVE_ZLIB */

    return 0;
}

psd_status psd_unzip_with_prediction(psd_uchar *src_buf, psd_int src_len,
                                     psd_uchar *dst_buf, psd_int dst_len,
                                     psd_int row_size, psd_int color_depth)
{
    psd_status status;
    int len;
    psd_uchar * buf;

    status = psd_unzip_without_prediction(src_buf, src_len, dst_buf, dst_len);
    if(!status)
        return status;

    buf = dst_buf;
    do {
        len = row_size;
        if (color_depth == 16)
        {
            while(--len)
            {
                buf[2] += buf[0] + ((buf[1] + buf[3]) >> 8);
                buf[3] += buf[1];
                buf += 2;
            }
            buf += 2;
            dst_len -= row_size * 2;
        }
        else
        {
            while(--len)
            {
                *(buf + 1) += *buf;
                buf ++;
            }
            buf ++;
            dst_len -= row_size;
        }
    } while(dst_len > 0);

    return 1;
}

/**********************************************************************/
/* End of third party block                                           */
/**********************************************************************/

QMap<quint16, QByteArray> fetchChannelsBytes(QIODevice *io, QVector<ChannelInfo*> channelInfoRecords,
                                            int row, int width, int channelSize, bool processMasks)
{
    const int uncompressedLength = width * channelSize;

    QMap<quint16, QByteArray> channelBytes;

    Q_FOREACH (ChannelInfo *channelInfo, channelInfoRecords) {
        // user supplied masks are ignored here
        if (!processMasks && channelInfo->channelId < -1) continue;

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
                PixelFunc pixelFunc,
                bool processMasks)
{
    KisOffsetKeeper keeper(io);

    if (layerRect.isEmpty()) {
        dbgFile << "Empty layer!";
        return;
    }

    if (infoRecords.first()->compressionType == Compression::ZIP ||
        infoRecords.first()->compressionType == Compression::ZIPWithPrediction) {

        const int numPixels = channelSize * layerRect.width() * layerRect.height();

        QMap<quint16, QByteArray> channelBytes;

        Q_FOREACH (ChannelInfo *info, infoRecords) {
            io->seek(info->channelDataStart);
            QByteArray compressedBytes = io->read(info->channelDataLength);
            QByteArray uncompressedBytes(numPixels, 0);

            bool status = false;
            if (infoRecords.first()->compressionType == Compression::ZIP) {
                status = psd_unzip_without_prediction((quint8*)compressedBytes.data(), compressedBytes.size(),
                                                      (quint8*)uncompressedBytes.data(), uncompressedBytes.size());
            } else {
                status = psd_unzip_with_prediction((quint8*)compressedBytes.data(), compressedBytes.size(),
                                                   (quint8*)uncompressedBytes.data(), uncompressedBytes.size(),
                                                   layerRect.width(), channelSize * 8);
            }

            if (!status) {
                QString error = QString("Failed to unzip channel data: id = %1, compression = %2").arg(info->channelId).arg(info->compressionType);
                dbgFile << "ERROR:" << error;
                dbgFile << "      " << ppVar(info->channelId);
                dbgFile << "      " << ppVar(info->channelDataStart);
                dbgFile << "      " << ppVar(info->channelDataLength);
                dbgFile << "      " << ppVar(info->compressionType);
                throw KisAslReaderUtils::ASLParseException(error);
            }

            channelBytes.insert(info->channelId, uncompressedBytes);
        }

        KisSequentialIterator it(dev, layerRect);
        int col = 0;
        while (it.nextPixel()) {
            pixelFunc(channelSize, channelBytes, col, it.rawData());
            col++;
        }

    } else {
        KisHLineIteratorSP it = dev->createHLineIteratorNG(layerRect.left(), layerRect.top(), layerRect.width());
        for (int i = 0 ; i < layerRect.height(); i++) {
            QMap<quint16, QByteArray> channelBytes;

            channelBytes = fetchChannelsBytes(io, infoRecords,
                                              i, layerRect.width(),
                                              channelSize, processMasks);

            for (qint64 col = 0; col < layerRect.width(); col++){
                pixelFunc(channelSize, channelBytes, col, it->rawData());
                it->nextPixel();
            }
            it->nextRow();
        }
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
        readCommon(device, io, layerRect, infoRecords, channelSize, &readGrayPixelCommon, false);
        break;
    case RGB:
        readCommon(device, io, layerRect, infoRecords, channelSize, &readRgbPixelCommon, false);
        break;
    case CMYK:
        readCommon(device, io, layerRect, infoRecords, channelSize, &readCmykPixelCommon, false);
        break;
    case Lab:
        readCommon(device, io, layerRect, infoRecords, channelSize, &readLabPixelCommon, false);
        break;
    case Bitmap:
    case Indexed:
    case MultiChannel:
    case DuoTone:
    case COLORMODE_UNKNOWN:
    default:
        QString error = QString("Unsupported color mode: %1").arg(colorMode);
        throw KisAslReaderUtils::ASLParseException(error);
    }
}

void readAlphaMaskChannels(QIODevice *io,
                           KisPaintDeviceSP device,
                           int channelSize,
                           const QRect &layerRect,
                           QVector<ChannelInfo*> infoRecords)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(infoRecords.size() == 1);
    readCommon(device, io, layerRect, infoRecords, channelSize, &readAlphaMaskPixelCommon, true);
}

void writeChannelDataRLE(QIODevice *io, const quint8 *plane, const int channelSize, const QRect &rc, const qint64 sizeFieldOffset, const qint64 rleBlockOffset, const bool writeCompressionType)
{
    typedef KisAslWriterUtils::OffsetStreamPusher<quint32> Pusher;
    QScopedPointer<Pusher> channelBlockSizeExternalTag;
    if (sizeFieldOffset >= 0) {
        channelBlockSizeExternalTag.reset(new Pusher(io, 0, sizeFieldOffset));
    }

    if (writeCompressionType) {
        SAFE_WRITE_EX(io, (quint16)Compression::RLE);
    }

    const bool externalRleBlock = rleBlockOffset >= 0;

    // the start of RLE sizes block
    const qint64 channelRLESizePos = externalRleBlock ? rleBlockOffset : io->pos();

    {
        QScopedPointer<KisOffsetKeeper> rleOffsetKeeper;

        if (externalRleBlock) {
            rleOffsetKeeper.reset(new KisOffsetKeeper(io));
            io->seek(rleBlockOffset);
        }

        // write zero's for the channel lengths block
        for(int i = 0; i < rc.height(); ++i) {
            // XXX: choose size for PSB!
            const quint16 fakeRLEBLockSize = 0;
            SAFE_WRITE_EX(io, fakeRLEBLockSize);
        }
    }

    quint32 stride = channelSize * rc.width();
    for (qint32 row = 0; row < rc.height(); ++row) {

        QByteArray uncompressed = QByteArray::fromRawData((const char*)plane + row * stride, stride);
        QByteArray compressed = Compression::compress(uncompressed, Compression::RLE);

        KisAslWriterUtils::OffsetStreamPusher<quint16> rleExternalTag(io, 0, channelRLESizePos + row * sizeof(quint16));

        if (io->write(compressed) != compressed.size()) {
            throw KisAslWriterUtils::ASLWriteException("Failed to write image data");
        }
    }
}

inline void preparePixelForWrite(quint8 *dataPlane,
                                 int numPixels,
                                 int channelSize,
                                 int channelId,
                                 psd_color_mode colorMode)
{
    // if the bitdepth > 8, place the bytes in the right order
    // if cmyk, invert the pixel value
    if (channelSize == 1) {
        if (channelId >= 0 && (colorMode == CMYK || colorMode == CMYK64)) {
            for (int i = 0; i < numPixels; ++i) {
                dataPlane[i] = 255 - dataPlane[i];
            }
        }
    }
    else if (channelSize == 2) {
        quint16 val;
        for (int i = 0; i < numPixels; ++i) {
            quint16 *pixelPtr = reinterpret_cast<quint16*>(dataPlane) + i;

            val = *pixelPtr;
            val = qFromBigEndian(val);
            if (channelId >= 0 && (colorMode == CMYK || colorMode == CMYK64)) {
                val = quint16_MAX - val;
            }
            *pixelPtr = val;
        }
    }
    else if (channelSize == 4) {
        quint32 val;
        for (int i = 0; i < numPixels; ++i) {
            quint32 *pixelPtr = reinterpret_cast<quint32*>(dataPlane) + i;

            val = *pixelPtr;
            val = qFromBigEndian(val);
            if (channelId >= 0 && (colorMode == CMYK || colorMode == CMYK64)) {
                val = quint16_MAX - val;
            }
            *pixelPtr = val;
        }
    }
}

void writePixelDataCommon(QIODevice *io,
                          KisPaintDeviceSP dev,
                          const QRect &rc,
                          psd_color_mode colorMode,
                          int channelSize,
                          bool alphaFirst,
                          const bool writeCompressionType,
                          QVector<ChannelWritingInfo> &writingInfoList)
{
    // Empty rects must be processed separately on a higher level!
    KIS_ASSERT_RECOVER_RETURN(!rc.isEmpty());

    QVector<quint8* > tmp = dev->readPlanarBytes(rc.x() - dev->x(), rc.y() - dev->y(), rc.width(), rc.height());
    const KoColorSpace *colorSpace = dev->colorSpace();

    QVector<quint8*> planes;

    { // prepare 'planes' array

        quint8 *alphaPlanePtr = 0;

        QList<KoChannelInfo*> origChannels = colorSpace->channels();
        Q_FOREACH (KoChannelInfo *ch, KoChannelInfo::displayOrderSorted(origChannels)) {
            int channelIndex = KoChannelInfo::displayPositionToChannelIndex(ch->displayPosition(), origChannels);

            quint8 *holder = 0;
            std::swap(holder, tmp[channelIndex]);

            if (ch->channelType() == KoChannelInfo::ALPHA) {
                std::swap(holder, alphaPlanePtr);
            } else {
                planes.append(holder);
            }
        }

        if (alphaPlanePtr) {
            if (alphaFirst) {
                planes.insert(0, alphaPlanePtr);
                KIS_ASSERT_RECOVER_NOOP(writingInfoList.first().channelId == -1);
            } else {
                planes.append(alphaPlanePtr);
                KIS_ASSERT_RECOVER_NOOP(
                    (writingInfoList.size() == planes.size() - 1) ||
                    (writingInfoList.last().channelId == -1));
            }
        }

        // now planes are holding pointers to quint8 arrays
        tmp.clear();
    }

    KIS_ASSERT_RECOVER_RETURN(planes.size() >= writingInfoList.size());

    const int numPixels = rc.width() * rc.height();

    // write down the planes

    try {
        for (int i = 0; i < writingInfoList.size(); i++) {
            const ChannelWritingInfo &info = writingInfoList[i];

            dbgFile << "\tWriting channel" << i << "psd channel id" << info.channelId;

            preparePixelForWrite(planes[i], numPixels, channelSize, info.channelId, colorMode);

            dbgFile << "\t\tchannel start" << ppVar(io->pos());

            writeChannelDataRLE(io, planes[i], channelSize, rc, info.sizeFieldOffset, info.rleBlockOffset, writeCompressionType);
        }

    } catch (KisAslWriterUtils::ASLWriteException &e) {
        qDeleteAll(planes);
        planes.clear();

        throw KisAslWriterUtils::ASLWriteException(PREPEND_METHOD(e.what()));
    }

    qDeleteAll(planes);
    planes.clear();
}

}
