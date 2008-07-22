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

#include "ShapeSpecificData.h"

#include <KoParagraphStyle.h>
#include <KoShapeBorderModel.h>
#include <KoTextBlockData.h>
#include <KoTextShapeData.h>

#include <QTextBlock>
#include <QTextLayout>

void ShapeSpecificData::initDimensions(QTextBlock textBlock, KoParagraphStyle *paragraphStyle)
{
    QTextLayout *layout = textBlock.layout();

    bool isSingleLine = (layout->lineCount() == 1);

    // border rectangle left and right
    m_border.setLeft(0.0);
    m_border.setRight(m_textShape->size().width());

    // first line rectangle
    m_firstLine = layout->lineAt(0).rect();
    m_firstLine.setRight(m_border.right() - paragraphStyle->rightMargin());

    // counter rectangle 
    KoTextBlockData *blockData = static_cast<KoTextBlockData*> (textBlock.userData());
    if (blockData != NULL) {
        m_counter = QRectF(blockData->counterPosition(), QSizeF(blockData->counterWidth() - blockData->counterSpacing(),
                    m_firstLine.height()));
    }

    // folowing lines rectangle
    if (!isSingleLine) {
        m_followingLines = QRectF(layout->lineAt(1).rect().topLeft(),
                layout->lineAt(layout->lineCount() - 1).rect().bottomRight());
    }
    else {
        m_followingLines = m_firstLine;
    }

    // border rectangle top and bottom
    m_border.setTop(m_firstLine.top() - paragraphStyle->topMargin());
    m_border.setBottom(isSingleLine ? m_firstLine.bottom() + paragraphStyle->bottomMargin()
            : m_followingLines.bottom() + paragraphStyle->bottomMargin());

    // workaround: the lines overlap slightly so right now we simply calculate the mean of the two y-values
    if (!isSingleLine) {
        qreal lineBreak((m_firstLine.bottom() + m_followingLines.top()) / 2.0);
        m_firstLine.setBottom(lineBreak);
        m_counter.setBottom(lineBreak);
        m_followingLines.setTop(lineBreak);
    }
}

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

QLineF ShapeSpecificData::baseline(RulerIndex ruler) const
{
    switch (ruler) {
        case firstIndentRuler:
            return QLineF(m_border.left(), m_firstLine.top(), m_border.left(), m_firstLine.bottom());
        case followingIndentRuler:
            return QLineF(m_border.left(), m_followingLines.top(), m_border.left(), m_followingLines.bottom());
        case rightMarginRuler:
            return QLineF(m_border.right(), m_followingLines.bottom(), m_border.right(), m_firstLine.top());
        case topMarginRuler:
            return QLineF(m_border.right(), m_border.top(), m_border.left(), m_border.top());
        case bottomMarginRuler:
            return QLineF(m_border.right(), m_followingLines.bottom(), m_border.left(), m_followingLines.bottom());
        default:
            return QLineF();
    }
}

QLineF ShapeSpecificData::separatorLine() const
{
    return QLineF(m_border.left(), m_firstLine.bottom(), m_firstLine.right(), m_firstLine.bottom());
}

