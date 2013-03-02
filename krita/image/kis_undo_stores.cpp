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

#include "kis_undo_stores.h"

#include "KoDocument.h"
#include <kundo2stack.h>


/*****************************************************************/
/*                KisSurrogateUndoStore                           */
/*****************************************************************/

KisSurrogateUndoStore::KisSurrogateUndoStore()
    : m_undoStack(new KUndo2Stack)
{
}

KisSurrogateUndoStore::~KisSurrogateUndoStore()
{
    delete m_undoStack;
}

const KUndo2Command* KisSurrogateUndoStore::presentCommand()
{
    return m_undoStack->command(m_undoStack->index() - 1);
}

void KisSurrogateUndoStore::undoLastCommand()
{
    m_undoStack->undo();
}

void KisSurrogateUndoStore::addCommand(KUndo2Command *command)
{
    if(!command) return;
    m_undoStack->push(command);
}

void KisSurrogateUndoStore::beginMacro(const QString& macroName)
{
    m_undoStack->beginMacro(macroName);
}

void KisSurrogateUndoStore::endMacro()
{
    m_undoStack->endMacro();
}

void KisSurrogateUndoStore::undo()
{
    m_undoStack->undo();
}

void KisSurrogateUndoStore::redo()
{
    m_undoStack->redo();
}

void KisSurrogateUndoStore::undoAll()
{
    while(m_undoStack->canUndo()) {
        m_undoStack->undo();
    }
}

void KisSurrogateUndoStore::redoAll()
{
    while(m_undoStack->canRedo()) {
        m_undoStack->redo();
    }
}

/*****************************************************************/
/*                KisDumbUndoStore                               */
/*****************************************************************/

const KUndo2Command* KisDumbUndoStore::presentCommand()
{
    return 0;
}

void KisDumbUndoStore::undoLastCommand()
{
    /**
     * Ermm.. Do we actually have one? We are dumb! ;)
     */
}

void KisDumbUndoStore::addCommand(KUndo2Command *command)
{
    /**
     * Ermm.. Done with it! :P
     */
    command->redo();
    delete command;
}

void KisDumbUndoStore::beginMacro(const QString& macroName)
{
    /**
     * Yes, sir! >:)
     */
    Q_UNUSED(macroName);
}

void KisDumbUndoStore::endMacro()
{
    /**
     * Roger that! :)
     */
}
