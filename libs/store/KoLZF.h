/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2005-2006 Ariya Hidayat <ariya@kde.org>
   SPDX-FileCopyrightText: 2015 Friedrich W. H. Kossebau <kossebau@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
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
