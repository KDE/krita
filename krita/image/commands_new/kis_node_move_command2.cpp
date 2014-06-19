/*
 *  Copyright (c) 2014 Stuart Dickson <stuartmd@kogmbh.com>
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
    KisSelectionMaskSP mask = dynamic_cast<KisSelectionMask*>(node.data());
    if (!mask) return;

    mask->notifySelectionChangedCompressed();
}
