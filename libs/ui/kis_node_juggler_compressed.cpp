/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include "kis_generator_layer.h"
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
 *        a layer into a single action. This behavior is implemented
 *        in tryMerge() method.
 */
struct MoveNodeStruct {
    MoveNodeStruct(KisImageSP _image, KisNodeSP _node, KisNodeSP _parent, KisNodeSP _above)
        : image(_image),
          node(_node),
          newParent(_parent),
          newAbove(_above),
          oldParent(_node->parent()),
          oldAbove(_node->prevSibling()),
          suppressNewParentRefresh(false),
          suppressOldParentRefresh(false)
    {
    }

    bool tryMerge(const MoveNodeStruct &rhs) {
        if (rhs.node != node) return false;

        bool result = true;

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
        if (oldParent && !suppressOldParentRefresh) {
            image->refreshGraphAsync(oldParent);
        }

        if (newParent && oldParent != newParent) {
            node->setDirty(image->bounds());
        }
    }

    void doUndoUpdates() {
        if (newParent && !suppressNewParentRefresh) {
            image->refreshGraphAsync(newParent);
        }

        if (oldParent && oldParent != newParent) {
            node->setDirty(image->bounds());
        }
    }

    void resolveParentCollisions(MoveNodeStruct *rhs) const {
        if (rhs->newParent == newParent) {
            rhs->suppressNewParentRefresh = true;
        }

        if (rhs->oldParent == oldParent) {
            rhs->suppressOldParentRefresh = true;
        }
    }

    KisImageSP image;
    KisNodeSP node;
    KisNodeSP newParent;
    KisNodeSP newAbove;

    KisNodeSP oldParent;
    KisNodeSP oldAbove;
    bool suppressNewParentRefresh;
    bool suppressOldParentRefresh;
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

    QPointer<KisNodeJugglerCompressed> m_parentJuggler;

public:
    BatchMoveUpdateData(KisNodeJugglerCompressed *parentJuggler)
        : m_parentJuggler(parentJuggler) {}

private:

