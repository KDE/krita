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

// from gimp's psd-util.c
int decompress(const QByteArray &input, QByteArray &output, int unpacked_len)
{
    output.resize(unpacked_len);

    /*
     *  Decode a PackBits chunk.
     */
    qint32 n;
    const char *src = input.data();
    char *dst = output.data();
    char dat;
    int unpack_left = unpacked_len;
    int pack_left = input.size();
    qint32 error_code = 0;
    int return_val = 0;

    while (unpack_left > 0 && pack_left > 0) {
        n = *src;
        src++;
        pack_left--;

        if (n == 128) /* nop */
            continue;
        else if (n > 128)
            n -= 256;

        if (n < 0) /* replicate next gchar |n|+ 1 times */
        {
            n = 1 - n;
            if (!pack_left) {
                dbgFile << "Input buffer exhausted in replicate";
                error_code = 1;
                break;
            }
            if (n > unpack_left) {
                dbgFile << "Overrun in packbits replicate of" << n - unpack_left << "chars";
                error_code = 2;
            }
            dat = *src;
            for (; n > 0; --n) {
                if (!unpack_left)
                    break;
                *dst = dat;
                dst++;
                unpack_left--;
            }
            if (unpack_left) {
                src++;
                pack_left--;
            }
        } else /* copy next n+1 gchars literally */
        {
            n++;
            for (; n > 0; --n) {
                if (!pack_left) {
                    dbgFile << "Input buffer exhausted in copy";
                    error_code = 3;
                    break;
                }
                if (!unpack_left) {
                    dbgFile << "Output buffer exhausted in copy";
                    error_code = 4;
                    break;
                }
                *dst = *src;
                dst++;
                unpack_left--;
                src++;
                pack_left--;
            }
        }
    }

    if (unpack_left > 0) {
        /* Pad with zeros to end of output buffer */
        for (n = 0; n < pack_left; ++n) {
            *dst = 0;
            dst++;
        }
    }

    if (unpack_left) {
        dbgFile << "Packbits decode - unpack left" << unpack_left;
        return_val -= unpack_left;
    }
    if (pack_left) {
        /* Some images seem to have a pad byte at the end of the packed data */
        if (error_code || pack_left != 1) {
            dbgFile << "Packbits decode - pack left" << pack_left;
            return_val = pack_left;
        }
    }

    if (error_code)
        dbgFile << "Error code" << error_code;

    return return_val;
}

QByteArray decompress(const QByteArray &data, int expected_length)
{
    QByteArray output(expected_length, '\0');
    const int result = KisRLE::decompress(data, output, expected_length);
    if (result != 0)
        return QByteArray();
    else
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

QByteArray psd_unzip_with_prediction(const QByteArray &src, int dst_len, int row_size, int color_depth)
{
    int len;

    QByteArray dst_buf = Compression::uncompress(dst_len, src, psd_compression_type::ZIP);
    if (dst_buf.size() == 0)
        return dst_buf;

    char *buf = dst_buf.data();
    do {
        len = row_size;
        if (color_depth == 16) {
            while (--len) {
                buf[2] += buf[0] + ((buf[1] + buf[3]) >> 8);
                buf[3] += buf[1];
                buf += 2;
            }
            buf += 2;
            dst_len -= row_size * 2;
        } else {
            while (--len) {
                *(buf + 1) += *buf;
                buf++;
            }
            buf++;
            dst_len -= row_size;
        }
    } while (dst_len > 0);

    return dst_buf;
}

/**********************************************************************/
/* End of third party block                                           */
/**********************************************************************/

QByteArray psd_zip_with_prediction(const QByteArray &src, int row_size, int color_depth)
{
    QByteArray tempbuf(src);

    int len;
    int dst_len = src.size();

    char *buf = tempbuf.data();
    do {
        len = row_size;
        if (color_depth == 16) {
            while (--len) {
                buf[2] -= buf[0] + ((buf[1] + buf[3]) >> 8);
                buf[3] -= buf[1];
                buf += 2;
            }
            buf += 2;
            dst_len -= row_size * 2;
        } else {
            while (--len) {
                *(buf + 1) -= *buf;
                buf++;
            }
            buf++;
            dst_len -= row_size;
        }
    } while (dst_len > 0);

    return Compression::compress(tempbuf, psd_compression_type::ZIP);
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
