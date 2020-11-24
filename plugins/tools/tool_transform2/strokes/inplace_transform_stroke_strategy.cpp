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
#include "kis_sync_lod_cache_stroke_strategy.h"
#include "kis_lod_transform.h"


InplaceTransformStrokeStrategy::InplaceTransformStrokeStrategy(ToolTransformArgs::TransformMode mode,
                                                               bool workRecursively,
                                                               const QString &filterId,
                                                               bool forceReset,
                                                               KisNodeSP rootNode,
                                                               KisSelectionSP selection,
                                                               KisStrokeUndoFacade *undoFacade,
                                                               KisUpdatesFacade *updatesFacade,
                                                               KisNodeSP imageRoot)
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
    m_s->imageRoot = imageRoot;

    // TODO: auto-select level of detail for preview
    m_s->previewLevelOfDetail = 2;

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

    m_s->reapplyTransform(args, jobs, this, m_s->previewLevelOfDetail);

    KritaUtils::addJobBarrier(jobs, [this, args]() {
        m_s->currentTransformArgs = args;
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

    QVector<KisStrokeJobData *> extraInitJobs;

    if (m_s->selection) {
        m_s->selection->setVisible(false);
        m_s->deactivatedSelections.append(m_s->selection);
    }

    KisSelectionMaskSP overlaySelectionMask =
            dynamic_cast<KisSelectionMask*>(m_s->rootNode->graphListener()->graphOverlayNode());
    if (overlaySelectionMask && m_s->rootNode != KisNodeSP(overlaySelectionMask)) {
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
        Q_FOREACH (KisNodeSP node, m_s->processedNodes) {
            m_s->prevDirtyRects[node] = node->extent();

            if (m_s->previewLevelOfDetail > 0) {
                KisLodTransform t(m_s->previewLevelOfDetail);
                m_s->prevDirtyPreviewRects[node] = t.map(node->extent());
            }

        }

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
        KritaUtils::addJobSequential(extraInitJobs, [this, node]() mutable {

            KisPaintDeviceSP device;
            CommandGroup commandGroup = Clear;

            if (KisExternalLayer *extLayer =
                           dynamic_cast<KisExternalLayer*>(node.data())) {

                if (m_s->mode == ToolTransformArgs::FREE_TRANSFORM ||
                    (m_s->mode == ToolTransformArgs::PERSPECTIVE_4POINT &&
                     extLayer->supportsPerspectiveTransform())) {

                    device = node->projection();
                    commandGroup = ClearTemporary;
                }
            } else if (KisTransformMask *mask = dynamic_cast<KisTransformMask*>(node.data())) {
                KIS_SAFE_ASSERT_RECOVER_NOOP(!m_s->selection);

                // NOTE: this action should be either sequential or barrier
                QMutexLocker l(&m_s->devicesCacheMutex);
                if (!m_s->transformMaskCacheHash.contains(mask)) {
                    KIS_SAFE_ASSERT_RECOVER_RETURN(m_s->updatesDisabled);

                    KisPaintDeviceSP dev = mask->buildSourcePreviewDevice();
                    m_s->transformMaskCacheHash.insert(mask, new KisPaintDevice(*dev));

                    return;
                }

            } else {
                device = node->paintDevice();
            }

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

                m_s->executeAndAddCommand(transaction.endAndTake(), this, commandGroup);
            }
        });
    }

    //extraInitJobs << new Data(toQShared(neHoldUIUpdatesCommand(m_s->updatesFacade, KisCommandUtils::FlipFlopCommand::FINALIZING)), false, KisStrokeJobData::BARRIER);

    KritaUtils::addJobBarrier(extraInitJobs, [this]() {
        QMutexLocker l(&m_s->dirtyRectsMutex);

        m_s->updatesFacade->enableDirtyRequests();
        m_s->updatesDisabled = false;

        m_updateTimer.start();
    });

    if (!lastCommandUndoJobs.isEmpty()) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(m_s->overriddenCommand);

        for (auto it = extraInitJobs.begin(); it != extraInitJobs.end(); ++it) {
            (*it)->setCancellable(false);
        }
    }


    if (m_s->previewLevelOfDetail > 0) {
        KritaUtils::addJobBarrier(extraInitJobs, [this]() {
            QVector<KisStrokeJobData*> lodSyncJobs;
            KisSyncLodCacheStrokeStrategy::createJobsData(lodSyncJobs,
                                                          m_s->imageRoot,
                                                          m_s->previewLevelOfDetail,
                                                          m_s->devicesCacheHash.values() +
                                                          m_s->transformMaskCacheHash.values());

            for (auto it = lodSyncJobs.begin(); it != lodSyncJobs.end(); ++it) {
                (*it)->setLevelOfDetailOverride(m_s->previewLevelOfDetail);
            }

            addMutatedJobs(lodSyncJobs);
        });
    }

    addMutatedJobs(extraInitJobs);
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

