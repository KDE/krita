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

#include "BibliographyGenerator.h"
#include "KoInlineCite.h"

#include <klocalizedstring.h>
#include <KoOdfBibliographyConfiguration.h>
#include <KoInlineTextObjectManager.h>
#include <KoParagraphStyle.h>
#include <KoTextDocument.h>
#include <KoStyleManager.h>

#include <QTextFrame>

static QList<SortKeyPair> sortKeys;

BibliographyGenerator::BibliographyGenerator(QTextDocument *bibDocument, const QTextBlock &block, KoBibliographyInfo *bibInfo)
    : QObject(bibDocument)
    , m_bibDocument(bibDocument)
    , m_bibInfo(bibInfo)
    , m_block(block)
{
    Q_ASSERT(bibDocument);
    Q_ASSERT(bibInfo);

    m_bibInfo->setGenerator(this);

    bibDocument->setUndoRedoEnabled(false);
    generate();
}

BibliographyGenerator::~BibliographyGenerator()
{
    delete m_bibInfo;
}

static bool compare_on(int keyIndex, KoInlineCite *c1, KoInlineCite *c2)
{
    if ( keyIndex == sortKeys.size() ) return false;
    else if (sortKeys[keyIndex].second == Qt::AscendingOrder) {
        if (c1->dataField( sortKeys[keyIndex].first ) < c2->dataField( sortKeys[keyIndex].first )) return true;
        else if (c1->dataField( sortKeys[keyIndex].first ) > c2->dataField( sortKeys[keyIndex].first )) return false;
    } else if (sortKeys[keyIndex].second == Qt::DescendingOrder) {
        if (c1->dataField( sortKeys[keyIndex].first ) < c2->dataField( sortKeys[keyIndex].first )) return false;
        else if (c1->dataField( sortKeys[keyIndex].first ) > c2->dataField( sortKeys[keyIndex].first )) return true;
    } else return compare_on( keyIndex + 1, c1, c2 );

    return false;
}

static bool lessThan(KoInlineCite *c1, KoInlineCite *c2)
{
    return compare_on(0, c1, c2);
}

static QList<KoInlineCite *> sort(QList<KoInlineCite *> cites, QList<SortKeyPair> keys)
{
    sortKeys = keys;
    sortKeys << QPair<QString, Qt::SortOrder>("identifier", Qt::AscendingOrder);
    std::sort(cites.begin(), cites.end(), lessThan);
    return cites;
}

