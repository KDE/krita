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
#ifndef KO_TEXT_COMMAND_BASE_H
#define KO_TEXT_COMMAND_BASE_H

#include <kundo2command.h>

#include "kritatext_export.h"

class KRITATEXT_EXPORT KoUndoableTool {
public:
    virtual ~KoUndoableTool(){}
    virtual void setAddUndoCommandAllowed(bool allowed) = 0;
};

/**
 * Base class for all commands that work together with a tool that needs to handle undo/redo
 * in a tricky way.
 * Due to the fact that QTextDocument has its own undo queue we need to do some trickery
 * to integrate that into the apps.
 * If your command in some way changes the document, it will create unwanted undo commands in the undoStack
 * unless you inherit from this class and simply implement your undo and redo like this:
@code
void MyCommand::redo() {
    TextCommandBase::redo();
    UndoRedoFinalizer finalizer(m_tool);
    // rest code
}

void MyCommand::undo() {
    TextCommandBase::undo();
    UndoRedoFinalizer finalizer(m_tool);
    // rest code
}
@endcode
 * @see TextTool::addCommand()
 */
class KRITATEXT_EXPORT KoTextCommandBase : public KUndo2Command
{
public:

    /// constructor
    explicit KoTextCommandBase(KUndo2Command *parent);
    virtual ~KoTextCommandBase();

    /// method called by the tool.
    void setTool(KoUndoableTool *tool);

    // reimplemented from KUndo2Command
    virtual void redo();
    // reimplemented from KUndo2Command
    virtual void undo();

    /// Sets the m_allowAddUndoCommand of the associated tool
    void setAllow(bool set);

protected:

    class KRITATEXT_EXPORT UndoRedoFinalizer
    {
    public:
        explicit UndoRedoFinalizer(KoTextCommandBase* parent) : m_parent(parent) {}
        ~UndoRedoFinalizer();
    private:
        KoTextCommandBase* m_parent;
    };

    KoUndoableTool *m_tool;
};

#endif