void InplaceTransformStrokeStrategy::SharedData::executeAndAddCommand(KUndo2Command *cmd, KisStrokeStrategyUndoCommandBased *interface, InplaceTransformStrokeStrategy::CommandGroup group)
{
    QMutexLocker l(&commandsMutex);
    KUndo2CommandSP sharedCommand = toQShared(cmd);
    interface->executeCommand(sharedCommand, false);
    commands.append(std::make_pair(group, sharedCommand));
}

void InplaceTransformStrokeStrategy::SharedData::notifyAllCommandsDone(KisStrokeStrategyUndoCommandBased *interface)
{
    for (auto it = commands.begin(); it != commands.end(); ++it) {
        if (it->first == Clear) {
            interface->notifyCommandDone(it->second, KisStrokeJobData::CONCURRENT, KisStrokeJobData::NORMAL);
        }
    }

    interface->notifyCommandDone(toQShared(new KUndo2Command()), KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::NORMAL);

    for (auto it = commands.begin(); it != commands.end(); ++it) {
        if (it->first == Transform) {
            interface->notifyCommandDone(it->second, KisStrokeJobData::CONCURRENT, KisStrokeJobData::NORMAL);
        }
    }
}

void InplaceTransformStrokeStrategy::SharedData::undoAllCommands(KisStrokeStrategyUndoCommandBased *interface)
{
    for (auto it = std::make_reverse_iterator(commands.end());
         it != std::make_reverse_iterator(commands.begin());
         ++it) {

        interface->executeCommand(it->second, true);
    }

    commands.clear();
}

void InplaceTransformStrokeStrategy::SharedData::undoTransformCommands(KisStrokeStrategyUndoCommandBased *interface, int levelOfDetail)
{
    for (auto it = std::make_reverse_iterator(commands.end());
         it != std::make_reverse_iterator(commands.begin());) {

        if ((levelOfDetail > 0 &&
             (it->first == TransformLod || it->first == TransformLodTemporary)) ||
            (levelOfDetail <= 0 &&
             (it->first == Transform || it->first == TransformTemporary))) {

            interface->executeCommand(it->second, true);
            it = std::make_reverse_iterator(commands.erase(std::next(it).base()));
        } else {
            ++it;
        }
    }
}

void InplaceTransformStrokeStrategy::SharedData::postAllUpdates(int levelOfDetail)
{

    QHash<KisNodeSP, QRect> &dirtyRects = effectiveDirtyRects(levelOfDetail);
    QHash<KisNodeSP, QRect> &prevDirtyRects = effectivePrevDirtyRects(levelOfDetail);

    Q_FOREACH (KisNodeSP node, processedNodes) {
        updatesFacade->refreshGraphAsync(node, dirtyRects[node] | prevDirtyRects[node]);
    }

    prevDirtyRects.clear();
    dirtyRects.swap(prevDirtyRects);
}

