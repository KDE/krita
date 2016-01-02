/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_node_juggler_compressed.h"

#include <QHash>
#include <QSharedPointer>
#include <QPointer>

#include "kis_global.h"
#include "kis_image.h"
#include "kis_processing_applicator.h"
#include "commands/kis_image_layer_move_command.h"
#include "commands/kis_image_layer_add_command.h"
#include "kis_signal_compressor.h"
#include "kis_command_utils.h"
#include "kis_layer_utils.h"
#include "kis_node_manager.h"
#include "kis_layer.h"
#include "kis_selection_mask.h"


/**
 * A special structure that stores information about a node that was
 * moved. The purpose of the object is twofold:
 *
 *     1) When the reordering stroke is already started than the
 *        parent and sibling nodes may be not consistent anymore. So
 *        we store it separately.
 *
 *     2) This objects allows merging (compressing) multiple moves of
 *        a layer into a signle action. This behavior is implemented
 *        in tryMerge() method.
 */
struct MoveNodeStruct {
    MoveNodeStruct(KisImageSP _image, KisNodeSP _node, KisNodeSP _parent, KisNodeSP _above)
        : image(_image),
          node(_node),
          newParent(_parent),
          newAbove(_above),
          oldParent(_node->parent()),
          oldAbove(_node->prevSibling())
    {
    }

    bool tryMerge(const MoveNodeStruct &rhs) {
        if (rhs.node != node) return false;

        bool result = true;

        // qDebug() << "Merging";
        // qDebug() << ppVar(node);
        // qDebug() << ppVar(oldParent) << ppVar(newParent);
        // qDebug() << ppVar(oldAbove) << ppVar(newAbove);
        // qDebug() << ppVar(rhs.oldParent) << ppVar(rhs.newParent);
        // qDebug() << ppVar(rhs.oldAbove) << ppVar(rhs.newAbove);

        if (newParent == rhs.oldParent) {
            // 'rhs' is newer
            newParent = rhs.newParent;
            newAbove = rhs.newAbove;
        } else if (oldParent == rhs.newParent) {
            // 'this' is newer
            oldParent = rhs.oldParent;
            oldAbove = rhs.oldAbove;
        } else {
            warnKrita << "MoveNodeStruct: Trying to merge unsequential moves!";
            result = false;
        }

        return result;
    }

    void doRedoUpdates() {
        if (oldParent) {
            image->refreshGraphAsync(oldParent);
        }

        if (newParent && oldParent != newParent) {
            node->setDirty(image->bounds());
        }
    }

    void doUndoUpdates() {
        if (newParent) {
            image->refreshGraphAsync(newParent);
        }

        if (oldParent && oldParent != newParent) {
            node->setDirty(image->bounds());
        }
    }

    KisImageSP image;
    KisNodeSP node;
    KisNodeSP newParent;
    KisNodeSP newAbove;

    KisNodeSP oldParent;
    KisNodeSP oldAbove;
};

typedef QSharedPointer<MoveNodeStruct> MoveNodeStructSP;
typedef QHash<KisNodeSP, MoveNodeStructSP> MovedNodesHash;


/**
 * All the commands executed bythe stroke system are running in the
 * background asynchronously. But, at the same time, they emit updates
 * in parallel to the ones emitted by the juggler. Therefore, the
 * juggler and all its commands should share some data: which updates
 * have been requested, but not yet dispatched (m_movedNodesInitial),
 * and what updates have already been processed and executed
 * (m_movedNodesUpdated). This object is shared via a shared pointer
 * and guarantees safe (including thread-safe) access to the shared
 * data.
 */
class BatchMoveUpdateData {
    MovedNodesHash m_movedNodesInitial;
    MovedNodesHash m_movedNodesUpdated;

    QMutex m_mutex;

private:

    static void addToHashLazy(MovedNodesHash *hash, MoveNodeStructSP moveStruct) {
        if (hash->contains(moveStruct->node)) {
            bool result = hash->value(moveStruct->node)->tryMerge(*moveStruct);
            KIS_ASSERT_RECOVER_NOOP(result);
        } else {
            hash->insert(moveStruct->node, moveStruct);
        }
    }

public:

