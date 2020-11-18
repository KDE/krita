/*
 *  Copyright (c) 2013,2020 Dmitry Kazakov <dimula73@gmail.com>
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

#include "inplace_transform_stroke_strategy.h"

#include <QMutexLocker>
#include "kundo2commandextradata.h"

#include "kis_node_progress_proxy.h"

#include <klocalizedstring.h>
#include <kis_node.h>
#include <kis_group_layer.h>
#include <kis_external_layer_iface.h>
#include <kis_transaction.h>
#include <kis_painter.h>
#include <kis_transform_worker.h>
#include <kis_transform_mask.h>
#include "kis_transform_mask_adapter.h"
#include "kis_transform_utils.h"
#include "kis_abstract_projection_plane.h"
#include "kis_recalculate_transform_mask_job.h"

#include "kis_projection_leaf.h"
#include "kis_modify_transform_mask_command.h"

#include "kis_sequential_iterator.h"
#include "kis_selection_mask.h"
#include "kis_image_config.h"
#include "kis_layer_utils.h"
#include <QQueue>
#include <KisDeleteLaterWrapper.h>
#include "transform_transaction_properties.h"
#include "krita_container_utils.h"
#include "commands_new/kis_saved_commands.h"
#include "kis_command_ids.h"
#include "KisRunnableStrokeJobUtils.h"
#include "commands_new/KisHoldUIUpdatesCommand.h"
#include "KisDecoratedNodeInterface.h"


class InitializeTransformModeStrokeStrategy : public QObject, public KisStrokeStrategyUndoCommandBased
{
   Q_OBJECT

public:
    InitializeTransformModeStrokeStrategy(InplaceTransformStrokeStrategy::SharedDataSP s)
        : KisStrokeStrategyUndoCommandBased(kundo2_i18n("Initialize Transform"), false, s->undoFacade),
          m_s(s)
    {
        // TODO: do we need a barrier job here?
        enableJob(JOB_INIT, KisStrokeJobData::BARRIER);
        enableJob(JOB_CANCEL, KisStrokeJobData::BARRIER);
        setMacroId(KisCommandUtils::TransformToolId);
    }

    void initStrokeCallback() override {
        KisStrokeStrategyUndoCommandBased::initStrokeCallback();

        QVector<KisStrokeJobData *> extraInitJobs;

        if (m_s->selection) {
            m_s->selection->setVisible(false);
            m_s->deactivatedSelections.append(m_s->selection);
        }

        KisSelectionMaskSP overlaySelectionMask =
                dynamic_cast<KisSelectionMask*>(m_s->rootNode->graphListener()->graphOverlayNode());
        if (overlaySelectionMask) {
            overlaySelectionMask->setDecorationsVisible(false);
            m_s->deactivatedOverlaySelectionMask = overlaySelectionMask;
        }

        m_s->processedNodes = InplaceTransformStrokeStrategy::fetchNodesList(m_s->mode, m_s->rootNode, m_s->workRecursively);

        bool argsAreInitialized = false;
        QVector<KisStrokeJobData *> lastCommandUndoJobs;

        if (!m_s->forceReset && InplaceTransformStrokeStrategy::tryFetchArgsFromCommandAndUndo(&m_s->initialTransformArgs,
                                                                                               m_s->mode,
                                                                                               m_s->rootNode,
                                                                                               m_s->processedNodes,
                                                                                               m_s->undoFacade,
                                                                                               &lastCommandUndoJobs,
                                                                                               &m_s->overriddenCommand)) {
            argsAreInitialized = true;
        } else if (!m_s->forceReset && InplaceTransformStrokeStrategy::tryInitArgsFromNode(m_s->rootNode, &m_s->initialTransformArgs)) {
            argsAreInitialized = true;
        }

        //extraInitJobs << new Data(new KisHoldUIUpdatesCommand(m_s->updatesFacade, KisCommandUtils::FlipFlopCommand::INITIALIZING), false, KisStrokeJobData::BARRIER);

        KritaUtils::addJobBarrier(extraInitJobs, [this]() {
            m_s->updatesFacade->disableDirtyRequests();
            m_s->updatesDisabled = true;
        });

        extraInitJobs << lastCommandUndoJobs;

        KritaUtils::addJobSequential(extraInitJobs, [this]() {
            /**
                 * We must request shape layers to rerender areas outside image bounds
                 */
            KisLayerUtils::forceAllHiddenOriginalsUpdate(m_s->rootNode);
        });

        KritaUtils::addJobBarrier(extraInitJobs, [this]() {
            /**
                 * We must ensure that the currently selected subtree
                 * has finished all its updates.
                 */
            KisLayerUtils::forceAllDelayedNodesUpdate(m_s->rootNode);
        });

        /// Disable all decorated nodes to generate outline
        /// and preview correctly. We will enable them back
        /// as soon as preview generation is finished.
        KritaUtils::addJobBarrier(extraInitJobs, [this]() {
            Q_FOREACH (KisNodeSP node, m_s->processedNodes) {
                KisDecoratedNodeInterface *decoratedNode = dynamic_cast<KisDecoratedNodeInterface*>(node.data());
                if (decoratedNode && decoratedNode->decorationsVisible()) {
                    decoratedNode->setDecorationsVisible(false);
                    m_disabledDecoratedNodes << decoratedNode;
                }
            }
        });

        KritaUtils::addJobBarrier(extraInitJobs,
                                  [this,
                                  argsAreInitialized]() mutable {
            QRect srcRect;

            if (m_s->selection) {
                srcRect = m_s->selection->selectedExactRect();
            } else {
                srcRect = QRect();
                Q_FOREACH (KisNodeSP node, m_s->processedNodes) {
                    // group layers may have a projection of layers
                    // that are locked and will not be transformed
                    if (node->inherits("KisGroupLayer")) continue;

                    if (const KisTransformMask *mask = dynamic_cast<const KisTransformMask*>(node.data())) {
                        srcRect |= mask->sourceDataBounds();
                    } else if (const KisSelectionMask *mask = dynamic_cast<const KisSelectionMask*>(node.data())) {
                        srcRect |= mask->selection()->selectedExactRect();
                    } else {
                        srcRect |= node->exactBounds();
                    }
                }
            }

            TransformTransactionProperties transaction(srcRect, &m_s->initialTransformArgs, m_s->rootNode, m_s->processedNodes);
            if (!argsAreInitialized) {
                m_s->initialTransformArgs = KisTransformUtils::resetArgsForMode(m_s->mode, m_s->filterId, transaction);
            }

            Q_EMIT sigTransactionGenerated(transaction, m_s->initialTransformArgs, this);
        });

        /// recover back visibility of decorated nodes
        KritaUtils::addJobBarrier(extraInitJobs, [this]() {
            Q_FOREACH (KisDecoratedNodeInterface *decoratedNode, m_disabledDecoratedNodes) {
                decoratedNode->setDecorationsVisible(true);
            }
            m_disabledDecoratedNodes.clear();
        });

        Q_FOREACH (KisNodeSP node, m_s->processedNodes) {
            KritaUtils::addJobConcurrent(extraInitJobs, [this, node]() {

                KisPaintDeviceSP device = node->paintDevice();
                if (device) {

                    {
                        QMutexLocker l(&m_s->devicesCacheMutex);

                        if (!m_s->devicesCacheHash.contains(device.data())) {
                            KisPaintDeviceSP cache;

                            if (m_s->selection) {
                                QRect srcRect = m_s->selection->selectedExactRect();

                                cache = device->createCompositionSourceDevice();
                                KisPainter gc(cache);
                                gc.setSelection(m_s->selection);
                                gc.bitBlt(srcRect.topLeft(), device, srcRect);
                            } else {
                                cache = device->createCompositionSourceDevice(device);
                            }

                            m_s->devicesCacheHash.insert(device.data(), cache);
                        }
                    }

                    KisTransaction transaction(device);
                    if (m_s->selection) {
                        device->clearSelection(m_s->selection);
                    } else {
                        QRect oldExtent = device->extent();
                        device->clear();
                        device->setDirty(oldExtent);
                    }

                    {
                        QMutexLocker l(&m_s->commandsMutex);
                        KUndo2CommandSP sharedCommand = toQShared(transaction.endAndTake());
                        executeCommand(sharedCommand, false);
                        m_s->clearCommands.append(sharedCommand);
                    }
                }
            });
        }

        //extraInitJobs << new Data(toQShared(neHoldUIUpdatesCommand(m_s->updatesFacade, KisCommandUtils::FlipFlopCommand::FINALIZING)), false, KisStrokeJobData::BARRIER);

        KritaUtils::addJobBarrier(extraInitJobs, [this]() {
            QMutexLocker l(&m_s->dirtyRectsMutex);

            m_s->updatesFacade->enableDirtyRequests();
            m_s->updatesDisabled = false;
        });

        if (!lastCommandUndoJobs.isEmpty()) {
            KIS_SAFE_ASSERT_RECOVER_NOOP(m_s->overriddenCommand);

            for (auto it = extraInitJobs.begin(); it != extraInitJobs.end(); ++it) {
                (*it)->setCancellable(false);
            }
        }


        addMutatedJobs(extraInitJobs);
    }

    bool postProcessToplevelCommand(KUndo2Command *command) override
    {
        return KisStrokeStrategyUndoCommandBased::postProcessToplevelCommand(command) &&
                m_isCancellingAction && m_s->postProcessToplevelCommand(command);
    }

    void cancelStrokeCallback() override {
        QVector<KisStrokeJobData *> mutatedJobs;

        m_s->cancelAction(mutatedJobs, this);

        if (!mutatedJobs.isEmpty()) {
            m_isCancellingAction = true;
            addMutatedJobs(mutatedJobs);
        }
    }

