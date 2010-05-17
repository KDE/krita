/* This file is part of the KDE project
 * Copyright (C) 2008 Florian Merz <florianmerz@gmx.de>
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
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

#include "ParagraphBase.h"

#include <KoCanvasBase.h>
#include <KoShape.h>
#include <KoShapeManager.h>
#include <KoTextDocumentLayout.h>
#include <KoTextShapeData.h>
#include <KoParagraphStyle.h>

#include <KLocale>

ParagraphBase::ParagraphBase(QObject *parent, KoCanvasBase *canvas)
        : QObject(parent),
        m_needsRepaint(false),
        m_canvas(canvas),
        m_document(NULL),
        m_paragraphStyle(NULL)
{}

ParagraphBase::~ParagraphBase()
{}

QTextBlock ParagraphBase::textBlock() const {
    return m_cursor.block();
}

bool ParagraphBase::hasActiveTextBlock() const {
    return !m_cursor.isNull();
}

void ParagraphBase::activateTextBlockAt(const QPointF &point)
{

    KoShape *shape = 0;
    KoTextShapeData *textShapeData = 0;
    foreach (KoShape *s, m_canvas->shapeManager()->shapesAt(QRectF(point, QSizeF(4, 4)))) {
        textShapeData = qobject_cast<KoTextShapeData*>(s->userData());
        if (textShapeData) { // found it!
            shape = s;
            break;
        }
    }
    if (shape == 0 || textShapeData == 0) {
        // there is no shape below the mouse position
        deactivateTextBlock();
        return;
    }

    QTextDocument *document = textShapeData->document();

    QPointF p = shape->absoluteTransformation(0).inverted().map(point);
    p += QPointF(0.0, textShapeData->documentOffset());

    int position = document->documentLayout()->hitTest(p, Qt::ExactHit);
    if (position == -1) {
        // there is no text below the mouse position
        deactivateTextBlock();
        return;
    }

    QTextBlock newBlock(document->findBlock(position));
    Q_ASSERT(newBlock.isValid());

    activateTextBlock(newBlock, document);
}

bool ParagraphBase::activatePreviousTextBlock()
{
    if (!textBlock().previous().isValid()) {
        return false;
    }

    activateTextBlock(textBlock().previous());
    return true;
}

bool ParagraphBase::activateNextTextBlock()
{
    if (!textBlock().next().isValid()) {
        return false;
    }

    activateTextBlock(textBlock().next());
    return true;
}

void ParagraphBase::activateTextBlock(QTextBlock newBlock, QTextDocument *document)
{
    if (!newBlock.isValid()) {
        return;
    }

    // the textblock is already activated, no need for a repaint and all that
    if (hasActiveTextBlock() && newBlock == textBlock()) {
        return;
    }

    if (document != NULL) {
        // we needed to pass in the document pointer separately, as there is no
        // easy way to get from the cursor to a non-const document pointer
        // but it doesn't make any sense to pass in a document pointer which is
        // different to the document the text block belongs to
        Q_ASSERT(newBlock.document() == document);
        m_document = document;
    }

    m_cursor = QTextCursor(newBlock);

    delete m_paragraphStyle;
    m_paragraphStyle = KoParagraphStyle::fromBlock(m_cursor.block(), this);

    addFragments();

    scheduleRepaint();
}

void ParagraphBase::deactivateTextBlock()
{
    if (!hasActiveTextBlock())
        return;

    // invalidate active cursor and delete shapes
    m_cursor = QTextCursor();
    m_fragments.clear();

    scheduleRepaint();
}

void ParagraphBase::addFragments()
{
    m_fragments.clear();

    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(textBlock().document()->documentLayout());
    Q_ASSERT(layout);

    QList<KoShape*> shapes = layout->shapes();
    foreach(KoShape *shape, shapes) {
        if (shapeContainsBlock(shape)) {
            m_fragments << ParagraphFragment(shape, textBlock(), m_paragraphStyle);
        }
    }
}

bool ParagraphBase::shapeContainsBlock(const KoShape *shape)
{
    QTextLayout *layout = textBlock().layout();
    qreal blockStart = layout->lineAt(0).y();

    QTextLine endLine = layout->lineAt(layout->lineCount() - 1);
    qreal blockEnd = endLine.y() + endLine.height();

    return (blockEnd >= shapeTop(shape) && blockStart < shapeBottom(shape));
}

qreal ParagraphBase::shapeTop(const KoShape *shape) const
{
    KoTextShapeData *textShapeData = qobject_cast<KoTextShapeData*>(shape->userData());
    Q_ASSERT(textShapeData);

    return textShapeData->documentOffset();
}

qreal ParagraphBase::shapeBottom(const KoShape *shape) const
{
    return shapeTop(shape) + shape->size().height();
}

void ParagraphBase::scheduleRepaint()
{
    m_needsRepaint = true;
}

bool ParagraphBase::needsRepaint() const
{
    return m_needsRepaint;
}

QString ParagraphBase::styleName() const
{
    KoParagraphStyle *style = paragraphStyle();
    while (style != NULL) {
        QString name = style->name();
        if (!name.isNull() && !name.isEmpty()) {
            return name;
        }
        style = style->parentStyle();
    }

    return QString(i18n("None"));
}

