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
#include <KoTextBlockData.h>

#include <QTextBlock>
#include <QTextLayout>

ParagraphFragment::ParagraphFragment(KoShape *shape, const QTextBlock &textBlock, KoParagraphStyle *style)
        : m_shape(shape)
{
    QTextLayout *layout = textBlock.layout();

    m_isSingleLine = (layout->lineCount() == 1);

    // border rectangle left and right
    m_border.setLeft(0.0);
    m_border.setRight(shape->size().width());

    // first line rectangle
    m_firstLine = layout->lineAt(0).rect();
    m_firstLine.setRight(m_border.right() - style->rightMargin());

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
    m_border.setTop(m_firstLine.top() - style->topMargin());
    m_border.setBottom(m_isSingleLine ? m_firstLine.bottom() + style->bottomMargin() : m_followingLines.bottom() + style->bottomMargin());

    // TODO: the lines overlap slightly so right now we simply
    // calculate the mean of the two y-values, should be handled properly
    if (!m_isSingleLine) {
        qreal lineBreak((m_firstLine.bottom() + m_followingLines.top()) / 2.0);
        m_firstLine.setBottom(lineBreak);
        m_counter.setBottom(lineBreak);
        m_followingLines.setTop(lineBreak);
    }
}