Q_SIGNALS:
    void sigTransactionGenerated(TransformTransactionProperties transaction, ToolTransformArgs args, void *cookie);

private:

private:
    InplaceTransformStrokeStrategy::SharedDataSP m_s;
    QVector<KisDecoratedNodeInterface*> m_disabledDecoratedNodes;
    bool m_isCancellingAction = false;
};


InplaceTransformStrokeStrategy::InplaceTransformStrokeStrategy(ToolTransformArgs::TransformMode mode,
                                                               bool workRecursively,
                                                               const QString &filterId,
                                                               bool forceReset,
                                                               KisNodeSP rootNode,
                                                               KisSelectionSP selection,
                                                               KisStrokeUndoFacade *undoFacade,
                                                               KisUpdatesFacade *updatesFacade)
    : KisStrokeStrategyUndoCommandBased(kundo2_i18n("Transform"), false, undoFacade),
      m_s(new SharedData())
{

    m_s->mode = mode;
    m_s->workRecursively = workRecursively;
    m_s->filterId = filterId;
    m_s->forceReset = forceReset;
    m_s->rootNode = rootNode;
    m_s->selection = selection;
    m_s->updatesFacade = updatesFacade;
    m_s->undoFacade = undoFacade;


    KIS_SAFE_ASSERT_RECOVER_NOOP(!selection || !dynamic_cast<KisTransformMask*>(rootNode.data()));
    setMacroId(KisCommandUtils::TransformToolId);

    /**
     * Since we do initialization in a separate thread we should run the
     * cancellation routine even when this stroke hasn't been initialized yet
     * (because the previous stroke may have been completed without
     * cancellation)
     */
    setNeedsExplicitCancel(true);
}

