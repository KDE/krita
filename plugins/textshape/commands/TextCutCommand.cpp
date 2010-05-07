/*
 This file is part of the KDE project
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
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
 * Boston, MA 02110-1301, USA.*/

#include "TextCutCommand.h"

#include <KoTextEditor.h>
#include <TextTool.h>
#include "ChangeTrackedDeleteCommand.h"
#include "DeleteCommand.h"
#include <KAction>
#include <klocale.h>

TextCutCommand::TextCutCommand(TextTool *tool, QUndoCommand *parent) :
    QUndoCommand (parent),
    m_tool(tool),
    m_first(true)
{
    setText(i18n("Cut"));
}

void TextCutCommand::undo()
{
    QUndoCommand::undo();
}

void TextCutCommand::redo()
{
    if (!m_first) {
        QUndoCommand::redo();
    } else {
        m_first = false;
        m_tool->copy();
        KoTextEditor *te = m_tool->m_textEditor.data();
        if (te == 0)
            return;
        if (m_tool->m_actionShowChanges->isChecked())
            te->addCommand(new ChangeTrackedDeleteCommand(ChangeTrackedDeleteCommand::NextChar, m_tool));
        else
            te->addCommand(new DeleteCommand(DeleteCommand::NextChar, m_tool));
    }
}