    void processUnhandledUpdates() {
        QMutexLocker l(&m_mutex);

        if (m_movedNodesInitial.isEmpty()) return;

        MovedNodesHash::const_iterator it = m_movedNodesInitial.constBegin();
        MovedNodesHash::const_iterator end = m_movedNodesInitial.constEnd();

        for (; it != end; ++it) {
            it.value()->doRedoUpdates();
            addToHashLazy(&m_movedNodesUpdated, it.value());
        }

        m_movedNodesInitial.clear();
    }

    void addInitialUpdate(MoveNodeStructSP moveStruct) {
        QMutexLocker l(&m_mutex);
        addToHashLazy(&m_movedNodesInitial, moveStruct);
    }

    void emitFinalUpdates(bool undo) {
        QMutexLocker l(&m_mutex);

        if (m_movedNodesUpdated.isEmpty()) return;

        MovedNodesHash::const_iterator it = m_movedNodesUpdated.constBegin();
        MovedNodesHash::const_iterator end = m_movedNodesUpdated.constEnd();

        for (; it != end; ++it) {
            if (!undo) {
                it.value()->doRedoUpdates();
            } else {
                it.value()->doUndoUpdates();
            }
        }
    }
};

typedef QSharedPointer<BatchMoveUpdateData> BatchMoveUpdateDataSP;

/**
 * A command that emits a update signals on the compressed move undo
 * or redo.
 */
class UpdateMovedNodesCommand : public KisCommandUtils::FlipFlopCommand
{
public:
    UpdateMovedNodesCommand(BatchMoveUpdateDataSP updateData, bool finalize, KUndo2Command *parent = 0)
        : FlipFlopCommand(finalize, parent),
          m_updateData(updateData)
    {
    }

    void end() {
        if (isFinalizing() && isFirstRedo()) {
            /**
             * When doing the first redo() some of the updates might
             * have already been executed by the juggler itself, so we
             * should process'unhandled' updates only
             */
            m_updateData->processUnhandledUpdates();
        } else {
            /**
             * When being executed by real undo/redo operations, we
             * should emit all the update signals. Noone else will do
             * that for us (juggler, which did it in the previous
             * case, might have already died).
             */
            m_updateData->emitFinalUpdates(isFinalizing());
        }
    }
private:
    BatchMoveUpdateDataSP m_updateData;
};

/**
 * A command to keep correct set of selected/active nodes thoroughout
 * the action.
 */
class KeepNodesSelectedCommand : public KisCommandUtils::FlipFlopCommand
{
public:
    KeepNodesSelectedCommand(const KisNodeList &selectedBefore,
                             const KisNodeList &selectedAfter,
                             KisNodeSP activeBefore,
                             KisNodeSP activeAfter,
                             KisImageSP image,
                             bool finalize, KUndo2Command *parent = 0)
        : FlipFlopCommand(finalize, parent),
          m_selectedBefore(selectedBefore),
          m_selectedAfter(selectedAfter),
          m_activeBefore(activeBefore),
          m_activeAfter(activeAfter),
          m_image(image)
    {
    }

    void end() {
        KisImageSignalType type;
        if (isFinalizing()) {
            type = ComplexNodeReselectionSignal(m_activeAfter, m_selectedAfter);
        } else {
            type = ComplexNodeReselectionSignal(m_activeBefore, m_selectedBefore);
        }
        m_image->signalRouter()->emitNotification(type);
    }

private:
    KisNodeList m_selectedBefore;
    KisNodeList m_selectedAfter;
    KisNodeSP m_activeBefore;
    KisNodeSP m_activeAfter;
    KisImageWSP m_image;
};

/**
 * A command to activate newly created selection masks after any action
 */
class ActivateSelectionMasksCommand : public KisCommandUtils::FlipFlopCommand
{
public:
    ActivateSelectionMasksCommand(const QList<KisSelectionMaskSP> &activeBefore,
                                  const QList<KisSelectionMaskSP> &activeAfter,
                                  bool finalize, KUndo2Command *parent = 0)
        : FlipFlopCommand(finalize, parent),
          m_activeBefore(activeBefore),
          m_activeAfter(activeAfter)
    {
    }

