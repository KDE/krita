/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_surrogate_undo_adapter.h"
#include "kundo2command.h"


class SurrogateUndoCommand : public KUndo2Command
{
public:
    SurrogateUndoCommand(KUndo2CommandSP command) : m_command(command) {}
    void undo() { m_command->undo(); }
    void redo() { m_command->redo(); }

private:
    KUndo2CommandSP m_command;
};


const KUndo2Command* KisSurrogateUndoAdapter::presentCommand()
{
    return m_undoStack.command(m_undoStack.index() - 1);
}

void KisSurrogateUndoAdapter::undoLastCommand()
{
    m_undoStack.undo();
}

void KisSurrogateUndoAdapter::addCommand(KUndo2Command *command)
{
    m_undoStack.push(command);
}

void KisSurrogateUndoAdapter::addCommand(KUndo2CommandSP command)
{
    m_undoStack.push(new SurrogateUndoCommand(command));
}

void KisSurrogateUndoAdapter::beginMacro(const QString& macroName)
{
    m_undoStack.beginMacro(macroName);
}

void KisSurrogateUndoAdapter::endMacro()
{
    m_undoStack.endMacro();
}

void KisSurrogateUndoAdapter::undo()
{
    m_undoStack.undo();
}

void KisSurrogateUndoAdapter::redo()
{
    m_undoStack.redo();
}

void KisSurrogateUndoAdapter::undoAll()
{
    while(m_undoStack.canUndo()) {
        m_undoStack.undo();
    }
}

void KisSurrogateUndoAdapter::redoAll()
{
    while(m_undoStack.canRedo()) {
        m_undoStack.redo();
    }
}

