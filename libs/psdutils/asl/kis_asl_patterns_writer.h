/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_ASL_PATTERNS_WRITER_H
#define __KIS_ASL_PATTERNS_WRITER_H

#include "kritapsdutils_export.h"

class QDomDocument;
class QIODevice;

#include <KoPattern.h>

#include "psd.h"

class KRITAPSDUTILS_EXPORT KisAslPatternsWriter
{
public:
    KisAslPatternsWriter(const QDomDocument &doc, QIODevice &device, psd_byte_order byteOrder);

    void writePatterns();

private:
    void addPattern(const KoPatternSP pattern);

    template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
    void addPatternImpl(const KoPatternSP pattern);

private:
    const QDomDocument &m_doc;
    QIODevice &m_device;

    int m_numPatternsWritten;
    psd_byte_order m_byteOrder;
};

#endif /* __KIS_ASL_PATTERNS_WRITER_H */
