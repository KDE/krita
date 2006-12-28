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
#include "KoVariable.h"
#include "KoTextDocumentLayout.h"
#include "KoTextShapeData.h"

#include <KoShape.h>

#include <QTextCursor>
#include <QPainter>
#include <QFontMetricsF>
#include <QTextDocument>
#include <QTextInlineObject>

KoVariable::KoVariable() : m_modified(true) {
    m_document = 0;
    m_lastPositionInDocument = -1;
}

void KoVariable::setValue(const QString &value) {
    if(m_value == value)
        return;
    m_value = value;
    m_modified = true;
    if(m_document) {
        KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout*> (m_document->documentLayout());
        if(lay)
            lay->documentChanged(m_lastPositionInDocument, 0, 0);
    }
}

void KoVariable::updatePosition(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat & format ) {
    m_document = document;
    m_lastPositionInDocument = posInDocument;
    Q_UNUSED(object);
    Q_UNUSED(format);
    // Variables are always 'in place' so the position is 100% defined by the text layout.

    KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout*> (m_document->documentLayout());
    if(lay == 0)
        return;
    KoShape *textShape = 0;
    foreach(KoShape *shape, lay->shapes()) {
        KoTextShapeData *data = dynamic_cast<KoTextShapeData*> (shape->userData());
        if(data == 0)
            continue;
        if(data->position() <= posInDocument && (data->endPosition() == -1 || data->endPosition() > posInDocument)) {
            textShape = shape;
            break;
        }
    }
    variableMoved(textShape, m_document, posInDocument);
}

void KoVariable::resize(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format, QPaintDevice *pd) {
    Q_UNUSED(document);
    Q_UNUSED(posInDocument);
    if(m_modified == false)
        return;
    Q_ASSERT(format.isCharFormat());
    QFontMetricsF fm(format.font(), pd);
    object.setWidth( fm.width(m_value) );
    object.setAscent(fm.ascent());
    object.setDescent(fm.descent());
    m_modified = true;
}

void KoVariable::paint(QPainter &painter, QPaintDevice *pd, const QTextDocument *document, const QRectF &rect, QTextInlineObject object, int posInDocument, const QTextCharFormat &format) {
    Q_UNUSED(document);
    Q_UNUSED(object);
    Q_UNUSED(posInDocument);

    // TODO set all the font properties from the format (color etc)
    QFont font(format.font(), pd);
    QTextLayout layout(m_value, font, pd);
    layout.setCacheEnabled(true);
    QList<QTextLayout::FormatRange> layouts;
    QTextLayout::FormatRange range;
    range.start=0;
    range.length=m_value.length();
    range.format = format;
    layouts.append(range);
    layout.setAdditionalFormats(layouts);

    QTextOption option(Qt::AlignLeft | Qt::AlignAbsolute);
    option.setTextDirection(object.textDirection());
    layout.setTextOption(option);
    layout.beginLayout();
    layout.createLine();
    layout.endLayout();
    layout.draw(&painter, rect.topLeft());
}

void KoVariable::variableMoved(const KoShape *shape, const QTextDocument *document, int posInDocument) {
    Q_UNUSED(shape);
    Q_UNUSED(document);
    Q_UNUSED(posInDocument);
}
