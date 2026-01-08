/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "move_stroke_strategy.h"

#include <klocalizedstring.h>
#include "kis_image_interfaces.h"
#include "kis_node.h"
#include "commands_new/kis_update_command.h"
#include "commands_new/kis_node_move_command2.h"
#include "kis_layer_utils.h"
#include "krita_utils.h"

#include "KisRunnableStrokeJobData.h"
#include "KisRunnableStrokeJobUtils.h"
#include "KisRunnableStrokeJobsInterface.h"
#include "kis_abstract_projection_plane.h"
#include "kis_image.h"
#include "kis_image_animation_interface.h"
#include "kis_raster_keyframe_channel.h"
#include "KisAnimAutoKey.h"

#include "kis_transform_mask.h"
#include "kis_transform_mask_params_interface.h"
#include "kis_keyframe_channel.h"
#include "kis_scalar_keyframe_channel.h"
#include "kis_image_animation_interface.h"
#include "commands_new/KisSimpleModifyTransformMaskCommand.h"
#include "commands_new/KisLazyCreateTransformMaskKeyframesCommand.h"

/* MoveNodeStrategyBase and descendants
 *
 * A set of strategies that define how to actually move
 * nodes of different types. Some nodes should be moved
 * with KisNodeMoveCommand2, others with KisTransaction,
 * transform masks with their own command.
 */

struct MoveNodeStrategyBase
{
    MoveNodeStrategyBase(KisNodeSP node)
        : m_node(node),
          m_initialOffset(node->x(), node->y())
    {
    }

    virtual ~MoveNodeStrategyBase() {}

    virtual QRect moveNode(const QPoint &offset) = 0;
    virtual void finishMove(KUndo2Command *parentCommand) = 0;
    virtual QRect cancelMove() = 0;

protected:
    QRect moveNodeCommon(const QPoint &offset) {
        const QPoint newOffset = m_initialOffset + offset;

        QRect dirtyRect = m_node->projectionPlane()->tightUserVisibleBounds();

        /**
         * Some layers, e.g. clones need an update to change extent(), so
         * calculate the dirty rect manually
         */
        QPoint currentOffset(m_node->x(), m_node->y());
        dirtyRect |= dirtyRect.translated(newOffset - currentOffset);

        m_node->setX(newOffset.x());
        m_node->setY(newOffset.y());

        KisNodeMoveCommand2::tryNotifySelection(m_node);
        return dirtyRect;
    }

protected:
    KisNodeSP m_node;
    QPoint m_initialOffset;
};

struct MoveNormalNodeStrategy : public MoveNodeStrategyBase
{
    MoveNormalNodeStrategy(KisNodeSP node)
        : MoveNodeStrategyBase(node)
    {
    }

    QRect moveNode(const QPoint &offset) override {
        return moveNodeCommon(offset);
    }

    void finishMove(KUndo2Command *parentCommand) override {
        const QPoint nodeOffset(m_node->x(), m_node->y());
        new KisNodeMoveCommand2(m_node, m_initialOffset, nodeOffset, parentCommand);
    }

    QRect cancelMove() override {
        return moveNode(QPoint());
    }

};

struct MoveTransformMaskStrategy : public MoveNodeStrategyBase
{
    MoveTransformMaskStrategy(KisNodeSP node)
        : MoveNodeStrategyBase(node)
    {
    }

    QRect moveNode(const QPoint &offset) override {
        QScopedPointer<KUndo2Command> cmd;
        QRect dirtyRect = m_node->projectionPlane()->tightUserVisibleBounds();

        KisTransformMask *mask = dynamic_cast<KisTransformMask*>(m_node.data());
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(mask, QRect());

        KisTransformMaskParamsInterfaceSP oldParams = mask->transformParams();
        KisTransformMaskParamsInterfaceSP params = oldParams->clone();
        params->translateDstSpace(offset - m_currentOffset);

        cmd.reset(new KisSimpleModifyTransformMaskCommand(mask, params));
        cmd->redo();

        if (m_undoCommand) {
            const bool mergeResult = m_undoCommand->mergeWith(cmd.get());
            KIS_SAFE_ASSERT_RECOVER_NOOP(mergeResult);
            cmd.reset();
        } else {
            m_undoCommand.swap(cmd);
        }

        m_currentOffset = offset;

        dirtyRect |= m_node->projectionPlane()->tightUserVisibleBounds();

        return dirtyRect;
    }

