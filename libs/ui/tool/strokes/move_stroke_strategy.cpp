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
#include "kis_layer_utils.h"
#include "krita_utils.h"


MoveStrokeStrategy::MoveStrokeStrategy(KisNodeList nodes,
                                       KisUpdatesFacade *updatesFacade,
                                       KisStrokeUndoFacade *undoFacade)
    : KisStrokeStrategyUndoCommandBased(kundo2_i18n("Move"), false, undoFacade),
      m_nodes(),
      m_updatesFacade(updatesFacade),
      m_updatesEnabled(true)
{
    m_nodes = KisLayerUtils::sortAndFilterMergableInternalNodes(nodes, true);

    KritaUtils::filterContainer<KisNodeList>(m_nodes,
                                             [this](KisNodeSP node) {
                                                 return
                                                     !KisLayerUtils::checkIsCloneOf(node, m_nodes) &&
                                                     node->isEditable();
                                             });

    Q_FOREACH(KisNodeSP subtree, m_nodes) {
        KisLayerUtils::recursiveApplyNodes(
            subtree,
            [this](KisNodeSP node) {
                if (KisLayerUtils::checkIsCloneOf(node, m_nodes) ||
                    !node->isEditable()) {

                    m_blacklistedNodes.insert(node);
                }
            });
    }

    setSupportsWrapAroundMode(true);
}

MoveStrokeStrategy::MoveStrokeStrategy(const MoveStrokeStrategy &rhs)
    : KisStrokeStrategyUndoCommandBased(rhs),
      m_nodes(rhs.m_nodes),
      m_blacklistedNodes(rhs.m_blacklistedNodes),
      m_updatesFacade(rhs.m_updatesFacade),
      m_finalOffset(rhs.m_finalOffset),
      m_dirtyRect(rhs.m_dirtyRect),
      m_dirtyRects(rhs.m_dirtyRects),
      m_updatesEnabled(rhs.m_updatesEnabled)
{
}

void MoveStrokeStrategy::saveInitialNodeOffsets(KisNodeSP node)
{
    if (!m_blacklistedNodes.contains(node)) {
        m_initialNodeOffsets.insert(node, QPoint(node->x(), node->y()));
    }

    KisNodeSP child = node->firstChild();
    while(child) {
        saveInitialNodeOffsets(child);
        child = child->nextSibling();
    }
}

void MoveStrokeStrategy::initStrokeCallback()
{
    Q_FOREACH(KisNodeSP node, m_nodes) {
        saveInitialNodeOffsets(node);
    }

    KisStrokeStrategyUndoCommandBased::initStrokeCallback();
}

void MoveStrokeStrategy::finishStrokeCallback()
{
    Q_FOREACH (KisNodeSP node, m_nodes) {
        KUndo2Command *updateCommand =
            new KisUpdateCommand(node, m_dirtyRects[node], m_updatesFacade, true);

        addMoveCommands(node, updateCommand);

        notifyCommandDone(KUndo2CommandSP(updateCommand),
                          KisStrokeJobData::SEQUENTIAL,
                          KisStrokeJobData::EXCLUSIVE);
    }

    if (!m_updatesEnabled) {
        Q_FOREACH (KisNodeSP node, m_nodes) {
            m_updatesFacade->refreshGraphAsync(node, m_dirtyRects[node]);
        }
    }

    KisStrokeStrategyUndoCommandBased::finishStrokeCallback();
}

void MoveStrokeStrategy::cancelStrokeCallback()
{
    if (!m_nodes.isEmpty()) {
        // FIXME: make cancel job exclusive instead
        m_updatesFacade->blockUpdates();
        moveAndUpdate(QPoint());
        m_updatesFacade->unblockUpdates();
    }

    KisStrokeStrategyUndoCommandBased::cancelStrokeCallback();
}

void MoveStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    Data *d = dynamic_cast<Data*>(data);

    if(!m_nodes.isEmpty() && d) {
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

#include "kis_selection_mask.h"
#include "kis_selection.h"

void MoveStrokeStrategy::moveAndUpdate(QPoint offset)
{
    Q_FOREACH (KisNodeSP node, m_nodes) {
        QRect dirtyRect = moveNode(node, offset);
        m_dirtyRects[node] |= dirtyRect;

        if (m_updatesEnabled) {
            m_updatesFacade->refreshGraphAsync(node, dirtyRect);
        }

        if (KisSelectionMask *mask = dynamic_cast<KisSelectionMask*>(node.data())) {
            Q_UNUSED(mask);
            //mask->selection()->notifySelectionChanged();
        }
    }
}

QRect MoveStrokeStrategy::moveNode(KisNodeSP node, QPoint offset)
{
    QRect dirtyRect;

    if (!m_blacklistedNodes.contains(node)) {
        dirtyRect = node->extent();
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
    }

    KisNodeSP child = node->firstChild();
    while(child) {
        dirtyRect |= moveNode(child, offset);
        child = child->nextSibling();
    }

    return dirtyRect;
}

void MoveStrokeStrategy::addMoveCommands(KisNodeSP node, KUndo2Command *parent)
{
    if (!m_blacklistedNodes.contains(node)) {
        QPoint nodeOffset(node->x(), node->y());
        new KisNodeMoveCommand2(node, nodeOffset - m_finalOffset, nodeOffset, parent);
    }

    KisNodeSP child = node->firstChild();
    while(child) {
        addMoveCommands(child, parent);
        child = child->nextSibling();
    }
}

void MoveStrokeStrategy::setUpdatesEnabled(bool value)
{
    m_updatesEnabled = value;
}

bool checkSupportsLodMoves(KisNodeSP subtree)
{
    return
        !KisLayerUtils::recursiveFindNode(
            subtree,
            [](KisNodeSP node) -> bool {
                return !node->supportsLodMoves();
            });
}


KisStrokeStrategy* MoveStrokeStrategy::createLodClone(int levelOfDetail)
{
    Q_UNUSED(levelOfDetail);

    Q_FOREACH (KisNodeSP node, m_nodes) {
        if (!checkSupportsLodMoves(node)) return 0;
    }

    MoveStrokeStrategy *clone = new MoveStrokeStrategy(*this);
    this->setUpdatesEnabled(false);
    return clone;
}
