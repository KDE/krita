/* This file is part of the KDE project
 * Copyright (C) 2008 Pierre Stirnweiss \pierre.stirnweiss_koffice@gadz.org>
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

#ifndef ACCEPTCHANGECOMMAND_H
#define ACCEPTCHANGECOMMAND_H

#include "commands/TextCommandBase.h"

class KoChangeTracker;

class QTextDocument;

class AcceptChangeCommand : public TextCommandBase
{
public:
    AcceptChangeCommand(int changeId, int changeStart, int ChangeEnd, QTextDocument *document, QUndoCommand *parent = 0);
    ~AcceptChangeCommand();

    virtual void redo();
    virtual void undo();

private:
    bool m_first;
    int m_changeId, m_changeStart, m_changeEnd;
    QTextDocument *m_document;
    KoChangeTracker *m_changeTracker;
};

#endif // ACCEPTCHANGECOMMAND_H