    void finishMove(KUndo2Command *parentCommand) override {
        KisCommandUtils::CompositeCommand *cmd = new KisCommandUtils::CompositeCommand(parentCommand);
        cmd->addCommand(m_undoCommand.take());
    }

    QRect cancelMove() override {
        return moveNode(QPoint());
    }

private:
    QPoint m_currentOffset;
    QScopedPointer<KUndo2Command> m_undoCommand;
};

struct MovePaintableNodeStrategy : public MoveNodeStrategyBase
{
    MovePaintableNodeStrategy(KisNodeSP node)
        : MoveNodeStrategyBase(node),
          m_transaction(node->paintDevice(), 0, -1, 0, KisTransaction::SuppressUpdates)
    {
        // TODO: disable updates in the transaction
    }

    QRect moveNode(const QPoint &offset) override {
        return moveNodeCommon(offset);
    }

    void finishMove(KUndo2Command *parentCommand) override {
        KisCommandUtils::CompositeCommand *cmd = new KisCommandUtils::CompositeCommand(parentCommand);

        KUndo2Command *transactionCommand = m_transaction.endAndTake();
        transactionCommand->redo();
        cmd->addCommand(transactionCommand);
    }

    QRect cancelMove() override {
        QRect dirtyRect = m_node->projectionPlane()->tightUserVisibleBounds();

        m_transaction.revert();

        dirtyRect |= m_node->projectionPlane()->tightUserVisibleBounds();

        return dirtyRect;
    }

private:
    KisTransaction m_transaction;
};

/*******************************************************/
/*    MoveStrokeStrategy::Private                      */
/*******************************************************/

struct MoveStrokeStrategy::Private {
    std::unordered_map<KisNodeSP, std::unique_ptr<MoveNodeStrategyBase>> strategy;
};

template <typename Functor>
void MoveStrokeStrategy::recursiveApplyNodes(KisNodeList nodes, Functor &&func) {
    Q_FOREACH(KisNodeSP subtree, nodes) {
        KisLayerUtils::recursiveApplyNodes(subtree,
            [&] (KisNodeSP node) {
                if (!m_blacklistedNodes.contains(node)) {
                    func(node);
                }
            });
    }
}

/*******************************************************/
/*    MoveStrokeStrategy                               */
/*******************************************************/

MoveStrokeStrategy::MoveStrokeStrategy(KisNodeSelectionRecipe nodeSelection,
                                       KisUpdatesFacade *updatesFacade,
                                       KisStrokeUndoFacade *undoFacade)
    : KisStrokeStrategyUndoCommandBased(kundo2_i18n("Move"), false, undoFacade),
      m_d(new Private()),
      m_requestedNodeSelection(nodeSelection),
      m_updatesFacade(updatesFacade),
      m_updatesEnabled(true)
{
    setSupportsWrapAroundMode(true);

    enableJob(KisSimpleStrokeStrategy::JOB_INIT, true, KisStrokeJobData::BARRIER);
}

MoveStrokeStrategy::MoveStrokeStrategy(KisNodeList nodes, KisUpdatesFacade *updatesFacade, KisStrokeUndoFacade *undoFacade)
    : MoveStrokeStrategy(KisNodeSelectionRecipe(nodes), updatesFacade, undoFacade)
{
}

MoveStrokeStrategy::~MoveStrokeStrategy()
{
}

MoveStrokeStrategy::MoveStrokeStrategy(const MoveStrokeStrategy &rhs, int lod)
    : QObject(),
      KisStrokeStrategyUndoCommandBased(rhs),
      m_d(new Private()),
      m_requestedNodeSelection(rhs.m_requestedNodeSelection, lod),
      m_nodes(rhs.m_nodes),
      m_blacklistedNodes(rhs.m_blacklistedNodes),
      m_updatesFacade(rhs.m_updatesFacade),
      m_finalOffset(rhs.m_finalOffset),
      m_dirtyRects(rhs.m_dirtyRects),
      m_updatesEnabled(rhs.m_updatesEnabled)
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(rhs.m_d->strategy.empty());
}

