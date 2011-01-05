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
    QList<KoText::Tab> tabList;
    struct KoText::Tab aTab;
    aTab.type = QTextOption::RightTab;
    aTab.leaderText = '.';
    aTab.position = 490 - outlineLevel * 8;
    tabList.append(aTab);
    style->setTabPositions(tabList);
    styleManager->add(style);
    return style;
}

static void createTabs(const QVariant &data, int outlineLevel, KoParagraphStyle *tocTemplateStyle) {
    QList<KoText::Tab> tabList = tocTemplateStyle->tabPositions();
    //qDebug() << "TESTX TAB LIST " << tabList.size();
    //TODO pavolk: can we modify existing style to have tab ?
    if (tabList.isEmpty()) {
        QVariant entryTabStopTypeVariant = attribute(data, "index-entry-tab-stop", "type", outlineLevel);
        //qDebug() << "TESTX ENTRY TAB STOP STYLE VARIANT " << entryTabStopTypeVariant;
        QTextOption::TabType entryTabStopType = (entryTabStopTypeVariant.isNull() ? QTextOption::RightTab : (QTextOption::TabType) entryTabStopTypeVariant.toInt());
        //qDebug() << "TESTX ENTRY TAB STOP STYLE INT " << entryTabStopType;
        QVariant entryTabStopLeaderCharVariant = attribute(data, "index-entry-tab-stop", "leader-char", outlineLevel);
        //qDebug() << "TESTX ENTRY TAB STOP LEADER CHAR VARIANT " << entryTabStopLeaderCharVariant;
        QString entryTabStopLeaderChar = (entryTabStopLeaderCharVariant.isNull() ? "." : entryTabStopLeaderCharVariant.toString());
        //qDebug() << "TESTX ENTRY TAB STOP LEADER CHAR " << entryTabStopLeaderChar;
        QList<KoText::Tab> tabList;
        struct KoText::Tab aTab;
        aTab.type = entryTabStopType;
        aTab.leaderText = entryTabStopLeaderChar;
        //TODO pavolk: we need to fill leader chars also at update time to formate page numbers !!
        aTab.position = 490;
        tabList.append(aTab);
        tocTemplateStyle->setTabPositions(tabList);
    }
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
    QVariant data = cursorFrame->format().property(KoText::TableOfContentsData);
    QTextDocument *doc = m_ToCFrame->document();
    KoTextDocument koDocument(doc);
    KoStyleManager *styleManager = koDocument.styleManager();
    //TODO pavolk: apply section toc style
    //QVariant tocStyleVariant = attribute(data, "table-of-content", "style-name");
    //qDebug() << "TESTX TOC STYLE VARIANT " << tocStyleVariant;
    //KoSectionStyle *tocStyle = getStyle(styleManager, tocStyleVariant);
    //QTextBlock tocTextBlock = cursor.block();
    //tocStyle->applyStyle(cursorFrame);
    //cursor.insertBlock(QTextBlockFormat(), QTextCharFormat());
    // Add the title
    // Do we want add title like this ?
    QVariant titleStyleVariant = attribute(data, "index-title-template", "style-name");
    KoParagraphStyle *titleStyle = getStyle(styleManager, titleStyleVariant);
    // Do we want default style or generate style also for title ?
    if (titleStyle) {
        //qDebug() << "TESTX GOT STYLE " << titleStyle->name();
    } else {
        //TODO pavolk: generate style for title ?
        titleStyle = styleManager->defaultParagraphStyle();
        //qDebug() << "TESTX USING DEFALT STYLE " << titleStyle->styleId();
    }
    QTextBlock titleTextBlock = cursor.block();
    titleStyle->applyStyle(titleTextBlock);
    cursor.insertText(tr2i18n("Table of Contents"));
    cursor.insertBlock(QTextBlockFormat(), QTextCharFormat());
    // Add TOC
    // looks for blocks to add in the ToC
    QTextBlock block = doc->begin();
    while (block.isValid()) {
        KoTextBlockData *bd = dynamic_cast<KoTextBlockData *>(block.userData());
        if (bd && bd->hasCounterData()) {
            cursor.insertBlock(QTextBlockFormat(), QTextCharFormat());
            QTextBlock tocEntryTextBlock = cursor.block();
            int outlineLevel = block.blockFormat().intProperty(KoParagraphStyle::OutlineLevel);
            QVariant tocTemplateStyleVariant = attribute(data, "table-of-content-entry-template", "style-name", outlineLevel);
            //TODO pavolk: How to use source styles ?
            //QVariant tocTemplateStyleVariant = attribute(data, "index-source-style", "style-name", outlineLevel);
            //qDebug() << "TESTX TOC TEMPLATE VARIANT " << tocTemplateStyleVariant;
            KoParagraphStyle *tocTemplateStyle = getStyle(styleManager, tocTemplateStyleVariant);
            if (tocTemplateStyle == 0) {
                //qDebug() << "TESTX GENERATE NEW STYLE";
                tocTemplateStyle = generateTemplateStyle(styleManager, outlineLevel);
            }
            createTabs(data, outlineLevel, tocTemplateStyle);
//            QList<KoText::Tab> tabList = tocTemplateStyle->tabPositions();
//            //qDebug() << "TESTX TAB LIST " << tabList.size();
//            //TODO pavolk: can we modify existing style to have tab ?
//            if (tabList.isEmpty()) {
//                QVariant entryTabStopTypeVariant = attribute(data, "index-entry-tab-stop", "type", outlineLevel);
//                //qDebug() << "TESTX ENTRY TAB STOP STYLE VARIANT " << entryTabStopTypeVariant;
//                QTextOption::TabType entryTabStopType = (entryTabStopTypeVariant.isNull() ? QTextOption::RightTab : (QTextOption::TabType) entryTabStopTypeVariant.toInt());
//                //qDebug() << "TESTX ENTRY TAB STOP STYLE INT " << entryTabStopType;
//                QVariant entryTabStopLeaderCharVariant = attribute(data, "index-entry-tab-stop", "leader-char", outlineLevel);
//                //qDebug() << "TESTX ENTRY TAB STOP LEADER CHAR VARIANT " << entryTabStopLeaderCharVariant;
//                QString entryTabStopLeaderChar = (entryTabStopLeaderCharVariant.isNull() ? "." : entryTabStopLeaderCharVariant.toString());
//                //qDebug() << "TESTX ENTRY TAB STOP LEADER CHAR " << entryTabStopLeaderChar;
//                QList<KoText::Tab> tabList;
//                struct KoText::Tab aTab;
//                aTab.type = entryTabStopType;
//                aTab.leaderText = entryTabStopLeaderChar;
//                //TODO pavolk: we need to fill leader chars also at update time to formate page numbers !!
//                aTab.position = 490;
//                tabList.append(aTab);
//                tocTemplateStyle->setTabPositions(tabList);
//            }
            tocTemplateStyle->applyStyle(tocEntryTextBlock);
            //TODO pavolk: create and insert hyperlinks ?
            //KoTextBlockData *bd = dynamic_cast<KoTextBlockData *>(block.userData());
            if (bd && bd->hasCounterData()) {
                cursor.insertText(bd->counterText());
                //qDebug() << "TESTX CHAPTER " << bd->counterText();
                cursor.insertText(QLatin1String(" "));
            }
            // Wrong page number, it will be corrected in the update
            // note that the fact that I use 4 chars is reused in the update method!
            cursor.insertText(block.text() + "\t");
            //qDebug() << "TESTX TEXT " << block.text();
            m_originalBlocksInToc.append(qMakePair(tocEntryTextBlock, block));
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
