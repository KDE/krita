/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_surrogate_undo_adapter.h"
#include "kis_undo_stores.h"


KisSurrogateUndoAdapter::KisSurrogateUndoAdapter()
    : KisUndoAdapter(new KisSurrogateUndoStore)
{
    m_undoStore = static_cast<KisSurrogateUndoStore*>(undoStore());
}

KisSurrogateUndoAdapter::~KisSurrogateUndoAdapter()
{
    delete m_undoStore;
}

const KUndo2Command* KisSurrogateUndoAdapter::presentCommand()
{
    return m_undoStore->presentCommand();
}

void KisSurrogateUndoAdapter::undoLastCommand()
{
    m_undoStore->undoLastCommand();
}

void KisSurrogateUndoAdapter::addCommand(KUndo2Command *command)
{
    m_undoStore->addCommand(command);
}

void KisSurrogateUndoAdapter::beginMacro(const KUndo2MagicString& macroName)
{
    m_undoStore->beginMacro(macroName);
}

void KisSurrogateUndoAdapter::endMacro()
{
    m_undoStore->endMacro();
}

void KisSurrogateUndoAdapter::undo()
{
    m_undoStore->undo();
}

void KisSurrogateUndoAdapter::redo()
{
    m_undoStore->redo();
}

void KisSurrogateUndoAdapter::undoAll()
{
    m_undoStore->undoAll();
}

void KisSurrogateUndoAdapter::redoAll()
{
    m_undoStore->redoAll();
}

