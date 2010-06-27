/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_LZF_COMPRESSION_H
#define __KIS_LZF_COMPRESSION_H

#include "kis_abstract_compression.h"

class KRITAIMAGE_EXPORT KisLzfCompression : public KisAbstractCompression
{
public:
    KisLzfCompression();
    virtual ~KisLzfCompression();

    qint32 compress(const quint8* input, qint32 inputLength, quint8* output, qint32 outputLength);
    qint32 decompress(const quint8* input, qint32 inputLength, quint8* output, qint32 outputLength);

    qint32 outputBufferSize(qint32 dataSize);

    //void adjustForDataSize(qint32 dataSize);
};

#endif /* __KIS_LZF_COMPRESSION_H */

