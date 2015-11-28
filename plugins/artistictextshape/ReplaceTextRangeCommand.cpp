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

#include "ReplaceTextRangeCommand.h"
#include "ArtisticTextShape.h"
#include <klocalizedstring.h>

ReplaceTextRangeCommand::ReplaceTextRangeCommand(ArtisticTextShape *shape, const QString &text, int from, int count, ArtisticTextTool *tool, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_tool(tool)
    , m_shape(shape)
    , m_from(from)
    , m_count(count)
{
    setText(kundo2_i18n("Replace text range"));
    m_newFormattedText.append(ArtisticTextRange(text, shape->fontAt(m_from)));
    m_oldFormattedText = shape->text();
}

ReplaceTextRangeCommand::ReplaceTextRangeCommand(ArtisticTextShape *shape, const ArtisticTextRange &text, int from, int count, ArtisticTextTool *tool, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_tool(tool)
    , m_shape(shape)
    , m_from(from)
    , m_count(count)
{
    setText(kundo2_i18n("Replace text range"));
    m_newFormattedText.append(text);
    m_oldFormattedText = shape->text();
}

ReplaceTextRangeCommand::ReplaceTextRangeCommand(ArtisticTextShape *shape, const QList<ArtisticTextRange> &text, int from, int count, ArtisticTextTool *tool, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_tool(tool)
    , m_shape(shape)
    , m_from(from)
    , m_count(count)
{
    setText(kundo2_i18n("Replace text range"));
    m_newFormattedText = text;
    m_oldFormattedText = shape->text();
}

void ReplaceTextRangeCommand::redo()
{
    KUndo2Command::redo();

    if (!m_shape) {
        return;
    }

    m_shape->replaceText(m_from, m_count, m_newFormattedText);

    if (m_tool) {
        int length = 0;
        Q_FOREACH (const ArtisticTextRange &range, m_newFormattedText) {
            length += range.text().length();
        }
        m_tool->setTextCursor(m_shape, m_from + length);
    }
}

void ReplaceTextRangeCommand::undo()
{
    KUndo2Command::undo();

    if (! m_shape) {
        return;
    }

    m_shape->clear();
    Q_FOREACH (const ArtisticTextRange &range, m_oldFormattedText) {
        m_shape->appendText(range);
    }
    if (m_tool) {
        m_tool->setTextCursor(m_shape, m_from);
    }
}
