/*
 * Copyright (C) 2011 Lukáš Tvrdý <lukas.tvrdy@ixonos.com>
 * Copyright (C) 2011 Ko GmbH <cbo@kogmbh.com>
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

#include "KoTableOfContentsGeneratorInfo.h"

#include <KoXmlNS.h>
#include <KoTextSharedLoadingData.h>
#include <KoParagraphStyle.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>

int KoTableOfContentsGeneratorInfo::styleNameToStyleId(KoTextSharedLoadingData *sharedLoadingData, const QString &styleName)
{
    //find styleId of a style based on its style:name property
    KoParagraphStyle * style = sharedLoadingData->paragraphStyle(styleName, true);
    if (style) {
        return style->styleId();
    }

    //if the previous way of finding styles fails fall back on using style:display-name property of a style
    QList<KoParagraphStyle *> paragraphStyles = sharedLoadingData->paragraphStyles(true);
    QList<KoParagraphStyle *>::const_iterator iter = paragraphStyles.constBegin();

    for (; iter != paragraphStyles.constEnd(); ++iter) {
        if ((*iter)->name() == styleName) {
            return (*iter)->styleId();
        }
    }

    return 0;
}


KoTableOfContentsGeneratorInfo::KoTableOfContentsGeneratorInfo(bool generateEntryTemplate)
    :
      m_indexScope("document")
    , m_outlineLevel(10)
    , m_relativeTabStopPosition(true)
    , m_useIndexMarks(true)
    , m_useIndexSourceStyles(false)
    , m_useOutlineLevel(true)
{
    // index-title-template
    // m_indexTitleTemplate.text = "title";
    if (generateEntryTemplate) {
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
            entryChapter->display = "number";
            entryChapter->outlineLevel = level;
            tocEntryTemplate.indexEntries.append(static_cast<IndexEntry*>(entryChapter));

            // index-entry-text
            IndexEntryText *entryText = new IndexEntryText(QString());
            tocEntryTemplate.indexEntries.append(static_cast<IndexEntry*>(entryText));

            // index-entry-tab-stop
            IndexEntryTabStop *entryTabStop = new IndexEntryTabStop(QString());
            entryTabStop->tab.type = QTextOption::RightTab;
            entryTabStop->setPosition("");
            entryTabStop->tab.leaderText = '.';
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
}

KoTableOfContentsGeneratorInfo::~KoTableOfContentsGeneratorInfo()
{
    foreach (const TocEntryTemplate &entryTemplate, m_entryTemplate) {
        qDeleteAll(entryTemplate.indexEntries);
    }
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

KoTableOfContentsGeneratorInfo *KoTableOfContentsGeneratorInfo::clone()
{
    KoTableOfContentsGeneratorInfo *newToCInfo=new KoTableOfContentsGeneratorInfo(false);
    newToCInfo->m_entryTemplate.clear();
    newToCInfo->m_name = QString(m_name);
    newToCInfo->m_styleName = QString(m_styleName);
    newToCInfo->m_indexScope = QString(m_indexScope);
    newToCInfo->m_outlineLevel = m_outlineLevel;
    newToCInfo->m_relativeTabStopPosition = m_relativeTabStopPosition;
    newToCInfo->m_useIndexMarks = m_useIndexMarks;
    newToCInfo->m_useIndexSourceStyles = m_useIndexSourceStyles;
    newToCInfo->m_useOutlineLevel = m_useOutlineLevel;
    newToCInfo->m_indexTitleTemplate = m_indexTitleTemplate;

    foreach (const TocEntryTemplate &tocTemplate, m_entryTemplate) {
        newToCInfo->m_entryTemplate.append(tocTemplate);
    }

    foreach (const IndexSourceStyles &indexSourceStyles, m_indexSourceStyles) {
        newToCInfo->m_indexSourceStyles.append(indexSourceStyles);
    }

    return newToCInfo;
}

