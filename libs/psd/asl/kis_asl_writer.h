/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_ASL_WRITER_H
#define __KIS_ASL_WRITER_H

#include "kritapsd_export.h"

class QDomDocument;
class QIODevice;


class KRITAPSD_EXPORT KisAslWriter
{
public:
    void writeFile(QIODevice *device, const QDomDocument &doc);
    void writePsdLfx2SectionEx(QIODevice *device, const QDomDocument &doc);
};

#endif /* __KIS_ASL_WRITER_H */
