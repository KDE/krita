/* This file is part of the KDE project
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

#include "KoTextShapeData.h"
#include <QTextDocument>

KoTextShapeData::KoTextShapeData()
: m_ownsDocument(true)
, m_dirty(true)
, m_textCursor(0)
, m_offset(0.0)
, m_position(-1)
, m_endPosition(-1)
{
    m_document = new QTextDocument();
    //m_document->setUseDesignMetrics(true); // TODO remove the comment when Qt fixes the first-parag-won't-break bug.
}

KoTextShapeData::~KoTextShapeData() {
    if(m_ownsDocument)
        delete m_document;
}

void KoTextShapeData::setDocument(QTextDocument *document, bool transferOwnership) {
    Q_ASSERT(document);
    if(m_ownsDocument && document != m_document)
        delete m_document;
    m_document = document;
    // The following avoids the normal case where the glyph metrices are rounded to integers and
    // hinted to the screen by freetype, which you of course don't want for WYSIWYG
    m_document->setUseDesignMetrics(true);
    m_ownsDocument = transferOwnership;
}

QTextDocument *KoTextShapeData::document() {
    return m_document;
}

#include "KoTextShapeData.moc"