    void end() {
        QList<KisSelectionMaskSP> *newActiveMasks;

        if (isFinalizing()) {
            newActiveMasks = &m_activeAfter;
        } else {
            newActiveMasks = &m_activeBefore;
        }

        Q_FOREACH (KisSelectionMaskSP mask, *newActiveMasks) {
            mask->setActive(true);
        }
    }

private:
    QList<KisSelectionMaskSP> m_activeBefore;
    QList<KisSelectionMaskSP> m_activeAfter;
};

KisNodeList sortAndFilterNodes(const KisNodeList &nodes, KisImageSP image, bool allowMasks = false) {
    KisNodeList filteredNodes = nodes;
    KisNodeList sortedNodes;

    KisLayerUtils::filterMergableNodes(filteredNodes, allowMasks);
    KisLayerUtils::sortMergableNodes(image->root(), filteredNodes, sortedNodes);

    return sortedNodes;
}

/**
 * A generalized command to muve up/down a set of layer
 */
struct LowerRaiseLayer : public KisCommandUtils::AggregateCommand {
    LowerRaiseLayer(BatchMoveUpdateDataSP updateData,
                    KisNodeManager *nodeManager,
                    KisImageSP image,
                    const KisNodeList &nodes,
                    KisNodeSP activeNode,
                    bool lower)
        : m_updateData(updateData),
          m_nodeManager(nodeManager),
          m_image(image),
          m_nodes(nodes),
          m_activeNode(activeNode),
          m_lower (lower) {}

    void populateChildCommands() {
        KisNodeList sortedNodes = sortAndFilterNodes(m_nodes, m_image);
        KisNodeSP headNode = m_lower ? sortedNodes.first() : sortedNodes.last();

        KisNodeSP parent = headNode->parent();
        KisNodeSP grandParent = parent ? parent->parent() : 0;

        KisNodeSP newAbove;
        KisNodeSP newParent;

        if (m_lower) {
            KisNodeSP prevNode = headNode->prevSibling();

            if (prevNode) {
                if (prevNode->inherits("KisGroupLayer") &&
                    !prevNode->collapsed()) {

                    newAbove = prevNode->lastChild();
                    newParent = prevNode;
                } else {
                    newAbove = prevNode->prevSibling();
                    newParent = parent;
                }
            } else if (grandParent &&
                       !headNode->inherits("KisMask")) { // TODO

                newAbove = parent->prevSibling();
                newParent = grandParent;
            }
        } else {
            KisNodeSP nextNode = headNode->nextSibling();

            if (nextNode) {
                if (nextNode->inherits("KisGroupLayer") &&
                    !nextNode->collapsed()) {

                    newAbove = 0;
                    newParent = nextNode;
                } else {
                    newAbove = nextNode;
                    newParent = parent;
                }
            } else if (grandParent &&
                       !headNode->inherits("KisMask")) { // TODO

                newAbove = parent;
                newParent = grandParent;
            }
        }

        if (!newParent) return;

        addCommand(new KeepNodesSelectedCommand(sortedNodes, sortedNodes,
                                                m_activeNode, m_activeNode,
                                                m_image, false));

        KisNodeSP currentAbove = newAbove;
        Q_FOREACH (KisNodeSP node, sortedNodes) {
            MoveNodeStructSP moveStruct = toQShared(new MoveNodeStruct(m_image, node, newParent, currentAbove));
            addCommand(new KisImageLayerMoveCommand(m_image, node, newParent, currentAbove, false));
            m_updateData->addInitialUpdate(moveStruct);
            currentAbove = node;
        }

        addCommand(new KeepNodesSelectedCommand(sortedNodes, sortedNodes,
                                                m_activeNode, m_activeNode,
                                                m_image, true));
    }

private:
    BatchMoveUpdateDataSP m_updateData;
    KisNodeManager *m_nodeManager;
    KisImageSP m_image;
    KisNodeList m_nodes;
    KisNodeSP m_activeNode;
    bool m_lower;
};

