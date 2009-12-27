/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#ifndef ABSTRACTTEXTCOMMMAND_H
#define ABSTRACTTEXTCOMMMAND_H

#include <QUndoCommand>

class TextTool;

/**
 * Base class for all commands that work together with the textTool.
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
class TextCommandBase : public QUndoCommand
{
public:
    /// constructor
    TextCommandBase(QUndoCommand *parent) : QUndoCommand(parent), m_tool(0) {}
    virtual ~TextCommandBase() {}
    /// method called by the tool.
    void setTool(TextTool *tool) {
        m_tool = tool;
    }

    // reimplemented from QUndoCommand
    virtual void redo();
    // reimplemented from QUndoCommand
    virtual void undo();

    /// Sets the m_allowAddUndoCommand of the associated tool
    void setAllow(bool set);
protected:
    class UndoRedoFinalizer
    {
    public:
        UndoRedoFinalizer(TextCommandBase* parent) : m_parent(parent) {}
        ~UndoRedoFinalizer();
    private:
        TextCommandBase* m_parent;
    };

    TextTool *m_tool;
};

#endif
