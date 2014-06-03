/*
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
#ifndef COMPRESSION_H
#define COMPRESSION_H

#include <QByteArray>

class Compression
{
public:

    enum CompressionType {
        Uncompressed = 0,
        RLE,
        ZIP,
        ZIPWithPrediction,
        Unknown
    };

    static QByteArray uncompress(quint32 unpacked_len, QByteArray bytes, CompressionType compressionType);
    static QByteArray compress(QByteArray bytes, CompressionType compressionType);
};

#endif // PSD_COMPRESSION_H
