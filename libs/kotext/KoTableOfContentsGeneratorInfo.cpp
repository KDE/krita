/*
 * Copyright (C) 2011 Lukáš Tvrdý <lukas.tvrdy@ixonos.com>
 * Copyright (C) 2011 Ko GmbH <cbo@kogmbh.com>
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
    case LINK_START:
        writer->startElement("text:index-entry-link-start");
        break;
    case CHAPTER:
        writer->startElement("text:index-entry-chapter");
        break;
    case SPAN:
        writer->startElement("text:index-entry-span");
        break;
    case TEXT:
        writer->startElement("text:index-entry-text");
        break;
    case TAB_STOP:
        writer->startElement("text:index-entry-tab-stop");
        break;
    case PAGE_NUMBER:
        writer->startElement("text:index-entry-page-number");
        break;
    case LINK_END:
        writer->startElement("text:index-entry-link-end");
        break;
    case UNKNOWN:
        break;
    }

    if (!styleName.isNull()) {
        writer->addAttribute("text:style-name", styleName);
    }

    addAttributes(writer);
    writer->endElement();
}


IndexEntryLinkStart::IndexEntryLinkStart(QString _styleName)
    : IndexEntry(_styleName, IndexEntry::LINK_START)
{

}


IndexEntryChapter::IndexEntryChapter(QString _styleName)
    : IndexEntry(_styleName, IndexEntry::CHAPTER)
    , display(QString())
    , outlineLevel(INVALID_OUTLINE_LEVEL)
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
        if (!text.isEmpty() && !text.isNull()) {
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


int KoTableOfContentsGeneratorInfo::styleNameToStyleId(KoTextSharedLoadingData *sharedLoadingData, QString styleName)
{
    KoParagraphStyle * style = sharedLoadingData->paragraphStyle(styleName, true);
    if (style) {
        return style->styleId();
    }

    return 0;
}


KoTableOfContentsGeneratorInfo::KoTableOfContentsGeneratorInfo()
  : 
   m_indexScope("document")
  , m_outlineLevel(10)
  , m_relativeTabStopPosition(true)
  , m_useIndexMarks(true)
  , m_useIndexSourceStyles(false)
  , m_useOutlineLevel(true)
  , m_generator(0)
{
    // index-title-template
    // m_indexTitleTemplate.text = "title";

    // table-of-content-entry-template
    for (int level = 1; level <= m_outlineLevel; level++)  {
        TocEntryTemplate tocEntryTemplate;
        tocEntryTemplate.outlineLevel = level;

        // index-entry-link-start
        IndexEntryLinkStart *link = new IndexEntryLinkStart(QString());
        tocEntryTemplate.indexEntries.append(static_cast<IndexEntry*>(link));

        // index-entry-chapter
        // use null String if the style name is not present, it means that we inherit it from the parent
        IndexEntryChapter *entryChapter = new IndexEntryChapter(QString());
        entryChapter->display = "number-and-name";
        entryChapter->outlineLevel = level;
        tocEntryTemplate.indexEntries.append(static_cast<IndexEntry*>(entryChapter));

        // index-entry-text
        IndexEntryText *entryText = new IndexEntryText(QString());
        tocEntryTemplate.indexEntries.append(static_cast<IndexEntry*>(entryText));

        // index-entry-tab-stop
        IndexEntryTabStop *entryTabStop = new IndexEntryTabStop(QString());
        entryTabStop->tab.type = QTextOption::RightTab;
        entryTabStop->setPosition("MAX");
        entryTabStop->tab.leaderText = ".";
        tocEntryTemplate.indexEntries.append(static_cast<IndexEntry*>(entryTabStop));

        // index-entry-page-number
        IndexEntryPageNumber *entryPageNumber = new IndexEntryPageNumber(QString());
        tocEntryTemplate.indexEntries.append(static_cast<IndexEntry*>(entryPageNumber) );

        // index-entry-link-end
        IndexEntryLinkEnd *linkend = new IndexEntryLinkEnd(QString());
        tocEntryTemplate.indexEntries.append(static_cast<IndexEntry*>(linkend));

        m_entryTemplate.append(tocEntryTemplate);
    }
}

KoTableOfContentsGeneratorInfo::~KoTableOfContentsGeneratorInfo()
{
    foreach (const TocEntryTemplate &entryTemplate, m_entryTemplate) {
        qDeleteAll(entryTemplate.indexEntries);
    }
    delete m_generator;
    m_generator = 0; // just to be safe
}


void KoTableOfContentsGeneratorInfo::loadOdf(KoTextSharedLoadingData *sharedLoadingData, const KoXmlElement& element)
{
    Q_ASSERT(element.localName() == "table-of-content-source" && element.namespaceURI() == KoXmlNS::text);

    foreach (const TocEntryTemplate &entryTemplate, m_entryTemplate) {
        qDeleteAll(entryTemplate.indexEntries);
    }
    m_entryTemplate.clear();

    m_indexScope = element.attribute("index-scope", "document"); // enum {document, chapter}
    m_outlineLevel = element.attribute("outline-level","1").toInt();
    m_relativeTabStopPosition = element.attribute("relative-tab-stop-position","true") == "true";
    m_useIndexMarks = element.attribute("use-index-marks","f") == "true";
    m_useIndexSourceStyles = element.attribute("use-index-source-styles","false") == "true";
    m_useOutlineLevel = element.attribute("use-outline-level","true") == "true";

    // three other children to visit
    KoXmlElement p;
    forEachElement(p, element) {
        if (p.namespaceURI() != KoXmlNS::text) {
            continue;
        }

        // first child
        if (p.localName() == "index-title-template") {
            m_indexTitleTemplate.styleName = p.attribute("style-name");
            m_indexTitleTemplate.styleId = styleNameToStyleId(sharedLoadingData, m_indexTitleTemplate.styleName);
            m_indexTitleTemplate.text = p.text();
        // second child
        } else if (p.localName() == "table-of-content-entry-template") {
            qDebug() <<"table-of-content-entry-template";
            TocEntryTemplate tocEntryTemplate;
            tocEntryTemplate.outlineLevel = p.attribute("outline-level").toInt();
            tocEntryTemplate.styleName = p.attribute("style-name");
            tocEntryTemplate.styleId = styleNameToStyleId(sharedLoadingData, tocEntryTemplate.styleName );

            KoXmlElement indexEntry;
            forEachElement(indexEntry, p) {
                if (indexEntry.namespaceURI() != KoXmlNS::text) {
                    continue;
                }

                if (indexEntry.localName() == "index-entry-chapter") {
                    // use null String if the style name is not present, it means that we inherit it from the parent
                    IndexEntryChapter *entryChapter = new IndexEntryChapter(
                        indexEntry.attribute("style-name", QString())
                    );

                    // display can be "name", "number", "number-and-name", "plain-number" or "plain-number-and-name"
                    // "number" is default according the specs ODF v1.2
                    entryChapter->display = indexEntry.attribute("display", "number");
                    entryChapter->outlineLevel = indexEntry.attribute("outline-level", QString::number(tocEntryTemplate.outlineLevel)).toInt();
                    tocEntryTemplate.indexEntries.append(static_cast<IndexEntry*>(entryChapter));

                } else if (indexEntry.localName() == "index-entry-text") {
                    IndexEntryText *entryText = new IndexEntryText(indexEntry.attribute("style-name", QString()));
                    tocEntryTemplate.indexEntries.append(static_cast<IndexEntry*>(entryText));

                } else if (indexEntry.localName() == "index-entry-page-number") {
                    IndexEntryPageNumber *entryPageNumber = new IndexEntryPageNumber(indexEntry.attribute("style-name", QString()));
                    tocEntryTemplate.indexEntries.append(static_cast<IndexEntry*>(entryPageNumber) );

                } else if (indexEntry.localName() == "index-entry-span") {
                    IndexEntrySpan *entrySpan = new IndexEntrySpan(indexEntry.attribute("style-name", QString()) );
                    entrySpan->text = indexEntry.text();
                    tocEntryTemplate.indexEntries.append(static_cast<IndexEntry*>(entrySpan));

                } else if (indexEntry.localName() == "index-entry-tab-stop") {
                    IndexEntryTabStop *entryTabStop = new IndexEntryTabStop(indexEntry.attribute("style-name", QString()));

                    QString type = indexEntry.attribute("type","right"); // left or right
                    if (type == "left") {
                        entryTabStop->tab.type = QTextOption::LeftTab;
                    } else {
                        entryTabStop->tab.type = QTextOption::RightTab;
                    }
                    entryTabStop->setPosition(indexEntry.attribute("position", QString()));
                    entryTabStop->tab.leaderText = indexEntry.attribute("leader-char",".");
                    tocEntryTemplate.indexEntries.append(static_cast<IndexEntry*>(entryTabStop));

                } else if (indexEntry.localName() == "index-entry-link-start") {
                    IndexEntryLinkStart *link = new IndexEntryLinkStart(indexEntry.attribute("style-name", QString()));
                    tocEntryTemplate.indexEntries.append(static_cast<IndexEntry*>(link));
                } else if (indexEntry.localName() == "index-entry-link-end") {
                    IndexEntryLinkEnd *link = new IndexEntryLinkEnd(indexEntry.attribute("style-name", QString()));
                    tocEntryTemplate.indexEntries.append(static_cast<IndexEntry*>(link));
                }
            }
            m_entryTemplate.append(tocEntryTemplate);

        // third child
        } else if (p.localName() == "index-source-styles" && p.namespaceURI() == KoXmlNS::text) {
            qDebug() <<"index-source-styles";
                IndexSourceStyles indexStyles;
                indexStyles.outlineLevel = p.attribute("outline-level").toInt();

                IndexSourceStyle indexStyle;
                KoXmlElement sourceElement;
                forEachElement(sourceElement, p) {
                    if (sourceElement.localName() == "index-source-style") {
                        indexStyle.styleName = sourceElement.attribute("style-name");
                        indexStyle.styleId = styleNameToStyleId(sharedLoadingData, indexStyle.styleName);
                        indexStyles.styles.append(indexStyle);
                    }
                }
                m_indexSourceStyles.append(indexStyles);
        }
    }// forEachElement
}


void KoTableOfContentsGeneratorInfo::saveOdf(KoXmlWriter * writer) const
{
    writer->startElement("text:table-of-content-source");
        writer->addAttribute("text:index-scope", m_indexScope);
        writer->addAttribute("text:outline-level", m_outlineLevel);
        writer->addAttribute("text:relative-tab-stop-position", m_relativeTabStopPosition);
        writer->addAttribute("text:use-index-marks", m_useIndexMarks);
        writer->addAttribute("text:use-index-source-styles", m_useIndexSourceStyles);
        writer->addAttribute("text:use-outline-level", m_useOutlineLevel);

        m_indexTitleTemplate.saveOdf(writer);

        foreach (const TocEntryTemplate &entry, m_entryTemplate) {
            entry.saveOdf(writer);
        }

        foreach (const IndexSourceStyles &sourceStyle, m_indexSourceStyles) {
            sourceStyle.saveOdf(writer);
        }

    writer->endElement(); // text:table-of-content-source
}

void KoTableOfContentsGeneratorInfo::setGenerator(ToCGenerator *generator)
{
    delete m_generator;
    m_generator = generator;
}

ToCGenerator *KoTableOfContentsGeneratorInfo::generator() const
{
    return m_generator;
}
