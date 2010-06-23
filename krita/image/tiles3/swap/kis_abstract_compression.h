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

#ifndef __KIS_ABSTRACT_COMPRESSION_H
#define __KIS_ABSTRACT_COMPRESSION_H

#include "krita_export.h"

/**
 * Base class for compression operations
 */

class KRITAIMAGE_EXPORT KisAbstractCompression
{
public:
    KisAbstractCompression();
    virtual ~KisAbstractCompression();

    /**
     * Compresses \a input buffer into \a outbut buffer.
     * \return false if output buffer is too short, otherwise true.
     */
    virtual bool compress(quint8* input, qint32 inputLength, quint8* output, qint32 outputLength) = 0;

    /**
     * Decompresses \a input buffer into \a outbut buffer.
     * \return false if output buffer is too short, otherwise true.
     */
    virtual bool decompress(quint8* input, qint32 inputLength, quint8* output, qint32 outputLength) = 0;

    /**
     * Some algorithms may decide to optimize them work depending on
     * the usual size of the data.
     * Default implementation of KisAbstractCompression class does nothing.
     */
    virtual void adjustForDataSize(qint32 dataSize);
};

#endif /* __KIS_ABSTRACT_COMPRESSION_H */

