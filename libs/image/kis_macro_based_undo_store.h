/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_MACRO_BASED_UNDO_STORE_H
#define __KIS_MACRO_BASED_UNDO_STORE_H

#include <QScopedPointer>
#include "kis_undo_store.h"

class KisSavedMacroCommand;


/**
 * This undo store is supposed to be stacked with a KisPostExecutionUndoAdapter.
 *
 * That chain looks like:
 *    KisPostExecutionUndoAdapter ->
 *    KisMacroBasedUndoStore ->
 *    KisSavedMacroCommand;
 *
 * So the store calls redo() on every command that is added via
 * addCommand();
 */
class KisMacroBasedUndoStore : public KisUndoStore
{
public:
    KisMacroBasedUndoStore(KisSavedMacroCommand *command);
    ~KisMacroBasedUndoStore() override;

    const KUndo2Command* presentCommand() override;
    void undoLastCommand() override;
    void addCommand(KUndo2Command *cmd) override;
    void beginMacro(const KUndo2MagicString& macroName) override;
    void endMacro() override;
    void purgeRedoState() override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_MACRO_BASED_UNDO_STORE_H */
