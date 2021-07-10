/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_ASL_READER_H
#define __KIS_ASL_READER_H

#include "kritapsdutils_export.h"
#include <QtGlobal>

class QDomDocument;
class QIODevice;

class KRITAPSDUTILS_EXPORT KisAslReader
{
public:
    QDomDocument readFile(QIODevice &device);
    QDomDocument readLfx2PsdSection(QIODevice &device);

    static QDomDocument readPsdSectionPattern(QIODevice &device, qint64 bytesLeft);
};

#endif /* __KIS_ASL_READER_H */