struct DuplicateLayers : public KisCommandUtils::AggregateCommand {
    DuplicateLayers(BatchMoveUpdateDataSP updateData,
                    KisNodeManager *nodeManager,
                    KisImageSP image,
                    const KisNodeList &nodes,
                    KisNodeSP activeNode)
        : m_updateData(updateData),
          m_nodeManager(nodeManager),
          m_image(image),
          m_nodes(nodes),
          m_activeNode(activeNode){}

    void populateChildCommands() {
        KisNodeList filteredNodes = sortAndFilterNodes(m_nodes, m_image, true);

        if (filteredNodes.isEmpty()) return;

        KisNodeSP newAbove = filteredNodes.last();
        KisNodeSP newParent = newAbove->parent();
        const int indexOfActiveNode = filteredNodes.indexOf(m_activeNode);
        QList<KisSelectionMaskSP> activeMasks = findActiveSelectionMasks(filteredNodes);
        const bool haveActiveMasks = !activeMasks.isEmpty();

        if (!newParent) return;

        addCommand(new KeepNodesSelectedCommand(filteredNodes, KisNodeList(),
                                                m_activeNode, KisNodeSP(),
                                                m_image, false));

        if (haveActiveMasks) {
            addCommand(new ActivateSelectionMasksCommand(activeMasks,
                                                         QList<KisSelectionMaskSP>(),
                                                         false));
        }

        KisNodeList newNodes;
        QList<KisSelectionMaskSP> newActiveMasks;
        KisNodeSP currentAbove = newAbove;
        Q_FOREACH (KisNodeSP node, filteredNodes) {
            KisNodeSP newNode = node->clone();

            const QString prefix = i18n("Copy of");
            QString newName = node->name();
            if (!newName.startsWith(prefix)) {
                newName = QString("%1 %2").arg(prefix).arg(newName);
                newNode->setName(newName);
            }

            newNodes << newNode;
            if (haveActiveMasks && toActiveSelectionMask(node)) {
                KisSelectionMaskSP mask = dynamic_cast<KisSelectionMask*>(newNode.data());
                newActiveMasks << mask;
            }

            MoveNodeStructSP moveStruct = toQShared(new MoveNodeStruct(m_image, newNode, newParent, currentAbove));
            m_updateData->addInitialUpdate(moveStruct);

            addCommand(new KisImageLayerAddCommand(m_image, newNode,
                                                   newParent,
                                                   currentAbove,
                                                   false, false));
            currentAbove = newNode;
        }


        if (haveActiveMasks) {
            addCommand(new ActivateSelectionMasksCommand(QList<KisSelectionMaskSP>(),
                                                         newActiveMasks,
                                                         true));
        }

        KisNodeSP newActiveNode = newNodes[qBound(0, indexOfActiveNode, newNodes.size() - 1)];

        addCommand(new KeepNodesSelectedCommand(KisNodeList(), newNodes,
                                                KisNodeSP(), newActiveNode,
                                                m_image, true));
    }
private:
    KisSelectionMaskSP toActiveSelectionMask(KisNodeSP node) {
        KisSelectionMask *mask = dynamic_cast<KisSelectionMask*>(node.data());
        return mask && mask->active() ? mask : 0;
    }


    QList<KisSelectionMaskSP> findActiveSelectionMasks(KisNodeList nodes) {
        QList<KisSelectionMaskSP> masks;
        foreach (KisNodeSP node, nodes) {
            KisSelectionMaskSP mask = toActiveSelectionMask(node);
            if (mask) {
                masks << mask;
            }
        }
        return masks;
    }
private:
    BatchMoveUpdateDataSP m_updateData;
    KisNodeManager *m_nodeManager;
    KisImageSP m_image;
    KisNodeList m_nodes;
    KisNodeSP m_activeNode;
};

