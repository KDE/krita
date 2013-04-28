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

#ifndef __KIS_POST_EXECUTION_UNDO_ADAPTER_H
#define __KIS_POST_EXECUTION_UNDO_ADAPTER_H

#include <krita_export.h>
#include "kis_types.h"

class KisUndoStore;
class KisSavedMacroCommand;
class KisStrokesFacade;

/**
 * KisPostExecutionUndoAdapter -- used by the strokes. It doesn't
 * call redo() when you add a command. It is assumed, that you have
 * already executed the command yourself and now just notify
 * the system about it. Warning: it doesn't inherit KisUndoAdapter
 * because it doesn't fit the contract of this class. And, more
 * important, KisTransaction should work differently with this class.
 */
class KRITAIMAGE_EXPORT KisPostExecutionUndoAdapter
{
public:
    KisPostExecutionUndoAdapter(KisUndoStore *undoStore, KisStrokesFacade *strokesFacade);

    void addCommand(KUndo2CommandSP command);

    KisSavedMacroCommand* createMacro(const QString& macroName);
    void addMacro(KisSavedMacroCommand *macro);

    inline void setUndoStore(KisUndoStore *undoStore) {
        m_undoStore = undoStore;
    }

private:
    KisUndoStore *m_undoStore;
    KisStrokesFacade *m_strokesFacade;
};

#endif /* __KIS_POST_EXECUTION_UNDO_ADAPTER_H */
