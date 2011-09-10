/* This file is part of the KDE project
 * Copyright (C) 2011 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
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

#include "TableOfContentsPreview.h"

#include "KoTableOfContentsGeneratorInfo.h"
#include "KoZoomHandler.h"
#include "KoTextDocumentLayout.h"
#include "TextTool.h"

#include <KoInlineTextObjectManager.h>
#include <KoParagraphStyle.h>
#include <KoPageProvider.h>


TableOfContentsPreview::TableOfContentsPreview(QWidget *parent) :
    QFrame(parent),
    m_styleManager(0),
    m_pm(0),
    m_textShape(0)
{
}

void TableOfContentsPreview::setStyleManager(KoStyleManager *styleManager)
{
    m_styleManager = styleManager;
}

void TableOfContentsPreview::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    m_zoomHandler.setZoom(0.9);
    m_zoomHandler.setDpi(72, 72);

    QPainter *p = new QPainter(this);
    p->save();
    p->translate(5.5, 1.5);
    p->setRenderHint(QPainter::Antialiasing);
    QRect rectang = rect();
    rectang.adjust(-4,-4,-4,-4);
    p->fillRect(rectang, QBrush(QColor(Qt::white)));
    m_textShape->paintComponent(*p, m_zoomHandler);
    p->restore();

    delete p;
}

void TableOfContentsPreview::updatePreview(KoTableOfContentsGeneratorInfo *newToCInfo)
{
    QTextBlockFormat tocFormat;
    QTextDocument *tocDocument = new QTextDocument();
    KoTextDocument(tocDocument).setStyleManager(m_styleManager);
    KoTableOfContentsGeneratorInfo *info = newToCInfo->clone();
   // info->m_indexTitleTemplate.text = newToCInfo->m_indexTitleTemplate.text;
   // info->m_useOutlineLevel = newToCInfo->m_useOutlineLevel;

    tocFormat.setProperty(KoParagraphStyle::TableOfContentsData, QVariant::fromValue<KoTableOfContentsGeneratorInfo*>(info) );
    tocFormat.setProperty(KoParagraphStyle::GeneratedDocument, QVariant::fromValue<QTextDocument*>(tocDocument) );

    m_textShape = new TextShape(&m_itom);
    m_textShape->setSize(size());
    QTextCursor cursor(m_textShape->textShapeData()->document());

    QTextCharFormat textCharFormat = cursor.blockCharFormat();
    textCharFormat.setFontPointSize(11);
    textCharFormat.setFontWeight(QFont::Normal);

    //the brush is set to the background colour so that the actual text block(Heading 1,Heading 1.1 etc.) does not appear in the preview
    textCharFormat.setProperty(QTextCharFormat::ForegroundBrush, QBrush(Qt::white));
    cursor.setCharFormat(textCharFormat);

    cursor.insertBlock(tocFormat);
    cursor.movePosition(QTextCursor::End,QTextCursor::MoveAnchor);

    //insert text for different heading styles
    QTextBlockFormat blockFormat;
    blockFormat.setProperty(KoParagraphStyle::OutlineLevel, 1);
    cursor.insertBlock(blockFormat,textCharFormat);
    cursor.insertText("Header 1");

    QTextBlockFormat blockFormat1;
    blockFormat1.setProperty(KoParagraphStyle::OutlineLevel, 2);
    cursor.insertBlock(blockFormat1,textCharFormat);
    cursor.insertText("Header 1.1");

    QTextBlockFormat blockFormat2;
    blockFormat2.setProperty(KoParagraphStyle::OutlineLevel, 2);
    cursor.insertBlock(blockFormat2,textCharFormat);
    cursor.insertText("Header 1.2");

    QTextBlockFormat blockFormat3;
    blockFormat3.setProperty(KoParagraphStyle::OutlineLevel, 1);
    cursor.insertBlock(blockFormat3,textCharFormat);
    cursor.insertText("Header 2");

    KoTextDocument(m_textShape->textShapeData()->document()).setStyleManager(m_styleManager);

    KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout*>(m_textShape->textShapeData()->document()->documentLayout());
    connect(lay, SIGNAL(finishedLayout()), this, SLOT(finishedPreviewLayout()));
    if(lay)
        lay->layout();
}

void TableOfContentsPreview::finishedPreviewLayout()
{
    update();
}
