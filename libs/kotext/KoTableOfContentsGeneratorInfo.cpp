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

#include <KoTableOfContentsGeneratorInfo.h>

#include <QTextCursor>

#include <KoXmlNS.h>

KoTableOfContentsGeneratorInfo::KoTableOfContentsGeneratorInfo()
{
    m_toc = new TableOfContent;
}

KoTableOfContentsGeneratorInfo::~KoTableOfContentsGeneratorInfo()
{
    if (m_toc){
        qDebug() << "KoTableOfContentsGeneratorInfo:I'm not owner of TableOfContent, so please delete it";
    }
}



void KoTableOfContentsGeneratorInfo::loadOdf(const KoXmlElement& element)
{
    Q_ASSERT(e.localName() == "table-of-content-source" && e.namespaceURI() == KoXmlNS::text);
    
    m_toc->tocSource.indexScope = element.attribute("index-scope", "document"); // enum {document, chapter}
    m_toc->tocSource.outlineLevel = element.attribute("outline-level").toInt();  
    m_toc->tocSource.relativeTabStopPosition = element.attribute("relative-tab-stop-position","true") == "true";
    m_toc->tocSource.useIndexMarks = element.attribute("use-index-marks","true") == "true"; 
    m_toc->tocSource.useIndexSourceStyles = element.attribute("use-index-source-styles","true") == "true";
    m_toc->tocSource.useOutlineLevel = element.attribute("use-outline-level","true") == "true";

    // three other children to visit
    KoXmlElement p;
    forEachElement(p, element){
        // first child 
        if (p.localName() == "index-title-template" && p.namespaceURI() == KoXmlNS::text) {
            m_toc->tocSource.indexTitleTemplate.styleName = p.attribute("style-name");
        
        // second child
        } else if (p.localName() == "table-of-content-entry-template" && p.namespaceURI() == KoXmlNS::text) {
            TocEntryTemplate tocEntryTemplate;
            tocEntryTemplate.outlineLevel = p.attribute("outline-level").toInt();
            tocEntryTemplate.styleName = p.attribute("style-name");
            
            KoXmlElement indexEntry;
            forEachElement(indexEntry, p) {
                if (indexEntry.localName() == "index-entry-chapter") {
                    // use null String if the style name is not present, it means that we inherit it from the parent
                    IndexEntryChapter * entryChapter = new IndexEntryChapter(
                        indexEntry.attribute("style-name", QString())
                    );

                    // display can be "name", "number", "number-and-name", "plain-number" or "plain-number-and-name"
                    // "number" is default according the specs ODF v1.2
                    entryChapter->display = indexEntry.attribute("display", "number");
                    entryChapter->outlineLevel = indexEntry.attribute("outline-level", QString::number(tocEntryTemplate.outlineLevel)).toInt();
                    //TODO: who will delete it? 
                    tocEntryTemplate.indexEntries.append( static_cast<IndexEntry*>(entryChapter) );
                    
                } else if (indexEntry.localName() == "index-entry-text") {
                    IndexEntryText * entryText = new IndexEntryText(indexEntry.attribute("style-name", QString()));
                    tocEntryTemplate.indexEntries.append( static_cast<IndexEntry*>(entryText) );

                } else if (indexEntry.localName() == "index-entry-page-number") {
                    IndexEntryPageNumber * entryPageNumber = new IndexEntryPageNumber(indexEntry.attribute("style-name", QString()));
                    tocEntryTemplate.indexEntries.append( static_cast<IndexEntry*>(entryPageNumber) );

                } else if (indexEntry.localName() == "index-entry-span") {
                    IndexEntrySpan * entrySpan = new IndexEntrySpan( indexEntry.attribute("style-name", QString()) );
                    entrySpan->text = indexEntry.text();
                    tocEntryTemplate.indexEntries.append( static_cast<IndexEntry*>(entrySpan) );
                    
                } else if (indexEntry.localName() == "index-entry-tab-stop"){
                    IndexEntryTabStop * entryTabStop = new IndexEntryTabStop(indexEntry.attribute("style-name", QString()));
                    entryTabStop->leaderChar = indexEntry.attribute("leader-char",".").at(0);
                    entryTabStop->position = indexEntry.attribute("position"); // length (size+unit)
                    entryTabStop->type = indexEntry.attribute("type"); // left or right
                    tocEntryTemplate.indexEntries.append( static_cast<IndexEntry*>(entryTabStop) );
                        
                } else if (indexEntry.localName() == "index-entry-link-start"){
                    IndexEntryLinkStart * link = new IndexEntryLinkStart(indexEntry.attribute("style-name", QString()));
                    tocEntryTemplate.indexEntries.append( static_cast<IndexEntry*>(link) );
                } else if (indexEntry.localName() == "index-entry-link-end"){
                    IndexEntryLinkEnd * link = new IndexEntryLinkEnd(indexEntry.attribute("style-name", QString()));
                    tocEntryTemplate.indexEntries.append( static_cast<IndexEntry*>(link) );
                }
            }
            m_toc->tocSource.entryTemplate.append(tocEntryTemplate);
        
        // third child
        } else if (p.localName() == "index-source-styles" && p.namespaceURI() == KoXmlNS::text) {
                IndexSourceStyles indexStyles;
                indexStyles.outlineLevel = p.attribute("outline-level").toInt();
            
                IndexSourceStyle indexStyle;
                KoXmlElement sourceElement;
                forEachElement(sourceElement, p){
                    if (sourceElement.localName() == "index-source-style"){
                        indexStyle.styleName = sourceElement.attribute("style-name");
                        indexStyles.styles.append(indexStyle);
                        
                    }
                }
                m_toc->tocSource.indexSourceStyles.append(indexStyles);
        }   
    }// forEachElement
}

