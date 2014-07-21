/*
 This file is part of the KDE project
 * Copyright (C) 2009 Ganesh Paramasivam <ganesh@crystalfab.com>
 * Copyright (C) 2012 C. Boemann <cbo@boemann.dk>
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

#ifndef INSERTNOTECOMMAND_H
#define INSERTNOTECOMMAND_H

#include "KoInlineNote.h"

#include <kundo2command.h>

#include <QWeakPointer>

class QTextDocument;

class InsertNoteCommand : public KUndo2Command
{
public:

    InsertNoteCommand(KoInlineNote::Type type, QTextDocument *document);
    virtual ~InsertNoteCommand();

    virtual void undo();
    virtual void redo();

    KoInlineNote *m_inlineNote;
private:
    QWeakPointer<QTextDocument> m_document;
    bool m_first;
    int m_framePosition; // a cursor position inside the frame at the time of creation
};

#endif // INSERTNODECOMMAND_H
