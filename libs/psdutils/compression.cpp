/*
 *  SPDX-FileCopyrightText: 2004-2007 Graphest Software <libpsd@graphest.com>
 *  SPDX-FileCopyrightText: 2007 John Marshall
 *  SPDX-FileCopyrightText: 2010 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "compression.h"

#include <QBuffer>
#include <QtEndian>
#include <algorithm>
#include <zlib.h>

#include <kis_debug.h>
#include <psd_utils.h>

namespace KisRLE
{
// from gimp's psd-save.c
int compress(const QByteArray &src, QByteArray &dst)
{
    int length = src.size();
    dst.resize(length * 2);
    dst.fill(0, length * 2);

    int remaining = length;
    quint8 i, j;
    quint32 dest_ptr = 0;
    const char *start = src.constData();

    length = 0;
    while (remaining > 0) {
        /* Look for characters matching the first */
        i = 0;
        while ((i < 128) && (remaining - i > 0) && (start[0] == start[i]))
            i++;

        if (i > 1) /* Match found */
        {
            dst[dest_ptr++] = static_cast<char>(-(i - 1));
            dst[dest_ptr++] = *start;

            start += i;
            remaining -= i;
            length += 2;
        } else { /* Look for characters different from the previous */
            i = 0;
            while ((i < 128) && (remaining - (i + 1) > 0) && (start[i] != start[(i + 1)] || remaining - (i + 2) <= 0 || start[i] != start[(i + 2)]))
                i++;

            /* If there's only 1 remaining, the previous WHILE stmt doesn't
             catch it */

            if (remaining == 1) {
                i = 1;
            }

            if (i > 0) /* Some distinct ones found */
            {
                dst[dest_ptr++] = static_cast<char>(i - 1U);
                for (j = 0; j < i; j++) {
                    dst[dest_ptr++] = start[j];
                }
                start += i;
                remaining -= i;
                length += i + 1;
            }
        }
    }
    dst.resize(length);
    return length;
}

QByteArray compress(const QByteArray &data)
{
    QByteArray output;
    const int result = KisRLE::compress(data, output);
    if (result <= 0)
        return QByteArray();
    else
        return output;
}

QByteArray decompress(const QByteArray &input, int unpacked_len)
{
    QByteArray output;
    output.resize(unpacked_len);

    const auto *src = input.cbegin();
    auto *dst = output.begin();

    while (src < input.end() && dst < output.end()) {
        // NOLINTNEXTLINE(*-reinterpret-cast,readability-identifier-length)
        const int8_t n = *reinterpret_cast<const int8_t *>(src);
        src += 1;

        if (n >= 0) { // copy next n+1 chars
            const int bytes = 1 + n;
            if (src + bytes > input.cend()) {
                errFile << "Input buffer exhausted in replicate of" << bytes << "chars, left" << (input.cend() - src);
                return {};
            }
            if (dst + bytes > output.end()) {
                errFile << "Overrun in packbits replicate of" << bytes << "chars, left" << (output.end() - dst);
                return {};
            }
            std::copy_n(src, bytes, dst);
            src += bytes;
            dst += bytes;
        } else if (n >= -127 && n <= -1) { // replicate next char -n+1 times
            const int bytes = 1 - n;
            if (src >= input.cend()) {
                errFile << "Input buffer exhausted in copy";
                return {};
            }
            if (dst + bytes > output.end()) {
                errFile << "Output buffer exhausted in copy of" << bytes << "chars, left" << (output.end() - dst);
                return {};
            }
            const auto byte = *src;
            std::fill_n(dst, bytes, byte);
            src += 1;
            dst += bytes;
        } else if (n == -128) {
            continue;
        }
    }

    if (dst < output.end()) {
        errFile << "Packbits decode - unpack left" << (output.end() - dst);
        std::fill(dst, output.end(), 0);
    }

    // If the input line was odd width, there's a padding byte
    if (src + 1 < input.cend()) {
        QByteArray leftovers;
        leftovers.resize(static_cast<int>(input.cend() - src));
        std::copy(src, input.cend(), leftovers.begin());
        errFile << "Packbits decode - pack left" << leftovers.size() << leftovers.toHex();
    }

    return output;
}
} // namespace KisRLE

