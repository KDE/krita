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
#include <klocale.h>

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

#include <QTextFrame>
#include <QTimer>
#include <KDebug>
#include <KoBookmark.h>
#include <KoInlineTextObjectManager.h>

BibliographyGenerator::BibliographyGenerator(QTextFrame *bibFrame, KoBibliographyInfo *bibInfo)
    : QObject(bibFrame)
    , m_bibFrame(bibFrame)
    , m_bibInfo(bibInfo)
{
    Q_ASSERT(bibFrame);
    Q_ASSERT(bibInfo);

    m_bibInfo->setGenerator(this);

    // connect to FinishedLayout
    KoTextDocumentLayout *docLayout = static_cast<KoTextDocumentLayout *>(bibFrame->document()->documentLayout());
    QObject::connect(docLayout, SIGNAL(finishedLayout()), this, SLOT(generate()));

    // do a generate right now to have a Bibliography with placeholder numbers.
    QTimer::singleShot(0, this, SLOT(generate()));
}

BibliographyGenerator::~BibliographyGenerator()
{
    //delete m_bibInfo;
}

static QString removeWhitespacePrefix(const QString& text)
{
    int firstNonWhitespaceCharIndex = 0;
    int lenght = text.length();
    while (firstNonWhitespaceCharIndex < lenght && text.at(firstNonWhitespaceCharIndex).isSpace()) {
        firstNonWhitespaceCharIndex++;
    }
    return text.right(lenght - firstNonWhitespaceCharIndex);
}

static KoParagraphStyle *generateTemplateStyle(KoStyleManager *styleManager) {
    KoParagraphStyle *style = new KoParagraphStyle();
//    style->setName("Contents " + QString::number(outlineLevel));
    style->setName("Bibliography_20 ");
    style->setParent(styleManager->paragraphStyle("Standard"));
//    style->setLeftMargin(QTextLength(QTextLength::FixedLength, (outlineLevel - 1) * 8));
    styleManager->add(style);
    return style;
}

void BibliographyGenerator::generate()
{
    if (!m_bibInfo)
        return;

    KoTextEditor &cursor = *KoTextDocument(m_bibFrame->document()).textEditor();
    cursor.setPosition(m_bibFrame->firstPosition(), QTextCursor::KeepAnchor);
    cursor.beginEditBlock();

    QTextDocument *doc = m_bibFrame->document();
    KoTextDocument koDocument(doc);
    KoStyleManager *styleManager = koDocument.styleManager();

    if (!m_bibInfo->m_indexTitleTemplate.text.isNull()) {
        KoParagraphStyle *titleStyle = styleManager->paragraphStyle(m_bibInfo->m_indexTitleTemplate.styleId);
        if (!titleStyle) {
            titleStyle = styleManager->defaultParagraphStyle();
        }

        QTextBlock titleTextBlock = cursor.block();
        titleStyle->applyStyle(titleTextBlock);

        cursor.insertText(m_bibInfo->m_indexTitleTemplate.text);
        cursor.insertBlock(QTextBlockFormat(), QTextCharFormat());
    }

    QTextBlock block = m_bibFrame->lastCursorPosition().block();
    int blockId = 0;
    while (block.isValid()) {
        QString bibEntryText = block.text();
        bibEntryText.remove(QChar::ObjectReplacementCharacter);
        bibEntryText.replace('\t',' ');
        bibEntryText = removeWhitespacePrefix(bibEntryText);        
        //if (block.blockFormat().hasProperty(KoParagraphStyle::OutlineLevel) && !bibEntryText.isEmpty()) {
        if (!bibEntryText.isEmpty()) {
            qDebug() << "bibentrytext is " << bibEntryText << endl;
            cursor.insertBlock(QTextBlockFormat(), QTextCharFormat());

            KoParagraphStyle *bibTemplateStyle = 0;
            qDebug() << "size is " << m_bibInfo->m_entryTemplate.size() << endl;
            BibliographyEntryTemplate bibEntryTemplate = m_bibInfo->m_entryTemplate["article"];     //default bib type.temporarily

            bibTemplateStyle = styleManager->paragraphStyle(bibEntryTemplate.styleId);
            if (bibTemplateStyle == 0) {
                bibTemplateStyle = generateTemplateStyle(styleManager);
            }

            QTextBlock bibEntryTextBlock = cursor.block();
            bibTemplateStyle->applyStyle( bibEntryTextBlock );

            //KoTextBlockData *bd = dynamic_cast<KoTextBlockData *>(block.userData());

            // save the current style due to hyperlinks
            QTextCharFormat savedCharFormat = cursor.charFormat();
            foreach (IndexEntry *entry, bibEntryTemplate.indexEntries) {
                switch(entry->name) {
                    case IndexEntry::BIBLIOGRAPHY: {
                        // have to add some code here acc. to bib type.
                        break;
                    }
                    case IndexEntry::SPAN: {
                        IndexEntrySpan * span = static_cast<IndexEntrySpan*>(entry);
                        cursor.insertText(span->text);
                        break;
                    }
                    case IndexEntry::TEXT: {
                        //IndexEntryText * text = static_cast<IndexEntryText*>(entry);
                        cursor.insertText(bibEntryText);
                        break;
                    }
                    case IndexEntry::TAB_STOP: {
                        cursor.insertText("\t");
                        break;
                    }
                    default:{
                        cursor.insertText("ERR nothing "+entry->name);
                        qDebug() << "New or unknown index entry";
                        break;
                    }
                }
            }// foreach
            cursor.setCharFormat(savedCharFormat);   // restore the cursor char format
        }
        block = block.next();
    }
    cursor.endEditBlock();
}


