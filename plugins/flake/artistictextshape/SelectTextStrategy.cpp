/* This file is part of the KDE project
 * Copyright (C) 2011 Jan Hambrecht <jaham@gmx.net>
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

#include "SelectTextStrategy.h"
#include "ArtisticTextTool.h"
#include "ArtisticTextToolSelection.h"

SelectTextStrategy::SelectTextStrategy(ArtisticTextTool *textTool, int cursor)
    : KoInteractionStrategy(textTool)
    , m_selection(0)
    , m_oldCursor(cursor)
    , m_newCursor(cursor)
{
    m_selection = dynamic_cast<ArtisticTextToolSelection *>(textTool->selection());
    Q_ASSERT(m_selection);
}

SelectTextStrategy::~SelectTextStrategy()
{

}

void SelectTextStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers /*modifiers*/)
{
    ArtisticTextTool *textTool = dynamic_cast<ArtisticTextTool *>(tool());
    if (!textTool) {
        return;
    }

    m_newCursor = textTool->cursorFromMousePosition(mouseLocation);
    if (m_newCursor >= 0) {
        m_selection->selectText(qMin(m_oldCursor, m_newCursor), qMax(m_oldCursor, m_newCursor));
    }
}

KUndo2Command *SelectTextStrategy::createCommand()
{
    return 0;
}

void SelectTextStrategy::finishInteraction(Qt::KeyboardModifiers /*modifiers*/)
{
    ArtisticTextTool *textTool = dynamic_cast<ArtisticTextTool *>(tool());
    if (!textTool) {
        return;
    }

    if (m_newCursor >= 0) {
        textTool->setTextCursor(m_selection->selectedShape(), m_newCursor);
    }
}
