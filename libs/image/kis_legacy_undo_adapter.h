/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_LEGACY_UNDO_ADAPTER_H
#define __KIS_LEGACY_UNDO_ADAPTER_H

#include "kis_undo_adapter.h"
#include "kis_types.h"


/**
 * KisLegacyUndoAdapter -- blocks the strokes and updates queue,
 * and then adds the command to a store
 */
class KRITAIMAGE_EXPORT KisLegacyUndoAdapter : public KisUndoAdapter
{
public:
    KisLegacyUndoAdapter(KisUndoStore *undoStore, KisImageWSP image);

    const KUndo2Command* presentCommand() override;
    void undoLastCommand() override;
    void addCommand(KUndo2Command *cmd) override;
    void beginMacro(const KUndo2MagicString& macroName) override;
    void endMacro() override;
private:
    KisImageWSP m_image;
    qint32 m_macroCounter;
};

#endif /* __KIS_LEGACY_UNDO_ADAPTER_H */
