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
#include <KoTextSharedLoadingData.h>
#include <KoParagraphStyle.h>
#include <KoXmlWriter.h>


IndexEntry::IndexEntry(QString _styleName, IndexEntry::IndexEntryName _name)
        :   styleName(_styleName),
            name(_name)
{

}


IndexEntry::~IndexEntry()
{

}


void IndexEntry::addAttributes(KoXmlWriter* writer) const
{
    Q_UNUSED(writer);
}


void IndexEntry::saveOdf(KoXmlWriter* writer) const
{
    switch (name) {
        case LINK_START     : writer->startElement("text:index-entry-link-start"); break;
        case CHAPTER        : writer->startElement("text:index-entry-chapter"); break;
        case SPAN           : writer->startElement("text:index-entry-span"); break;
        case TEXT           : writer->startElement("text:index-entry-text");  break;
        case TAB_STOP       : writer->startElement("text:index-entry-tab-stop"); break;
        case PAGE_NUMBER    : writer->startElement("text:index-entry-page-number"); break;
        case LINK_END       : writer->startElement("text:index-entry-link-end"); break;
        case UNKNOWN        : break;
    }

    if (!styleName.isNull()) {
        writer->addAttribute("text:style-name", styleName);
    }

    addAttributes(writer);
    writer->endElement();
}


IndexEntryLinkStart::IndexEntryLinkStart(QString _styleName): IndexEntry(_styleName, IndexEntry::LINK_START)
{

}


IndexEntryChapter::IndexEntryChapter(QString _styleName):   IndexEntry(_styleName, IndexEntry::CHAPTER),
                                                            outlineLevel(INVALID_OUTLINE_LEVEL),
                                                            display(QString())
{

}


void IndexEntryChapter::addAttributes(KoXmlWriter* writer) const
{
    if (!display.isNull()) {
        writer->addAttribute("text:display", display);
    }
    writer->addAttribute("text:outline-level", outlineLevel);
}


IndexEntrySpan::IndexEntrySpan(QString _styleName): IndexEntry(_styleName, IndexEntry::SPAN)
{
    text = QString();
}


void IndexEntrySpan::addAttributes(KoXmlWriter* writer) const
{
    if (!text.isNull() && !text.isEmpty()) {
        writer->addTextNode(text);
    }
}


IndexEntryText::IndexEntryText(QString _styleName): IndexEntry(_styleName,IndexEntry::TEXT)
{

}


IndexEntryTabStop::IndexEntryTabStop(QString _styleName): IndexEntry(_styleName, IndexEntry::TAB_STOP)
{

}


