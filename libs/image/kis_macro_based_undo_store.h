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
