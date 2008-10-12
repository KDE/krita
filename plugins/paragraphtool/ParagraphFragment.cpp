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
#include <KoShape.h>
#include <KoShapeBorderModel.h>
#include <KoTextBlockData.h>
#include <KoTextShapeData.h>
#include <KoViewConverter.h>

#include <KDebug>

#include <QTextBlock>
#include <QTextLayout>

ParagraphFragment::ParagraphFragment(Ruler* rulers, KoShape *shape, QTextBlock textBlock, KoParagraphStyle *style)
        : m_shape(shape),
        m_rulers(rulers)
{
    for (int ruler = 0; ruler != maxRuler; ++ruler) {
        m_rulerFragments[ruler].setRuler(&rulers[ruler]);
    }
    m_isSingleLine = (textBlock.layout()->lineCount() == 1);

    initDimensions(textBlock, style);

    initRulers();
}

void ParagraphFragment::initDimensions(QTextBlock textBlock, KoParagraphStyle *paragraphStyle)
{
    QTextLayout *layout = textBlock.layout();

    // border rectangle left and right
    m_border.setLeft(0.0);
    m_border.setRight(shape()->size().width());

    // first line rectangle
    m_firstLine = layout->lineAt(0).rect();
    m_firstLine.setRight(m_border.right() - paragraphStyle->rightMargin());

    // counter rectangle
    KoTextBlockData *blockData = static_cast<KoTextBlockData*>(textBlock.userData());
    if (blockData != NULL) {
        m_counter = QRectF(blockData->counterPosition(), QSizeF(blockData->counterWidth() - blockData->counterSpacing(), m_firstLine.height()));
    }

    // following lines rectangle
    if (!m_isSingleLine) {
        m_followingLines = QRectF(layout->lineAt(1).rect().topLeft(), layout->lineAt(layout->lineCount() - 1).rect().bottomRight());
    } else {
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

void ParagraphFragment::initRulers()
{
    qreal top(shapeTop());
    qreal bottom(shapeBottom());

    qreal rightTop = qMax(top, m_firstLine.top());
    qreal followingTop = qMax(top, m_followingLines.top());
    qreal followingBottom = qMin(bottom, m_followingLines.bottom());

    m_paintSeparator = !m_isSingleLine && rightTop != followingTop && m_rulerFragments[followingIndentRuler].isVisible();

    // first line
    m_rulerFragments[firstIndentRuler].setVisible(top < m_firstLine.bottom());
    m_rulerFragments[firstIndentRuler].setBaseline(mapTextToDocument(QLineF(m_border.left(), m_firstLine.top(), m_border.left(), m_firstLine.bottom())));
    m_rulers[firstIndentRuler].addFragment(m_rulerFragments[firstIndentRuler]);

    // following lines
    m_rulerFragments[followingIndentRuler].setVisible(top < m_followingLines.bottom() && bottom > m_followingLines.top() && !m_isSingleLine);
    m_rulerFragments[followingIndentRuler].setBaseline(mapTextToDocument(QLineF(m_border.left(), followingTop, m_border.left(), followingBottom)));
    m_rulers[followingIndentRuler].addFragment(m_rulerFragments[followingIndentRuler]);

    // right margin
    m_rulerFragments[rightMarginRuler].setVisible(true);
    m_rulerFragments[rightMarginRuler].setBaseline(mapTextToDocument(QLineF(m_border.right(), followingBottom, m_border.right(), rightTop)));
    m_rulers[rightMarginRuler].addFragment(m_rulerFragments[rightMarginRuler]);

    // top margin
    m_rulerFragments[topMarginRuler].setVisible(top <= m_firstLine.top());
    m_rulerFragments[topMarginRuler].setBaseline(mapTextToDocument(QLineF(m_border.right(), m_border.top(), m_border.left(), m_border.top())));
    m_rulers[topMarginRuler].addFragment(m_rulerFragments[topMarginRuler]);

    // bottom margin
    m_rulerFragments[bottomMarginRuler].setVisible(bottom >= m_followingLines.bottom());
    m_rulerFragments[bottomMarginRuler].setBaseline(mapTextToDocument(QLineF(m_border.right(), m_followingLines.bottom(), m_border.left(), m_followingLines.bottom())));
    m_rulers[bottomMarginRuler].addFragment(m_rulerFragments[bottomMarginRuler]);
}

void ParagraphFragment::paint(QPainter &painter) const
{
    if (m_paintSeparator) {
        painter.drawLine(mapTextToDocument(QLineF(m_border.left(), m_firstLine.bottom(), m_firstLine.right(), m_firstLine.bottom())));
    }
}

qreal ParagraphFragment::shapeTop() const
{
    KoTextShapeData *textShapeData = dynamic_cast<KoTextShapeData*>(shape()->userData());
    if (textShapeData == NULL) {
        return 0;
    }

    return textShapeData->documentOffset();
}

qreal ParagraphFragment::shapeBottom() const
{
    return shapeTop() + shape()->size().height();
}

QPointF ParagraphFragment::mapTextToDocument(QPointF point) const
{
    QMatrix matrix = shape()->absoluteTransformation(NULL);
    matrix.translate(0.0, -shapeTop());
    return matrix.map(point);
}

QLineF ParagraphFragment::mapTextToDocument(QLineF line) const
{
    QMatrix matrix = shape()->absoluteTransformation(NULL);
    matrix.translate(0.0, -shapeTop());
    return matrix.map(line);
}