void InplaceTransformStrokeStrategy::SharedData::transformNode(KisNodeSP node, const ToolTransformArgs &config, KisStrokeStrategyUndoCommandBased *interface, int levelOfDetail)
{
    KisPaintDeviceSP device = node->paintDevice();

    CommandGroup commandGroup =
        levelOfDetail > 0 ? TransformLod : Transform;

    if (KisExternalLayer *extLayer =
        dynamic_cast<KisExternalLayer*>(node.data())) {

            if (config.mode() == ToolTransformArgs::FREE_TRANSFORM ||
                    (config.mode() == ToolTransformArgs::PERSPECTIVE_4POINT &&
                     extLayer->supportsPerspectiveTransform())) {

                if (levelOfDetail <= 0) {
                    const QRect oldDirtyRect = extLayer->extent();

                    QVector3D transformedCenter;
                    KisTransformWorker w = KisTransformUtils::createTransformWorker(config, 0, 0, &transformedCenter);
                    QTransform t = w.transform();
                    KUndo2Command *cmd = extLayer->transform(t);

                    executeAndAddCommand(cmd, interface, Transform);
                    addDirtyRect(node, oldDirtyRect | node->extent(), 0);
                    return;
                } else {
                    device = node->projection();
                    commandGroup = TransformLodTemporary;
                }
            }

    } else {
        device = node->paintDevice();
    }

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

        executeAndAddCommand(transaction.endAndTake(), interface, commandGroup);
        addDirtyRect(node, cachedPortion->extent() | device->extent(), levelOfDetail);

    } else if (KisTransformMask *transformMask =
               dynamic_cast<KisTransformMask*>(node.data())) {

        const QRect oldDirtyRect = transformMask->extent();

        if (levelOfDetail <= 0) {

            KUndo2Command *cmd = new KisModifyTransformMaskCommand(transformMask,
                                                                   KisTransformMaskParamsInterfaceSP(
                                                                       new KisTransformMaskAdapter(config)));
            cmd->redo();
            executeAndAddCommand(cmd, interface, commandGroup);
            addDirtyRect(node, oldDirtyRect | transformMask->extent(), levelOfDetail);
        } else {
            KisPaintDeviceSP cachedPortion;

            {
                QMutexLocker l(&devicesCacheMutex);
                cachedPortion = transformMaskCacheHash[transformMask];
            }

            KIS_SAFE_ASSERT_RECOVER_RETURN(cachedPortion);

            KisPaintDeviceSP src = new KisPaintDevice(*cachedPortion);
            KisPaintDeviceSP dst = new KisPaintDevice(cachedPortion->colorSpace());
            dst->prepareClone(src);

            KisTransaction transaction(dst);

            KisProcessingVisitor::ProgressHelper helper(node);
            transformAndMergeDevice(config, src,
                                    dst, &helper);

            transformMask->overrideStaticCacheDevice(dst);
            // no undo information is needed!

            addDirtyRect(node, oldDirtyRect | transformMask->extent(), levelOfDetail);
        }
    }
}

void InplaceTransformStrokeStrategy::SharedData::reapplyTransform(ToolTransformArgs args,
                                                                  QVector<KisStrokeJobData *> &mutatedJobs,
                                                                  KisStrokeStrategyUndoCommandBased *interface,
                                                                  int levelOfDetail)
{
    if (levelOfDetail > 0) {
        KisLodTransform t(levelOfDetail);
        args.transformSrcAndDst(t.transform());
    }

    KritaUtils::addJobBarrier(mutatedJobs, levelOfDetail,
                              [this, interface, args, levelOfDetail]() {
        updatesFacade->disableDirtyRequests();
        updatesDisabled = true;
        undoTransformCommands(interface, levelOfDetail);
    });

    Q_FOREACH (KisNodeSP node, processedNodes) {
        KritaUtils::addJobConcurrent(mutatedJobs, levelOfDetail,
                                     [this, node, args, interface, levelOfDetail]() {
            transformNode(node, args, interface, levelOfDetail);
        });
    }

    KritaUtils::addJobBarrier(mutatedJobs, levelOfDetail, [this, levelOfDetail]() {
        updatesFacade->enableDirtyRequests();
        updatesDisabled = false;
        postAllUpdates(levelOfDetail);
    });
}

