/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_document_undo_store.h"

#include "KisDocument.h"
#include <kundo2stack.h>


/*****************************************************************/
/*                KisDocumentUndoStore                           */
/*****************************************************************/

KisDocumentUndoStore::KisDocumentUndoStore(KisDocument *doc)
    : m_doc(doc)
{
    connect(doc->undoStack(), SIGNAL(indexChanged(int)), this, SIGNAL(historyStateChanged()));
}

const KUndo2Command* KisDocumentUndoStore::presentCommand()
{
    return m_doc->undoStack()->command(m_doc->undoStack()->index() - 1);
}

void KisDocumentUndoStore::undoLastCommand()
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

void KisDocumentUndoStore::addCommand(KUndo2Command *command)
{
    if(!command) return;
    m_doc->addCommand(command);
}

void KisDocumentUndoStore::beginMacro(const KUndo2MagicString& macroName)
{
    m_doc->beginMacro(macroName);
}

void KisDocumentUndoStore::endMacro()
{
    m_doc->endMacro();
}

void KisDocumentUndoStore::purgeRedoState()
{
    m_doc->undoStack()->purgeRedoState();
}
