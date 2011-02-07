/* This file is part of the KDE project
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 Jean Nicolas Artaud <jean.nicolas.artaud@kogmbh.com>
 * Copyright (C) 2011 Pavol Korinek <pavol.korinek@ixonos.com>
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

#include "ToCGenerator.h"
#include <klocale.h>

#include <KoParagraphStyle.h>
#include <KoTextDocumentLayout.h>
#include <KoTextShapeData.h>
#include <KoTextPage.h>
#include <KoShape.h>
#include <KoTextDocument.h>
#include <KoTextBlockData.h>
#include <KoStyleManager.h>
#include <KoTextLoader.h>



#include <QTextFrame>
#include <QTimer>
#include <KDebug>
#include <KoBookmark.h>
#include <KoInlineTextObjectManager.h>

static const QString INVALID_HREF_TARGET = "INVALID_HREF";

ToCGenerator::ToCGenerator(QTextFrame *tocFrame)
    : QObject(tocFrame),
    m_state(NeverGeneratedState),
    m_ToCFrame(tocFrame),
    m_tocInfo(0),
    m_pageWidthPt(0.0)
{
    Q_ASSERT(tocFrame);
/*
    // do a generate right now to have a ToC with placeholder numbers.
    QTimer::singleShot(0, this, SLOT(documentLayoutFinished()));

    // disabled for now as this requires us to update the list items in 'update' too
*/
}

ToCGenerator::~ToCGenerator()
{
    delete m_tocInfo;
}


void ToCGenerator::setPageWidth(qreal pageWidthPt)
{
    m_pageWidthPt = pageWidthPt;
}


void ToCGenerator::documentLayoutFinished()
{
    switch (m_state) {
    case DirtyState:
    case NeverGeneratedState:
        generate();
        m_state = WithoutPageNumbersState;
        break;
    case WithoutPageNumbersState:
        update();
        m_state = GeneratedState;
    case GeneratedState:
        break;
    };
}

static KoParagraphStyle *generateTemplateStyle(KoStyleManager *styleManager, int outlineLevel) {
    KoParagraphStyle *style = new KoParagraphStyle();
    style->setName("Contents " + QString::number(outlineLevel));
    style->setParent(styleManager->paragraphStyle("Standard"));
    style->setLeftMargin((outlineLevel - 1) * 8);
    styleManager->add(style);
    return style;
}

QString ToCGenerator::fetchBookmarkRef(QTextBlock block, KoInlineTextObjectManager* inlineTextObjectManager)
{
    QTextBlock::iterator it;
    for (it = block.begin(); !(it.atEnd()); ++it) {
        QTextFragment currentFragment = it.fragment();
        // most possibly inline object
        if (currentFragment.text()[0].unicode() == QChar::ObjectReplacementCharacter && currentFragment.isValid()) {
            KoInlineObject *inlineObject = inlineTextObjectManager->inlineTextObject( currentFragment.charFormat() );
            KoBookmark * isBookmark = dynamic_cast<KoBookmark*>(inlineObject);
            if (isBookmark) {
                return isBookmark->name();
                break;
            }
        }
    }

    return QString();
}


