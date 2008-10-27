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

#include <QTextBlock>
#include <QTextLayout>

ParagraphFragment::ParagraphFragment(Ruler* rulers, KoShape *shape, QTextBlock textBlock, KoParagraphStyle *style)
        : m_shape(shape),
        m_rulers(rulers)
{
    initDimensions(textBlock, style);

    initRulers();
}

void ParagraphFragment::initDimensions(QTextBlock textBlock, KoParagraphStyle *paragraphStyle)
{
    QTextLayout *layout = textBlock.layout();

    m_isSingleLine = (layout->lineCount() == 1);

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

    // matrix to map text to document coordinates
    QMatrix matrix = shape()->absoluteTransformation(NULL);
    matrix.translate(0.0, -shapeTop());

    qreal rightTop = qMax(top, m_firstLine.top());
    qreal followingTop = qMax(top, m_followingLines.top());
    qreal followingBottom = qMin(bottom, m_followingLines.bottom());

    // first line
    RulerFragment firstFragment;
    firstFragment.setVisible(top < m_firstLine.bottom());
    firstFragment.setBaseline(matrix.map(QLineF(m_border.left(), m_firstLine.top(), m_border.left(), m_firstLine.bottom())));
    m_rulers[firstIndentRuler].addFragment(firstFragment);

    // following lines
    RulerFragment followingFragment;
    followingFragment.setVisible(top < m_followingLines.bottom() && bottom > m_followingLines.top() && !m_isSingleLine);
    followingFragment.setBaseline(matrix.map(QLineF(m_border.left(), followingTop, m_border.left(), followingBottom)));
    m_rulers[followingIndentRuler].addFragment(followingFragment);

    // right margin
    RulerFragment rightFragment;
    rightFragment.setVisible(true);
    rightFragment.setBaseline(matrix.map(QLineF(m_border.right(), followingBottom, m_border.right(), rightTop)));
    m_rulers[rightMarginRuler].addFragment(rightFragment);

    // top margin
    RulerFragment topFragment;
    topFragment.setVisible(top <= m_firstLine.top());
    topFragment.setBaseline(matrix.map(QLineF(m_border.right(), m_border.top(), m_border.left(), m_border.top())));
    m_rulers[topMarginRuler].addFragment(topFragment);

    // bottom margin
    RulerFragment bottomFragment;
    bottomFragment.setVisible(bottom >= m_followingLines.bottom());
    bottomFragment.setBaseline(matrix.map(QLineF(m_border.right(), m_followingLines.bottom(), m_border.left(), m_followingLines.bottom())));
    m_rulers[bottomMarginRuler].addFragment(bottomFragment);

    // line spacing
    RulerFragment lineFragment;
    lineFragment.setVisible(!m_isSingleLine && rightTop != followingTop);
    lineFragment.setBaseline(matrix.map(QLineF(m_firstLine.right(), m_firstLine.bottom(), m_border.left(), m_firstLine.bottom())));
    m_rulers[lineSpacingRuler].addFragment(lineFragment);
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

