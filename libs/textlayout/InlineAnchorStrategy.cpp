/* This file is part of the KDE project
 * Copyright (C) 2011 Matus Hanzes <matus.hanzes@ixonos.com>
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

#include "InlineAnchorStrategy.h"

#include <KoShapeContainer.h>
#include <KoTextShapeData.h>

#include <QTextLayout>
#include <QTextBlock>

InlineAnchorStrategy::InlineAnchorStrategy(KoTextAnchor *anchor, KoTextLayoutRootArea *rootArea)
        : AnchorStrategy(anchor, rootArea)
{
}

InlineAnchorStrategy::~InlineAnchorStrategy()
{
}

bool InlineAnchorStrategy::moveSubject()
{
    if (!m_anchor->shape()->parent()) {
        return false; // let's fake we moved to force another relayout
    }

    KoTextShapeData *data = qobject_cast<KoTextShapeData*>(m_anchor->shape()->parent()->userData());
    if (!data) {
        return false; // let's fake we moved to force another relayout
    }

    QPointF newPosition;
    QTextBlock block = m_anchor->document()->findBlock(m_anchor->positionInDocument());
    QTextLayout *layout = block.layout();

    // set anchor bounding rectangle horizontal position and size
    if (!countHorizontalPos(newPosition, block, layout)) {
        return false; // let's fake we moved to force another relayout
    }

    // set anchor bounding rectangle vertical position
    if (!countVerticalPos(newPosition, data, block, layout)) {
        return false; // let's fake we moved to force another relayout
    }

    if (newPosition == m_anchor->shape()->position()) {
        return true;
    }

    // set the shape to the proper position based on the data
    m_anchor->shape()->update();
    m_anchor->shape()->setPosition(newPosition);
    m_anchor->shape()->update();

    return true; // fake no move as we don't wrap around inline so no need to waste cpu
}

bool InlineAnchorStrategy::countHorizontalPos(QPointF &newPosition, QTextBlock &block, QTextLayout *layout)
{
    if (layout->lineCount() != 0) {
        QTextLine tl = layout->lineForTextPosition(m_anchor->positionInDocument() - block.position());
        if (tl.isValid()) {
            newPosition.setX(tl.cursorToX(m_anchor->positionInDocument() - block.position()));
        } else {
            return false; // lets go for a second round.
        }
    } else {
        return false; // lets go for a second round.
    }
    return true;
}

bool InlineAnchorStrategy::countVerticalPos(QPointF &newPosition, KoTextShapeData *data, QTextBlock &block, QTextLayout *layout)
{
    if (layout->lineCount()) {
        QTextLine tl = layout->lineForTextPosition(m_anchor->positionInDocument() - block.position());
        Q_ASSERT(tl.isValid());
        if (m_anchor->inlineObjectAscent() > 0) {
            newPosition.setY(tl.y() + tl.ascent() - m_anchor->inlineObjectAscent() - data->documentOffset());
        } else {
            newPosition.setY(tl.y() + tl.ascent() + m_anchor->inlineObjectDescent() - m_anchor->shape()->size().height() - data->documentOffset());
        }
    } else {
        return false; // lets go for a second round.
    }
    return true;
}
