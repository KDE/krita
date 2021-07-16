/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_ASL_WRITER_H
#define __KIS_ASL_WRITER_H

#include "kritapsdutils_export.h"
#include "psd.h"

class QDomDocument;
class QIODevice;

class KRITAPSDUTILS_EXPORT KisAslWriter
{
public:
    KisAslWriter(psd_byte_order byteOrder = psd_byte_order::psdBigEndian);

    void writeFile(QIODevice &device, const QDomDocument &doc);
    void writePsdLfx2SectionEx(QIODevice &device, const QDomDocument &doc);

private:
    psd_byte_order m_byteOrder;
};

#endif /* __KIS_ASL_WRITER_H */
