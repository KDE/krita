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

#include <klocalizedstring.h>
#include "kis_image_interfaces.h"
#include "kis_node.h"
#include "commands_new/kis_update_command.h"
#include "commands_new/kis_node_move_command2.h"


MoveStrokeStrategy::MoveStrokeStrategy(KisNodeSP node,
                                       KisUpdatesFacade *updatesFacade,
                                       KisPostExecutionUndoAdapter *undoAdapter)
    : KisStrokeStrategyUndoCommandBased(kundo2_i18n("Move"), false, undoAdapter),
      m_node(node),
      m_updatesFacade(updatesFacade),
      m_undoEnabled(true),
      m_updatesEnabled(true)
{
    setSupportsWrapAroundMode(true);
}

MoveStrokeStrategy::MoveStrokeStrategy(const MoveStrokeStrategy &rhs, bool suppressUndo)
    : KisStrokeStrategyUndoCommandBased(rhs, suppressUndo),
      m_node(rhs.m_node),
      m_updatesFacade(rhs.m_updatesFacade),
      m_finalOffset(rhs.m_finalOffset),
      m_dirtyRect(rhs.m_dirtyRect),
      m_undoEnabled(rhs.m_undoEnabled),
      m_updatesEnabled(rhs.m_updatesEnabled)
{
}

void MoveStrokeStrategy::setNode(KisNodeSP node)
{
    Q_ASSERT(!m_node);
    m_node = node;
}

void MoveStrokeStrategy::saveInitialNodeOffsets(KisNodeSP node)
{
    m_initialNodeOffsets.insert(node, QPoint(node->x(), node->y()));

    KisNodeSP child = node->firstChild();
    while(child) {
        saveInitialNodeOffsets(child);
        child = child->nextSibling();
    }
}

void MoveStrokeStrategy::initStrokeCallback()
{
    if (m_node) {
        saveInitialNodeOffsets(m_node);
    }

    KisStrokeStrategyUndoCommandBased::initStrokeCallback();
}

void MoveStrokeStrategy::finishStrokeCallback()
{
    if(m_node && m_undoEnabled) {
        KUndo2Command *updateCommand =
            new KisUpdateCommand(m_node, m_dirtyRect, m_updatesFacade, true);

        addMoveCommands(m_node, updateCommand);

        notifyCommandDone(KUndo2CommandSP(updateCommand),
                          KisStrokeJobData::SEQUENTIAL,
                          KisStrokeJobData::EXCLUSIVE);
    }

    if (m_node && !m_updatesEnabled) {
        m_updatesFacade->refreshGraphAsync(m_node, m_dirtyRect);
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
        m_finalOffset = d->offset;
    }
    else {
        KisStrokeStrategyUndoCommandBased::doStrokeCallback(data);
    }
}

void MoveStrokeStrategy::moveAndUpdate(QPoint offset)
{
    QRect dirtyRect = moveNode(m_node, offset);
    m_dirtyRect |= dirtyRect;

    if (m_updatesEnabled) {
        m_updatesFacade->refreshGraphAsync(m_node, dirtyRect);
    }
}

QRect MoveStrokeStrategy::moveNode(KisNodeSP node, QPoint offset)
{
    QRect dirtyRect = node->extent();
    QPoint newOffset = m_initialNodeOffsets[node] + offset;

    /**
     * Some layers, e.g. clones need an update to change extent(), so
     * calculate the dirty rect manually
     */
    QPoint currentOffset(node->x(), node->y());
    dirtyRect |= dirtyRect.translated(newOffset - currentOffset);

    node->setX(newOffset.x());
    node->setY(newOffset.y());
    KisNodeMoveCommand2::tryNotifySelection(node);

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

    new KisNodeMoveCommand2(node, nodeOffset - m_finalOffset, nodeOffset, parent);

    KisNodeSP child = node->firstChild();
    while(child) {
        addMoveCommands(child, parent);
        child = child->nextSibling();
    }
}

void MoveStrokeStrategy::setUndoEnabled(bool value)
{
    m_undoEnabled = value;
}

void MoveStrokeStrategy::setUpdatesEnabled(bool value)
{
    m_updatesEnabled = value;
}

bool checkSupportsLodMoves(KisNodeSP node)
{
    if (!node->supportsLodMoves()) {
        return false;
    }

    KisNodeSP child = node->firstChild();
    while(child) {
        if (!checkSupportsLodMoves(child)) {
            return false;
        }
        child = child->nextSibling();
    }

    return true;
}


KisStrokeStrategy* MoveStrokeStrategy::createLodClone(int levelOfDetail)
{
    if (!checkSupportsLodMoves(m_node)) return 0;

    MoveStrokeStrategy *clone = new MoveStrokeStrategy(*this, levelOfDetail > 0);
    clone->setUndoEnabled(false);
    this->setUpdatesEnabled(false);
    return clone;
}
