/* This file is part of the KDE project
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 Jean Nicolas Artaud <jean.nicolas.artaud@kogmbh.com>
 * Copyright (C) 2011 Pavol Korinek <pavol.korinek@ixonos.com>
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

#include "ToCGenerator.h"

#include <klocalizedstring.h>

#include "KoTextDocumentLayout.h"
#include "KoTextLayoutRootArea.h"
#include "DummyDocumentLayout.h"

#include <KoParagraphStyle.h>
#include <KoTextPage.h>
#include <KoTextDocument.h>
#include <KoTextBlockData.h>
#include <KoStyleManager.h>
#include <KoTableOfContentsGeneratorInfo.h>

#include <QTextDocument>
#include <TextLayoutDebug.h>
#include <KoBookmark.h>
#include <KoTextRangeManager.h>

static const QString INVALID_HREF_TARGET = "INVALID_HREF";

ToCGenerator::ToCGenerator(QTextDocument *tocDocument, KoTableOfContentsGeneratorInfo *tocInfo)
    : QObject(tocDocument)
    , m_ToCDocument(tocDocument)
    , m_ToCInfo(tocInfo)
    , m_document(0)
    , m_documentLayout(0)
{
    Q_ASSERT(tocDocument);
    Q_ASSERT(tocInfo);

    tocDocument->setUndoRedoEnabled(false);
    tocDocument->setDocumentLayout(new DummyDocumentLayout(tocDocument));
    KoTextDocument(tocDocument).setRelativeTabs(tocInfo->m_relativeTabStopPosition);
}

ToCGenerator::~ToCGenerator()
{
    delete m_ToCInfo;
}

void ToCGenerator::setBlock(const QTextBlock &block)
{
    m_block = block;
    m_documentLayout = static_cast<KoTextDocumentLayout *>(m_block.document()->documentLayout());
    m_document = m_documentLayout->document();
}

QString ToCGenerator::fetchBookmarkRef(const QTextBlock &block, KoTextRangeManager *textRangeManager)
{
    QHash<int, KoTextRange *> ranges = textRangeManager->textRangesChangingWithin(block.document(), block.position(), block.position() + block.length(), block.position(), block.position() + block.length());

    foreach (KoTextRange *range, ranges) {
        KoBookmark *bookmark = dynamic_cast<KoBookmark *>(range);
        if (bookmark) {
            return bookmark->name();
        }
    }
    return QString();
}


static QString removeWhitespacePrefix(const QString& text)
{
    int firstNonWhitespaceCharIndex = 0;
    const int length = text.length();
    while (firstNonWhitespaceCharIndex < length && text.at(firstNonWhitespaceCharIndex).isSpace()) {
        firstNonWhitespaceCharIndex++;
    }
    return text.right(length - firstNonWhitespaceCharIndex);
}


bool ToCGenerator::generate()
{
    if (!m_ToCInfo)
        return true;

    m_preservePagebreak = m_ToCDocument->begin().blockFormat().intProperty(KoParagraphStyle::BreakBefore) & KoText::PageBreak;

    m_success = true;

    QTextCursor cursor = m_ToCDocument->rootFrame()->lastCursorPosition();
    cursor.setPosition(m_ToCDocument->rootFrame()->firstPosition(), QTextCursor::KeepAnchor);
    cursor.beginEditBlock();
    cursor.insertBlock(QTextBlockFormat(), QTextCharFormat());

    KoStyleManager *styleManager = KoTextDocument(m_document).styleManager();

    if (!m_ToCInfo->m_indexTitleTemplate.text.isEmpty()) {
        KoParagraphStyle *titleStyle = styleManager->paragraphStyle(m_ToCInfo->m_indexTitleTemplate.styleId);

        // titleStyle == 0? then it might be in unused styles
        if (!titleStyle) {
            titleStyle = styleManager->unusedStyle(m_ToCInfo->m_indexTitleTemplate.styleId); // this should return true only for ToC template preview
        }

        if (!titleStyle) {
            titleStyle = styleManager->defaultTableOfcontentsTitleStyle();
        }

        QTextBlock titleTextBlock = cursor.block();
        titleStyle->applyStyle(titleTextBlock);

        cursor.insertText(m_ToCInfo->m_indexTitleTemplate.text);
        if (m_preservePagebreak) {
            QTextBlockFormat blockFormat;
            blockFormat.setProperty(KoParagraphStyle::BreakBefore, KoText::PageBreak);
            cursor.mergeBlockFormat(blockFormat);
            m_preservePagebreak = false;
        }
        cursor.insertBlock(QTextBlockFormat(), QTextCharFormat());
    }

    // Add TOC
    // Iterate through all blocks to generate TOC
    QTextBlock block = m_document->rootFrame()->firstCursorPosition().block();
    int blockId = 0;
    for (; block.isValid(); block = block.next()) {
        // Choose only TOC blocks
        if (m_ToCInfo->m_useOutlineLevel) {
            if (block.blockFormat().hasProperty(KoParagraphStyle::OutlineLevel)) {
                int level = block.blockFormat().intProperty(KoParagraphStyle::OutlineLevel);
                generateEntry(level, cursor, block, blockId);
                continue;
            }
        }

        if (m_ToCInfo->m_useIndexSourceStyles) {
            bool inserted = false;
            foreach (const IndexSourceStyles &indexSourceStyles, m_ToCInfo->m_indexSourceStyles) {
                foreach (const IndexSourceStyle &indexStyle, indexSourceStyles.styles) {
                    if (indexStyle.styleId == block.blockFormat().intProperty(KoParagraphStyle::StyleId)) {
                        generateEntry(indexSourceStyles.outlineLevel, cursor, block, blockId);
                        inserted = true;
                        break;
                    }
                }
                if (inserted)
                    break;
            }
            if (inserted)
                continue;
        }

        if (m_ToCInfo->m_useIndexMarks) {
            if (false) {
                generateEntry(1, cursor, block, blockId);
                continue;
            }
        }
    }
    cursor.endEditBlock();

    m_documentLayout->documentChanged(m_block.position(),1,1);
    return m_success;
}

static bool compareTab(const QVariant &tab1, const QVariant &tab2)
{
    return tab1.value<KoText::Tab>().position < tab2.value<KoText::Tab>().position;
}


void ToCGenerator::generateEntry(int outlineLevel, QTextCursor &cursor, QTextBlock &block, int &blockId)
{
    KoStyleManager *styleManager = KoTextDocument(m_document).styleManager();

    QString tocEntryText = block.text();
    tocEntryText.remove(QChar::ObjectReplacementCharacter);
    // some headings contain tabs, replace all occurrences with spaces
    tocEntryText.replace('\t',' ').remove(0x200B);
    tocEntryText = removeWhitespacePrefix(tocEntryText);

    // Add only blocks with text
    if (!tocEntryText.isEmpty()) {
        KoParagraphStyle *tocTemplateStyle = 0;

        if (outlineLevel >= 1 && (outlineLevel-1) < m_ToCInfo->m_entryTemplate.size()
                    && outlineLevel <= m_ToCInfo->m_outlineLevel) {
            // List's index starts with 0, outline level starts with 0
            const TocEntryTemplate *tocEntryTemplate = &m_ToCInfo->m_entryTemplate.at(outlineLevel - 1);

            // ensure that we fetched correct entry template
            Q_ASSERT(tocEntryTemplate->outlineLevel == outlineLevel);
            if (tocEntryTemplate->outlineLevel != outlineLevel) {
                qDebug() << "TOC outline level not found correctly " << outlineLevel;
            }

            tocTemplateStyle = styleManager->paragraphStyle(tocEntryTemplate->styleId);
            if (tocTemplateStyle == 0) {
                tocTemplateStyle = styleManager->defaultTableOfContentsEntryStyle(outlineLevel);
            }

            QTextBlockFormat blockFormat;
            if (m_preservePagebreak) {
                blockFormat.setProperty(KoParagraphStyle::BreakBefore, KoText::PageBreak);
                m_preservePagebreak = false;
            }
            cursor.insertBlock(blockFormat, QTextCharFormat());

            QTextBlock tocEntryTextBlock = cursor.block();
            tocTemplateStyle->applyStyle( tocEntryTextBlock );

            KoTextBlockData bd(block);

            // save the current style due to hyperlinks
            QTextCharFormat savedCharFormat = cursor.charFormat();
            foreach (IndexEntry * entry, tocEntryTemplate->indexEntries) {
                switch(entry->name) {
                    case IndexEntry::LINK_START: {
                        //IndexEntryLinkStart *linkStart = static_cast<IndexEntryLinkStart*>(entry);

                        QString target = fetchBookmarkRef(block, m_documentLayout->textRangeManager());

                        if (target.isNull()) {
                            // generate unique name for the bookmark
                            target = tocEntryText + "|outline" + QString::number(blockId);
                            blockId++;

                            // insert new KoBookmark
                            QTextCursor blockCursor(block);
                            KoBookmark *bookmark = new KoBookmark(blockCursor);
                            bookmark->setName(target);
                            m_documentLayout->textRangeManager()->insert(bookmark);
                        }

                        if (!target.isNull()) {
                            // copy it to alter subset of properties
                            QTextCharFormat linkCf(savedCharFormat);
                            linkCf.setAnchor(true);
                            linkCf.setProperty(KoCharacterStyle::AnchorType, KoCharacterStyle::Anchor);
                            linkCf.setAnchorHref('#'+ target);

                            QBrush foreground = linkCf.foreground();
                            foreground.setColor(Qt::blue);

                            linkCf.setForeground(foreground);
                            linkCf.setProperty(KoCharacterStyle::UnderlineStyle, KoCharacterStyle::SolidLine);
                            linkCf.setProperty(KoCharacterStyle::UnderlineType, KoCharacterStyle::SingleLine);
                            cursor.setCharFormat(linkCf);
                        }
                        break;
                    }
                    case IndexEntry::CHAPTER: {
                        //IndexEntryChapter *chapter = static_cast<IndexEntryChapter*>(entry);
                        cursor.insertText(bd.counterText());
                        break;
                    }
                    case IndexEntry::SPAN: {
                        IndexEntrySpan *span = static_cast<IndexEntrySpan*>(entry);
                        cursor.insertText(span->text);
                        break;
                    }
                    case IndexEntry::TEXT: {
                        //IndexEntryText *text = static_cast<IndexEntryText*>(entry);
                        cursor.insertText(tocEntryText);
                        break;
                    }
                    case IndexEntry::TAB_STOP: {
                        IndexEntryTabStop *tabEntry = static_cast<IndexEntryTabStop*>(entry);

                        cursor.insertText("\t");

                        QTextBlockFormat blockFormat = cursor.blockFormat();
                        QList<QVariant> tabList =            (blockFormat.property(KoParagraphStyle::TabPositions)).value<QList<QVariant> >();

                        if (tabEntry->m_position.isEmpty()) {
                            tabEntry->tab.position = KoTextLayoutArea::MaximumTabPos;
                        } // else the position is already parsed into tab.position
                        tabList.append(QVariant::fromValue<KoText::Tab>(tabEntry->tab));
                        qSort(tabList.begin(), tabList.end(), compareTab);
                        blockFormat.setProperty(KoParagraphStyle::TabPositions, QVariant::fromValue<QList<QVariant> >(tabList));
                        cursor.setBlockFormat(blockFormat);
                        break;
                    }
                    case IndexEntry::PAGE_NUMBER: {
                        //IndexEntryPageNumber *pageNumber = static_cast<IndexEntryPageNumber*>(entry);
                        cursor.insertText(resolvePageNumber(block));
                        break;
                    }
                    case IndexEntry::LINK_END: {
                        //IndexEntryLinkEnd *linkEnd = static_cast<IndexEntryLinkEnd*>(entry);
                        cursor.setCharFormat(savedCharFormat);
                        break;
                    }
                    default:{
                        qDebug() << "New or unknown index entry";
                        break;
                    }
                }
            }// foreach
            cursor.setCharFormat(savedCharFormat);   // restore the cursor char format
        }
    }
}

QString ToCGenerator::resolvePageNumber(const QTextBlock &headingBlock)
{
    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(m_document->documentLayout());
    KoTextLayoutRootArea *rootArea = layout->rootAreaForPosition(headingBlock.position());
    if (rootArea) {
        if (rootArea->page()) {
            return QString::number(rootArea->page()->visiblePageNumber());
        }
    else qDebug()<<"had root but no page";
    }
    m_success = false;
    return "###";
}
