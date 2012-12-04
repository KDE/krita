/*
 *  Copyright (c) 2007 by John Marshall
 *  Copyright (c) 2010 Boudewijn Rempt <boud@valdyas.org>
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
#include "compression.h"

#include <QBuffer>
#include "psd_utils.h"
#include "kis_debug.h"
#include <netinet/in.h> // htonl

// from gimp's psd-save.c
static quint32 pack_pb_line (const QByteArray &src,
                             QByteArray &dst)
{
    quint32 length = src.size();
    quint32 remaining = length;
    quint8  i, j;
    quint32 dest_ptr = 0;
    const char *start = src.constData();

    length = 0;
    while (remaining > 0)
    {
        /* Look for characters matching the first */
        i = 0;
        while ((i < 128) &&
               (remaining - i > 0) &&
               (start[0] == start[i]) )
            i++;

        if (i > 1)              /* Match found */
        {

            dst[dest_ptr++] = -(i - 1);
            dst[dest_ptr++] = *start;

            start += i;
            remaining -= i;
            length += 2;
        }
        else       /* Look for characters different from the previous */
        {
            i = 0;
            while ((i < 128)             &&
                   (remaining - (i + 1) > 0) &&
                   (start[i] != start[(i + 1)] ||
                    remaining - (i + 2) <= 0  || start[i] != start[(i+2)]))
                i++;

            /* If there's only 1 remaining, the previous WHILE stmt doesn't
             catch it */

            if (remaining == 1)
            {
                i = 1;
            }

            if (i > 0)               /* Some distinct ones found */
            {
                dst[dest_ptr++] = i - 1;
                for (j = 0; j < i; j++)
                {
                    dst[dest_ptr++] = start[j];
                }
                start += i;
                remaining -= i;
                length += i + 1;
            }

        }
    }
    return length;
}


// from gimp's psd-util.c
quint32 decode_packbits(const char *src, char* dst, quint16 packed_len, quint32 unpacked_len)
{
    /*
     *  Decode a PackBits chunk.
     */
    qint32    n;
    char      dat;
    qint32    unpack_left = unpacked_len;
    qint32    pack_left = packed_len;
    qint32    error_code = 0;
    qint32    return_val = 0;

    while (unpack_left > 0 && pack_left > 0)
    {
        n = *src;
        src++;
        pack_left--;

        if (n == 128)     /* nop */
            continue;
        else if (n > 128)
            n -= 256;

        if (n < 0)        /* replicate next gchar |n|+ 1 times */
        {
            n  = 1 - n;
            if (! pack_left)
            {
                dbgFile << "Input buffer exhausted in replicate";
                error_code = 1;
                break;
            }
            if (n > unpack_left)
            {
                dbgFile << "Overrun in packbits replicate of" << n - unpack_left << "chars";
                error_code = 2;
            }
            dat = *src;
            for (; n > 0; --n)
            {
                if (! unpack_left)
                    break;
                *dst = dat;
                dst++;
                unpack_left--;
            }
            if (unpack_left)
            {
                src++;
                pack_left--;
            }
        }
        else              /* copy next n+1 gchars literally */
        {
            n++;
            for (; n > 0; --n)
            {
                if (! pack_left)
                {
                    dbgFile << "Input buffer exhausted in copy";
                    error_code = 3;
                    break;
                }
                if (! unpack_left)
                {
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

    if (unpack_left > 0)
    {
        /* Pad with zeros to end of output buffer */
        for (n = 0; n < pack_left; ++n)
        {
            *dst = 0;
            dst++;
        }
    }

    if (unpack_left)
    {
        dbgFile << "Packbits decode - unpack left" << unpack_left;
        return_val -= unpack_left;
    }
    if (pack_left)
    {
        /* Some images seem to have a pad byte at the end of the packed data */
        if (error_code || pack_left != 1)
        {
            dbgFile << "Packbits decode - pack left" << pack_left;
            return_val = pack_left;
        }
    }

    if (error_code)
        dbgFile << "Error code" <<  error_code;

    return return_val;
}

QByteArray Compression::uncompress(quint32 unpacked_len, QByteArray bytes, Compression::CompressionType compressionType)
{
    if (unpacked_len > 30000) return QByteArray();
    if (bytes.size() < 1) return QByteArray();


    switch(compressionType) {
    case Uncompressed:
        return bytes;
    case RLE:
    {
        char *dst = new char[unpacked_len];
        decode_packbits(bytes.constData(), dst, bytes.length(), unpacked_len);
        QByteArray ba(dst, unpacked_len);
        delete[] dst;
        return ba;
     }
    case ZIP:
    case ZIPWithPrediction:
    {
        // prepend the expected length of the pixels in big-endian
        // format to the byte array as qUncompress expects...

        QByteArray b;
        QBuffer buf(&b);
        quint32 val = ntohl(unpacked_len);
        buf.write((char*)&val, 4);
        b.append(bytes);

        // and let's hope that this is sufficient...
        return qUncompress(bytes);
    }
    default:
        qFatal("Cannot uncompress layer data: invalid compression type");
    }


    return QByteArray();
}

QByteArray Compression::compress(QByteArray bytes, Compression::CompressionType compressionType)
{
    if (bytes.size() < 1) return QByteArray();

    switch(compressionType) {
    case Uncompressed:
        return bytes;
    case RLE:
    {
        QByteArray dst;
        int packed_len = pack_pb_line(bytes, dst);
        Q_ASSERT(packed_len == dst.size());
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

