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
    QTextFrame *cursorFrame = cursor.currentFrame();
    QVariant tocVariant = cursorFrame->format().property(KoText::TableOfContentsData);
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
    // Add the title
    QVariant titleStyleVariant = attribute(tocVariant, "index-title-template", "style-name");
    // Check title text
    if ( !titleStyleVariant.isNull()) {
        // Get style for title
        KoParagraphStyle *titleStyle = getStyle(styleManager, titleStyleVariant);
        if ( ! titleStyle) {
            titleStyle = styleManager->defaultParagraphStyle();
            qDebug() << "TESTX USING DEFALT STYLE " << titleStyle->styleId();
            QTextBlock titleTextBlock = cursor.block();
            titleStyle->applyStyle(titleTextBlock);
            //TODO: load and insert title text
            cursor.insertText(tr2i18n("Table of Contents"));
            cursor.insertBlock(QTextBlockFormat(), QTextCharFormat());            
        }
    }
    // Add TOC
    // Iterate through all blocks to generate TOC
    QTextBlock block = doc->begin();
    while (block.isValid()) {
        KoTextBlockData *bd = dynamic_cast<KoTextBlockData *>(block.userData());
        // Choose only TOC blocks
        if (bd && bd->hasCounterData()) {
            cursor.insertBlock(QTextBlockFormat(), QTextCharFormat());
            QTextBlock tocEntryTextBlock = cursor.block();
            int outlineLevel = block.blockFormat().intProperty(KoParagraphStyle::OutlineLevel);
            QVariant tocTemplateStyleVariant = attribute(tocVariant, "table-of-content-entry-template", "style-name", outlineLevel);
            KoParagraphStyle *tocTemplateStyle = getStyle(styleManager, tocTemplateStyleVariant);
            if (tocTemplateStyle == 0) {
                qDebug() << "TESTX GENERATE NEW STYLE";
                tocTemplateStyle = generateTemplateStyle(styleManager, outlineLevel);
            }
            tocTemplateStyle->applyStyle(tocEntryTextBlock);
            //TODO generate TOC links
            QVariantHash tagMap = tocVariant.toHash();
            QVariant tabVariant = tagMap.value("index-entry-tab-stop " + QString::number(outlineLevel));
            QList<QVariant> list;
            list.append(tabVariant);
            QTextCursor textCursor(tocEntryTextBlock);            
            QTextBlockFormat blockFormat = textCursor.blockFormat();
            blockFormat.setProperty(KoParagraphStyle::TabPositions, list);
            textCursor.setBlockFormat(blockFormat);

            cursor.insertText(bd->counterText());
            //qDebug() << "TESTX CHAPTER " << bd->counterText();
            cursor.insertText(QLatin1String(" "));
            cursor.insertText(block.text() + "\t");
            //qDebug() << "TESTX TEXT " << block.text();
            m_originalBlocksInToc.append(qMakePair(tocEntryTextBlock, block));
        }
        block = block.next();
    }
    // Alternativelly: we can create index body snapshot and paste as one !
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
        //alternative 1
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
        // alternative 2
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
