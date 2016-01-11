/*
 * Copyright (C) 2011 Lukáš Tvrdý <lukas.tvrdy@ixonos.com>
 * Copyright (C) 2011 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KO_TABLE_OF_CONTENTS_GENERATOR_INFO
#define KO_TABLE_OF_CONTENTS_GENERATOR_INFO

#define ppVar( var ) #var << "=" << var
//#define DEBUG_TOC_STRUCTURE

#include <QList>
#include <QString>

#include "ToCBibGeneratorInfo.h"

class KoTextSharedLoadingData;
class KoXmlWriter;

class KRITATEXT_EXPORT KoTableOfContentsGeneratorInfo
{
public:
    explicit KoTableOfContentsGeneratorInfo(bool generateEntryTemplate = true);
    ~KoTableOfContentsGeneratorInfo();
    void loadOdf(KoTextSharedLoadingData *sharedLoadingData, const KoXmlElement &element);
    void saveOdf(KoXmlWriter *writer) const;
    KoTableOfContentsGeneratorInfo *clone();

    QString m_name;
    QString m_styleName;
    // TODO: add support for those according ODF v1.2
    // text: protected
    // text: protection-key
    // text:protection-key-digest-algorithm
    // xml:id
    QString m_indexScope; // enum {document, chapter}
    int m_outlineLevel;
    bool m_relativeTabStopPosition;
    bool m_useIndexMarks;
    bool m_useIndexSourceStyles;
    bool m_useOutlineLevel;

    IndexTitleTemplate m_indexTitleTemplate;
    QList<TocEntryTemplate> m_entryTemplate; // N-entries
    QList<IndexSourceStyles> m_indexSourceStyles;

private:
    int styleNameToStyleId(KoTextSharedLoadingData *sharedLoadingData, const QString &styleName);
};

Q_DECLARE_METATYPE(KoTableOfContentsGeneratorInfo *)

#endif
