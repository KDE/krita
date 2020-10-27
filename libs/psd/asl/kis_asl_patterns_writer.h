/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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
