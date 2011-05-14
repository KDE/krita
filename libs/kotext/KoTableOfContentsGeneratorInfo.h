/*
 * Copyright (C) 2011 Lukáš Tvrdý <lukas.tvrdy@ixonos.com>
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

#include <QDebug>
#include <QList>
#include <QString>
#include <QVariant>

#include <KoXmlReader.h>
#include "KoText.h"
#include <KoXmlWriter.h>

class KoTextSharedLoadingData;

const int INVALID_OUTLINE_LEVEL = 0;

class IndexEntry
{

public:
    enum IndexEntryName {UNKNOWN, LINK_START, CHAPTER, SPAN, TEXT, TAB_STOP, PAGE_NUMBER, LINK_END};

    IndexEntry(QString _styleName, IndexEntryName _name = IndexEntry::UNKNOWN);
    virtual ~IndexEntry();
    virtual void addAttributes(KoXmlWriter * writer) const;
    void saveOdf(KoXmlWriter * writer) const;

    QString styleName;
    IndexEntryName name;
};


class IndexEntryLinkStart : public IndexEntry
{

public:
    IndexEntryLinkStart(QString _styleName);

};


class IndexEntryChapter : public IndexEntry
{

public:
    IndexEntryChapter(QString _styleName);
    virtual void addAttributes(KoXmlWriter* writer) const;

    QString display;
    int outlineLevel;
};


class IndexEntrySpan : public IndexEntry
{

public:
    IndexEntrySpan(QString _styleName);
    virtual void addAttributes(KoXmlWriter* writer) const;

    QString text;
};


class IndexEntryText : public IndexEntry
{

public:
    IndexEntryText(QString _styleName);
};


class IndexEntryTabStop : public IndexEntry
{

public:
    IndexEntryTabStop(QString _styleName);
    virtual void addAttributes(KoXmlWriter* writer) const;
    // for saving let's save the original unit,
    // for KoText::Tab we need to covert to PostScript points
    void setPosition(const QString &position);

    KoText::Tab tab;
private:
    QString m_position;
};


class IndexEntryPageNumber : public IndexEntry
{

public:
    IndexEntryPageNumber(QString _styleName);
};


class IndexEntryLinkEnd : public IndexEntry
{

public:
    IndexEntryLinkEnd(QString _styleName);
};


class TocEntryTemplate
{

public:
    void saveOdf(KoXmlWriter * writer) const;

    int outlineLevel;
    QString styleName;
    int styleId;
    QList<IndexEntry*> indexEntries;
};


class IndexTitleTemplate
{

public:
    void saveOdf(KoXmlWriter * writer) const;

    QString styleName;
    int styleId;
    QString text;
};


class IndexSourceStyle
{

public:
    void saveOdf(KoXmlWriter * writer) const;

    QString styleName;
    int styleId;
};


class IndexSourceStyles
{

public:
    void saveOdf(KoXmlWriter * writer) const;

    int outlineLevel;
    QList<IndexSourceStyle> styles;
};

class KOTEXT_EXPORT KoTableOfContentsGeneratorInfo
{

public:
    KoTableOfContentsGeneratorInfo();
    ~KoTableOfContentsGeneratorInfo();
    void loadOdf(KoTextSharedLoadingData *sharedLoadingData, const KoXmlElement &element);
    void saveOdf(KoXmlWriter *writer) const;

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
    int styleNameToStyleId(KoTextSharedLoadingData *sharedLoadingData, QString styleName);

};

Q_DECLARE_METATYPE(KoTableOfContentsGeneratorInfo *)

#endif