void MoveStrokeStrategy::initStrokeCallback()
{
    /**
     * Our LodN might have already prepared the list of nodes for us,
     * so we should reuse it to avoid different nodes to be moved in
     * LodN and Lod0 modes.
     */
    if (m_updatesEnabled) {
        m_nodes = m_requestedNodeSelection.selectNodesToProcess();

        if (!m_nodes.isEmpty()) {
            m_nodes = KisLayerUtils::sortAndFilterMergeableInternalNodes(m_nodes, true);
        }

        KritaUtils::filterContainer<KisNodeList>(m_nodes, [this](KisNodeSP node) {
            /**
             * We shouldn't try to transform standalone fully empty filter masks. That 
             * just doesn't make sense.
             */
            const bool isEmptyFilterMask = node->inherits("KisFilterMask") && node->paintDevice()
                && node->paintDevice()->nonDefaultPixelArea().isEmpty();

            return !isEmptyFilterMask &&
                !KisLayerUtils::checkIsCloneOf(node, m_nodes) &&
                node->isEditable(true);
        });
        Q_FOREACH(KisNodeSP subtree, m_nodes) {
            KisLayerUtils::recursiveApplyNodes(
                        subtree,
                        [this](KisNodeSP node) {
                if (KisLayerUtils::checkIsCloneOf(node, m_nodes) ||
                        !node->isEditable(false) ||
                        (dynamic_cast<KisTransformMask*>(node.data()) &&
                         KisLayerUtils::checkIsChildOf(node, m_nodes))) {

                    m_blacklistedNodes.insert(node);
                }
            });
        }

        if (m_sharedNodes) {
            *m_sharedNodes = std::make_pair(m_nodes, m_blacklistedNodes);
        }
    } else {
        KIS_SAFE_ASSERT_RECOVER_RETURN(m_sharedNodes);
        std::tie(m_nodes, m_blacklistedNodes) = *m_sharedNodes;
    }

    if (m_nodes.isEmpty()) {
        Q_EMIT sigStrokeStartedEmpty();
        return;
    }

    QVector<KisRunnableStrokeJobData*> jobs;

    KritaUtils::addJobBarrier(jobs, [this]() {
        Q_FOREACH(KisNodeSP node, m_nodes) {
            KisLayerUtils::forceAllHiddenOriginalsUpdate(node);
        }
    });

    KritaUtils::addJobBarrier(jobs, [this]() {
        Q_FOREACH(KisNodeSP node, m_nodes) {
            KisLayerUtils::forceAllDelayedNodesUpdate(node);
        }
    });

    KritaUtils::addJobBarrier(jobs, [this]() {
        QRect handlesRect;

        /**
         * Collect handles rect
         */
        recursiveApplyNodes(m_nodes,
            [&handlesRect, this] (KisNodeSP node) {
                if (node->inherits("KisFilterMask") && m_nodes.contains(node)) {
                    /**
                     * Since the mask is contained in `m_nodes`, we are explicitly
                     * asked to transform it by the user, we should show all the non-
                     * default pixels it has (since that is exactly what we are
                     * transforming)
                     */
                    if (node->paintDevice()) {
                        handlesRect |= node->paintDevice()->nonDefaultPixelArea();
                    }
                } else {
                    handlesRect |= node->projectionPlane()->tightUserVisibleBounds();
                }
            });

        KisStrokeStrategyUndoCommandBased::initStrokeCallback();

        Q_FOREACH(KisNodeSP node, m_nodes) {
            if (node->hasEditablePaintDevice()) {
                KUndo2Command *autoKeyframeCommand =
                        KisAutoKey::tryAutoCreateDuplicatedFrame(node->paintDevice(),
                                                                 KisAutoKey::SupportsLod);
                if (autoKeyframeCommand) {
                    runAndSaveCommand(toQShared(autoKeyframeCommand), KisStrokeJobData::BARRIER, KisStrokeJobData::NORMAL);
                }
            } else if (KisTransformMask *mask = dynamic_cast<KisTransformMask*>(node.data())) {
                const bool maskAnimated = KisLazyCreateTransformMaskKeyframesCommand::maskHasAnimation(mask);

                if (maskAnimated) {
                    runAndSaveCommand(toQShared(new KisLazyCreateTransformMaskKeyframesCommand(mask)), KisStrokeJobData::BARRIER, KisStrokeJobData::NORMAL);
                }
            }
        }

        /**
         * Create strategies and start the transactions when necessary
         */
        recursiveApplyNodes(m_nodes,
            [this] (KisNodeSP node) {
                if (dynamic_cast<KisTransformMask*>(node.data())) {
                    m_d->strategy.emplace(node, new MoveTransformMaskStrategy(node));
                } else if (node->paintDevice()) {
                    m_d->strategy.emplace(node, new MovePaintableNodeStrategy(node));
                } else {
                    m_d->strategy.emplace(node, new MoveNormalNodeStrategy(node));
                }
            });

        if (m_updatesEnabled) {
            KisLodTransform t(m_nodes.first()->image()->currentLevelOfDetail());
            handlesRect = t.mapInverted(handlesRect);

            Q_EMIT this->sigHandlesRectCalculated(handlesRect);
        }

        m_updateTimer.start();
    });

    runnableJobsInterface()->addRunnableJobs(jobs);
}

