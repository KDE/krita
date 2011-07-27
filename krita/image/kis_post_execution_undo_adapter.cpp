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
#include "kis_image.h"
#include "commands/kis_scheduled_undo_command.h"


KisPostExecutionUndoAdapter::KisPostExecutionUndoAdapter(KisUndoStore *undoStore,
                                                         KisImageWSP image)
    : m_undoStore(undoStore),
      m_image(image)
{
}

void KisPostExecutionUndoAdapter::addCommand(KUndo2CommandSP command)
{
    if(!command) return;

    KUndo2Command *commandPointer =
        new KisScheduledUndoCommand(command, m_image, false);

    m_undoStore->addCommand(commandPointer);
}

void KisPostExecutionUndoAdapter::beginMacro(const QString& macroName)
{
    m_undoStore->beginMacro(macroName);
}

void KisPostExecutionUndoAdapter::endMacro()
{
    m_undoStore->endMacro();
}

