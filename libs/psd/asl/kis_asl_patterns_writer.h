/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_ASL_PATTERNS_WRITER_H
#define __KIS_ASL_PATTERNS_WRITER_H

#include "kritapsd_export.h"

class QDomDocument;
class QIODevice;

#include <KoPattern.h>

class KRITAPSD_EXPORT KisAslPatternsWriter
{
public:
    KisAslPatternsWriter(const QDomDocument &doc, QIODevice *device);

    void writePatterns();

private:
    void addPattern(const KoPatternSP pattern);

private:
    const QDomDocument &m_doc;
    QIODevice *m_device;

    int m_numPatternsWritten;
};

#endif /* __KIS_ASL_PATTERNS_WRITER_H */
