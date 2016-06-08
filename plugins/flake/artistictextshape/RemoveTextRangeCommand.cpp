/* This file is part of the KDE project
 * Copyright (C) 2007,2011 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Rob Buis <buis@kde.org>
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

#include "RemoveTextRangeCommand.h"
#include "ArtisticTextShape.h"
#include <klocalizedstring.h>

RemoveTextRangeCommand::RemoveTextRangeCommand(ArtisticTextTool *tool, ArtisticTextShape *shape, int from, unsigned int count)
    : m_tool(tool)
    , m_shape(shape)
    , m_from(from)
    , m_count(count)
{
    m_cursor = tool->textCursor();
    setText(kundo2_i18n("Remove text range"));
}

void RemoveTextRangeCommand::redo()
{
    KUndo2Command::redo();

    if (!m_shape) {
        return;
    }

    if (m_tool) {
        if (m_cursor > m_from) {
            m_tool->setTextCursor(m_shape, m_from);
        }
    }
    m_text = m_shape->removeText(m_from, m_count);
}

void RemoveTextRangeCommand::undo()
{
    KUndo2Command::undo();

    if (!m_shape) {
        return;
    }

    m_shape->insertText(m_from, m_text);

    if (m_tool) {
        m_tool->setTextCursor(m_shape, m_cursor);
    }
}