void ToCGenerator::generate()
{
    QTextCursor cursor = m_ToCFrame->lastCursorPosition();
    cursor.setPosition(m_ToCFrame->firstPosition(), QTextCursor::KeepAnchor);
    cursor.beginEditBlock();

    QVariant data = cursor.currentFrame()->format().property(KoText::TableOfContentsData);
    m_tocInfo = data.value<KoTableOfContentsGeneratorInfo*>();
    TableOfContent * m_tocData = m_tocInfo->tableOfContentData();

    QTextDocument *doc = m_ToCFrame->document();
    KoTextDocument koDocument(doc);
    KoStyleManager *styleManager = koDocument.styleManager();

    if ( !m_tocData->tocSource.indexTitleTemplate.text.isNull() ) {
        KoParagraphStyle *titleStyle = styleManager->paragraphStyle(m_tocData->tocSource.indexTitleTemplate.styleId);
        if (!titleStyle) {
            titleStyle = styleManager->defaultParagraphStyle();
        }

        QTextBlock titleTextBlock = cursor.block();
        titleStyle->applyStyle(titleTextBlock);

        cursor.insertText( m_tocData->tocSource.indexTitleTemplate.text );
        cursor.insertBlock(QTextBlockFormat(), QTextCharFormat());
    }


    // Add TOC
    // Iterate through all blocks to generate TOC
    QTextBlock block = doc->begin();
    int blockId = 0;
    while (block.isValid() && !isWhitespaceOnly(block.text())) {
        // Choose only TOC blocks
        if (block.blockFormat().hasProperty(KoParagraphStyle::OutlineLevel) && !block.text().isEmpty()) {

            cursor.insertBlock(QTextBlockFormat(), QTextCharFormat());

            int outlineLevel = block.blockFormat().intProperty(KoParagraphStyle::OutlineLevel);

            KoParagraphStyle *tocTemplateStyle = 0;
            if (outlineLevel >= 1 && (outlineLevel-1) < m_tocData->tocSource.entryTemplate.size()) {
                // List's index starts with 0, outline level starts with 0
                TocEntryTemplate tocEntryTemplate = m_tocData->tocSource.entryTemplate.at(outlineLevel - 1);
                // ensure that we fetched correct entry template
                Q_ASSERT(tocEntryTemplate.outlineLevel == outlineLevel);
                if (tocEntryTemplate.outlineLevel != outlineLevel) {
                    qDebug() << "TOC outline level not found correctly " << outlineLevel;
                }

                tocTemplateStyle = styleManager->paragraphStyle( tocEntryTemplate.styleId );
                if (tocTemplateStyle == 0) {
                    tocTemplateStyle = generateTemplateStyle(styleManager, outlineLevel);
                }

                QTextBlock tocEntryTextBlock = cursor.block();
                tocTemplateStyle->applyStyle( tocEntryTextBlock );

                KoTextBlockData *bd = dynamic_cast<KoTextBlockData *>(block.userData());

                // save the current style due to hyperlinks
                QTextCharFormat savedCharFormat = cursor.charFormat();
                foreach (IndexEntry * entry,tocEntryTemplate.indexEntries) {
                    switch(entry->name) {
                        case IndexEntry::LINK_START: {
                            //IndexEntryLinkStart * linkStart = static_cast<IndexEntryLinkStart*>(entry);

                            QString target;
                            KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>( block.document()->documentLayout() );
                            if (layout) {
                                target = fetchBookmarkRef(block, layout->inlineTextObjectManager());

                                if (target.isNull()) {
                                    // generate unique name for the bookmark
                                    target = bd->counterText() + block.text() + "|outline" + QString::number(blockId);
                                    blockId++;

                                    // insert new KoBookmark
                                    KoBookmark *bookmark = new KoBookmark(target,block.document());
                                    bookmark->setType(KoBookmark::SinglePosition);
                                    QTextCursor blockCursor(block);
                                    layout->inlineTextObjectManager()->insertInlineObject(blockCursor, bookmark);
                                }

                            }

                            // copy it to alter subset of properties
                            QTextCharFormat linkCf(savedCharFormat);
                            linkCf.setAnchor(true);
                            linkCf.setAnchorHref(target);

                            QBrush foreground = linkCf.foreground();
                            foreground.setColor(Qt::blue);

                            linkCf.setForeground(foreground);
                            linkCf.setProperty(KoCharacterStyle::UnderlineStyle, KoCharacterStyle::SolidLine);
                            linkCf.setProperty(KoCharacterStyle::UnderlineType, KoCharacterStyle::SingleLine);
                            cursor.setCharFormat(linkCf);
                            break;
                        }
                        case IndexEntry::CHAPTER: {
                            //IndexEntryChapter * chapter = static_cast<IndexEntryChapter*>(entry);
                            cursor.insertText(bd->counterText());
                            break;
                        }
                        case IndexEntry::SPAN: {
                            IndexEntrySpan * span = static_cast<IndexEntrySpan*>(entry);
                            cursor.insertText(span->text);
                            break;
                        }
                        case IndexEntry::TEXT: {
                            //IndexEntryText * text = static_cast<IndexEntryText*>(entry);
                            QString text = block.text();
                            // causes problems when rendering the tabstop
                            // see Layout::decorateParagraph
                            text.remove(QChar::ObjectReplacementCharacter);
                            // some headings contain tabs, replace them with space
                            text.replace('\t',' ');
                            cursor.insertText(text);
                            break;
                        }
                        case IndexEntry::TAB_STOP: {
                            IndexEntryTabStop * tabStop = static_cast<IndexEntryTabStop*>(entry);

                            QList<QVariant> tabStops;
                            if (tabStop->tab.type == QTextOption::RightTab){
                                tabStop->tab.position = m_pageWidthPt;
                            }

                            tabStops.append( QVariant::fromValue<KoText::Tab>(tabStop->tab) );

                            QTextBlockFormat blockFormat = cursor.blockFormat();
                            blockFormat.setProperty(KoParagraphStyle::TabPositions, tabStops);
                            cursor.setBlockFormat(blockFormat);
                            cursor.insertText("\t");
                            break;
                        }
                        case IndexEntry::PAGE_NUMBER: {
                            //IndexEntryPageNumber * pageNumber = static_cast<IndexEntryPageNumber*>(entry);
                            cursor.insertText( QString(QChar::ReplacementCharacter) );
                            break;
                        }
                        case IndexEntry::LINK_END: {
                            //IndexEntryLinkEnd * linkEnd = static_cast<IndexEntryLinkEnd*>(entry);
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

                m_originalBlocksInToc.append(qMakePair(cursor.block(), block));

            } else {
                qDebug() << "Invalid outline level: " << outlineLevel;
            }
        }
        block = block.next();
    }
    cursor.endEditBlock();
}


void ToCGenerator::update()
{
    QTextDocument *doc = m_ToCFrame->document();
    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(doc->documentLayout());
    QTextCursor cursor = m_ToCFrame->lastCursorPosition();
    cursor.beginEditBlock();
    foreach (const BlockPair &blockPair, m_originalBlocksInToc) {
        QTextBlock tocEntryBlock = blockPair.first;
        QTextBlock headingBlock = blockPair.second;
        KoShape *shape = layout->shapeForPosition(headingBlock.position());
        if (shape) {
            KoTextShapeData *shapeData = qobject_cast<KoTextShapeData *>(shape->userData());
            Q_ASSERT(shapeData);
            if (shapeData && shapeData->page()) {
                QString blockText = tocEntryBlock.text();
                int position = blockText.indexOf(QChar::ReplacementCharacter);
                // Replace page number, possibly more than one time in one block
                while (position > -1) {
                    cursor.setPosition(tocEntryBlock.position() + position);
                    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
                    QString pageNumber = QString::number(shapeData->page()->pageNumber());
                    cursor.insertText(pageNumber);
                    position = blockText.indexOf(QChar::ReplacementCharacter, position + 1);
                }
            }
        }
    }
    cursor.endEditBlock();
    m_originalBlocksInToc.clear();
}

bool ToCGenerator::isWhitespaceOnly(QString text) const
{
    text.remove(' ');
    text.remove('\t');
    return text.isEmpty();
}


#include <ToCGenerator.moc>
