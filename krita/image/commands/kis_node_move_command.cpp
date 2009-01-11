/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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

#include "kis_node_commands.h"
#include "kis_node.h"

KisNodeMoveCommand::KisNodeMoveCommand(KisNodeSP node, const QPoint& oldpos, const QPoint& newpos) :
        QUndoCommand(i18n("Move")),
        m_node(node)
{
    m_oldPos = oldpos;
    m_newPos = newpos;

    QRect currentBounds = m_node->exactBounds();
    QRect oldBounds = currentBounds;
    oldBounds.translate(oldpos.x() - newpos.x(), oldpos.y() - newpos.y());

    m_updateRect = currentBounds | oldBounds;
}

KisNodeMoveCommand::~KisNodeMoveCommand()
{
}

void KisNodeMoveCommand::redo()
{
    moveTo(m_newPos);
}

void KisNodeMoveCommand::undo()
{
    moveTo(m_oldPos);
}

void KisNodeMoveCommand::moveTo(const QPoint& pos)
{
    m_node->setX(pos.x());
    m_node->setY(pos.y());

    m_node->setDirty(m_updateRect);
}
