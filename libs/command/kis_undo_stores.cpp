/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_undo_stores.h"

#include <kundo2stack.h>


/*****************************************************************/
/*                KisSurrogateUndoStore                           */
/*****************************************************************/

KisSurrogateUndoStore::KisSurrogateUndoStore()
    : m_undoStack(new KUndo2Stack)
{
    connect(m_undoStack, SIGNAL(indexChanged(int)), this, SIGNAL(historyStateChanged()));
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

void KisSurrogateUndoStore::beginMacro(const KUndo2MagicString& macroName)
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

void KisSurrogateUndoStore::purgeRedoState()
{
    m_undoStack->purgeRedoState();
}

void KisSurrogateUndoStore::clear()
{
    m_undoStack->clear();
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

    emit historyStateChanged();
}

void KisDumbUndoStore::addCommand(KUndo2Command *command)
{
    /**
     * Ermm.. Done with it! :P
     */
    command->redo();
    delete command;

    emit historyStateChanged();
}

void KisDumbUndoStore::beginMacro(const KUndo2MagicString& macroName)
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

    emit historyStateChanged();
}

void KisDumbUndoStore::purgeRedoState()
{
    /**
     * Erm... what? %)
     */
}
