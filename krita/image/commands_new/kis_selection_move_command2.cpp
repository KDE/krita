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
