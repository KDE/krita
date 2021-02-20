/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_POST_EXECUTION_UNDO_ADAPTER_H
#define __KIS_POST_EXECUTION_UNDO_ADAPTER_H

#include <kritaimage_export.h>
#include "kis_types.h"

class KUndo2MagicString;
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

    KisSavedMacroCommand* createMacro(const KUndo2MagicString& macroName);
    void addMacro(KisSavedMacroCommand *macro);

    void setUndoStore(KisUndoStore *undoStore);
    KisStrokesFacade* strokesFacade() const;

private:
    KisUndoStore *m_undoStore;
    KisStrokesFacade *m_strokesFacade;
};

#endif /* __KIS_POST_EXECUTION_UNDO_ADAPTER_H */
