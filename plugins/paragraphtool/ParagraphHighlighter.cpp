/* This file is part of the KDE project
 * Copyright (C) 2008 Florian Merz <florianmerz@gmx.de>
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

#include "ParagraphHighlighter.h"

#include <KoCanvasBase.h>
#include <KoPointerEvent.h>
#include <KoShape.h>
#include <KoShapeManager.h>
#include <KoTextDocumentLayout.h>
#include <KoTextShapeData.h>

#include <KDebug>

#include <QAbstractTextDocumentLayout>
#include <QPainter>
#include <QTextDocument>

#include <assert.h>

static bool shapeContainsBlock(const KoShape *shape, QTextBlock textBlock)
{
    QTextLayout *layout = textBlock.layout();
    qreal blockStart = layout->lineAt(0).y();

    QTextLine endLine = layout->lineAt(layout->lineCount() - 1);
    qreal blockEnd = endLine.y() + endLine.height();

    KoTextShapeData *textShapeData = dynamic_cast<KoTextShapeData*>(shape->userData());
    if (textShapeData == NULL) {
        return false;
    }

    qreal shapeStart = textShapeData->documentOffset();
    qreal shapeEnd = shapeStart + shape->size().height();

    return (blockEnd >= shapeStart && blockStart < shapeEnd);
}

void ParagraphHighlighter::addShapes()
{
    m_shapes.clear();

    KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*>(textBlock().document()->documentLayout());
    assert(layout != NULL);

    QList<KoShape*> shapes = layout->shapes();
    foreach(KoShape *shape, shapes) {
        if (shapeContainsBlock(shape, textBlock())) {
            m_shapes << shape;
        }
    }
}

ParagraphHighlighter::ParagraphHighlighter(QObject *parent, KoCanvasBase *canvas)
        : QObject(parent), m_canvas(canvas)
{}

ParagraphHighlighter::~ParagraphHighlighter()
{}

void ParagraphHighlighter::paint(QPainter &painter, const KoViewConverter &converter)
{
    m_needsRepaint = false;

    if (!hasActiveTextBlock()) {
        return;
    }

    foreach (const KoShape *shape, m_shapes) {
        KoTextShapeData *textShapeData = dynamic_cast<KoTextShapeData*>(shape->userData());
        assert(textShapeData != NULL);

        painter.save();

        qreal shapeTop = textShapeData->documentOffset();
        qreal shapeBottom = textShapeData->documentOffset() + shape->size().height();

        painter.setPen(Qt::black);
        painter.setMatrix(shape->absoluteTransformation(&converter) * painter.matrix());
        KoShape::applyConversion(painter, converter);
        painter.translate(0.0, -shapeTop);

        QTextLayout *layout = textBlock().layout();
        QRectF rectangle = layout->boundingRect();

        rectangle.setTop(qMax(rectangle.top(), shapeTop));
        rectangle.setBottom(qMin(rectangle.bottom(), shapeBottom));
        painter.drawRect(rectangle);

        painter.restore();
    }
}

void ParagraphHighlighter::scheduleRepaint()
{
    m_needsRepaint = true;
}

bool ParagraphHighlighter::needsRepaint() const
{
    return m_needsRepaint;
}

QRectF ParagraphHighlighter::dirtyRectangle()
{
    QRectF repaintRectangle = m_storedRepaintRectangle;

    m_storedRepaintRectangle = QRectF();
    foreach(KoShape *shape, m_shapes) {
        m_storedRepaintRectangle = m_storedRepaintRectangle | shape->boundingRect();
    }
    repaintRectangle |= m_storedRepaintRectangle;

    return repaintRectangle;
}

void ParagraphHighlighter::mousePressEvent(KoPointerEvent *)
{}

void ParagraphHighlighter::mouseReleaseEvent(KoPointerEvent *)
{}

void ParagraphHighlighter::mouseMoveEvent(KoPointerEvent *event)
{
    m_mousePosition = event->point;

    activateTextBlockAt(event->point);
}

void ParagraphHighlighter::keyPressEvent(QKeyEvent *)
{}

void ParagraphHighlighter::keyReleaseEvent(QKeyEvent *)
{}

QTextBlock ParagraphHighlighter::textBlock() const {
    return m_cursor.block();
}

bool ParagraphHighlighter::hasActiveTextBlock() const {
    return !m_cursor.isNull();
}

void ParagraphHighlighter::activateTextBlockAt(const QPointF &point)
{
    KoShape *shape = dynamic_cast<KoShape*>(m_canvas->shapeManager()->shapeAt(point));
    if (shape == NULL) {
        // there is no shape below the mouse position
        deactivateTextBlock();
        return;
    }

    KoTextShapeData *textShapeData = dynamic_cast<KoTextShapeData*>(shape->userData());
    if (textShapeData == NULL) {
        // the shape below the mouse position is not a text shape
        deactivateTextBlock();
        return;
    }

    QTextDocument *document = textShapeData->document();

    QPointF p = shape->transformation().inverted().map(point);
    p += QPointF(0.0, textShapeData->documentOffset());

    int position = document->documentLayout()->hitTest(p, Qt::ExactHit);
    if (position == -1) {
        // there is no text below the mouse position
        deactivateTextBlock();
        return;
    }

    QTextBlock newBlock(document->findBlock(position));
    assert(newBlock.isValid());

    activateTextBlock(newBlock);
}

void ParagraphHighlighter::activateTextBlock(QTextBlock newBlock)
{
    // the textblock is already activated, no need for a repaint and all that
    if (hasActiveTextBlock() && newBlock == textBlock()) {
        return;
    }

    m_cursor = QTextCursor(newBlock);
    addShapes();
    scheduleRepaint();
}

void ParagraphHighlighter::deactivateTextBlock()
{
    if (!hasActiveTextBlock())
        return;

    // invalidate active cursor and delete shapes
    m_cursor = QTextCursor();
    m_shapes.clear();
    scheduleRepaint();
}