void MoveStrokeStrategy::finishStrokeCallback()
{
    Q_FOREACH (KisNodeSP node, m_nodes) {
        KUndo2Command *updateCommand =
            new KisUpdateCommand(node, m_dirtyRects[node], m_updatesFacade, true);

        recursiveApplyNodes({node}, [this, updateCommand](KisNodeSP node) {
            KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->strategy.find(node) != m_d->strategy.end());

            MoveNodeStrategyBase *strategy = m_d->strategy[node].get();

            strategy->finishMove(updateCommand);
        });

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
        m_finalOffset = QPoint();
        m_hasPostponedJob = true;

        QVector<KisRunnableStrokeJobData*> jobs;

        KritaUtils::addJobBarrierExclusive(jobs, [this]() {
            Q_FOREACH (KisNodeSP node, m_nodes) {
                QRect dirtyRect;

                recursiveApplyNodes({node},
                    [this, &dirtyRect] (KisNodeSP node) {
                        MoveNodeStrategyBase *strategy =
                            m_d->strategy[node].get();
                        KIS_SAFE_ASSERT_RECOVER_RETURN(strategy);

                        dirtyRect |= strategy->cancelMove();
                    });

                m_dirtyRects[node] |= dirtyRect;

                /// Q_EMIT updates not looking onto the
                /// updatesEnabled switch, since that is
                /// the end of the stroke
                m_updatesFacade->refreshGraphAsync(node, dirtyRect);
            }
        });

        runnableJobsInterface()->addRunnableJobs(jobs);
    }

    KisStrokeStrategyUndoCommandBased::cancelStrokeCallback();
}

void MoveStrokeStrategy::tryPostUpdateJob(bool forceUpdate)
{
    if (!m_hasPostponedJob) return;

    if (forceUpdate ||
        (m_updateTimer.elapsed() > m_updateInterval &&
         !m_updatesFacade->hasUpdatesRunning())) {

        addMutatedJob(new BarrierUpdateData(forceUpdate));
    }
}

void MoveStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    if (PickLayerData *pickData = dynamic_cast<PickLayerData*>(data)) {
        KisNodeSelectionRecipe clone = m_requestedNodeSelection;
        clone.pickPoint = pickData->pos;
        Q_EMIT sigLayersPicked(clone.selectNodesToProcess());
        return;
    }

    Data *d = dynamic_cast<Data*>(data);

    if (!m_nodes.isEmpty() && d) {
        /**
         * NOTE: we do not care about threading here, because
         * all our jobs are declared sequential
         */
        m_finalOffset = d->offset;
        m_hasPostponedJob = true;
        tryPostUpdateJob(false);

    } else if (BarrierUpdateData *barrierData =
               dynamic_cast<BarrierUpdateData*>(data)) {

        doCanvasUpdate(barrierData->forceUpdate);

    } else if (KisAsynchronousStrokeUpdateHelper::UpdateData *updateData =
               dynamic_cast<KisAsynchronousStrokeUpdateHelper::UpdateData*>(data)) {

        tryPostUpdateJob(updateData->forceUpdate);

    } else {
        KisStrokeStrategyUndoCommandBased::doStrokeCallback(data);
    }
}