namespace KisZip
{
// Based on the reverse of psd_unzip_without_prediction
int compress(const char *input, int unpacked_len, char *dst, int maxout)
{
    z_stream stream{};
    int state;

    stream.data_type = Z_BINARY;
    stream.next_in = const_cast<Bytef *>(reinterpret_cast<const Bytef *>(input));
    stream.avail_in = static_cast<uInt>(unpacked_len);
    stream.next_out = reinterpret_cast<Bytef *>(dst);
    stream.avail_out = static_cast<uInt>(maxout);

    dbgFile << "Expected unpacked length:" << unpacked_len << ", maxout:" << maxout;

    if (deflateInit(&stream, -1) != Z_OK) {
        dbgFile << "Failed deflate initialization";
        return 0;
    }

    int flush = Z_PARTIAL_FLUSH;

    do {
        state = deflate(&stream, flush);
        if (state == Z_STREAM_END) {
            dbgFile << "Finished deflating";
            flush = Z_FINISH;
        } else if (state != Z_OK) {
            dbgFile << "Error deflating" << state << stream.msg;
            break;
        }
    } while (stream.avail_in > 0);

    if (state != Z_OK || stream.avail_in > 0) {
        dbgFile << "Failed deflating" << state << stream.msg;
        return 0;
    }

    dbgFile << "Success, deflated size:" << stream.total_out;

    return static_cast<int>(stream.total_out);
}

QByteArray compress(const QByteArray &data)
{
    QByteArray output(data.length() * 4, '\0');
    const int result = KisZip::compress(data.constData(), data.size(), output.data(), output.size());
    output.resize(result);
    return output;
}

/**********************************************************************/
/* Two functions copied from the abandoned PSDParse library (GPL)     */
/* See: http://www.telegraphics.com.au/svn/psdparse/trunk/psd_zip.c   */
/* Created by Patrick in 2007.02.02, libpsd@graphest.com              */
/* Modifications by Toby Thain <toby@telegraphics.com.au>             */
/* Refactored by L. E. Segovia <amy@amyspark.me>, 2021.06.30          */
/**********************************************************************/
int psd_unzip_without_prediction(const char *src, int packed_len, char *dst, int unpacked_len)
{
    z_stream stream{};
    int state;

    stream.data_type = Z_BINARY;
    stream.next_in = const_cast<Bytef *>(reinterpret_cast<const Bytef *>(src));
    stream.avail_in = static_cast<uInt>(packed_len);
    stream.next_out = reinterpret_cast<Bytef *>(dst);
    stream.avail_out = static_cast<uInt>(unpacked_len);

    if (inflateInit(&stream) != Z_OK)
        return 0;

    int flush = Z_PARTIAL_FLUSH;

    do {
        state = inflate(&stream, flush);
        if (state == Z_STREAM_END) {
            dbgFile << "Finished inflating";
            break;
        } else if (state == Z_DATA_ERROR) {
            dbgFile << "Error inflating" << state << stream.msg;
            if (inflateSync(&stream) != Z_OK)
                return 0;
            continue;
        }
    } while (stream.avail_out > 0);

    if ((state != Z_STREAM_END && state != Z_OK) || stream.avail_out > 0) {
        dbgFile << "Failed inflating" << state << stream.msg;
        return 0;
    }

    return static_cast<int>(stream.total_out);
}

template<typename T>
inline void psd_unzip_with_prediction(QByteArray &dst_buf, int row_size);

template<>
inline void psd_unzip_with_prediction<uint8_t>(QByteArray &dst_buf, const int row_size)
{
    auto *buf = reinterpret_cast<uint8_t *>(dst_buf.data());
    int len = 0;
    int dst_len = dst_buf.size();

    while (dst_len > 0) {
        len = row_size;
        while (--len) {
            *(buf + 1) += *buf;
            buf++;
        }
        buf++;
        dst_len -= row_size;
    }
}

template<>
inline void psd_unzip_with_prediction<uint16_t>(QByteArray &dst_buf, const int row_size)
{
    auto *buf = reinterpret_cast<uint8_t *>(dst_buf.data());
    int len = 0;
    int dst_len = dst_buf.size();

    while (dst_len > 0) {
        len = row_size;
        while (--len) {
            buf[2] += buf[0] + ((buf[1] + buf[3]) >> 8);
            buf[3] += buf[1];
            buf += 2;
        }
        buf += 2;
        dst_len -= row_size * 2;
    }
}

QByteArray psd_unzip_with_prediction(const QByteArray &src, int dst_len, int row_size, int color_depth)
{
    QByteArray dst_buf = Compression::uncompress(dst_len, src, psd_compression_type::ZIP);

    if (dst_buf.size() == 0)
        return {};

    if (color_depth == 32) {
        // Placeholded for future implementation.
        errKrita << "Unsupported bit depth for prediction";
        return {};
    } else if (color_depth == 16) {
        psd_unzip_with_prediction<quint16>(dst_buf, row_size);
    } else {
        psd_unzip_with_prediction<quint8>(dst_buf, row_size);
    }

    return dst_buf;
}

/**********************************************************************/
/* End of third party block                                           */
/**********************************************************************/

template<typename T>
inline void psd_zip_with_prediction(QByteArray &dst_buf, int row_size);

template<>
inline void psd_zip_with_prediction<uint8_t>(QByteArray &dst_buf, const int row_size)
{
    auto *buf = reinterpret_cast<uint8_t *>(dst_buf.data());
    int len = 0;
    int dst_len = dst_buf.size();

    while (dst_len > 0) {
        len = row_size;
        while (--len) {
            *(buf + 1) -= *buf;
            buf++;
        }
        buf++;
        dst_len -= row_size;
    }
}

template<>
inline void psd_zip_with_prediction<uint16_t>(QByteArray &dst_buf, const int row_size)
{
    auto *buf = reinterpret_cast<uint8_t *>(dst_buf.data());
    int len = 0;
    int dst_len = dst_buf.size();

    while (dst_len > 0) {
        len = row_size;
        while (--len) {
            buf[2] -= buf[0] + ((buf[1] + buf[3]) >> 8);
            buf[3] -= buf[1];
            buf += 2;
        }
        buf += 2;
        dst_len -= row_size * 2;
    }
}

QByteArray psd_zip_with_prediction(const QByteArray &src, int row_size, int color_depth)
{
    QByteArray dst_buf(src);
    if (color_depth == 32) {
        // Placeholded for future implementation.
        errKrita << "Unsupported bit depth for prediction";
        return {};
    } else if (color_depth == 16) {
        psd_zip_with_prediction<quint16>(dst_buf, row_size);
    } else {
        psd_zip_with_prediction<quint8>(dst_buf, row_size);
    }

    return Compression::compress(dst_buf, psd_compression_type::ZIP);
}

QByteArray decompress(const QByteArray &data, int expected_length)
{
    QByteArray output(expected_length, '\0');
    const int result = psd_unzip_without_prediction(data.constData(), data.size(), output.data(), expected_length);
    if (result == 0)
        return QByteArray();
    else
        return output;
}
} // namespace KisZip

QByteArray Compression::uncompress(int unpacked_len, QByteArray bytes, psd_compression_type compressionType, int row_size, int color_depth)
{
    if (bytes.size() < 1)
        return QByteArray();

    switch (compressionType) {
    case Uncompressed:
        return bytes;
    case RLE:
        return KisRLE::decompress(bytes, unpacked_len);
    case ZIP:
        return KisZip::decompress(bytes, unpacked_len);
    case ZIPWithPrediction:
        return KisZip::psd_unzip_with_prediction(bytes, unpacked_len, row_size, color_depth);
    default:
        qFatal("Cannot uncompress layer data: invalid compression type");
    }

    return QByteArray();
}

QByteArray Compression::compress(QByteArray bytes, psd_compression_type compressionType, int row_size, int color_depth)
{
    if (bytes.size() < 1)
        return QByteArray();

    switch (compressionType) {
    case Uncompressed:
        return bytes;
    case RLE:
        return KisRLE::compress(bytes);
    case ZIP:
        return KisZip::compress(bytes);
    case ZIPWithPrediction:
        return KisZip::psd_zip_with_prediction(bytes, row_size, color_depth);
    default:
        qFatal("Cannot compress layer data: invalid compression type");
    }

    return QByteArray();
}
