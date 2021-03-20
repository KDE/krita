/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SURROGATE_UNDO_ADAPTER_H
#define __KIS_SURROGATE_UNDO_ADAPTER_H

#include "kis_undo_adapter.h"

class KisSurrogateUndoStore;
class KUndo2MagicString;


class KRITAIMAGE_EXPORT KisSurrogateUndoAdapter : public KisUndoAdapter
{
public:
    KisSurrogateUndoAdapter();
    ~KisSurrogateUndoAdapter() override;

    const KUndo2Command* presentCommand() override;
    void undoLastCommand() override;
    void addCommand(KUndo2Command *command) override;
    void beginMacro(const KUndo2MagicString& macroName) override;
    void endMacro() override;

    void undo();
    void redo();

    void undoAll();
    void redoAll();

private:
    KisSurrogateUndoStore *m_undoStore;
};

#endif /* __KIS_SURROGATE_UNDO_ADAPTER_H */
