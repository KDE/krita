/* This file is part of the KDE project
 * Copyright (C) 2008 Florian Merz <florianmerz@web.de>
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

#include "ShapeSpecificData.h"

#include <KoShapeBorderModel.h>
#include <KoTextShapeData.h>

#include <QTextBlock>
#include <QTextLayout>

qreal ShapeSpecificData::shapeStartOffset() const
{
    KoTextShapeData *textShapeData = static_cast<KoTextShapeData*> (textShape()->userData());

    return textShapeData->documentOffset();
}

qreal ShapeSpecificData::shapeEndOffset() const
{
    KoTextShapeData *textShapeData = static_cast<KoTextShapeData*> (textShape()->userData());

    return textShapeData->documentOffset() + textShape()->size().height();
}

bool ShapeSpecificData::shapeContainsBlock(QTextBlock textBlock) const
{
    QTextLayout *layout = textBlock.layout();
    qreal blockStart = layout->lineAt(0).y();

    QTextLine endLine = layout->lineAt(layout->lineCount()-1);
    qreal blockEnd = endLine.y() + endLine.height();

    qreal shapeStart = shapeStartOffset();
    qreal shapeEnd = shapeEndOffset();

    if (blockEnd < shapeStart || blockStart > shapeEnd) {
        return false;
    }
    else {
        return true;
    }
}

QPointF ShapeSpecificData::mapDocumentToShape(QPointF point) const
{
    QMatrix matrix = textShape()->absoluteTransformation(NULL);
    matrix.translate(0.0, -shapeStartOffset());
    return matrix.inverted().map(point);
}

QRectF ShapeSpecificData::dirtyRectangle() const
{
    if (m_textShape == NULL)
        return QRectF();

    QRectF boundingRect( QPointF(0, 0), textShape()->size() );

    if(textShape()->border()) {
        KoInsets insets;
        textShape()->border()->borderInsets(textShape(), insets);
        boundingRect.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
    }

    // adjust for arrow heads and label (although we can't be sure about the label)
    boundingRect.adjust(-50.0, -50.0, 50.0, 50.0);

    boundingRect = textShape()->absoluteTransformation(0).mapRect(boundingRect);

    return boundingRect;
}