void BibliographyGenerator::generate()
{
    if (!m_bibInfo)
        return;

    QTextCursor cursor = m_bibDocument->rootFrame()->lastCursorPosition();
    cursor.setPosition(m_bibDocument->rootFrame()->firstPosition(), QTextCursor::KeepAnchor);
    cursor.beginEditBlock();

    KoTextDocument koDocument(m_block.document());
    KoStyleManager *styleManager = koDocument.styleManager();

    if (!m_bibInfo->m_indexTitleTemplate.text.isNull()) {
        KoParagraphStyle *titleStyle = styleManager->paragraphStyle(m_bibInfo->m_indexTitleTemplate.styleId);
        if (!titleStyle) {
            titleStyle = styleManager->defaultBibliographyTitleStyle();
            m_bibInfo->m_indexTitleTemplate.styleName = titleStyle->name();
        }

        QTextBlock titleTextBlock = cursor.block();
        titleStyle->applyStyle(titleTextBlock);

        cursor.insertText(m_bibInfo->m_indexTitleTemplate.text);
        cursor.insertBlock();
    }

    QTextCharFormat savedCharFormat = cursor.charFormat();

    QList<KoInlineCite*> citeList;
    if (styleManager->bibliographyConfiguration()->sortByPosition()) {
        citeList = koDocument.inlineTextObjectManager()->
            citationsSortedByPosition(false, m_block.document()->firstBlock());
    } else {
        citeList = sort(koDocument.inlineTextObjectManager()->
                        citationsSortedByPosition(false, m_block.document()->firstBlock()),
                        koDocument.styleManager()->bibliographyConfiguration()->sortKeys());
    }

    foreach (KoInlineCite *cite, citeList)
    {
        KoParagraphStyle *bibTemplateStyle = 0;
        BibliographyEntryTemplate bibEntryTemplate;
        if (m_bibInfo->m_entryTemplate.contains(cite->bibliographyType())) {

            bibEntryTemplate = m_bibInfo->m_entryTemplate[cite->bibliographyType()];

            bibTemplateStyle = styleManager->paragraphStyle(bibEntryTemplate.styleId);
            if (bibTemplateStyle == 0) {
                bibTemplateStyle = styleManager->defaultBibliographyEntryStyle(bibEntryTemplate.bibliographyType);
                bibEntryTemplate.styleName = bibTemplateStyle->name();
            }
        } else {
            continue;
        }

        cursor.insertBlock(QTextBlockFormat(),QTextCharFormat());

        QTextBlock bibEntryTextBlock = cursor.block();
        bibTemplateStyle->applyStyle(bibEntryTextBlock);
        bool spanEnabled = false;           //true if data field is not empty

        foreach (IndexEntry * entry, bibEntryTemplate.indexEntries) {
            switch(entry->name) {
                case IndexEntry::BIBLIOGRAPHY: {
                    IndexEntryBibliography *indexEntry = static_cast<IndexEntryBibliography *>(entry);
                    cursor.insertText(QString(((spanEnabled)?" ":"")).append(cite->dataField(indexEntry->dataField)));
                    spanEnabled = !cite->dataField(indexEntry->dataField).isEmpty();
                    break;
                }
                case IndexEntry::SPAN: {
                    if(spanEnabled) {
                        IndexEntrySpan *span = static_cast<IndexEntrySpan*>(entry);
                        cursor.insertText(span->text);
                    }
                    break;
                }
                case IndexEntry::TAB_STOP: {
                    IndexEntryTabStop *tabEntry = static_cast<IndexEntryTabStop*>(entry);

                    cursor.insertText("\t");

                    QTextBlockFormat blockFormat = cursor.blockFormat();
                    QList<QVariant> tabList;
                    if (tabEntry->m_position == "MAX") {
                        tabEntry->tab.position = m_maxTabPosition;
                    } else {
                        tabEntry->tab.position = tabEntry->m_position.toDouble();
                    }
                    tabList.append(QVariant::fromValue<KoText::Tab>(tabEntry->tab));
                    blockFormat.setProperty(KoParagraphStyle::TabPositions, QVariant::fromValue<QList<QVariant> >(tabList));
                    cursor.setBlockFormat(blockFormat);
                    break;
                }
                default:{
                    break;
                }
            }
        }// foreach
    }
    cursor.setCharFormat(savedCharFormat);   // restore the cursor char format
}

QMap<QString, BibliographyEntryTemplate> BibliographyGenerator::defaultBibliographyEntryTemplates()
{
    QMap<QString, BibliographyEntryTemplate> entryTemplates;
    foreach (const QString &bibType, KoOdfBibliographyConfiguration::bibTypes) {
        BibliographyEntryTemplate bibEntryTemplate;

        //Now creating default IndexEntries for all BibliographyEntryTemplates
        IndexEntryBibliography *identifier = new IndexEntryBibliography(QString());
        IndexEntryBibliography *author = new IndexEntryBibliography(QString());
        IndexEntryBibliography *title = new IndexEntryBibliography(QString());
        IndexEntryBibliography *year = new IndexEntryBibliography(QString());
        IndexEntrySpan *firstSpan = new IndexEntrySpan(QString());
        IndexEntrySpan *otherSpan = new IndexEntrySpan(QString());

        identifier->dataField = "identifier";
        author->dataField = "author";
        title->dataField = "title";
        year->dataField = "year";
        firstSpan->text = ":";
        otherSpan->text = ",";

        bibEntryTemplate.bibliographyType = bibType;
        bibEntryTemplate.indexEntries.append(static_cast<IndexEntry *>(identifier));
        bibEntryTemplate.indexEntries.append(static_cast<IndexEntry *>(firstSpan));
        bibEntryTemplate.indexEntries.append(static_cast<IndexEntry *>(author));
        bibEntryTemplate.indexEntries.append(static_cast<IndexEntry *>(otherSpan));
        bibEntryTemplate.indexEntries.append(static_cast<IndexEntry *>(title));
        bibEntryTemplate.indexEntries.append(static_cast<IndexEntry *>(otherSpan));
        bibEntryTemplate.indexEntries.append(static_cast<IndexEntry *>(year));

        entryTemplates[bibType] = bibEntryTemplate;
    }
    return entryTemplates;
}
