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
#define DEBUG_TOC_STRUCTURE

#include <QDebug>
#include <QList>
#include <QString>
#include <QVariant>

#include <KoXmlReader.h>
#include "KoText.h"

const int INVALID_OUTLINE_LEVEL = -1;

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
    
    virtual void dump() const{
        switch (name){
            case UNKNOWN: qDebug() << "UNKNOWN"; break;
            case LINK_START     : qDebug() << "LINK_START"; break;
            case CHAPTER        : qDebug() << "CHAPTER"; break;
            case SPAN           : qDebug() << "SPAN"; break;
            case TEXT           : qDebug() << "TEXT";  break;
            case TAB_STOP       : qDebug() << "TAB_STOP"; break;
            case PAGE_NUMBER    : qDebug() << "PAGE_NUMBER"; break;
            case LINK_END       : qDebug() << "LINK_END"; break;
        }
        qDebug() << ppVar(styleName);
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
    
    QString display;
    int outlineLevel;
    
    virtual void dump() const{
        IndexEntry::dump();
        qDebug() << ppVar(display);
        qDebug() << ppVar(outlineLevel);
    }

};

class IndexEntrySpan : public IndexEntry {

public:
    IndexEntrySpan(QString _styleName)
    :IndexEntry(_styleName, IndexEntry::SPAN)
    {       
        // text is null string
    }

    virtual void dump() const{
        IndexEntry::dump();
        qDebug() << ppVar(text);
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


    virtual void dump() const{
        IndexEntry::dump();
        qDebug() << ppVar(tab.leaderText);
        qDebug() << ppVar(tab.position);
        qDebug() << ppVar(tab.type);
    }

    KoText::Tab tab;
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
    quint32 outlineLevel;
    QString styleName;
    int styleId;
    QList<IndexEntry*> indexEntries;
    
    void dump() const{
        qDebug() << "TocEntryTemplate";
        qDebug() << ppVar(outlineLevel);
        qDebug() << ppVar(styleName);
        
        foreach(IndexEntry* e,indexEntries){
            e->dump();
        }
    }
};

struct IndexTitleTemplate{
    QString styleName;
    int styleId;
    QString text;
};

struct IndexSourceStyle{
  QString styleName;  
  int styleId;
};

struct IndexSourceStyles {
    int outlineLevel;
    QList<IndexSourceStyle> styles;
    
    void dump() const{
        qDebug() << "IndexSourceStyles";
        qDebug() << ppVar(outlineLevel);
        
        foreach(const IndexSourceStyle &s, styles){
            qDebug() << "indexSourceStyle" << ppVar(s.styleName);
        }
        
    }
    
};

class TableOfContentSource {

public:
    QString indexScope; // enum {document, chapter}
    quint32 outlineLevel; 
    bool relativeTabStopPosition;
    bool useIndexMarks; 
    bool useIndexSourceStyles;
    bool useOutlineLevel;
    
    IndexTitleTemplate indexTitleTemplate;
    QList<TocEntryTemplate> entryTemplate; // N-entries
    QList<IndexSourceStyles> indexSourceStyles;
    
    void dump() const{
        qDebug() << "TableOfContentSource";
        qDebug() << ppVar(indexScope);
        qDebug() << ppVar(outlineLevel);
        qDebug() << ppVar(relativeTabStopPosition);
        qDebug() << ppVar(useIndexMarks);
        qDebug() << ppVar(useIndexSourceStyles);
        qDebug() << ppVar(useOutlineLevel);
        
        // has only style name
        qDebug() << "IndexTitleTemplate";
        qDebug() << indexTitleTemplate.styleName;
        
        foreach(const TocEntryTemplate &t,entryTemplate){
            t.dump();
        }
        
        foreach(const IndexSourceStyles &s,indexSourceStyles){
            s.dump();
        }
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


    void dump(){
        #ifdef DEBUG_TOC_STRUCTURE    
            qDebug() << "TableOfContent";
            qDebug() << ppVar(name) << ppVar(styleName);
            tocSource.dump();
        #endif        
    }
};

Q_DECLARE_METATYPE(TableOfContent*)

class KoTextSharedLoadingData;

class KoTableOfContentsGeneratorInfo {

public:
    
    KoTableOfContentsGeneratorInfo();
    ~KoTableOfContentsGeneratorInfo();
    
    void loadOdf(const KoXmlElement &element);
    void saveOdf(); // TODO
    
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

#endif