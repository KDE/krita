/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_macro_based_undo_store.h"

#include "kis_saved_commands.h"
#include "kis_global.h"


struct KisMacroBasedUndoStore::Private
{
    Private(KisSavedMacroCommand *_command) : command(_command) {}
    KisSavedMacroCommand *command;
};

KisMacroBasedUndoStore::KisMacroBasedUndoStore(KisSavedMacroCommand *_command)
    : m_d(new Private(_command))
{
}

KisMacroBasedUndoStore::~KisMacroBasedUndoStore()
{
}

const KUndo2Command* KisMacroBasedUndoStore::presentCommand()
{
    KIS_ASSERT(0 && "Not implemented");
    return 0;
}

void KisMacroBasedUndoStore::undoLastCommand()
{
    KIS_ASSERT(0 && "Not implemented");
}

void KisMacroBasedUndoStore::addCommand(KUndo2Command *cmd)
{
    /**
     * This store if stacked with a post-execution undo adapter,
     * so we should call redo() explicitly when adding a command.
     */
    cmd->redo();
    m_d->command->addCommand(toQShared(cmd));
}

void KisMacroBasedUndoStore::beginMacro(const KUndo2MagicString& macroName)
{
    Q_UNUSED(macroName);
    KIS_ASSERT(0 && "Not implemented");
}

void KisMacroBasedUndoStore::endMacro()
{
    KIS_ASSERT(0 && "Not implemented");
}

void KisMacroBasedUndoStore::purgeRedoState()
{
    KIS_ASSERT(0 && "Not implemented");
}

