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

#include "ParagraphFragment.h"

#include <KoParagraphStyle.h>
#include <KoShapeBorderModel.h>
#include <KoTextBlockData.h>
#include <KoTextShapeData.h>

#include <KDebug>

#include <QTextBlock>
#include <QTextLayout>

ParagraphFragment::ParagraphFragment(Ruler* rulers, TextShape *textShape, QTextBlock textBlock, KoParagraphStyle *style)
    : m_textShape(textShape)
{
    for (int ruler = 0; ruler != maxRuler; ++ruler) {
        m_rulerFragments[ruler].setRuler(&rulers[ruler]);
    }
    m_isSingleLine = (textBlock.layout()->lineCount() == 1);

    initDimensions(textBlock, style);

    initVisibility();

    initBaselines();
}

void ParagraphFragment::initDimensions(QTextBlock textBlock, KoParagraphStyle *paragraphStyle)
{
    QTextLayout *layout = textBlock.layout();

    // border rectangle left and right
    m_border.setLeft(0.0);
    m_border.setRight(m_textShape->size().width());

    // first line rectangle
    m_firstLine = layout->lineAt(0).rect();
    m_firstLine.setRight(m_border.right() - paragraphStyle->rightMargin());

    // counter rectangle 
    KoTextBlockData *blockData = static_cast<KoTextBlockData*> (textBlock.userData());
    if (blockData != NULL) {
        m_counter = QRectF(blockData->counterPosition(), QSizeF(blockData->counterWidth() - blockData->counterSpacing(), m_firstLine.height()));
    }

    // folowing lines rectangle
    if (!m_isSingleLine) {
        m_followingLines = QRectF(layout->lineAt(1).rect().topLeft(), layout->lineAt(layout->lineCount() - 1).rect().bottomRight());
    }
    else {
        m_followingLines = m_firstLine;
    }

    // border rectangle top and bottom
    m_border.setTop(m_firstLine.top() - paragraphStyle->topMargin());
    m_border.setBottom(m_isSingleLine ? m_firstLine.bottom() + paragraphStyle->bottomMargin() : m_followingLines.bottom() + paragraphStyle->bottomMargin());

    // TODO: the lines overlap slightly so right now we simply
    // calculate the mean of the two y-values, should be handled properly
    if (!m_isSingleLine) {
        qreal lineBreak((m_firstLine.bottom() + m_followingLines.top()) / 2.0);
        m_firstLine.setBottom(lineBreak);
        m_counter.setBottom(lineBreak);
        m_followingLines.setTop(lineBreak);
    }
}

void ParagraphFragment::initVisibility()
{
    qreal top(shapeTop());
    qreal bottom(shapeBottom());

    // first line
    m_rulerFragments[firstIndentRuler].setVisible(top < m_firstLine.bottom());

    // following lines
    m_rulerFragments[followingIndentRuler].setVisible(top < m_followingLines.bottom() && bottom > m_followingLines.top() && !m_isSingleLine);

    // right margin
    m_rulerFragments[rightMarginRuler].setVisible(true);

    // top margin
    m_rulerFragments[topMarginRuler].setVisible(top <= m_firstLine.top());

    // bottom margin
    m_rulerFragments[bottomMarginRuler].setVisible(bottom >= m_followingLines.bottom());
}

void ParagraphFragment::initBaselines()
{
    qreal top(shapeTop());
    qreal bottom(shapeBottom());

    qreal rightTop = qMax(top, m_firstLine.top());
    qreal followingTop = qMax(top, m_followingLines.top());
    qreal followingBottom = qMin(bottom, m_followingLines.bottom());

    m_paintSeparator = !m_isSingleLine && rightTop != followingTop && m_rulerFragments[followingIndentRuler].isVisible();

    m_rulerFragments[firstIndentRuler].setBaseline(QLineF(m_border.left(), m_firstLine.top(), m_border.left(), m_firstLine.bottom()));

    m_rulerFragments[followingIndentRuler].setBaseline(QLineF(m_border.left(), followingTop, m_border.left(), followingBottom));

    m_rulerFragments[rightMarginRuler].setBaseline(QLineF(m_border.right(), followingBottom, m_border.right(), rightTop));

    m_rulerFragments[topMarginRuler].setBaseline(QLineF(m_border.right(), m_border.top(), m_border.left(), m_border.top()));

    m_rulerFragments[bottomMarginRuler].setBaseline(QLineF(m_border.right(), m_followingLines.bottom(), m_border.left(), m_followingLines.bottom()));
}

