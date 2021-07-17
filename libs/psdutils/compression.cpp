/*
 *  SPDX-FileCopyrightText: 2007 John Marshall
 *  SPDX-FileCopyrightText: 2010 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "compression.h"

#include "kis_debug.h"
#include "psd_utils.h"
#include <QBuffer>
#include <QtEndian>

// from gimp's psd-save.c
static quint32 pack_pb_line(const QByteArray &src, QByteArray &dst)
{
    quint32 length = src.size();
    dst.resize(length * 2);
    dst.fill(0, length * 2);

    quint32 remaining = length;
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
            dst[dest_ptr++] = -(i - 1);
            dst[dest_ptr++] = *start;

            start += i;
            remaining -= i;
            length += 2;
        } else /* Look for characters different from the previous */
        {
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
                dst[dest_ptr++] = i - 1;
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

// from gimp's psd-util.c
quint32 decode_packbits(const char *src, char *dst, quint16 packed_len, quint32 unpacked_len)
{
    /*
     *  Decode a PackBits chunk.
     */
    qint32 n;
    char dat;
    qint32 unpack_left = unpacked_len;
    qint32 pack_left = packed_len;
    qint32 error_code = 0;
    qint32 return_val = 0;

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

QByteArray Compression::uncompress(quint32 unpacked_len, QByteArray bytes, Compression::CompressionType compressionType)
{
    if (unpacked_len > 30000)
        return QByteArray();
    if (bytes.size() < 1)
        return QByteArray();

    switch (compressionType) {
    case Uncompressed:
        return bytes;
    case RLE: {
        QByteArray ba;
        ba.resize(static_cast<int>(unpacked_len));
        decode_packbits(bytes.constData(), ba.data(), static_cast<quint16>(bytes.length()), unpacked_len);
        return ba;
    }
    case ZIP:
    case ZIPWithPrediction: {
        // prepend the expected length of the pixels in big-endian
        // format to the byte array as qUncompress expects...

        QByteArray b;
        QBuffer buf(&b);
        quint32 val = qFromBigEndian(unpacked_len);
        buf.write((char *)&val, 4);
        b.append(bytes);

        // and let's hope that this is sufficient...
        return qUncompress(b);
    }
    default:
        qFatal("Cannot uncompress layer data: invalid compression type");
    }

    return QByteArray();
}

QByteArray Compression::compress(QByteArray bytes, Compression::CompressionType compressionType)
{
    if (bytes.size() < 1)
        return QByteArray();

    switch (compressionType) {
    case Uncompressed:
        return bytes;
    case RLE: {
        QByteArray dst;
        int packed_len = pack_pb_line(bytes, dst);
        Q_ASSERT(packed_len == dst.size());
        Q_UNUSED(packed_len);
        return dst;
    }
    case ZIP:
    case ZIPWithPrediction:
        return qCompress(bytes);
    default:
        qFatal("Cannot compress layer data: invalid compression type");
    }

    return QByteArray();
}
