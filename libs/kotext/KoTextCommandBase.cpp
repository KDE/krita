/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2011 Boudewijn Rempt <boud@kde.org>
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

#include "KoTextCommandBase.h"


KoTextCommandBase::KoTextCommandBase(KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_tool(0)
{
}

KoTextCommandBase::~KoTextCommandBase()
{
}


void KoTextCommandBase::redo()
{
    KUndo2Command::redo();
    if (m_tool) {
        m_tool->setAddUndoCommandAllowed(false);
    }
}

void KoTextCommandBase::setTool(KoUndoableTool *tool) {
    m_tool = tool;
}


void KoTextCommandBase::undo()
{
    KUndo2Command::undo();
    if (m_tool) {
        m_tool->setAddUndoCommandAllowed(false);
    }
}

void KoTextCommandBase::setAllow(bool set)
{
    if (m_tool) {
        m_tool->setAddUndoCommandAllowed(set);
    }
}

KoTextCommandBase::UndoRedoFinalizer::~UndoRedoFinalizer()
{
    if (m_parent) {
        m_parent->setAllow(true);
    }
}
