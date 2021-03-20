/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

    emit historyStateChanged();
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

