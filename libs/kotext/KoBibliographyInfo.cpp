/* This file is part of the KDE project
 * Copyright (C) 2011 Smit Patel <smitpatel24@gmail.com>
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

#include "KoBibliographyInfo.h"
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoTextSharedLoadingData.h>
#include <KoParagraphStyle.h>

#include <QTextCursor>

const QList<QString> KoBibliographyInfo::bibTypes = QList<QString>() << "article" << "book" << "booklet" << "conference"
                                                                     << "email" << "inbook" << "incollection"
                                                                     << "inproceedings" << "journal" << "manual"
                                                                     << "mastersthesis" << "misc" << "phdthesis"
                                                                     << "proceedings" << "techreport" << "unpublished"
                                                                     << "www" << "custom1" << "custom2"
                                                                     << "custom3" << "custom4" << "custom5";

const QList<QString> KoBibliographyInfo::bibDataFields = QList<QString>() << "address" << "annote" << "author"
                                                                          << "bibliography-type" << "booktitle"
                                                                          << "chapter" << "custom1" << "custom2"
                                                                          << "custom3" << "custom4" << "custom5"
                                                                          << "edition" << "editor" << "howpublished"
                                                                          << "idenfier" << "institution" << "isbn"
                                                                          << "issn" << "journal" << "month" << "note"
                                                                          << "number" << "organizations" << "pages"
                                                                          << "publisher" << "report-type" << "school"
                                                                          << "series" << "title" << "url" << "volume"
                                                                          << "year";
int KoBibliographyInfo::styleNameToStyleId(KoTextSharedLoadingData *sharedLoadingData, QString styleName)
{
    KoParagraphStyle * style = sharedLoadingData->paragraphStyle(styleName, true);
    if (style) {
        return style->styleId();
    }

    return 0;
}

KoBibliographyInfo::KoBibliographyInfo()
  : m_generator(0)
{
}

KoBibliographyInfo::~KoBibliographyInfo()
{
    foreach (const BibliographyEntryTemplate &entryTemplate, m_entryTemplate.values()) {
        qDeleteAll(entryTemplate.indexEntries);
    }
    delete m_generator;
    m_generator = 0; // just to be safe
}

void KoBibliographyInfo::loadOdf(KoTextSharedLoadingData *sharedLoadingData, const KoXmlElement& element)
{
    Q_ASSERT(element.localName() == "bibliography-source" && element.namespaceURI() == KoXmlNS::text);

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
        } else if (p.localName() == "bibliography-entry-template") {
            BibliographyEntryTemplate bibEntryTemplate;
            bibEntryTemplate.styleName = p.attribute("style-name");
            bibEntryTemplate.bibliographyType = p.attribute("bibliography-type");
            bibEntryTemplate.styleId = styleNameToStyleId(sharedLoadingData, bibEntryTemplate.styleName );

            KoXmlElement indexEntry;
            forEachElement(indexEntry, p) {
                if (indexEntry.namespaceURI() != KoXmlNS::text) {
                    continue;
                }

                if (indexEntry.localName() == "index-entry-bibliography") {
                    // use null String if the style name is not present, it means that we inherit it from the parent
                    IndexEntryBibliography * entryBibliography = new IndexEntryBibliography(
                        indexEntry.attribute("style-name", QString())
                    );

                    entryBibliography->dataField = indexEntry.attribute("bibliography-data-field", "article");
                    bibEntryTemplate.indexEntries.append(static_cast<IndexEntry*>(entryBibliography));

                } else if (indexEntry.localName() == "index-entry-span") {
                    IndexEntrySpan * entrySpan = new IndexEntrySpan(indexEntry.attribute("style-name", QString()));
                    entrySpan->text = indexEntry.text();
                    bibEntryTemplate.indexEntries.append(static_cast<IndexEntry*>(entrySpan));

                } else if (indexEntry.localName() == "index-entry-tab-stop") {
                    IndexEntryTabStop * entryTabStop = new IndexEntryTabStop(indexEntry.attribute("style-name", QString()));

                    QString type = indexEntry.attribute("type","right"); // left or right
                    if (type == "left") {
                        entryTabStop->tab.type = QTextOption::LeftTab;
                    } else {
                        entryTabStop->tab.type = QTextOption::RightTab;
                    }
                    entryTabStop->setPosition(indexEntry.attribute("position", QString()));
                    entryTabStop->tab.leaderText = indexEntry.attribute("leader-char",".");
                    bibEntryTemplate.indexEntries.append(static_cast<IndexEntry*>(entryTabStop));
                }
            }
            m_entryTemplate[bibEntryTemplate.bibliographyType] = bibEntryTemplate;

        // third child
        }
    }// forEachElement
}

void KoBibliographyInfo::saveOdf(KoXmlWriter * writer) const
{
    writer->startElement("text:bibliography-source");

        m_indexTitleTemplate.saveOdf(writer);

        foreach (const BibliographyEntryTemplate &entry, m_entryTemplate.values()) {
            entry.saveOdf(writer);
        }

    writer->endElement();
}

void KoBibliographyInfo::setGenerator(BibliographyGeneratorInterface *generator)
{
    delete m_generator;
    m_generator = generator;
}

void KoBibliographyInfo::setEntryTemplates(QMap<QString, BibliographyEntryTemplate> &entryTemplates)
{
    m_entryTemplate = entryTemplates;
}

BibliographyGeneratorInterface *KoBibliographyInfo::generator() const
{
    return m_generator;
}