InplaceTransformStrokeStrategy::InplaceTransformStrokeStrategy(const InplaceTransformStrokeStrategy &rhs, int levelOfDetail)
    : KisStrokeStrategyUndoCommandBased(rhs),
      m_s(rhs.m_s)
{
    Q_UNUSED(levelOfDetail);
    KIS_SAFE_ASSERT_RECOVER_NOOP(m_s->processedNodes.isEmpty());
    KIS_SAFE_ASSERT_RECOVER_NOOP(m_s->clearCommands.isEmpty());
    KIS_SAFE_ASSERT_RECOVER_NOOP(m_s->transformCommands.isEmpty());
}

InplaceTransformStrokeStrategy::~InplaceTransformStrokeStrategy()
{
}

bool InplaceTransformStrokeStrategy::shouldRestartStrokeOnModeChange(ToolTransformArgs::TransformMode oldMode, ToolTransformArgs::TransformMode newMode, KisNodeList processedNodes)
{
    bool hasExternalLayers = false;
    Q_FOREACH (KisNodeSP node, processedNodes) {
        if (node->inherits("KisShapeLayer")) {
            hasExternalLayers = true;
            break;
        }
    }

    bool result = false;

    if (hasExternalLayers) {
        result =
            (oldMode == ToolTransformArgs::FREE_TRANSFORM) !=
            (newMode == ToolTransformArgs::FREE_TRANSFORM);
    }

    return result;
}

void InplaceTransformStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    if (UpdateTransformData *upd = dynamic_cast<UpdateTransformData*>(data)) {
        m_pendingUpdateArgs = upd->args;
        tryPostUpdateJob(false);

    } else if (BarrierUpdateData *barrierData =
               dynamic_cast<BarrierUpdateData*>(data)) {

        doCanvasUpdate(barrierData->forceUpdate);

    } else if (KisAsyncronousStrokeUpdateHelper::UpdateData *updateData =
               dynamic_cast<KisAsyncronousStrokeUpdateHelper::UpdateData*>(data)) {

        tryPostUpdateJob(updateData->forceUpdate);

    } else {
        KisStrokeStrategyUndoCommandBased::doStrokeCallback(data);
    }

}

void InplaceTransformStrokeStrategy::tryPostUpdateJob(bool forceUpdate)
{
    if (!m_pendingUpdateArgs) return;

    if (forceUpdate ||
        (m_updateTimer.elapsed() > m_updateInterval &&
         !m_s->updatesFacade->hasUpdatesRunning())) {

        addMutatedJob(new BarrierUpdateData(forceUpdate));
    }
}

void InplaceTransformStrokeStrategy::doCanvasUpdate(bool forceUpdate)
{
    if (!m_pendingUpdateArgs) return;

    if (!forceUpdate &&
            (m_updateTimer.elapsed() < m_updateInterval ||
             m_s->updatesFacade->hasUpdatesRunning())) {

        return;
    }

    QVector<KisStrokeJobData *> jobs;

    ToolTransformArgs args = *m_pendingUpdateArgs;
    m_pendingUpdateArgs = boost::none;

    m_s->reapplyTransform(args, jobs, this);

    KritaUtils::addJobBarrier(jobs, [this]() {
        m_updateTimer.restart();
        // sanity check that no job has been squeezed inbetween
        KIS_SAFE_ASSERT_RECOVER_RETURN(!m_pendingUpdateArgs);
    });

    addMutatedJobs(jobs);
}

void InplaceTransformStrokeStrategy::transformAndMergeDevice(const ToolTransformArgs &config,
                                                      KisPaintDeviceSP src,
                                                      KisPaintDeviceSP dst,
                                                      KisProcessingVisitor::ProgressHelper *helper)
{
    KoUpdaterPtr mergeUpdater = src != dst ? helper->updater() : 0;

    KisTransformUtils::transformDevice(config, src, helper);
    if (src != dst) {
        QRect mergeRect = src->extent();
        KisPainter painter(dst);
        painter.setProgress(mergeUpdater);
        painter.bitBlt(mergeRect.topLeft(), src, mergeRect);
        painter.end();
    }
}

struct TransformExtraData : public KUndo2CommandExtraData
{
    ToolTransformArgs savedTransformArgs;
    KisNodeSP rootNode;
    KisNodeList transformedNodes;

    KUndo2CommandExtraData* clone() const override {
        return new TransformExtraData(*this);
    }
};

bool InplaceTransformStrokeStrategy::postProcessToplevelCommand(KUndo2Command *command)
{
    return KisStrokeStrategyUndoCommandBased::postProcessToplevelCommand(command) &&
        m_s->postProcessToplevelCommand(command);
}


