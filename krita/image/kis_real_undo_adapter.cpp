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

#include "kis_real_undo_adapter.h"

#include "KoDocument.h"
#include <kundo2stack.h>
#include "kis_image.h"
#include "kis_transaction_data.h"
#include "commands/kis_scheduled_undo_command.h"


KisRealUndoAdapter::KisRealUndoAdapter(KoDocument *doc)
    : m_doc(doc)
{
}

const KUndo2Command* KisRealUndoAdapter::presentCommand()
{
    return m_doc->undoStack()->command(m_doc->undoStack()->index() - 1);
}

void KisRealUndoAdapter::undoLastCommand()
{
    /**
     * FIXME: Added as a workaround for being able to cancel creation
     * of the new adjustment mask (or any other mask whose
     * creation can be cancelled).
     *
     * Ideally, we should use "addToIndex-commit" technique like git does.
     * When a user presses Create button, we call command->redo()
     * and save this command in a cache. When the user confirms creation
     * of the layer with "OK" button, we "commit" the command to the undoStack.
     * If the user changes his mind and presses Cancel, we just call
     * command->undo() and remove the cache without committing it
     * to the undoStack
     */
    m_doc->undoStack()->undo();
}

void KisRealUndoAdapter::addCommand(KUndo2CommandSP command)
{
    if(!command) return;

    /**
     * FIXME: the transaction commits the data without calling
     * redo() method, so this is a workaround until all the tools
     * are ported to strokes
     */
    if(dynamic_cast<KisTransactionData*>(command.data())) {
        command->redo();
    }

    KUndo2Command *commandPointer =
        new KisScheduledUndoCommand(command, image(), true);

    m_doc->addCommand(commandPointer);
}

void KisRealUndoAdapter::beginMacro(const QString& macroName)
{
    m_doc->beginMacro(macroName);
}

void KisRealUndoAdapter::endMacro()
{
    m_doc->endMacro();
}

