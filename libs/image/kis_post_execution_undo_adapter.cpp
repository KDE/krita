/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_post_execution_undo_adapter.h"

#include "kis_undo_store.h"
#include "kis_image_interfaces.h"
#include "commands_new/kis_saved_commands.h"


KisPostExecutionUndoAdapter::KisPostExecutionUndoAdapter(KisUndoStore *undoStore,
                                                         KisStrokesFacade *strokesFacade)
    : m_undoStore(undoStore),
      m_strokesFacade(strokesFacade)
{
}

void KisPostExecutionUndoAdapter::addCommand(KUndo2CommandSP command)
{
    if(!command) return;
    KisSavedCommand *m = new KisSavedCommand(command, m_strokesFacade);

    m_undoStore->addCommand(m);
}

KisSavedMacroCommand* KisPostExecutionUndoAdapter::createMacro(const KUndo2MagicString& macroName)
{
    return new KisSavedMacroCommand(macroName, m_strokesFacade);
}

void KisPostExecutionUndoAdapter::addMacro(KisSavedMacroCommand *macro)
{
    m_undoStore->addCommand(macro);
}

void KisPostExecutionUndoAdapter::setUndoStore(KisUndoStore *undoStore)
{
    m_undoStore = undoStore;
}

KisStrokesFacade* KisPostExecutionUndoAdapter::strokesFacade() const
{
    return m_strokesFacade;
}