bool InplaceTransformStrokeStrategy::fetchArgsFromCommand(const KUndo2Command *command, ToolTransformArgs *args, KisNodeSP *rootNode, KisNodeList *transformedNodes)
{
    const TransformExtraData *data = dynamic_cast<const TransformExtraData*>(command->extraData());

    if (data) {
        *args = data->savedTransformArgs;
        *rootNode = data->rootNode;
        *transformedNodes = data->transformedNodes;
    }

    return bool(data);
}

QList<KisNodeSP> InplaceTransformStrokeStrategy::fetchNodesList(ToolTransformArgs::TransformMode mode, KisNodeSP root, bool recursive)
{
    QList<KisNodeSP> result;

    auto fetchFunc =
        [&result, mode, root] (KisNodeSP node) {
        if (node->isEditable(node == root) &&
                (!node->inherits("KisShapeLayer") || mode == ToolTransformArgs::FREE_TRANSFORM) &&
                !node->inherits("KisFileLayer") &&
                (!node->inherits("KisTransformMask") || node == root)) {

                result << node;
            }
    };

    if (recursive) {
        KisLayerUtils::recursiveApplyNodes(root, fetchFunc);
    } else {
        fetchFunc(root);
    }

    return result;
}

bool InplaceTransformStrokeStrategy::tryInitArgsFromNode(KisNodeSP node, ToolTransformArgs *args)
{
    bool result = false;

    if (KisTransformMaskSP mask =
        dynamic_cast<KisTransformMask*>(node.data())) {

        KisTransformMaskParamsInterfaceSP savedParams =
            mask->transformParams();

        KisTransformMaskAdapter *adapter =
            dynamic_cast<KisTransformMaskAdapter*>(savedParams.data());

        if (adapter) {
            *args = adapter->transformArgs();
            result = true;
        }
    }

    return result;
}

bool InplaceTransformStrokeStrategy::tryFetchArgsFromCommandAndUndo(ToolTransformArgs *outArgs,
                                                                    ToolTransformArgs::TransformMode mode,
                                                                    KisNodeSP currentNode,
                                                                    KisNodeList selectedNodes,
                                                                    KisStrokeUndoFacade *undoFacade,
                                                                    QVector<KisStrokeJobData *> *undoJobs,
                                                                    const KisSavedMacroCommand **overriddenCommand)
{
    bool result = false;

    const KUndo2Command *lastCommand = undoFacade->lastExecutedCommand();
    KisNodeSP oldRootNode;
    KisNodeList oldTransformedNodes;

    ToolTransformArgs args;

    if (lastCommand &&
        InplaceTransformStrokeStrategy::fetchArgsFromCommand(lastCommand, &args, &oldRootNode, &oldTransformedNodes) &&
        args.mode() == mode &&
        oldRootNode == currentNode) {

        if (KritaUtils::compareListsUnordered(oldTransformedNodes, selectedNodes)) {
            args.saveContinuedState();

            *outArgs = args;

            const KisSavedMacroCommand *command = dynamic_cast<const KisSavedMacroCommand*>(lastCommand);
            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(command, false);

            // the jobs are fetched as !shouldGoToHistory,
            // so there is no need to put them into
            // m_s->skippedWhileMergeCommands
            command->getCommandExecutionJobs(undoJobs, true, false);
            *overriddenCommand = command;

            result = true;
        }
    }

    return result;
}

void InplaceTransformStrokeStrategy::initStrokeCallback()
{
    KisStrokeStrategyUndoCommandBased::initStrokeCallback();
    m_updateTimer.start();
}

void InplaceTransformStrokeStrategy::finishStrokeCallback()
{
    QVector<KisStrokeJobData *> mutatedJobs;

    m_s->finishAction(mutatedJobs, this);

    if (!mutatedJobs.isEmpty()) {
        addMutatedJobs(mutatedJobs);
    }
}

void InplaceTransformStrokeStrategy::cancelStrokeCallback()
{
    QVector<KisStrokeJobData *> mutatedJobs;

    m_s->cancelAction(mutatedJobs, this);

    if (!mutatedJobs.isEmpty()) {
        addMutatedJobs(mutatedJobs);
    }
}