    static void addToHashLazy(MovedNodesHash *hash, MoveNodeStructSP moveStruct) {
        if (hash->contains(moveStruct->node)) {
            bool result = hash->value(moveStruct->node)->tryMerge(*moveStruct);
            KIS_ASSERT_RECOVER_NOOP(result);
        } else {
            MovedNodesHash::const_iterator it = hash->constBegin();
            MovedNodesHash::const_iterator end = hash->constEnd();

            for (; it != end; ++it) {
                it.value()->resolveParentCollisions(moveStruct.data());
            }

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
        {
            QMutexLocker l(&m_mutex);
            addToHashLazy(&m_movedNodesInitial, moveStruct);

            // the juggler might directly forward the signal to processUnhandledUpdates,
            // which would also like to get a lock, so we should release it beforehand
        }
        if (m_parentJuggler) {
            emit m_parentJuggler->requestUpdateAsyncFromCommand();
        }
    }

    void emitFinalUpdates(KisCommandUtils::FlipFlopCommand::State state) {
        QMutexLocker l(&m_mutex);

        if (m_movedNodesUpdated.isEmpty()) return;

        MovedNodesHash::const_iterator it = m_movedNodesUpdated.constBegin();
        MovedNodesHash::const_iterator end = m_movedNodesUpdated.constEnd();

        for (; it != end; ++it) {
            if (state == KisCommandUtils::FlipFlopCommand::State::FINALIZING) {
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

    void partB() override {
        State currentState = getState();

        if (currentState == FINALIZING && isFirstRedo()) {
            /**
             * When doing the first redo() some of the updates might
             * have already been executed by the juggler itself, so we
             * should process'unhandled' updates only
             */
            m_updateData->processUnhandledUpdates();
        } else {
            /**
             * When being executed by real undo/redo operations, we
             * should emit all the update signals. No one else will do
             * that for us (juggler, which did it in the previous
             * case, might have already died).
             */
            m_updateData->emitFinalUpdates(currentState);
        }
    }
private:
    BatchMoveUpdateDataSP m_updateData;
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

    void partA() override {
        QList<KisSelectionMaskSP> *newActiveMasks;

        if (getState() == FINALIZING) {
            newActiveMasks = &m_activeAfter;
        } else {
            newActiveMasks = &m_activeBefore;
        }

        Q_FOREACH (KisSelectionMaskSP mask, *newActiveMasks) {
            mask->setActive(false);
        }
    }

    void partB() override {
        QList<KisSelectionMaskSP> *newActiveMasks;

        if (getState() == FINALIZING) {
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


/**
 * A generalized command to muve up/down a set of layer
 */
struct LowerRaiseLayer : public KisCommandUtils::AggregateCommand {
    LowerRaiseLayer(BatchMoveUpdateDataSP updateData,
                    KisImageSP image,
                    const KisNodeList &nodes,
                    KisNodeSP activeNode,
                    bool lower)
        : m_updateData(updateData),
          m_image(image),
          m_nodes(nodes),
          m_activeNode(activeNode),
          m_lower (lower) {}

    enum NodesType {
        AllLayers,
        Mixed,
        AllMasks
    };

    NodesType getNodesType(KisNodeList nodes) {
        bool hasLayers = false;
        bool hasMasks = false;

        Q_FOREACH (KisNodeSP node, nodes) {
            hasLayers |= bool(qobject_cast<KisLayer*>(node.data()));
            hasMasks |= bool(qobject_cast<KisMask*>(node.data()));
        }

        return hasLayers && hasMasks ? Mixed :
            hasLayers ? AllLayers :
            AllMasks;
    }

    bool allowsAsChildren(KisNodeSP parent, KisNodeList nodes) {
        if (!parent->isEditable(false)) return false;

        Q_FOREACH (KisNodeSP node, nodes) {
            if (!parent->allowAsChild(node)) return false;
        }

        return true;
    }

    void populateChildCommands() override {
        KisNodeList sortedNodes = KisLayerUtils::sortAndFilterAnyMergableNodesSafe(m_nodes, m_image);
        KisNodeSP headNode = m_lower ? sortedNodes.first() : sortedNodes.last();
        const NodesType nodesType = getNodesType(sortedNodes);

        KisNodeSP parent = headNode->parent();
        KisNodeSP grandParent = parent ? parent->parent() : 0;

        if (!parent->isEditable(false)) return;

        KisNodeSP newAbove;
        KisNodeSP newParent;

        if (m_lower) {
            KisNodeSP prevNode = headNode->prevSibling();

            if (prevNode) {
                if (allowsAsChildren(prevNode, sortedNodes) &&
                    !prevNode->collapsed()) {

                    newAbove = prevNode->lastChild();
                    newParent = prevNode;
                } else {
                    newAbove = prevNode->prevSibling();
                    newParent = parent;
                }
            } else if ((nodesType == AllLayers && grandParent) ||
                       (nodesType == AllMasks && grandParent && grandParent->parent())) {
                newAbove = parent->prevSibling();
                newParent = grandParent;

            } else if (nodesType == AllMasks &&
                       grandParent && !grandParent->parent() &&
                       (prevNode = parent->prevSibling()) &&
                       prevNode->inherits("KisLayer")) {

                newAbove = prevNode->lastChild();
                newParent = prevNode; // NOTE: this is an updated 'prevNode'!
            }
        } else {
            KisNodeSP nextNode = headNode->nextSibling();

            if (nextNode) {
                if (allowsAsChildren(nextNode, sortedNodes) &&
                        !nextNode->collapsed()) {
                    newAbove = 0;
                    newParent = nextNode;
                } else {
                    newAbove = nextNode;
                    newParent = parent;
                }
            } else if ((nodesType == AllLayers && grandParent) ||
                       (nodesType == AllMasks && grandParent && grandParent->parent())) {
                newAbove = parent;
                newParent = grandParent;

            } else if (nodesType == AllMasks &&
                       grandParent && !grandParent->parent() &&
                       (nextNode = parent->nextSibling()) &&
                       nextNode->inherits("KisLayer")) {

                newAbove = 0;
                newParent = nextNode; // NOTE: this is an updated 'nextNode'!
            }
        }

        if (!newParent) return;

        addCommand(new KisLayerUtils::KeepNodesSelectedCommand(sortedNodes, sortedNodes,
                                                               m_activeNode, m_activeNode,
                                                               m_image, false));

        KisNodeSP currentAbove = newAbove;
        Q_FOREACH (KisNodeSP node, sortedNodes) {
            if (node->parent() != newParent && !newParent->allowAsChild(node)) {
                continue;
            }

            MoveNodeStructSP moveStruct = toQShared(new MoveNodeStruct(m_image, node, newParent, currentAbove));
            addCommand(new KisImageLayerMoveCommand(m_image, node, newParent, currentAbove, false));
            m_updateData->addInitialUpdate(moveStruct);
            currentAbove = node;
        }

        addCommand(new KisLayerUtils::KeepNodesSelectedCommand(sortedNodes, sortedNodes,
                                                               m_activeNode, m_activeNode,
                                                               m_image, true));
    }

private:
    BatchMoveUpdateDataSP m_updateData;
    KisImageSP m_image;
    KisNodeList m_nodes;
    KisNodeSP m_activeNode;
    bool m_lower;
};

struct DuplicateLayers : public KisCommandUtils::AggregateCommand {
    enum Mode {
        MOVE,
        COPY,
        ADD
    };

    DuplicateLayers(BatchMoveUpdateDataSP updateData,
                    KisImageSP image,
                    const KisNodeList &nodes,
                    KisNodeSP dstParent,
                    KisNodeSP dstAbove,
                    KisNodeSP activeNode,
                    Mode mode)
        : m_updateData(updateData),
          m_image(image),
          m_nodes(nodes),
          m_dstParent(dstParent),
          m_dstAbove(dstAbove),
          m_activeNode(activeNode),
          m_mode(mode) {}

    void populateChildCommands() override {
        KisNodeList filteredNodes = KisLayerUtils::sortAndFilterAnyMergableNodesSafe(m_nodes, m_image);

        if (filteredNodes.isEmpty()) return;

        KisNodeSP newAbove = filteredNodes.last();

        // make sure we don't add the new layer into a locked group
        while (newAbove->parent() && !newAbove->parent()->isEditable(false)) {
            newAbove = newAbove->parent();
        }

        KisNodeSP newParent = newAbove->parent();

        // override parent if provided externally
        if (m_dstParent) {
            newAbove = m_dstAbove;
            newParent = m_dstParent;
        }

        const int indexOfActiveNode = filteredNodes.indexOf(m_activeNode);
        QList<KisSelectionMaskSP> activeMasks = findActiveSelectionMasks(filteredNodes);

        // we will deactivate the masks before processing, so we should
        // save their list in a convenient form
        QSet<KisNodeSP> activeMaskNodes;
        Q_FOREACH (KisSelectionMaskSP mask, activeMasks) {
            activeMaskNodes.insert(mask);
        }

        const bool haveActiveMasks = !activeMasks.isEmpty();

        if (!newParent) return;

        addCommand(new KisLayerUtils::KeepNodesSelectedCommand(filteredNodes, KisNodeList(),
                                                               m_activeNode, KisNodeSP(),
                                                               m_image, false));

        if (haveActiveMasks) {
            /**
             * We should first disable the currently active masks, after the operation
             * completed their cloned counterparts will be activated instead.
             *
             * HINT: we should deactivate the masks before cloning, because otherwise
             *       KisGroupLayer::allowAsChild() will not let the second mask to be
             *       added to the list of child nodes. See bug 382315.
             */

            addCommand(new ActivateSelectionMasksCommand(activeMasks,
                                                         QList<KisSelectionMaskSP>(),
                                                         false));
        }

        KisNodeList newNodes;
        QList<KisSelectionMaskSP> newActiveMasks;
        KisNodeSP currentAbove = newAbove;
        Q_FOREACH (KisNodeSP node, filteredNodes) {
            if (m_mode == COPY || m_mode == ADD) {
                KisNodeSP newNode;

                if (m_mode == COPY) {
                    newNode = node->clone();
                    KisLayerUtils::addCopyOfNameTag(newNode);
                } else {
                    newNode = node;
                }

                newNodes << newNode;
                if (haveActiveMasks && activeMaskNodes.contains(node)) {
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
            } else if (m_mode == MOVE) {
                KisNodeSP newNode = node;

                newNodes << newNode;
                if (haveActiveMasks && activeMaskNodes.contains(node)) {
                    KisSelectionMaskSP mask = dynamic_cast<KisSelectionMask*>(newNode.data());
                    newActiveMasks << mask;
                }

                MoveNodeStructSP moveStruct = toQShared(new MoveNodeStruct(m_image, newNode, newParent, currentAbove));
                m_updateData->addInitialUpdate(moveStruct);

                addCommand(new KisImageLayerMoveCommand(m_image, newNode,
                                                        newParent,
                                                        currentAbove,
                                                        false));
                currentAbove = newNode;
            }
        }


        if (haveActiveMasks) {
            /**
             * Activate the cloned counterparts of the masks after the operation
             * is complete.
             */
            addCommand(new ActivateSelectionMasksCommand(QList<KisSelectionMaskSP>(),
                                                         newActiveMasks,
                                                         true));
        }

        KisNodeSP newActiveNode = newNodes[qBound(0, indexOfActiveNode, newNodes.size() - 1)];

        addCommand(new KisLayerUtils::KeepNodesSelectedCommand(KisNodeList(), newNodes,
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
    KisImageSP m_image;
    KisNodeList m_nodes;
    KisNodeSP m_dstParent;
    KisNodeSP m_dstAbove;
    KisNodeSP m_activeNode;
    Mode m_mode;
};

struct RemoveLayers : private KisLayerUtils::RemoveNodeHelper, public KisCommandUtils::AggregateCommand {
    RemoveLayers(BatchMoveUpdateDataSP updateData,
                 KisImageSP image,
                 const KisNodeList &nodes,
                 KisNodeSP activeNode)
        : m_updateData(updateData),
          m_image(image),
          m_nodes(nodes),
          m_activeNode(activeNode){}

    void populateChildCommands() override {
        KisNodeList filteredNodes = m_nodes;
        KisLayerUtils::filterMergableNodes(filteredNodes, true);
        KisLayerUtils::filterUnlockedNodes(filteredNodes);

        if (filteredNodes.isEmpty()) return;

        Q_FOREACH (KisNodeSP node, filteredNodes) {
            MoveNodeStructSP moveStruct = toQShared(new MoveNodeStruct(m_image, node, KisNodeSP(), KisNodeSP()));
            m_updateData->addInitialUpdate(moveStruct);
        }

        addCommand(new KisLayerUtils::KeepNodesSelectedCommand(filteredNodes, KisNodeList(),
                                                               m_activeNode, KisNodeSP(),
                                                               m_image, false));

        safeRemoveMultipleNodes(filteredNodes, m_image);

        addCommand(new KisLayerUtils::KeepNodesSelectedCommand(filteredNodes, KisNodeList(),
                                                               m_activeNode, KisNodeSP(),
                                                               m_image, true));
    }
protected:
    void addCommandImpl(KUndo2Command *cmd) override {
        addCommand(cmd);
    }

private:
    BatchMoveUpdateDataSP m_updateData;
    KisImageSP m_image;
    KisNodeList m_nodes;
    KisNodeSP m_activeNode;
};

struct KisNodeJugglerCompressed::Private
{
    Private(KisNodeJugglerCompressed *juggler, const KUndo2MagicString &_actionName, KisImageSP _image, KisNodeManager *_nodeManager, int _timeout)
        : actionName(_actionName),
          image(_image),
          nodeManager(_nodeManager),
          compressor(_timeout, KisSignalCompressor::FIRST_ACTIVE_POSTPONE_NEXT),
          selfDestructionCompressor(3 * _timeout, KisSignalCompressor::POSTPONE),
          updateData(new BatchMoveUpdateData(juggler)),
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
    : m_d(new Private(this, actionName, image, nodeManager, timeout))
{

    KisImageSignalVector emitSignals;

    m_d->applicator.reset(
        new KisProcessingApplicator(m_d->image, 0,
                                    KisProcessingApplicator::NONE,
                                    emitSignals,
                                    actionName));
    connect(this, SIGNAL(requestUpdateAsyncFromCommand()), SLOT(startTimers()));
    connect(&m_d->compressor, SIGNAL(timeout()), SLOT(slotUpdateTimeout()));

    connect(m_d->image, SIGNAL(sigStrokeCancellationRequested()), SLOT(slotEndStrokeRequested()));
    connect(m_d->image, SIGNAL(sigUndoDuringStrokeRequested()), SLOT(slotCancelStrokeRequested()));
    connect(m_d->image, SIGNAL(sigStrokeEndRequestedActiveNodeFiltered()), SLOT(slotEndStrokeRequested()));
    connect(m_d->image, SIGNAL(sigAboutToBeDeleted()), SLOT(slotImageAboutToBeDeleted()));

    m_d->applicator->applyCommand(
        new UpdateMovedNodesCommand(m_d->updateData, false));
    m_d->isStarted = true;
}

KisNodeJugglerCompressed::~KisNodeJugglerCompressed()
{
    KIS_ASSERT_RECOVER(!m_d->applicator) {
        m_d->applicator->end();
        m_d->applicator.reset();
    }
}

bool KisNodeJugglerCompressed::canMergeAction(const KUndo2MagicString &actionName)
{
    return actionName == m_d->actionName;
}

void KisNodeJugglerCompressed::lowerNode(const KisNodeList &nodes)
{
    KisNodeSP activeNode = m_d->nodeManager ? m_d->nodeManager->activeNode() : 0;

    m_d->applicator->applyCommand(
        new LowerRaiseLayer(m_d->updateData,
                            m_d->image,
                            nodes, activeNode, true));

}

void KisNodeJugglerCompressed::raiseNode(const KisNodeList &nodes)
{
    KisNodeSP activeNode = m_d->nodeManager ? m_d->nodeManager->activeNode() : 0;

    m_d->applicator->applyCommand(
        new LowerRaiseLayer(m_d->updateData,
                            m_d->image,
                            nodes, activeNode, false));
}

void KisNodeJugglerCompressed::removeNode(const KisNodeList &nodes)
{
    KisNodeSP activeNode = m_d->nodeManager ? m_d->nodeManager->activeNode() : 0;

    m_d->applicator->applyCommand(
        new RemoveLayers(m_d->updateData,
                         m_d->image,
                         nodes, activeNode));
}

void KisNodeJugglerCompressed::duplicateNode(const KisNodeList &nodes)
{
    KisNodeSP activeNode = m_d->nodeManager ? m_d->nodeManager->activeNode() : 0;

    m_d->applicator->applyCommand(
        new DuplicateLayers(m_d->updateData,
                            m_d->image,
                            nodes,
                            KisNodeSP(), KisNodeSP(),
                            activeNode,
                            DuplicateLayers::COPY));
}

void KisNodeJugglerCompressed::copyNode(const KisNodeList &nodes, KisNodeSP dstParent, KisNodeSP dstAbove)
{
    KisNodeSP activeNode = m_d->nodeManager ? m_d->nodeManager->activeNode() : 0;

    m_d->applicator->applyCommand(
        new DuplicateLayers(m_d->updateData,
                            m_d->image,
                            nodes,
                            dstParent, dstAbove,
                            activeNode,
                            DuplicateLayers::COPY));
}

void KisNodeJugglerCompressed::moveNode(const KisNodeList &nodes, KisNodeSP dstParent, KisNodeSP dstAbove)
{
    KisNodeSP activeNode = m_d->nodeManager ? m_d->nodeManager->activeNode() : 0;

    m_d->applicator->applyCommand(
        new DuplicateLayers(m_d->updateData,
                            m_d->image,
                            nodes,
                            dstParent, dstAbove,
                            activeNode,
                            DuplicateLayers::MOVE));
}

void KisNodeJugglerCompressed::addNode(const KisNodeList &nodes, KisNodeSP dstParent, KisNodeSP dstAbove)
{
    KisNodeSP activeNode = m_d->nodeManager ? m_d->nodeManager->activeNode() : 0;

    m_d->applicator->applyCommand(
        new DuplicateLayers(m_d->updateData,
                            m_d->image,
                            nodes,
                            dstParent, dstAbove,
                            activeNode,
                            DuplicateLayers::ADD));
}

void KisNodeJugglerCompressed::moveNode(KisNodeSP node, KisNodeSP parent, KisNodeSP above)
{
    m_d->applicator->applyCommand(new KisImageLayerMoveCommand(m_d->image, node, parent, above, false));

    MoveNodeStructSP moveStruct = toQShared(new MoveNodeStruct(m_d->image, node, parent, above));

    m_d->updateData->addInitialUpdate(moveStruct);
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
    // The juggler could have been already finished explicitly
    // by slotEndStrokeRequested(). In such a case the final updates
    // will be issued by the last command of the stroke.

    if (!m_d->updateData) return;
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
    m_d->image.clear();
    m_d->updateData.clear();
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

void KisNodeJugglerCompressed::slotImageAboutToBeDeleted()
{
    if (!m_d->isStarted) return;

    m_d->applicator->end();
    cleanup();
}

bool KisNodeJugglerCompressed::isEnded() const
{
    return !m_d->isStarted;
}
