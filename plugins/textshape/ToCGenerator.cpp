/* This file is part of the KDE project
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 Jean Nicolas Artaud <jean.nicolas.artaud@kogmbh.com>
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

#include <KoTableOfContentsGeneratorInfo.h>

#include <QTextFrame>
#include <QTimer>
#include <KDebug>

ToCGenerator::ToCGenerator(QTextFrame *tocFrame)
    : QObject(tocFrame),
    m_state(NeverGeneratedState),
    m_ToCFrame(tocFrame)
{
    Q_ASSERT(tocFrame);
/*
    // do a generate right now to have a ToC with placeholder numbers.
    QTimer::singleShot(0, this, SLOT(documentLayoutFinished()));

    // disabled for now as this requires us to update the list items in 'update' too
*/
}

void ToCGenerator::documentLayoutFinished()
{
    switch (m_state) {
    case DirtyState:
    case NeverGeneratedState:
        //qDebug() << "TESTX TOC GENERATE\n";
        generate();
        m_state = WithoutPageNumbersState;
        break;
    case WithoutPageNumbersState:
        //qDebug() << "TESTX TOC UPDATE\n";
        update();
        m_state = GeneratedState;
    case GeneratedState:
        //qDebug() << "TESTX TOC --- \n";
        break;
    };
}

static QVariant attribute(const QVariant &data, const QString &tagName, const QString &attrName, int outlineLevel = 0) {
    QVariantHash tagMap = data.toHash();
    QString outlineLevelTagName = tagName;
    if (outlineLevel > 0) {
        outlineLevelTagName = outlineLevelTagName + " " + QString::number(outlineLevel);
    }
    QVariant tag = tagMap.value(outlineLevelTagName);
    //qDebug() << "TESTX KEY " << outlineLevelTagName;
    //qDebug() << "TESTX TAG " << tag;
    QVariant attr;
    if (tag.isValid()) {
        QVariantHash attrMap = tag.toHash();
        attr = attrMap.value(attrName);
    }
    //qDebug() << "TESTX ATTR " << attr;
    return attr;
}

static KoParagraphStyle *getStyle(KoStyleManager *styleManager, const QVariant &styleVariant) {
    KoParagraphStyle *style = 0;
    if (styleVariant.isValid()) {
        int styleId = styleVariant.toInt();
        //qDebug() << "TESTX TITLE STYLE ID " << styleId;
        style = styleManager->paragraphStyle(styleId);
    }
    return style;
}

static KoParagraphStyle *generateTemplateStyle(KoStyleManager *styleManager, int outlineLevel) {
    KoParagraphStyle *style = new KoParagraphStyle();
    style->setName("Contents " + QString::number(outlineLevel));
    style->setParent(styleManager->paragraphStyle("Standard"));
    style->setLeftMargin((outlineLevel - 1) * 8);
    styleManager->add(style);
    return style;
}

