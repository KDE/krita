/* This file is part of the KDE project
 * Copyright (C) 2011 Smit Patel <smitpatel24@gmail.com>
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
#ifndef TOCBIBGENERATORINFO_H
#define TOCBIBGENERATORINFO_H

#include <KoText.h>

const int INVALID_OUTLINE_LEVEL = 0;

class KoXmlWriter;

class KRITATEXT_EXPORT IndexEntry
{
public:
    enum IndexEntryName {UNKNOWN, LINK_START, CHAPTER, SPAN, TEXT, TAB_STOP, PAGE_NUMBER, LINK_END, BIBLIOGRAPHY};

    explicit IndexEntry(const QString &_styleName, IndexEntryName _name = IndexEntry::UNKNOWN);
    virtual IndexEntry *clone();
    virtual ~IndexEntry();
    virtual void addAttributes(KoXmlWriter * writer) const;
    void saveOdf(KoXmlWriter * writer) const;

    QString styleName;
    IndexEntryName name;
};


class IndexEntryLinkStart : public IndexEntry
{
public:
    explicit IndexEntryLinkStart(const QString &_styleName);
    IndexEntry *clone();
};


class IndexEntryChapter : public IndexEntry
{
public:
    explicit IndexEntryChapter(const QString &_styleName);
    IndexEntry *clone();
    virtual void addAttributes(KoXmlWriter* writer) const;

    QString display;
    int outlineLevel;
};


class  KRITATEXT_EXPORT IndexEntrySpan : public IndexEntry
{
public:
    explicit IndexEntrySpan(const QString &_styleName);
    IndexEntry *clone();
    virtual void addAttributes(KoXmlWriter* writer) const;

    QString text;
};


class IndexEntryText : public IndexEntry
{
public:
    explicit IndexEntryText(const QString &_styleName);
    IndexEntry *clone();
};


class KRITATEXT_EXPORT IndexEntryTabStop : public IndexEntry
{
public:
    explicit IndexEntryTabStop(const QString &_styleName);
    IndexEntry *clone();
    virtual void addAttributes(KoXmlWriter* writer) const;
    // for saving let's save the original unit,
    // for KoText::Tab we need to convert to PostScript points
    void setPosition(const QString &position);

    KoText::Tab tab;
    QString m_position;
};


class IndexEntryPageNumber : public IndexEntry
{
public:
    explicit IndexEntryPageNumber(const QString &_styleName);
    IndexEntry *clone();
};


class IndexEntryLinkEnd : public IndexEntry
{
public:
    explicit IndexEntryLinkEnd(const QString &_styleName);
    IndexEntry *clone();
};

class KRITATEXT_EXPORT TocEntryTemplate
{
public:
    TocEntryTemplate();
    TocEntryTemplate(const TocEntryTemplate &other);
    void saveOdf(KoXmlWriter * writer) const;

    int outlineLevel;
    QString styleName;
    int styleId;
    QList<IndexEntry*> indexEntries;
};


class KRITATEXT_EXPORT IndexTitleTemplate
{
public:
    void saveOdf(KoXmlWriter * writer) const;

    QString styleName;
    int styleId;
    QString text;
};


class KRITATEXT_EXPORT IndexSourceStyle
{
public:
    IndexSourceStyle(const IndexSourceStyle& indexSourceStyle);
    IndexSourceStyle();
    void saveOdf(KoXmlWriter * writer) const;

    QString styleName;
    int styleId;
};


class KRITATEXT_EXPORT IndexSourceStyles
{
public:
    IndexSourceStyles();
    IndexSourceStyles(const IndexSourceStyles &indexSourceStyles);
    void saveOdf(KoXmlWriter * writer) const;

    int outlineLevel;
    QList<IndexSourceStyle> styles;
};

class KRITATEXT_EXPORT IndexEntryBibliography : public IndexEntry
{
public:
    explicit IndexEntryBibliography(const QString &_styleName);
    IndexEntry *clone();
    virtual void addAttributes(KoXmlWriter* writer) const;

    QString dataField;
};

class KRITATEXT_EXPORT BibliographyEntryTemplate
{
public:
    BibliographyEntryTemplate();
    BibliographyEntryTemplate(const BibliographyEntryTemplate &other);
    void saveOdf(KoXmlWriter * writer) const;

    QString styleName;
    int styleId;
    QList<IndexEntry*> indexEntries;
    QString bibliographyType;
};

Q_DECLARE_METATYPE(IndexEntry::IndexEntryName)
#endif // TOCBIBGENERATORINFO_H