void InplaceTransformStrokeStrategy::slotForwardTransactionGenerated(TransformTransactionProperties transaction, ToolTransformArgs args, void *cookie)
{
    Q_UNUSED(cookie);
    Q_EMIT sigTransactionGenerated(transaction, args, this);
}

KisStrokeStrategy* InplaceTransformStrokeStrategy::createLegacyInitializingStroke()
{
    InitializeTransformModeStrokeStrategy *strategy = new InitializeTransformModeStrokeStrategy(m_s);
    connect(strategy, SIGNAL(sigTransactionGenerated(TransformTransactionProperties, ToolTransformArgs, void *)),
            this, SLOT(slotForwardTransactionGenerated(TransformTransactionProperties, ToolTransformArgs, void *)));

    return strategy;
}

KisStrokeStrategy *InplaceTransformStrokeStrategy::createLodClone(int levelOfDetail)
{
    return 0;

    InplaceTransformStrokeStrategy *clone = new InplaceTransformStrokeStrategy(*this, levelOfDetail);
    connect(clone, SIGNAL(sigTransactionGenerated(TransformTransactionProperties, ToolTransformArgs, void *)),
            this, SIGNAL(sigTransactionGenerated(TransformTransactionProperties, ToolTransformArgs, void *)));

    m_isOverriddenByClone = true;

    // m_sharedNodes.reset(new KisNodeList());
    // clone->m_sharedNodes = m_sharedNodes;

    return clone;
}

InplaceTransformStrokeStrategy::BarrierUpdateData::BarrierUpdateData(bool _forceUpdate)
    : KisAsyncronousStrokeUpdateHelper::UpdateData(_forceUpdate, BARRIER, NORMAL)
{
}

KisStrokeJobData *InplaceTransformStrokeStrategy::BarrierUpdateData::createLodClone(int levelOfDetail)
{
    return new BarrierUpdateData(*this, levelOfDetail);
}

InplaceTransformStrokeStrategy::BarrierUpdateData::BarrierUpdateData(const InplaceTransformStrokeStrategy::BarrierUpdateData &rhs, int levelOfDetail)
    : KisAsyncronousStrokeUpdateHelper::UpdateData (rhs, levelOfDetail)
{
}

void InplaceTransformStrokeStrategy::SharedData::executeAndAddClearCommand(KUndo2Command *cmd, KisStrokeStrategyUndoCommandBased *interface)
{
    QMutexLocker l(&commandsMutex);
    KUndo2CommandSP sharedCommand = toQShared(cmd);
    interface->executeCommand(sharedCommand, false);
    clearCommands.append(sharedCommand);
}

void InplaceTransformStrokeStrategy::SharedData::executeAndAddTransformCommand(KUndo2Command *cmd, KisStrokeStrategyUndoCommandBased *interface)
{
    QMutexLocker l(&commandsMutex);
    KUndo2CommandSP sharedCommand = toQShared(cmd);
    interface->executeCommand(sharedCommand, false);
    transformCommands.append(sharedCommand);
}

void InplaceTransformStrokeStrategy::SharedData::notifyAllCommandsDone(KisStrokeStrategyUndoCommandBased *interface)
{
    Q_FOREACH (KUndo2CommandSP cmd, clearCommands) {
        interface->notifyCommandDone(cmd, KisStrokeJobData::CONCURRENT, KisStrokeJobData::NORMAL);
    }

    interface->notifyCommandDone(toQShared(new KUndo2Command()), KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::NORMAL);

    Q_FOREACH (KUndo2CommandSP cmd, transformCommands) {
        interface->notifyCommandDone(cmd, KisStrokeJobData::CONCURRENT, KisStrokeJobData::NORMAL);
    }
}

void InplaceTransformStrokeStrategy::SharedData::undoClearCommands(KisStrokeStrategyUndoCommandBased *interface)
{
    for (auto it = std::make_reverse_iterator(clearCommands.end());
         it != std::make_reverse_iterator(clearCommands.begin());
         ++it) {

        interface->executeCommand(*it, true);
    }
    clearCommands.clear();
}