RulerIndex ParagraphFragment::hitTest(const QPointF &point) const
{
    QPointF mappedPoint(mapDocumentToText(point));

    for (int ruler = 0; ruler != maxRuler; ++ruler) {
        if (m_rulerFragments[ruler].hitTest(mappedPoint)) {
            return static_cast<RulerIndex>(ruler);
        }
    }

    return noRuler;
}

bool ParagraphFragment::hitTest(RulerIndex ruler, const QPointF &point) const
{
    QPointF mappedPoint(mapDocumentToText(point));
    return m_rulerFragments[ruler].hitTest(mappedPoint);
}

void ParagraphFragment::moveRulerTo(RulerIndex ruler, const QPointF &point, bool smoothMovement) const
{
    QPointF mappedPoint(mapDocumentToText(point));
    m_rulerFragments[ruler].moveTo(mappedPoint, smoothMovement);
}


QLineF ParagraphFragment::labelConnector(RulerIndex ruler) const
{
    return mapTextToDocument(m_rulerFragments[ruler].labelConnector());
}

void ParagraphFragment::paint(QPainter &painter, const KoViewConverter &converter) const
{
    painter.save();

    // transform painter from view coordinate system to shape
    // coordinate system
    painter.setMatrix(textShape()->absoluteTransformation(&converter) * painter.matrix());
    KoShape::applyConversion(painter, converter);
    painter.translate(0.0, -shapeTop());

    painter.setPen(Qt::darkGray);

    if (m_paintSeparator) {
        painter.drawLine(QLineF(m_border.left(), m_firstLine.bottom(), m_firstLine.right(), m_firstLine.bottom()));
    }

    for (int ruler = 0; ruler != maxRuler; ++ruler) {
        m_rulerFragments[ruler].paint(painter);
    }

    painter.restore();
}

QRectF ParagraphFragment::dirtyRectangle() const
{
    if (m_textShape == NULL)
        return QRectF();

    QRectF boundingRect( QPointF(0, 0), textShape()->size() );

    if(textShape()->border()) {
        KoInsets insets;
        textShape()->border()->borderInsets(textShape(), insets);
        boundingRect.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
    }

    // adjust for arrow heads and label
    // (although we can't be sure about the label)
    boundingRect.adjust(-50.0, -50.0, 50.0, 50.0);

    boundingRect = textShape()->absoluteTransformation(0).mapRect(boundingRect);

    return boundingRect;
}

qreal ParagraphFragment::shapeTop() const
{
    KoTextShapeData *textShapeData = static_cast<KoTextShapeData*> (textShape()->userData());

    return textShapeData->documentOffset();
}

qreal ParagraphFragment::shapeBottom() const
{
    return shapeTop() + textShape()->size().height();
}

QPointF ParagraphFragment::mapDocumentToText(QPointF point) const
{
    QMatrix matrix = textShape()->absoluteTransformation(NULL);
    matrix.translate(0.0, -shapeTop());
    return matrix.inverted().map(point);
}

QPointF ParagraphFragment::mapTextToDocument(QPointF point) const
{
    QMatrix matrix = textShape()->absoluteTransformation(NULL);
    matrix.translate(0.0, -shapeTop());
    return matrix.map(point);
}

QLineF ParagraphFragment::mapTextToDocument(QLineF line) const
{
    QMatrix matrix = textShape()->absoluteTransformation(NULL);
    matrix.translate(0.0, -shapeTop());
    return matrix.map(line);
}

