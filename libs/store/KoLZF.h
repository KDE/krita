/* This file is part of the KDE project
   Copyright (C) 2005-2006 Ariya Hidayat <ariya@kde.org>
   Copyright (C) 2015 Friedrich W. H. Kossebau <kossebau@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KO_LZF_H
#define KO_LZF_H

class QByteArray;

namespace KoLZF
{

/**
 * @param input where to read the data to compress from
 * @param length length of the input
 * @param output where to write the compressed data to
 * @param maxout maximal usable length of output, needs to be at least 2 bytes
 * @return the length of data written to output, or, on failure, 0
 */
int compress(const void* input, int length, void* output, int maxout);

/**
 * @param input where to read the data to decompress from
 * @param length length of the input
 * @param output where to write the decompressed data to
 * @param maxout maximal usable length of output
 * @return the length of data written to output, or, on failure, 0
 */
int decompress(const void* input, int length, void* output, int maxout);

/**
 * @param data the data to compress
 * @return the compressed data (with KoLZF header)
 */
QByteArray compress(const QByteArray& data);

/**
 * @param data the data to decompress (with KoLZF header)
 * @param output where to write the decompressed data to
 *               Existing content will be lost.
 *               On failure will be an empty QByteArray.
 */
void decompress(const QByteArray &data, QByteArray &output);

}

#endif
