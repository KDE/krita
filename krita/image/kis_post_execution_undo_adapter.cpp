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
    m_undoStore->addCommand(new KisSavedCommand(command, m_strokesFacade));
}

KisSavedMacroCommand* KisPostExecutionUndoAdapter::createMacro(const QString& macroName)
{
    return new KisSavedMacroCommand(macroName, m_strokesFacade);
}

void KisPostExecutionUndoAdapter::addMacro(KisSavedMacroCommand *macro)
{
    m_undoStore->addCommand(macro);
}
