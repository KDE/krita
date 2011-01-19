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

const int INVALID_OUTLINE_LEVEL = 0;

// DATA structure that holds the information

class IndexEntry {

public:
    enum IndexEntryName {UNKNOWN, LINK_START, CHAPTER, SPAN, TEXT, TAB_STOP, PAGE_NUMBER, LINK_END};
    IndexEntry(QString _styleName, IndexEntryName _name = IndexEntry::UNKNOWN)
        :   styleName(_styleName),
            name(_name)
    {

    }

    virtual ~IndexEntry(){}

    QString styleName;
    IndexEntryName name;

    virtual void addAttributes(KoXmlWriter * writer) const{
        Q_UNUSED(writer);
    }

    void saveOdf(KoXmlWriter * writer) const{
        switch (name){
            case LINK_START     : writer->startElement("text:index-entry-link-start"); break;
            case CHAPTER        : writer->startElement("text:index-entry-chapter"); break;
            case SPAN           : writer->startElement("text:index-entry-span"); break;
            case TEXT           : writer->startElement("text:index-entry-text");  break;
            case TAB_STOP       : writer->startElement("text:index-entry-tab-stop"); break;
            case PAGE_NUMBER    : writer->startElement("text:index-entry-page-number"); break;
            case LINK_END       : writer->startElement("text:index-entry-link-end"); break;
            case UNKNOWN        : break;
        }

        if (!styleName.isNull()){
            writer->addAttribute("text:style-name", styleName);
        }
        addAttributes(writer);
        writer->endElement();
    }


};

class IndexEntryLinkStart : public IndexEntry {

public:
    IndexEntryLinkStart(QString _styleName)
    :IndexEntry(_styleName, IndexEntry::LINK_START)
    {}
};

class IndexEntryChapter : public IndexEntry {

public:
    IndexEntryChapter(QString _styleName)
    :IndexEntry(_styleName, IndexEntry::CHAPTER)
    {
            outlineLevel = INVALID_OUTLINE_LEVEL;
            // display is null String
    }

    virtual void addAttributes(KoXmlWriter* writer) const{
        if (!display.isNull()) {
            writer->addAttribute("text:display", display);
        }
        writer->addAttribute("text:outline-level", outlineLevel);
    }

    QString display;
    int outlineLevel;
};

class IndexEntrySpan : public IndexEntry {

public:
    IndexEntrySpan(QString _styleName)
    :IndexEntry(_styleName, IndexEntry::SPAN)
    {
        // text is null string
    }

    virtual void addAttributes(KoXmlWriter* writer) const{
        if (!text.isNull() && !text.isEmpty()){
            writer->addTextNode(text);
        }
    }

    QString text;
};

class IndexEntryText : public IndexEntry{

public:
    IndexEntryText(QString _styleName)
    :IndexEntry(_styleName,IndexEntry::TEXT)
    {}
};

class IndexEntryTabStop : public IndexEntry {

public:

    IndexEntryTabStop(QString _styleName)
    :IndexEntry(_styleName, IndexEntry::TAB_STOP)
    {}

    virtual void addAttributes(KoXmlWriter* writer) const{
        writer->addAttribute("style:leader-char",tab.leaderText);
        // If the value of this attribute is left, the style:position attribute shall also be present.
        // Otherwise, this attribute shall be omitted.
        if (tab.type == QTextOption::LeftTab) {
            writer->addAttribute("style:type", "left");
            writer->addAttribute("style:position", m_position);
        } else {
            Q_ASSERT(tab.type == QTextOption::RightTab);
            writer->addAttribute("style:type", "right");
        }
    }

    KoText::Tab tab;
    // for saving let's save the original unit,
    // for KoText::Tab we need to covert to PostScript points
    void setPosition(const QString &position){
        m_position = position;
        tab.position = 550; //KoUnit::parseValue(position);
    }
private:
    QString m_position;
};

class IndexEntryPageNumber : public IndexEntry {

public:
    IndexEntryPageNumber(QString _styleName)
    :IndexEntry(_styleName, IndexEntry::PAGE_NUMBER)
    {}
};