struct RemoveLayers : private KisLayerUtils::RemoveNodeHelper, public KisCommandUtils::AggregateCommand {
    RemoveLayers(BatchMoveUpdateDataSP updateData,
                 KisNodeManager *nodeManager,
                 KisImageSP image,
                 const KisNodeList &nodes,
                 KisNodeSP activeNode)
        : m_updateData(updateData),
          m_nodeManager(nodeManager),
          m_image(image),
          m_nodes(nodes),
          m_activeNode(activeNode){}

    void populateChildCommands() {
        KisNodeList filteredNodes = m_nodes;
        KisLayerUtils::filterMergableNodes(filteredNodes);

        if (filteredNodes.isEmpty()) return;

        Q_FOREACH (KisNodeSP node, filteredNodes) {
            MoveNodeStructSP moveStruct = toQShared(new MoveNodeStruct(m_image, node, KisNodeSP(), KisNodeSP()));
            m_updateData->addInitialUpdate(moveStruct);
        }

        const bool lastLayer = scanForLastLayer(m_image, filteredNodes);

        addCommand(new KeepNodesSelectedCommand(filteredNodes, KisNodeList(),
                                                m_activeNode, KisNodeSP(),
                                                m_image, false));

        safeRemoveMultipleNodes(filteredNodes, m_image);
        if (lastLayer) {
            KisLayerSP newLayer = m_nodeManager->constructDefaultLayer();
            addCommand(new KisImageLayerAddCommand(m_image, newLayer,
                                                   m_image->root(),
                                                   KisNodeSP(),
                                                   false, false));
        }

        addCommand(new KeepNodesSelectedCommand(filteredNodes, KisNodeList(),
                                                m_activeNode, KisNodeSP(),
                                                m_image, true));
    }
protected:
    virtual void addCommandImpl(KUndo2Command *cmd) {
        addCommand(cmd);
    }

    static bool scanForLastLayer(KisImageWSP image, KisNodeList nodesToRemove) {
        bool removeLayers = false;
        Q_FOREACH(KisNodeSP nodeToRemove, nodesToRemove) {
            if (dynamic_cast<KisLayer*>(nodeToRemove.data())) {
                removeLayers = true;
                break;
            }
        }
        if (!removeLayers) return false;

        bool lastLayer = true;
        KisNodeSP node = image->root()->firstChild();
        while (node) {
            if (!nodesToRemove.contains(node) &&
                dynamic_cast<KisLayer*>(node.data())) {

                lastLayer = false;
                break;
            }
            node = node->nextSibling();
        }

        return lastLayer;
    }
private:
    BatchMoveUpdateDataSP m_updateData;
    KisNodeManager *m_nodeManager;
    KisImageSP m_image;
    KisNodeList m_nodes;
    KisNodeSP m_activeNode;
};

struct KisNodeJugglerCompressed::Private
{
    Private(const KUndo2MagicString &_actionName, KisImageSP _image, KisNodeManager *_nodeManager, int _timeout)
        : actionName(_actionName),
          image(_image),
          nodeManager(_nodeManager),
          compressor(_timeout, KisSignalCompressor::POSTPONE),
          selfDestructionCompressor(3 * _timeout, KisSignalCompressor::POSTPONE),
          updateData(new BatchMoveUpdateData),
          autoDelete(false),
          isStarted(false)
    {}

    KUndo2MagicString actionName;
    KisImageSP image;
    KisNodeManager *nodeManager;
    QScopedPointer<KisProcessingApplicator> applicator;

    KisSignalCompressor compressor;
    KisSignalCompressor selfDestructionCompressor;

    BatchMoveUpdateDataSP updateData;

    bool autoDelete;
    bool isStarted;
};

