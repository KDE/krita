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

#include "AddTextRangeCommand.h"
#include "ArtisticTextShape.h"
#include <klocalizedstring.h>

AddTextRangeCommand::AddTextRangeCommand(ArtisticTextTool *tool, ArtisticTextShape *shape, const QString &text, int from)
    : m_tool(tool)
    , m_shape(shape)
    , m_plainText(text)
    , m_formattedText(QString()
    , QFont())
    , m_from(from)
{
    setText(kundo2_i18n("Add text range"));
    m_oldFormattedText = shape->text();
}

AddTextRangeCommand::AddTextRangeCommand(ArtisticTextTool *tool, ArtisticTextShape *shape, const ArtisticTextRange &text, int from)
    : m_tool(tool), m_shape(shape), m_formattedText(text), m_from(from)
{
    setText(kundo2_i18n("Add text range"));
    m_oldFormattedText = shape->text();
}

void AddTextRangeCommand::redo()
{
    KUndo2Command::redo();

    if (!m_shape) {
        return;
    }

    if (m_plainText.isEmpty()) {
        m_shape->insertText(m_from, m_formattedText);
    } else {
        m_shape->insertText(m_from, m_plainText);
    }

    if (m_tool) {
        if (m_plainText.isEmpty()) {
            m_tool->setTextCursor(m_shape, m_from + m_formattedText.text().length());
        } else {
            m_tool->setTextCursor(m_shape, m_from + m_plainText.length());
        }
    }
}

void AddTextRangeCommand::undo()
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