void InplaceTransformStrokeStrategy::SharedData::undoTransformCommands(KisStrokeStrategyUndoCommandBased *interface)
{
    for (auto it = std::make_reverse_iterator(transformCommands.end());
         it != std::make_reverse_iterator(transformCommands.begin());
         ++it) {

        interface->executeCommand(*it, true);
    }
    transformCommands.clear();
}

void InplaceTransformStrokeStrategy::SharedData::postAllUpdates()
{
    Q_FOREACH (KisNodeSP node, processedNodes) {
        updatesFacade->refreshGraphAsync(node, dirtyRects[node] | prevDirtyRects[node]);
    }

    prevDirtyRects.clear();
    dirtyRects.swap(prevDirtyRects);
}

void InplaceTransformStrokeStrategy::SharedData::transformNode(KisNodeSP node, const ToolTransformArgs &config, KisStrokeStrategyUndoCommandBased *interface)
{
    KisPaintDeviceSP device = node->paintDevice();

    if (device) {
        KisPaintDeviceSP cachedPortion;

        {
            QMutexLocker l(&devicesCacheMutex);
            cachedPortion = devicesCacheHash[device.data()];
        }

        KIS_SAFE_ASSERT_RECOVER_RETURN(cachedPortion);

        KisPaintDeviceSP src = new KisPaintDevice(*cachedPortion);

        KisTransaction transaction(device);

        KisProcessingVisitor::ProgressHelper helper(node);
        transformAndMergeDevice(config, src,
                                device, &helper);

        executeAndAddTransformCommand(transaction.endAndTake(), interface);
        addDirtyRect(node, cachedPortion->extent() | device->extent());

    } else if (KisExternalLayer *extLayer =
               dynamic_cast<KisExternalLayer*>(node.data())) {

        if (config.mode() == ToolTransformArgs::FREE_TRANSFORM ||
                (config.mode() == ToolTransformArgs::PERSPECTIVE_4POINT &&
                 extLayer->supportsPerspectiveTransform())) {

            const QRect oldDirtyRect = extLayer->extent();

            QVector3D transformedCenter;
            KisTransformWorker w = KisTransformUtils::createTransformWorker(config, 0, 0, &transformedCenter);
            QTransform t = w.transform();
            KUndo2Command *cmd = extLayer->transform(t);

            executeAndAddTransformCommand(cmd, interface);
            addDirtyRect(node, oldDirtyRect | node->extent());
        }

    } else if (KisTransformMask *transformMask =
               dynamic_cast<KisTransformMask*>(node.data())) {

        const QRect oldDirtyRect = transformMask->extent();

        KUndo2Command *cmd = new KisModifyTransformMaskCommand(transformMask,
                                                               KisTransformMaskParamsInterfaceSP(
                                                                   new KisTransformMaskAdapter(config)));
        executeAndAddTransformCommand(cmd, interface);
        addDirtyRect(node, oldDirtyRect | transformMask->extent());
    }
}

void InplaceTransformStrokeStrategy::SharedData::reapplyTransform(ToolTransformArgs args, QVector<KisStrokeJobData *> &mutatedJobs, KisStrokeStrategyUndoCommandBased *interface)
{
    KritaUtils::addJobBarrier(mutatedJobs, [this, interface, args]() {
        updatesFacade->disableDirtyRequests();
        updatesDisabled = true;
        undoTransformCommands(interface);
        currentTransformArgs = args;
    });

    Q_FOREACH (KisNodeSP node, processedNodes) {
        KritaUtils::addJobConcurrent(mutatedJobs, [this, node, args, interface]() {
            transformNode(node, args, interface);
        });
    }

    KritaUtils::addJobBarrier(mutatedJobs, [this]() {
        updatesFacade->enableDirtyRequests();
        updatesDisabled = false;
        postAllUpdates();
    });
}

