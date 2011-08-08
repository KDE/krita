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
#include "DummyDocumentLayout.h"
#include <klocale.h>
#include <kdebug.h>

#include "KoTextDocumentLayout.h"
#include "KoTextLayoutRootArea.h"
#include "KoTextShapeData.h"
#include <KoParagraphStyle.h>
#include <KoTextPage.h>
#include <KoShape.h>
#include <KoTextDocument.h>
#include <KoTextBlockData.h>
#include <KoStyleManager.h>
#include <KoTextEditor.h>
#include <KoPostscriptPaintDevice.h>

#include <QTextFrame>
#include <QTimer>
#include <KDebug>
#include <KoBookmark.h>
#include <KoInlineTextObjectManager.h>

BibliographyGenerator::BibliographyGenerator(QTextDocument *bibDocument, QTextBlock block, KoBibliographyInfo *bibInfo, const QTextDocument *doc)
    : QObject(bibDocument)
    , document(doc)
    , m_bibDocument(bibDocument)
    , m_bibInfo(bibInfo)
    , m_block(block)
{
    Q_ASSERT(bibDocument);
    Q_ASSERT(bibInfo);

    m_bibInfo->setGenerator(this);

    bibDocument->setUndoRedoEnabled(false);
    bibDocument->setDocumentLayout(new DummyDocumentLayout(bibDocument));

    m_documentLayout = static_cast<KoTextDocumentLayout *>(m_block.document()->documentLayout());
    m_document = m_documentLayout->document();

    connect(m_documentLayout, SIGNAL(finishedLayout()), this, SLOT(generate()));

    // do a generate right now to have a Bibliography with placeholder numbers.
    //generate();
}

BibliographyGenerator::~BibliographyGenerator()
{
    delete m_bibInfo;
}

static KoParagraphStyle *generateTemplateStyle(KoStyleManager *styleManager,QString bibType) {
    KoParagraphStyle *style = new KoParagraphStyle();
    style->setName("Bibliography_"+bibType);
    style->setParent(styleManager->paragraphStyle("Standard"));
    styleManager->add(style);
    return style;
}

void BibliographyGenerator::generate()
{
    if (!m_bibInfo)
        return;

    QTextCursor cursor = m_bibDocument->rootFrame()->lastCursorPosition();
    cursor.setPosition(m_bibDocument->rootFrame()->firstPosition(), QTextCursor::KeepAnchor);
    cursor.beginEditBlock();

    KoStyleManager *styleManager = KoTextDocument(m_document).styleManager();

    if (!m_bibInfo->m_indexTitleTemplate.text.isNull()) {
        KoParagraphStyle *titleStyle = styleManager->paragraphStyle(m_bibInfo->m_indexTitleTemplate.styleId);
        if (!titleStyle) {
            titleStyle = styleManager->defaultParagraphStyle();
        }

        QTextBlock titleTextBlock = cursor.block();
        titleStyle->applyStyle(titleTextBlock);

        cursor.insertText(m_bibInfo->m_indexTitleTemplate.text);
        //cursor.insertBlock(QTextBlockFormat(), QTextCharFormat());
    }

    QTextBlock block = m_bibDocument->rootFrame()->firstCursorPosition().block();
    QTextCharFormat savedCharFormat = cursor.charFormat();
    QList<KoInlineCite*> citeList = KoTextDocument(document).inlineTextObjectManager()->citations(false).values();

    qDebug() << "TOTAL CITES DETECTED AT BIB IS " <<citeList.size() << "\n" << m_bibInfo->m_indexTitleTemplate.text;
    foreach (KoInlineCite *cite, citeList)
    {
        KoParagraphStyle *bibTemplateStyle = 0;
        BibliographyEntryTemplate bibEntryTemplate;
        if (m_bibInfo->m_entryTemplate.keys().contains(cite->bibliographyType())) {
            // List's index starts with 0, outline level starts with 0
            bibEntryTemplate = m_bibInfo->m_entryTemplate.value(cite->bibliographyType());

            bibTemplateStyle = styleManager->paragraphStyle(bibEntryTemplate.styleId);
            if (bibTemplateStyle == 0) {
                bibTemplateStyle = generateTemplateStyle(styleManager, cite->bibliographyType());
            }
        } else {
            qDebug() << "Bibliography meta-data has not BibliographyEntryTemplate for " << cite->bibliographyType();
            continue;
        }
            \
        cursor.insertBlock(QTextBlockFormat(), QTextCharFormat());
        //cursor.insertBlock(cursor.blockFormat(),cursor.charFormat());

        QTextBlock bibEntryTextBlock = cursor.block();
        bibTemplateStyle->applyStyle( bibEntryTextBlock );
        bool spanEnabled = false;           //true if data field value is not blank
        QString deb;
        foreach (IndexEntry * entry, bibEntryTemplate.indexEntries) {
            switch(entry->name) {
                case IndexEntry::BIBLIOGRAPHY: {
                    IndexEntryBibliography *indexEntry = static_cast<IndexEntryBibliography *>(entry);
                    cursor.insertText(cite->dataField(indexEntry->dataField));
                    deb.append(cite->dataField(indexEntry->dataField));
                    spanEnabled = (cite->dataField(indexEntry->dataField).length()>0);
                    break;
                }
                case IndexEntry::SPAN: {
                    if(spanEnabled) {
                        IndexEntrySpan *span = static_cast<IndexEntrySpan*>(entry);
                        cursor.insertText(span->text);
                        deb.append(span->text);
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
                    qDebug() << "New or unknown index entry";
                    break;
                }
            }
        }// foreach
        qDebug() << "\t" << deb;
    }
    cursor.setCharFormat(savedCharFormat);   // restore the cursor char format
}