class IndexEntryLinkEnd : public IndexEntry {

public:
    IndexEntryLinkEnd(QString _styleName)
    :IndexEntry(_styleName, IndexEntry::LINK_END)
    {}
};

class TocEntryTemplate{
public:
    int outlineLevel;
    QString styleName;
    int styleId;
    QList<IndexEntry*> indexEntries;

    void saveOdf(KoXmlWriter * writer) const{
        writer->startElement("text:table-of-content-entry-template");
            writer->addAttribute("text:outline-level", outlineLevel);
            writer->addAttribute("text:style-name", styleName);

            foreach(IndexEntry* e,indexEntries){
                e->saveOdf(writer);
            }

        writer->endElement();
    }
};

class IndexTitleTemplate{
public:
    QString styleName;
    int styleId;
    QString text;

    void saveOdf(KoXmlWriter * writer) const{
        writer->startElement("text:index-title-template");
            writer->addAttribute("text:style-name", styleName);
            if ( !text.isEmpty() && !text.isNull() ) {
                writer->addTextNode(text);
            }
        writer->endElement();
    }

};

class IndexSourceStyle{
public:
    QString styleName;
    int styleId;

    void saveOdf(KoXmlWriter * writer) const {
        writer->startElement("text:index-source-styles");
        if (!styleName.isNull()){
            writer->addAttribute("text:style-name",styleName);
        }
        writer->endElement();
    }
};

class IndexSourceStyles {
public:
    int outlineLevel;
    QList<IndexSourceStyle> styles;

    void saveOdf(KoXmlWriter * writer) const {
        writer->startElement("text:index-source-styles");
            writer->addAttribute("text:outline-level", outlineLevel);
            foreach(const IndexSourceStyle &s, styles) {
                s.saveOdf(writer);
            }
        writer->endElement();
    }

};

class TableOfContentSource {

public:
    QString indexScope; // enum {document, chapter}
    int outlineLevel;
    bool relativeTabStopPosition;
    bool useIndexMarks;
    bool useIndexSourceStyles;
    bool useOutlineLevel;

    IndexTitleTemplate indexTitleTemplate;
    QList<TocEntryTemplate> entryTemplate; // N-entries
    QList<IndexSourceStyles> indexSourceStyles;

    void saveOdf(KoXmlWriter * writer) const{
        writer->startElement("text:table-of-content-source");
            writer->addAttribute("text:index-scope", indexScope);
            writer->addAttribute("text:outline-level", outlineLevel);
            writer->addAttribute("text:relative-tab-stop-position", relativeTabStopPosition);
            writer->addAttribute("text:use-index-marks", useIndexMarks);
            writer->addAttribute("text:use-index-source-styles", useIndexSourceStyles);
            writer->addAttribute("text:use-outline-level", useOutlineLevel);

            indexTitleTemplate.saveOdf(writer);

            foreach (TocEntryTemplate entryTemplate, entryTemplate){
                entryTemplate.saveOdf(writer);
            }

            foreach (IndexSourceStyles sourceStyle, indexSourceStyles){
                sourceStyle.saveOdf(writer);
            }

        writer->endElement(); // text:table-of-content-source
    }



};

class TableOfContent{

public:
    QString name;
    QString styleName;
    // TODO: add support for those according ODF v1.2
    // text: protected
    // text: protection-key
    // text:protection-key-digest-algorithm
    // xml:id
    TableOfContentSource tocSource;

};


class KoTextSharedLoadingData;

class KOTEXT_EXPORT KoTableOfContentsGeneratorInfo {

public:

    KoTableOfContentsGeneratorInfo();
    ~KoTableOfContentsGeneratorInfo();

    void loadOdf(const KoXmlElement &element);
    void saveOdf(KoXmlWriter * writer) const;

    void setSharedLoadingData(KoTextSharedLoadingData * loadingData);

    TableOfContent * tableOfContentData() const {
        return m_toc;
    }


private:
    TableOfContent * m_toc;
    KoTextSharedLoadingData * m_sharedLoadingData;

private:
    int styleNameToStyleId(QString styleName);
};

Q_DECLARE_METATYPE(KoTableOfContentsGeneratorInfo*)

#endif