/*
 This file is part of the KDE project
 * Copyright (C) 2009 Ganesh Paramasivam <ganesh@crystalfab.com>
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

#ifndef DELETECOMMAND_H
#define DELETECOMMAND_H

#include <QUndoStack>
#include "TextCommandBase.h"
#include <QList>

class TextTool;
class QTextCursor;
class KoChangeTrackerElement;

class DeleteCommand : public TextCommandBase
{
public:
    enum DeleteMode {
        PreviousChar,
        NextChar
    };

    DeleteCommand(DeleteMode mode, TextTool *tool, QUndoCommand* parent = 0);
    ~DeleteCommand();

    virtual void undo();
    virtual void redo();

    virtual int id() const;
    virtual bool mergeWith ( const QUndoCommand *command);

private:
    TextTool *m_tool;
    bool m_first;
    bool m_undone;
    DeleteMode m_mode;
    QList<int> m_removedElements;
    int m_addedChangeElement;

    virtual void deleteChar();
    virtual void deletePreviousChar();
    virtual void deleteSelection(QTextCursor &selection);
    virtual void removeChangeElement(int changeId);
};

#endif // DELTECOMMAND_H
