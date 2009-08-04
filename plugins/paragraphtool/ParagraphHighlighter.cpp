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

#include <KoShape.h>
#include <KoTextShapeData.h>

#include <QAbstractTextDocumentLayout>
#include <QPainter>

#include <assert.h>


ParagraphHighlighter::ParagraphHighlighter(QObject *parent, KoCanvasBase *canvas)
        : ParagraphBase(parent, canvas)
{}

ParagraphHighlighter::~ParagraphHighlighter()
{}

void ParagraphHighlighter::paint(QPainter &painter, const KoViewConverter &converter)
{
    m_needsRepaint = false;

    if (!hasActiveTextBlock()) {
        return;
    }

    foreach (const ParagraphFragment &fragment, fragments()) {
        KoShape *shape = fragment.shape();
        KoTextShapeData *textShapeData = qobject_cast<KoTextShapeData*>(shape->userData());
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

QRectF ParagraphHighlighter::dirtyRectangle()
{
    QRectF repaintRectangle = m_storedRepaintRectangle;

    m_storedRepaintRectangle = QRectF();
    foreach(const ParagraphFragment &fragment, fragments()) {
        m_storedRepaintRectangle = m_storedRepaintRectangle | fragment.shape()->boundingRect();
    }
    repaintRectangle |= m_storedRepaintRectangle;

    return repaintRectangle;
}

