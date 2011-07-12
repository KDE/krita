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

BibliographyGenerator::BibliographyGenerator(QTextDocument *bibDocument, QTextBlock block, KoBibliographyInfo *bibInfo)
    : QObject(bibDocument)
    , m_bibDocument(bibDocument)
    , m_bibInfo(bibInfo)
    , m_block(block)
{
    Q_ASSERT(bibFrame);
    Q_ASSERT(bibInfo);

    m_bibInfo->setGenerator(this);

    bibDocument->setUndoRedoEnabled(false);
    bibDocument->setDocumentLayout(new BibDocumentLayout(bibDocument));
    m_documentLayout = static_cast<KoTextDocumentLayout *>(m_block.document()->documentLayout());
    m_document = m_documentLayout->document();

    connect(m_documentLayout, SIGNAL(finishedLayout()), this, SLOT(generate()));

    // do a generate right now to have a Bibliography with placeholder numbers.
    generate();
}

BibliographyGenerator::~BibliographyGenerator()
{
    delete m_bibInfo;
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
        cursor.insertBlock(QTextBlockFormat(), QTextCharFormat());
    }

    QTextBlock block = m_document->rootFrame()->firstCursorPosition().block();
    int blockId = 0;
    while (block.isValid()) {
        QString bibEntryText = block.text();
        bibEntryText.remove(QChar::ObjectReplacementCharacter);
        bibEntryText.replace('\t',' ');
        bibEntryText = removeWhitespacePrefix(bibEntryText);        
        //if (block.blockFormat().hasProperty(KoParagraphStyle::OutlineLevel) && !bibEntryText.isEmpty()) {
        if (!bibEntryText.isEmpty()) {
            qDebug() << "bibentrytext is " << bibEntryText << endl;

            KoParagraphStyle *bibTemplateStyle = 0;
            qDebug() << "size is " << m_bibInfo->m_entryTemplate.size() << endl;
            BibliographyEntryTemplate bibEntryTemplate = m_bibInfo->m_entryTemplate["article"];     //default bib type.temporarily

            bibTemplateStyle = styleManager->paragraphStyle(bibEntryTemplate.styleId);
            if (bibTemplateStyle == 0) {
                bibTemplateStyle = generateTemplateStyle(styleManager);
            }

            cursor.insertBlock(QTextBlockFormat(), QTextCharFormat());
            QTextBlock bibEntryTextBlock = cursor.block();
            bibTemplateStyle->applyStyle( bibEntryTextBlock );

            //KoTextBlockData *bd = dynamic_cast<KoTextBlockData *>(block.userData());

            // save the current style due to hyperlinks
            QTextCharFormat savedCharFormat = cursor.charFormat();
            foreach (IndexEntry *entry, bibEntryTemplate.indexEntries) {
                switch(entry->name) {
                    case IndexEntry::BIBLIOGRAPHY: {
                        // have to add some code here acc. to bib type.
                        cursor.insertText(bibEntryText);
                        break;
                    }
                    case IndexEntry::SPAN: {
                        IndexEntrySpan * span = static_cast<IndexEntrySpan*>(entry);
                        cursor.insertText(span->text);
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
    KoTextLayoutRootArea *rootArea = m_documentLayout->rootAreaForPosition(m_block.position());

    if (rootArea) {
        rootArea->setDirty();
    }
}

BibDocumentLayout::BibDocumentLayout(QTextDocument *doc)
        : QAbstractTextDocumentLayout(doc)
{
    setPaintDevice(new KoPostscriptPaintDevice());
}

BibDocumentLayout::~BibDocumentLayout()
{
}

QRectF BibDocumentLayout::blockBoundingRect(const QTextBlock &block) const
{
    Q_UNUSED(block);
    return QRect();
}

QSizeF BibDocumentLayout::documentSize() const
{
    return QSizeF();
}

void BibDocumentLayout::draw(QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context)
{
    // WARNING Text shapes ask their root area directly to paint.
    // It saves a lot of extra traversal, that is quite costly for big
    // documents
    Q_UNUSED(painter);
    Q_UNUSED(context);
}


int BibDocumentLayout::hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const
{
    Q_UNUSED(point);
    Q_UNUSED(accuracy);
    Q_ASSERT(false); //we should not call this method.
    return -1;
}

QRectF BibDocumentLayout::frameBoundingRect(QTextFrame*) const
{
    return QRectF();
}

int BibDocumentLayout::pageCount() const
{
    return 1;
}

void BibDocumentLayout::documentChanged(int position, int charsRemoved, int charsAdded)
{
    Q_UNUSED(position);
    Q_UNUSED(charsRemoved);
    Q_UNUSED(charsAdded);
}

/*
void BibDocumentLayout::drawInlineObject(QPainter *, const QRectF &, QTextInlineObject , int , const QTextFormat &)
{
}

// This method is called by qt every time  QTextLine.setWidth()/setNumColums() is called
void BibDocumentLayout::positionInlineObject(QTextInlineObject , int , const QTextFormat &)
{
}

void BibDocumentLayout::resizeInlineObject(QTextInlineObject , int , const QTextFormat &)
{
}
*/