void InplaceTransformStrokeStrategy::SharedData::finalizeStrokeImpl(QVector<KisStrokeJobData *> &mutatedJobs, KisStrokeStrategyUndoCommandBased *interface, bool saveCommands)
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


    if (saveCommands) {
        KritaUtils::addJobBarrier(mutatedJobs, [this, interface]() {
            notifyAllCommandsDone(interface);
        });
    }
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

    if (previewLevelOfDetail > 0) {
        mutatedJobs << new Data(new KisHoldUIUpdatesCommand(updatesFacade, KisCommandUtils::FlipFlopCommand::INITIALIZING), false, KisStrokeJobData::BARRIER);

        KritaUtils::addJobSequential(mutatedJobs, [this]() {
            Q_FOREACH (KisTransformMask *mask, transformMaskCacheHash.keys()) {
                mask->overrideStaticCacheDevice(0);
            }
        });

        reapplyTransform(currentTransformArgs, mutatedJobs, interface, 0);
        mutatedJobs << new Data(new KisHoldUIUpdatesCommand(updatesFacade, KisCommandUtils::FlipFlopCommand::FINALIZING), false, KisStrokeJobData::BARRIER);

        KritaUtils::addJobSequential(mutatedJobs, [this]() {
            Q_FOREACH (KisTransformMask *mask, transformMaskCacheHash.keys()) {
                mask->threadSafeForceStaticImageUpdate();
            }
        });
    }

    finalizeStrokeImpl(mutatedJobs, interface, true);

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

    KIS_SAFE_ASSERT_RECOVER_NOOP(transformMaskCacheHash.isEmpty() ||
                                 (transformMaskCacheHash.size() == 1 && processedNodes.size() == 1));

    const bool isChangingTransformMask = !transformMaskCacheHash.isEmpty();

    if (initialTransformArgs.isIdentity()) {
        KritaUtils::addJobBarrier(mutatedJobs, [this, interface]() {
            undoTransformCommands(interface, 0);
            undoAllCommands(interface);
        });
        finalizeStrokeImpl(mutatedJobs, interface, false);

        KritaUtils::addJobSequential(mutatedJobs, [this]() {
            Q_FOREACH (KisTransformMask *mask, transformMaskCacheHash.keys()) {
                mask->overrideStaticCacheDevice(0);
                mask->threadSafeForceStaticImageUpdate();
            }
        });

        KritaUtils::addJobBarrier(mutatedJobs, [interface]() {
            interface->KisStrokeStrategyUndoCommandBased::cancelStrokeCallback();
        });
    } else {
        KIS_SAFE_ASSERT_RECOVER_NOOP(isChangingTransformMask || overriddenCommand);

        KritaUtils::addJobSequential(mutatedJobs, [this]() {
            Q_FOREACH (KisTransformMask *mask, transformMaskCacheHash.keys()) {
                mask->overrideStaticCacheDevice(0);
            }
        });

        reapplyTransform(initialTransformArgs, mutatedJobs, interface, 0);
        finalizeStrokeImpl(mutatedJobs, interface, bool(overriddenCommand));

        KritaUtils::addJobSequential(mutatedJobs, [this]() {
            Q_FOREACH (KisTransformMask *mask, transformMaskCacheHash.keys()) {
                mask->threadSafeForceStaticImageUpdate();
            }
        });

        if (overriddenCommand) {
            KritaUtils::addJobBarrier(mutatedJobs, [interface]() {
                interface->KisStrokeStrategyUndoCommandBased::finishStrokeCallback();
            });
        } else {
            KritaUtils::addJobBarrier(mutatedJobs, [interface]() {
                interface->KisStrokeStrategyUndoCommandBased::cancelStrokeCallback();
            });
        }
    }
}

void InplaceTransformStrokeStrategy::SharedData::addDirtyRect(KisNodeSP node, const QRect &rect, int levelOfDetail) {
    QMutexLocker l(&dirtyRectsMutex);
    effectiveDirtyRects(levelOfDetail)[node] |= rect;
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
