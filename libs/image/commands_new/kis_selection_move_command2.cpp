/*
 *  SPDX-FileCopyrightText: 2014 Stuart Dickson <stuartmd@kogmbh.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_selection_move_command2.h"

//#include "kis_image_interfaces.h"
#include "kis_selection.h"
//#include "kis_node.h"
#include "kis_image.h"

KisSelectionMoveCommand2::KisSelectionMoveCommand2(KisSelectionSP object, const QPoint& oldPos, const QPoint& newPos, KUndo2Command *parent)
    : KisMoveCommandCommon<KisSelectionSP>(object, oldPos, newPos, parent)
{
}

void KisSelectionMoveCommand2::redo() {
    KisMoveCommandCommon<KisSelectionSP>::redo();
    m_object->notifySelectionChanged();
}

void KisSelectionMoveCommand2::undo() {
    KisMoveCommandCommon<KisSelectionSP>::undo();
    m_object->notifySelectionChanged();
}