void MoveStrokeStrategy::doCanvasUpdate(bool forceUpdate)
{
    if (!forceUpdate &&
            (m_updateTimer.elapsed() < m_updateInterval ||
             m_updatesFacade->hasUpdatesRunning())) {

        return;
    }

    if (!m_hasPostponedJob) return;

    Q_FOREACH (KisNodeSP node, m_nodes) {
        QRect dirtyRect;

        recursiveApplyNodes({node},
            [this, &dirtyRect] (KisNodeSP node) {
                MoveNodeStrategyBase *strategy =
                    m_d->strategy[node].get();
                KIS_SAFE_ASSERT_RECOVER_RETURN(strategy);

                dirtyRect |= strategy->moveNode(m_finalOffset);
            });

        m_dirtyRects[node] |= dirtyRect;

        if (m_updatesEnabled) {
            m_updatesFacade->refreshGraphAsync(node, dirtyRect);
        }
    }

    m_hasPostponedJob = false;
    m_updateTimer.restart();
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
    KisNodeList nodesToCheck;

    if (m_requestedNodeSelection.mode == KisNodeSelectionRecipe::SelectedLayer) {
        nodesToCheck = m_requestedNodeSelection.selectedNodes;
    } else if (!m_requestedNodeSelection.selectedNodes.isEmpty()){
        /**
         * Since this function is executed in the GUI thread, we cannot properly
         * pick the layers. Therefore we should use pessimistic approach and
         * check if there are non-lodn-capable nodes in the entire image.
         */
        nodesToCheck.append(KisLayerUtils::findRoot(m_requestedNodeSelection.selectedNodes.first()));
    }

    Q_FOREACH (KisNodeSP node, nodesToCheck) {
        if (!checkSupportsLodMoves(node)) return 0;
    }

    MoveStrokeStrategy *clone = new MoveStrokeStrategy(*this, levelOfDetail);
    connect(clone, SIGNAL(sigHandlesRectCalculated(QRect)), this, SIGNAL(sigHandlesRectCalculated(QRect)));
    connect(clone, SIGNAL(sigStrokeStartedEmpty()), this, SIGNAL(sigStrokeStartedEmpty()));
    connect(clone, SIGNAL(sigLayersPicked(const KisNodeList&)), this, SIGNAL(sigLayersPicked(const KisNodeList&)));
    this->setUpdatesEnabled(false);
    m_sharedNodes.reset(new std::pair<KisNodeList, QSet<KisNodeSP>>());
    clone->m_sharedNodes = m_sharedNodes;
    return clone;
}

MoveStrokeStrategy::Data::Data(QPoint _offset)
    : KisStrokeJobData(SEQUENTIAL, NORMAL),
      offset(_offset)
{
}

KisStrokeJobData *MoveStrokeStrategy::Data::createLodClone(int levelOfDetail)
{
    return new Data(*this, levelOfDetail);
}

MoveStrokeStrategy::Data::Data(const MoveStrokeStrategy::Data &rhs, int levelOfDetail)
    : KisStrokeJobData(rhs)
{
    KisLodTransform t(levelOfDetail);
    offset = t.map(rhs.offset);
}

MoveStrokeStrategy::PickLayerData::PickLayerData(QPoint _pos)
    : KisStrokeJobData(SEQUENTIAL, NORMAL),
      pos(_pos)
{
}

KisStrokeJobData *MoveStrokeStrategy::PickLayerData::createLodClone(int levelOfDetail) {
    return new PickLayerData(*this, levelOfDetail);
}

MoveStrokeStrategy::PickLayerData::PickLayerData(const MoveStrokeStrategy::PickLayerData &rhs, int levelOfDetail)
    : KisStrokeJobData(rhs)
{
    KisLodTransform t(levelOfDetail);
    pos = t.map(rhs.pos);
}

MoveStrokeStrategy::BarrierUpdateData::BarrierUpdateData(bool _forceUpdate)
    : KisAsynchronousStrokeUpdateHelper::UpdateData(_forceUpdate, BARRIER, EXCLUSIVE)
{}

KisStrokeJobData *MoveStrokeStrategy::BarrierUpdateData::createLodClone(int levelOfDetail) {
    return new BarrierUpdateData(*this, levelOfDetail);
}

MoveStrokeStrategy::BarrierUpdateData::BarrierUpdateData(const MoveStrokeStrategy::BarrierUpdateData &rhs, int levelOfDetail)
    : KisAsynchronousStrokeUpdateHelper::UpdateData(rhs, levelOfDetail)
{
}
