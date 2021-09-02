/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_ASL_READER_H
#define __KIS_ASL_READER_H

#include "kritapsdutils_export.h"
#include "psd.h"
#include <QtGlobal>

class QDomDocument;
class QIODevice;

class KRITAPSDUTILS_EXPORT KisAslReader
{
public:
    QDomDocument readFile(QIODevice &device);

    static QDomDocument readLfx2PsdSection(QIODevice &device, psd_byte_order byteOrder = psd_byte_order::psdBigEndian);
    static QDomDocument readPsdSectionPattern(QIODevice &device, qint64 bytesLeft, psd_byte_order byteOrder = psd_byte_order::psdBigEndian);
};

#endif /* __KIS_ASL_READER_H */