KisNodeJugglerCompressed::KisNodeJugglerCompressed(const KUndo2MagicString &actionName, KisImageSP image, KisNodeManager *nodeManager, int timeout)
    : m_d(new Private(actionName, image, nodeManager, timeout))
{
    connect(m_d->image, SIGNAL(sigStrokeCancellationRequested()), SLOT(slotEndStrokeRequested()));
    connect(m_d->image, SIGNAL(sigUndoDuringStrokeRequested()), SLOT(slotCancelStrokeRequested()));
    connect(m_d->image, SIGNAL(sigStrokeEndRequestedActiveNodeFiltered()), SLOT(slotEndStrokeRequested()));

    KisImageSignalVector emitSignals;
    emitSignals << ModifiedSignal;

    m_d->applicator.reset(
        new KisProcessingApplicator(m_d->image, 0,
                                    KisProcessingApplicator::NONE,
                                    emitSignals,
                                    actionName));
    connect(&m_d->compressor, SIGNAL(timeout()), SLOT(slotUpdateTimeout()));

    m_d->applicator->applyCommand(
        new UpdateMovedNodesCommand(m_d->updateData, false));
    m_d->isStarted = true;
}

KisNodeJugglerCompressed::~KisNodeJugglerCompressed()
{
}

bool KisNodeJugglerCompressed::canMergeAction(const KUndo2MagicString &actionName)
{
    return actionName == m_d->actionName;
}

void KisNodeJugglerCompressed::lowerNode(const KisNodeList &nodes)
{
    m_d->applicator->applyCommand(
        new LowerRaiseLayer(m_d->updateData, m_d->nodeManager,
                            m_d->image,
                            nodes, m_d->nodeManager->activeNode(), true));

    startTimers();
}

void KisNodeJugglerCompressed::raiseNode(const KisNodeList &nodes)
{
    m_d->applicator->applyCommand(
        new LowerRaiseLayer(m_d->updateData, m_d->nodeManager,
                            m_d->image,
                            nodes, m_d->nodeManager->activeNode(), false));
    startTimers();
}

void KisNodeJugglerCompressed::removeNode(const KisNodeList &nodes)
{
    m_d->applicator->applyCommand(
        new RemoveLayers(m_d->updateData, m_d->nodeManager,
                         m_d->image,
                         nodes, m_d->nodeManager->activeNode()));

    startTimers();
}

void KisNodeJugglerCompressed::duplicateNode(const KisNodeList &nodes)
{
    m_d->applicator->applyCommand(
        new DuplicateLayers(m_d->updateData, m_d->nodeManager,
                            m_d->image,
                            nodes, m_d->nodeManager->activeNode()));

    startTimers();
}

void KisNodeJugglerCompressed::moveNode(KisNodeSP node, KisNodeSP parent, KisNodeSP above)
{
    m_d->applicator->applyCommand(new KisImageLayerMoveCommand(m_d->image, node, parent, above, false));

    MoveNodeStructSP moveStruct = toQShared(new MoveNodeStruct(m_d->image, node, parent, above));

    m_d->updateData->addInitialUpdate(moveStruct);
    startTimers();
}

void KisNodeJugglerCompressed::startTimers()
{
    m_d->compressor.start();

    if (m_d->autoDelete) {
        m_d->selfDestructionCompressor.start();
    }
}

void KisNodeJugglerCompressed::slotUpdateTimeout()
{
    m_d->updateData->processUnhandledUpdates();
}

void KisNodeJugglerCompressed::end()
{
    if (!m_d->isStarted) return;

    m_d->applicator->applyCommand(
        new UpdateMovedNodesCommand(m_d->updateData, true));

    m_d->applicator->end();
    cleanup();
}

void KisNodeJugglerCompressed::cleanup()
{
    m_d->applicator.reset();
    m_d->compressor.stop();
    m_d->isStarted = false;

    if (m_d->autoDelete) {
        m_d->selfDestructionCompressor.stop();
        this->deleteLater();
    }
}

void KisNodeJugglerCompressed::setAutoDelete(bool value)
{
    m_d->autoDelete = value;
    connect(&m_d->selfDestructionCompressor, SIGNAL(timeout()), SLOT(end()));
}

void KisNodeJugglerCompressed::slotEndStrokeRequested()
{
    if (!m_d->isStarted) return;
    end();
}

void KisNodeJugglerCompressed::slotCancelStrokeRequested()
{
    if (!m_d->isStarted) return;

    m_d->applicator->cancel();
    cleanup();
}

bool KisNodeJugglerCompressed::isEnded() const
{
    return !m_d->isStarted;
}

#include "kis_node_juggler_compressed.moc"