void IndexEntryTabStop::addAttributes(KoXmlWriter* writer) const
{
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


void IndexEntryTabStop::setPosition(const QString& position)
{
    m_position = position;
    tab.position = KoUnit::parseValue(position);
}


IndexEntryPageNumber::IndexEntryPageNumber(QString _styleName): IndexEntry(_styleName, IndexEntry::PAGE_NUMBER)
{

}


IndexEntryLinkEnd::IndexEntryLinkEnd(QString _styleName): IndexEntry(_styleName, IndexEntry::LINK_END)
{

}


void TocEntryTemplate::saveOdf(KoXmlWriter* writer) const
{
    writer->startElement("text:table-of-content-entry-template");
        writer->addAttribute("text:outline-level", outlineLevel);
        writer->addAttribute("text:style-name", styleName);

        foreach(IndexEntry* e,indexEntries) {
            e->saveOdf(writer);
        }

    writer->endElement();
}


void IndexTitleTemplate::saveOdf(KoXmlWriter* writer) const
{
    writer->startElement("text:index-title-template");
        writer->addAttribute("text:style-name", styleName);
        if ( !text.isEmpty() && !text.isNull() ) {
            writer->addTextNode(text);
        }
    writer->endElement();
}


void IndexSourceStyle::saveOdf(KoXmlWriter* writer) const
{
    writer->startElement("text:index-source-styles");
    if (!styleName.isNull()) {
        writer->addAttribute("text:style-name",styleName);
    }
    writer->endElement();
}


void IndexSourceStyles::saveOdf(KoXmlWriter* writer) const
{
    writer->startElement("text:index-source-styles");
        writer->addAttribute("text:outline-level", outlineLevel);
        foreach(const IndexSourceStyle &s, styles) {
            s.saveOdf(writer);
        }
    writer->endElement();
}


void TableOfContentSource::saveOdf(KoXmlWriter* writer) const
{
    writer->startElement("text:table-of-content-source");
        writer->addAttribute("text:index-scope", indexScope);
        writer->addAttribute("text:outline-level", outlineLevel);
        writer->addAttribute("text:relative-tab-stop-position", relativeTabStopPosition);
        writer->addAttribute("text:use-index-marks", useIndexMarks);
        writer->addAttribute("text:use-index-source-styles", useIndexSourceStyles);
        writer->addAttribute("text:use-outline-level", useOutlineLevel);

        indexTitleTemplate.saveOdf(writer);

        foreach (TocEntryTemplate entryTemplate, entryTemplate) {
            entryTemplate.saveOdf(writer);
        }

        foreach (IndexSourceStyles sourceStyle, indexSourceStyles) {
            sourceStyle.saveOdf(writer);
        }

    writer->endElement(); // text:table-of-content-source
}


TableOfContent* KoTableOfContentsGeneratorInfo::tableOfContentData() const
{
    return m_toc;
}


int KoTableOfContentsGeneratorInfo::styleNameToStyleId(QString styleName)
{
    KoParagraphStyle * style = m_sharedLoadingData->paragraphStyle(styleName, true);
    if (style) {
        return style->styleId();
    }

    qDebug() << "Style " << styleName << " has not been found.";
    return 0;
}


KoTableOfContentsGeneratorInfo::KoTableOfContentsGeneratorInfo()
{
    m_toc = new TableOfContent;
    m_sharedLoadingData = 0;
}

KoTableOfContentsGeneratorInfo::~KoTableOfContentsGeneratorInfo()
{
    foreach (const TocEntryTemplate &entryTemplate, m_toc->tocSource.entryTemplate) {
        qDeleteAll(entryTemplate.indexEntries);
    }
    delete m_toc;
}


void KoTableOfContentsGeneratorInfo::loadOdf(const KoXmlElement& element)
{
    Q_ASSERT(element.localName() == "table-of-content-source" && element.namespaceURI() == KoXmlNS::text);

    m_toc->tocSource.indexScope = element.attribute("index-scope", "document"); // enum {document, chapter}
    m_toc->tocSource.outlineLevel = element.attribute("outline-level").toInt();
    m_toc->tocSource.relativeTabStopPosition = element.attribute("relative-tab-stop-position","true") == "true";
    m_toc->tocSource.useIndexMarks = element.attribute("use-index-marks","true") == "true";
    m_toc->tocSource.useIndexSourceStyles = element.attribute("use-index-source-styles","true") == "true";
    m_toc->tocSource.useOutlineLevel = element.attribute("use-outline-level","true") == "true";

    // three other children to visit
    KoXmlElement p;
    forEachElement(p, element) {
        if (p.namespaceURI() != KoXmlNS::text) {
            continue;
        }

        // first child
        if (p.localName() == "index-title-template") {
            m_toc->tocSource.indexTitleTemplate.styleName = p.attribute("style-name");
            m_toc->tocSource.indexTitleTemplate.styleId = styleNameToStyleId( m_toc->tocSource.indexTitleTemplate.styleName );
            m_toc->tocSource.indexTitleTemplate.text = p.text();
        // second child
        } else if (p.localName() == "table-of-content-entry-template") {
            TocEntryTemplate tocEntryTemplate;
            tocEntryTemplate.outlineLevel = p.attribute("outline-level").toInt();
            tocEntryTemplate.styleName = p.attribute("style-name");
            tocEntryTemplate.styleId = styleNameToStyleId( tocEntryTemplate.styleName );

            KoXmlElement indexEntry;
            forEachElement(indexEntry, p) {
                if (indexEntry.namespaceURI() != KoXmlNS::text) {
                    continue;
                }

                if (indexEntry.localName() == "index-entry-chapter") {
                    // use null String if the style name is not present, it means that we inherit it from the parent
                    IndexEntryChapter * entryChapter = new IndexEntryChapter(
                        indexEntry.attribute("style-name", QString())
                    );

                    // display can be "name", "number", "number-and-name", "plain-number" or "plain-number-and-name"
                    // "number" is default according the specs ODF v1.2
                    entryChapter->display = indexEntry.attribute("display", "number");
                    entryChapter->outlineLevel = indexEntry.attribute("outline-level", QString::number(tocEntryTemplate.outlineLevel)).toInt();
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

                } else if (indexEntry.localName() == "index-entry-tab-stop") {
                    IndexEntryTabStop * entryTabStop = new IndexEntryTabStop(indexEntry.attribute("style-name", QString()));

                    QString type = indexEntry.attribute("type"); // left or right
                    if (type == "right") {
                        entryTabStop->tab.type = QTextOption::RightTab;
                    } else if (type == "left") {
                        entryTabStop->tab.type = QTextOption::LeftTab;
                    } else {
                        entryTabStop->tab.type = QTextOption::RightTab;
                    }
                    entryTabStop->tab.leaderText = indexEntry.attribute("leader-char",".");
                    entryTabStop->setPosition( indexEntry.attribute("position", QString("490pt")) );
                    tocEntryTemplate.indexEntries.append( static_cast<IndexEntry*>(entryTabStop) );

                } else if (indexEntry.localName() == "index-entry-link-start") {
                    IndexEntryLinkStart * link = new IndexEntryLinkStart(indexEntry.attribute("style-name", QString()));
                    tocEntryTemplate.indexEntries.append( static_cast<IndexEntry*>(link) );
                } else if (indexEntry.localName() == "index-entry-link-end") {
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
                forEachElement(sourceElement, p) {
                    if (sourceElement.localName() == "index-source-style") {
                        indexStyle.styleName = sourceElement.attribute("style-name");
                        indexStyle.styleId = styleNameToStyleId( indexStyle.styleName );
                        indexStyles.styles.append( indexStyle );
                    }
                }
                m_toc->tocSource.indexSourceStyles.append(indexStyles);
        }
    }// forEachElement
}


void KoTableOfContentsGeneratorInfo::saveOdf(KoXmlWriter * writer) const
{
    m_toc->tocSource.saveOdf(writer);
}


void KoTableOfContentsGeneratorInfo::setSharedLoadingData(KoTextSharedLoadingData* loadingData)
{
    m_sharedLoadingData = loadingData;
}

