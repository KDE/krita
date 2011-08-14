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

#include "move_stroke_strategy.h"

#include "kis_image_interfaces.h"
#include "kis_node.h"
#include "commands_new/kis_update_command.h"
#include "commands_new/kis_node_move_command2.h"


MoveStrokeStrategy::MoveStrokeStrategy(KisNodeSP node,
                                       KisUpdatesFacade *updatesFacade,
                                       KisPostExecutionUndoAdapter *undoAdapter,
                                       KisUndoAdapter *legacyUndoAdapter)
    : KisStrokeStrategyUndoCommandBased("Move stroke", false, undoAdapter),
      m_node(node),
      m_updatesFacade(updatesFacade),
      m_legacyUndoAdapter(legacyUndoAdapter)
{
}

void MoveStrokeStrategy::setNode(KisNodeSP node)
{
    Q_ASSERT(!m_node);
    m_node = node;
}

void MoveStrokeStrategy::finishStrokeCallback()
{
    if(m_node) {
        KUndo2Command *updateCommand =
            new KisUpdateCommand(m_node, m_dirtyRect, m_updatesFacade, true);

        addMoveCommands(m_node, updateCommand);

        notifyCommandDone(KUndo2CommandSP(updateCommand),
                          KisStrokeJobData::SEQUENTIAL,
                          KisStrokeJobData::EXCLUSIVE);
    }

    KisStrokeStrategyUndoCommandBased::finishStrokeCallback();
}

void MoveStrokeStrategy::cancelStrokeCallback()
{
    if(m_node) {

        // FIXME: make cancel job exclusive instead
        m_updatesFacade->blockUpdates();
        moveAndUpdate(-m_finalOffset);
        m_updatesFacade->unblockUpdates();
    }

    KisStrokeStrategyUndoCommandBased::cancelStrokeCallback();
}

void MoveStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    Data *d = dynamic_cast<Data*>(data);

    if(m_node && d) {
        moveAndUpdate(d->offset);

        /**
         * NOTE: we do not care about threading here, because
         * all our jobs are declared sequential
         */
        m_finalOffset += d->offset;
    }
    else {
        KisStrokeStrategyUndoCommandBased::doStrokeCallback(data);
    }
}

void MoveStrokeStrategy::moveAndUpdate(QPoint offset)
{
    QRect dirtyRect = moveNode(m_node, offset);
    m_dirtyRect |= dirtyRect;

    m_updatesFacade->refreshGraphAsync(m_node, dirtyRect);
}

QRect MoveStrokeStrategy::moveNode(KisNodeSP node, QPoint offset)
{
    QRect dirtyRect = node->extent();
    dirtyRect |= dirtyRect.translated(offset);

    node->setX(node->x() + offset.x());
    node->setY(node->y() + offset.y());

    KisNodeSP child = node->firstChild();
    while(child) {
        dirtyRect |= moveNode(child, offset);
        child = child->nextSibling();
    }

    return dirtyRect;
}

void MoveStrokeStrategy::addMoveCommands(KisNodeSP node, KUndo2Command *parent)
{
    QPoint nodeOffset(node->x(), node->y());

    new KisNodeMoveCommand2(node, nodeOffset - m_finalOffset, nodeOffset,
                            m_legacyUndoAdapter, parent);

    KisNodeSP child = node->firstChild();
    while(child) {
        addMoveCommands(child, parent);
        child = child->nextSibling();
    }
}
