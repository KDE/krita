/*
 *  SPDX-FileCopyrightText: 2014 Stuart Dickson <stuartmd@kogmbh.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_node_move_command2.h"

#include "kis_selection_mask.h"


KisNodeMoveCommand2::KisNodeMoveCommand2(KisNodeSP object, const QPoint& oldPos, const QPoint& newPos, KUndo2Command *parent)
    : KisMoveCommandCommon<KisNodeSP>(object, oldPos, newPos, parent)
{
}

void KisNodeMoveCommand2::redo() {
    KisMoveCommandCommon<KisNodeSP>::redo();
    tryNotifySelection(m_object);
}

void KisNodeMoveCommand2::undo() {
    KisMoveCommandCommon<KisNodeSP>::undo();
    tryNotifySelection(m_object);
}

void KisNodeMoveCommand2::tryNotifySelection(KisNodeSP node)
{
    KisSelectionMask *mask = dynamic_cast<KisSelectionMask*>(node.data());
    if (!mask) return;

    mask->notifySelectionChangedCompressed();
}
