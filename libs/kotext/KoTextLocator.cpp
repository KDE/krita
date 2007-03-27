/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "KoTextLocator.h"
#include "KoTextBlockData.h"
#include "KoTextShapeData.h"
#include "styles/KoListStyle.h"

#include <KoShape.h>

#include <KDebug>
#include <QTextDocument>
#include <QTextList>
#include <QTextInlineObject>
#include <QTextBlock>

KoTextLocator::KoTextLocator()
    : KoInlineObject(false),
    m_dirty(false),
    m_document(0),
    m_cursorPosition(0),
    m_chapterPosition(0)
{
}

void KoTextLocator::updatePosition(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format) {
    Q_UNUSED(object);
    Q_UNUSED(format);
    if(m_document != document || m_cursorPosition != posInDocument) {
        m_dirty = true;
        m_document = document;
        m_cursorPosition = posInDocument;
kDebug() << "KoTextLocator page: " << pageNumber() << ", chapter: " << chapter() << endl;
    }
}

void KoTextLocator::resize(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format, QPaintDevice *pd) {
    Q_UNUSED(object);
    Q_UNUSED(pd);
    Q_UNUSED(format);
    Q_UNUSED(document);
    Q_UNUSED(posInDocument);
}

void KoTextLocator::paint (QPainter &, QPaintDevice *, const QTextDocument *, const QRectF &, QTextInlineObject , int , const QTextCharFormat &) {
    // nothing to paint.
}

QString KoTextLocator::chapter() const {
    const_cast<KoTextLocator*>(this)->update();
    if(m_chapterPosition < 0)
        return QString();
    QTextBlock block = m_document->findBlock(m_chapterPosition);
    return block.text();
}

KoTextBlockData *KoTextLocator::chapterBlockData() const {
    const_cast<KoTextLocator*>(this)->update();
    if(m_chapterPosition < 0)
        return 0;
    QTextBlock block = m_document->findBlock(m_chapterPosition);
    return dynamic_cast<KoTextBlockData*> (block.userData());
}

int KoTextLocator::pageNumber() const {
    const_cast<KoTextLocator*>(this)->update();

// TODO make page a variable
    KoShape *shape = shapeForPosition(m_document, m_cursorPosition);
    if(shape == 0)
        return -1;
    KoTextShapeData *data = static_cast<KoTextShapeData*> (shape->userData());
    return data->pageNumber();
}

void KoTextLocator::update() {
    if(m_dirty == false)
        return;
    m_dirty = false;
    m_chapterPosition = 1;
    if(m_document == 0)
        return;

    QTextBlock block = m_document->findBlock(m_cursorPosition);
    while(block.isValid()) {
        QTextList *list = block.textList();
        if(list) {
            QTextListFormat lf = list->format();
            int level = lf.intProperty(KoListStyle::Level);
            if(level == 1) {
                m_chapterPosition = block.position();
                break;
            }
        }
        block = block.previous();
    }
}