void ToCGenerator::generate()
{
    // Add a frame to the current layout
    //QTextFrameFormat tocFormat = m_ToCFrame->frameFormat();
    QTextCursor cursor = m_ToCFrame->lastCursorPosition();    
    cursor.setPosition(m_ToCFrame->firstPosition(), QTextCursor::KeepAnchor);
    cursor.beginEditBlock();
    // Toc elements hierarchy
    // table-of-content
    // text:table-of-content-source
    //   text:index-title-template
    //   text:table-of-content-entry-template
    //     text:index-entry
    QVariant data = cursor.currentFrame()->format().property(KoText::TableOfContentsData);
    TableOfContent * tocDescription = data.value<TableOfContent*>();
    
    QTextDocument *doc = m_ToCFrame->document();
    KoTextDocument koDocument(doc);
    KoStyleManager *styleManager = koDocument.styleManager();
    //TODO pavolk: apply section toc style
    //QVariant tocStyleVariant = attribute(tocVariant, "table-of-content", "style-name");
    //qDebug() << "TESTX TOC STYLE VARIANT " << tocStyleVariant;
    //KoSectionStyle *tocStyle = getStyle(styleManager, tocStyleVariant);
    //QTextBlock tocTextBlock = cursor.block();
    //tocStyle->applyStyle(cursorFrame);
    //cursor.insertBlock(QTextBlockFormat(), QTextCharFormat());
    // Add TOC title
    if ( !tocDescription->tocSource.indexTitleTemplate.text.isNull() ){
        KoParagraphStyle *titleStyle = styleManager->paragraphStyle(tocDescription->tocSource.indexTitleTemplate.styleId);
        // Do we want default style or generate style also for title ?
        if (!titleStyle) {
            titleStyle = styleManager->defaultParagraphStyle();
            //qDebug() << "TESTX USING DEFALT STYLE " << titleStyle->styleId();
        } 
        
        QTextBlock titleTextBlock = cursor.block();
        titleStyle->applyStyle(titleTextBlock);
        qDebug() << "\t\t\tInserting text " << tocDescription->tocSource.indexTitleTemplate.text;
        cursor.insertText( tocDescription->tocSource.indexTitleTemplate.text );
        cursor.insertBlock(QTextBlockFormat(), QTextCharFormat());
    }
    

    // Add TOC
    // Iterate through all blocks to generate TOC
    QTextBlock block = doc->begin();
    while (block.isValid()) {
        
        KoTextBlockData *bd = dynamic_cast<KoTextBlockData *>(block.userData());
        // Choose only TOC blocks
        if (bd && bd->hasCounterData()) {
            cursor.insertBlock(QTextBlockFormat(), QTextCharFormat());
            
            int outlineLevel = block.blockFormat().intProperty(KoParagraphStyle::OutlineLevel);

            KoParagraphStyle *tocTemplateStyle = 0;
            if (outlineLevel >= 1 && (outlineLevel-1) < tocDescription->tocSource.entryTemplate.size()){
                // List's index starts with 0, outline level starts with 0
                TocEntryTemplate tocEntryTemplate = tocDescription->tocSource.entryTemplate.at(outlineLevel - 1);
                // ensure that we fetched correct entry template
                Q_ASSERT(tocEntryTemplate.outlineLevel == outlineLevel);
                if (tocEntryTemplate.outlineLevel != outlineLevel){
                    qDebug() << "TOC outline level not found correctly " << outlineLevel;
                }
                
                tocTemplateStyle = styleManager->paragraphStyle( tocEntryTemplate.styleId );
                if (tocTemplateStyle == 0) {
                    //qDebug() << "TESTX GENERATE NEW STYLE";
                    tocTemplateStyle = generateTemplateStyle(styleManager, outlineLevel);
                    
                }

                QTextBlock tocEntryTextBlock = cursor.block();
                tocTemplateStyle->applyStyle( tocEntryTextBlock );


                // save the current style due to hyperlinks 
                QTextCharFormat savedCharFormat = cursor.charFormat(); 
                foreach (IndexEntry * entry,tocEntryTemplate.indexEntries){
                    switch(entry->name){
                        case IndexEntry::LINK_START: {
                            IndexEntryLinkStart * linkStart = static_cast<IndexEntryLinkStart*>(entry);
                            
                            // TODO: fetch correct target, style is ignored
                            QString target = "#NOWHERE";
                            QTextCharFormat linkCf(savedCharFormat);   // and copy it to alter it
                            linkCf.setAnchor(true);
                            linkCf.setAnchorHref(target);

                            QBrush foreground = linkCf.foreground();
                            // TODO: maybe the style can contain info about the color of the hyperlink
                            foreground.setColor(Qt::blue);

                            linkCf.setForeground(foreground);
                            linkCf.setProperty(KoCharacterStyle::UnderlineStyle, KoCharacterStyle::SolidLine);
                            linkCf.setProperty(KoCharacterStyle::UnderlineType, KoCharacterStyle::SingleLine);

                            cursor.setCharFormat(linkCf);
                            break;
                        }
                        case IndexEntry::CHAPTER: {
                            IndexEntryChapter * chapter = static_cast<IndexEntryChapter*>(entry);
                            
                            cursor.insertText(bd->counterText());
                            
                            break;
                        }
                        case IndexEntry::SPAN: {
                            IndexEntrySpan * span = static_cast<IndexEntrySpan*>(entry);
                            
                            cursor.insertText(span->text);

                            break;
                        }
                        case IndexEntry::TEXT: {
                            IndexEntryText * text = static_cast<IndexEntryText*>(entry);
                            
                            cursor.insertText(block.text());
                            
                            break;
                        }
                        case IndexEntry::TAB_STOP: {
                            IndexEntryTabStop * tabStop = static_cast<IndexEntryTabStop*>(entry);
                            
                            QList<QVariant> tabStops;
                            tabStops.append( QVariant::fromValue<KoText::Tab>(tabStop->tab) );
                            
                            QTextBlockFormat blockFormat = cursor.blockFormat();
                            blockFormat.setProperty(KoParagraphStyle::TabPositions, tabStops);
                            cursor.setBlockFormat(blockFormat);
                            cursor.insertText("\t");

                            break;
                        }
                        case IndexEntry::PAGE_NUMBER: {
                            IndexEntryPageNumber * pageNumber = static_cast<IndexEntryPageNumber*>(entry);
                            break;
                        }
                        case IndexEntry::LINK_END: {
                            IndexEntryLinkEnd * linkEnd = static_cast<IndexEntryLinkEnd*>(entry);
                            
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
                
                // TODO: what is this doing?
                m_originalBlocksInToc.append(qMakePair(cursor.block(), block));

            } else {
                qDebug() << "Invalid outline level";
            }
        }
        block = block.next();
    }
    // Alternative: we can create index body snapshot and paste as one !
    // KoTextLoader loader(ctxt, m_document->documentRdfBase());
    // KoXmlElement e;
    // loader.loadBody(e, cursor);
    cursor.endEditBlock();
}

void ToCGenerator::update()
{
    QTextDocument *doc = m_ToCFrame->document();
    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(doc->documentLayout());
    QTextCursor cursor = m_ToCFrame->lastCursorPosition();
    cursor.beginEditBlock();
    foreach (const BlockPair &blockPair, m_originalBlocksInToc) {
        QTextBlock entryBlock = blockPair.first;
        //qDebug() << "TESTX entry block position " << entryBlock.position() << ", size " << entryBlock.length();
        QTextBlock headingBlock = blockPair.second;
        KoShape *shape = layout->shapeForPosition(headingBlock.position());
        // Alternative 1
        if (shape) {
            KoTextShapeData *shapeData = qobject_cast<KoTextShapeData *>(shape->userData());
            Q_ASSERT(shapeData);
            if (shapeData && shapeData->page()) {
                QString pageNumber = QString::number(shapeData->page()->pageNumber());
                //qDebug() << "TESTX page number " << QString::number(shapeData->page()->pageNumber());
                cursor.setPosition(entryBlock.position() + entryBlock.text().length());
                cursor.insertText(pageNumber);
            }
        }
        // Alternative 2
//        Q_ASSERT(shape);
//        KoTextShapeData *shapeData = qobject_cast<KoTextShapeData *>(shape->userData());
//        Q_ASSERT(shapeData);
//        Q_ASSERT(shapeData->page());
//        QString pageNumber = QString::number(shapeData->page()->pageNumber();
//        qDebug() << "TESTX page number " << QString::number(shapeData->page()->pageNumber());
//        cursor.setPosition(entryBlock.position() + entryBlock.length() - pageNumber.size());
//        Q_ASSERT(cursor.position() < m_ToCFrame->lastPosition());
//        cursor.insertText(pageNumber);
    }
    cursor.endEditBlock();
    m_originalBlocksInToc.clear();
}

#include <ToCGenerator.moc>
