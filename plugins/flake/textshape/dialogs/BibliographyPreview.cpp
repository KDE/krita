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

#include "BibliographyPreview.h"

#include "KoBibliographyInfo.h"
#include "KoZoomHandler.h"
#include "KoTextDocumentLayout.h"
#include "TextTool.h"

#include <KoInlineTextObjectManager.h>
#include <KoParagraphStyle.h>
#include <KoPageProvider.h>
#include <KoShapePaintingContext.h>

BibliographyPreview::BibliographyPreview(QWidget *parent) 
    : QFrame(parent)
    , m_textShape(0)
    , m_pm(0)
    , m_styleManager(0)
    , m_previewPixSize(QSize(0, 0))
{
}

BibliographyPreview::~BibliographyPreview()
{
    deleteTextShape();

    if (m_pm) {
        delete m_pm;
        m_pm = 0;
    }
}

void BibliographyPreview::setStyleManager(KoStyleManager *styleManager)
{
    m_styleManager = styleManager;
}

void BibliographyPreview::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter *p = new QPainter(this);
    p->save();
    p->translate(5.5, 1.5);
    p->setRenderHint(QPainter::Antialiasing);
    QRect rectang = rect();
    rectang.adjust(-4, -4, -4, -4);

    if (m_pm) {
        p->drawPixmap(rectang, *m_pm, m_pm->rect());
    } else {
        p->fillRect(rectang, QBrush(QColor(Qt::white)));
    }

    p->restore();

    delete p;
}

void BibliographyPreview::updatePreview(KoBibliographyInfo *newbibInfo)
{
    QTextBlockFormat bibFormat;
    QTextDocument *bibDocument = new QTextDocument(this);
    KoTextDocument(bibDocument).setStyleManager(m_styleManager);
    KoBibliographyInfo *info = newbibInfo->clone();

    bibFormat.setProperty(KoParagraphStyle::BibliographyData, QVariant::fromValue<KoBibliographyInfo *>(info));
    bibFormat.setProperty(KoParagraphStyle::GeneratedDocument, QVariant::fromValue<QTextDocument *>(bibDocument));

    deleteTextShape();

    m_textShape = new TextShape(&m_itom, &m_tlm);
    if (m_previewPixSize.isEmpty()) {
        m_textShape->setSize(size());
    } else {
        m_textShape->setSize(m_previewPixSize);
    }
    QTextCursor cursor(m_textShape->textShapeData()->document());

    QTextCharFormat textCharFormat = cursor.blockCharFormat();
    textCharFormat.setFontPointSize(16);
    textCharFormat.setFontWeight(QFont::Bold);

    textCharFormat.setProperty(QTextCharFormat::ForegroundBrush, QBrush(Qt::black));
    cursor.setCharFormat(textCharFormat);

    cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);

    QTextBlockFormat titleBlockFormat;
    cursor.insertBlock(titleBlockFormat, textCharFormat);
    cursor.insertText(info->m_indexTitleTemplate.text);

    textCharFormat.setFontPointSize(12);
    textCharFormat.setFontWeight(QFont::Normal);
    QTextBlockFormat blockFormat;
    cursor.insertBlock(blockFormat, textCharFormat);
    cursor.insertBlock(blockFormat, textCharFormat);
    cursor.insertText("CIT01: Title, Author, Organisation, URL");

    KoTextDocument(m_textShape->textShapeData()->document()).setStyleManager(m_styleManager);

    KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout *>(m_textShape->textShapeData()->document()->documentLayout());
    connect(lay, SIGNAL(finishedLayout()), this, SLOT(finishedPreviewLayout()));
    if (lay) {
        lay->layout();
    }
}

void BibliographyPreview::finishedPreviewLayout()
{
    if (m_pm) {
        delete m_pm;
        m_pm = 0;
    }

    if (m_previewPixSize.isEmpty()) {
        m_pm = new QPixmap(size());
    } else {
        m_pm = new QPixmap(m_previewPixSize);
    }
    m_pm->fill(Qt::white);
    m_zoomHandler.setZoom(0.9);
    m_zoomHandler.setDpi(72, 72);
    QPainter p(m_pm);

    if (m_textShape) {
        if (m_previewPixSize.isEmpty()) {
            m_textShape->setSize(size());
        } else {
            m_textShape->setSize(m_previewPixSize);
        }
        KoShapePaintingContext paintContext; //FIXME
        m_textShape->paintComponent(p, m_zoomHandler, paintContext);
    }
    emit pixmapGenerated();
    update();
}

QPixmap BibliographyPreview::previewPixmap()
{
    return QPixmap(*m_pm);
}

void BibliographyPreview::deleteTextShape()
{
    if (m_textShape) {
        KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout *>(m_textShape->textShapeData()->document()->documentLayout());
        if (lay) {
            lay->setContinuousLayout(false);
            lay->setBlockLayout(true);
        }
        delete m_textShape;
        m_textShape = 0;
    }
}

void BibliographyPreview::setPreviewSize(const QSize &size)
{
    m_previewPixSize = size;
}
