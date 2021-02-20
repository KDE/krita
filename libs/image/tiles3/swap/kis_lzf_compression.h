/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_LZF_COMPRESSION_H
#define __KIS_LZF_COMPRESSION_H

#include "kis_abstract_compression.h"

class KRITAIMAGE_EXPORT KisLzfCompression : public KisAbstractCompression
{
public:
    KisLzfCompression();
    ~KisLzfCompression() override;

    qint32 compress(const quint8* input, qint32 inputLength, quint8* output, qint32 outputLength) override;
    qint32 decompress(const quint8* input, qint32 inputLength, quint8* output, qint32 outputLength) override;

    qint32 outputBufferSize(qint32 dataSize) override;

    //void adjustForDataSize(qint32 dataSize);
};

#endif /* __KIS_LZF_COMPRESSION_H */