void InplaceTransformStrokeStrategy::SharedData::finalizeStrokeImpl(QVector<KisStrokeJobData *> &mutatedJobs, KisStrokeStrategyUndoCommandBased *interface)
{
    KritaUtils::addJobBarrier(mutatedJobs, [this]() {
        Q_FOREACH (KisSelectionSP selection, this->deactivatedSelections) {
            selection->setVisible(true);
        }

        if (deactivatedOverlaySelectionMask) {
            deactivatedOverlaySelectionMask->selection()->setVisible(true);
            deactivatedOverlaySelectionMask->setDirty();
        }
    });

    KritaUtils::addJobBarrier(mutatedJobs, [this, interface]() {
        notifyAllCommandsDone(interface);
    });
}

void InplaceTransformStrokeStrategy::SharedData::finishAction(QVector<KisStrokeJobData *> &mutatedJobs, KisStrokeStrategyUndoCommandBased *interface)
{
    /**
     * Since our finishStrokeCallback() initiates new jobs,
     * cancellation request may come even after
     * finishStrokeCallback() (cancellations may be called
     * until there are no jobs left in the stroke's queue).
     *
     * Therefore we should check for double-entry here and
     * make sure the finilizing jobs are no cancellable.
     */

    if (finalizingActionsStarted) return;
    finalizingActionsStarted = true;

    if (currentTransformArgs.isIdentity() && !overriddenCommand) {
        cancelAction(mutatedJobs, interface);
        return;
    }

    finalizeStrokeImpl(mutatedJobs, interface);

    KritaUtils::addJobBarrier(mutatedJobs, [interface]() {
        interface->KisStrokeStrategyUndoCommandBased::finishStrokeCallback();
    });
}

void InplaceTransformStrokeStrategy::SharedData::cancelAction(QVector<KisStrokeJobData *> &mutatedJobs, KisStrokeStrategyUndoCommandBased *interface)
{
    if (updatesDisabled) {
        updatesFacade->enableDirtyRequests();
    }

    /**
     * Since our finishStrokeCallback() initiates new jobs,
     * cancellation request may come even after
     * finishStrokeCallback() (cancellations may be called
     * until there are no jobs left in the stroke's queue).
     *
     * Therefore we should check for double-entry here and
     * make sure the finilizing jobs are no cancellable.
     */

    if (finalizingActionsStarted) return;
    finalizingActionsStarted = true;

    if (initialTransformArgs.isIdentity()) {
        KritaUtils::addJobBarrier(mutatedJobs, [this, interface]() {
            undoTransformCommands(interface);
            undoClearCommands(interface);
        });
        finalizeStrokeImpl(mutatedJobs, interface);

        KritaUtils::addJobBarrier(mutatedJobs, [interface]() {
            interface->KisStrokeStrategyUndoCommandBased::cancelStrokeCallback();
        });
    } else {
        reapplyTransform(initialTransformArgs, mutatedJobs, interface);
        finalizeStrokeImpl(mutatedJobs, interface);
        KritaUtils::addJobBarrier(mutatedJobs, [interface]() {
            interface->KisStrokeStrategyUndoCommandBased::finishStrokeCallback();
        });
    }
}

void InplaceTransformStrokeStrategy::SharedData::addDirtyRect(KisNodeSP node, const QRect &rect) {
    QMutexLocker l(&dirtyRectsMutex);
    dirtyRects[node] |= rect;
}

bool InplaceTransformStrokeStrategy::SharedData::postProcessToplevelCommand(KUndo2Command *command)
{
    TransformExtraData *data = new TransformExtraData();
    data->savedTransformArgs = currentTransformArgs;
    data->rootNode = rootNode;
    data->transformedNodes = processedNodes;

    command->setExtraData(data);

    KisSavedMacroCommand *macroCommand = dynamic_cast<KisSavedMacroCommand*>(command);
    KIS_SAFE_ASSERT_RECOVER_NOOP(macroCommand);

    if (overriddenCommand && macroCommand) {
        macroCommand->setOverrideInfo(overriddenCommand, skippedWhileMergeCommands);
    }

    return true;
}

#include "inplace_transform_stroke_strategy.moc"
