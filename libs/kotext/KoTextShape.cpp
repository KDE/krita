/* This file is part of the KDE project
 * Copyright (C) 2006 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include "KoTextShape.h"

#include <QTextLayout>
#include <QFont>
#include <QAbstractTextDocumentLayout>


// ############ KoTextShape ################

KoTextShape::KoTextShape()
: m_document(0)
{
    setDocument(new QTextDocument());
}

KoTextShape::~KoTextShape() {
    delete m_document;
}

void KoTextShape::paint(QPainter &painter, KoViewConverter &converter) {
    painter.fillRect(converter.documentToView(QRectF(QPointF(0.0,0.0), size())), background());
    applyConversion(painter, converter);
    QAbstractTextDocumentLayout::PaintContext pc;
    pc.cursorPosition = -1;

    m_document->setPageSize(size());
    m_document->documentLayout()->draw( &painter, pc);
}

void KoTextShape::setDocument(QTextDocument *document) {
    Q_ASSERT(document);
    delete m_document;
    m_document = document;
    // The following avoids the normal case where the glyph metrices are rounded to integers and
    // hinted to the screen by freetype, which you of course don't want for WYSIWYG
    m_document->setUseDesignMetrics(true);
}

QTextDocument *KoTextShape::takeDocument() {
    QTextDocument *doc = m_document;
    m_document = 0;
    return doc;
}
