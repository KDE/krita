/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 C. Boemann <cbo@boemann.dk>
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

#include <klocale.h>
#include <QPoint>
#include "kis_node.h"
#include <kis_undo_adapter.h>


KisNodeMoveCommand2::KisNodeMoveCommand2(KisNodeSP node,
                                         const QPoint& oldPos,
                                         const QPoint& newPos,
                                         KisUndoAdapter *undoAdapter,
                                         KUndo2Command *parent)
    : KUndo2Command(i18nc("(qtundo-format)", "Move"), parent),
      m_oldPos(oldPos),
      m_newPos(newPos),
      m_node(node),
      m_undoAdapter(undoAdapter)
{
}

KisNodeMoveCommand2::~KisNodeMoveCommand2()
{
}

void KisNodeMoveCommand2::redo()
{
    moveTo(m_newPos);
}

void KisNodeMoveCommand2::undo()
{
    moveTo(m_oldPos);
}

void KisNodeMoveCommand2::moveTo(const QPoint& pos)
{
    /**
     * FIXME: Hack alert:
     * Our iterators don't have guarantees on thread-safety
     * when the offset varies. When it is fixed, remove the locking.
     * see: KisIterator::stressTest(), KisToolMove::mousePressEvent()
     */
    m_node->setX(pos.x());
    m_node->setY(pos.y());

    if(m_undoAdapter && m_node->inherits("KisSelectionMask")) {
        m_undoAdapter->emitSelectionChanged();
    }
}
