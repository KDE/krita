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



InplaceTransformStrokeStrategy::InplaceTransformStrokeStrategy(ToolTransformArgs::TransformMode mode,
                                                               bool workRecursively,
                                                               const QString &filterId,
                                                               bool forceReset,
                                                               KisNodeSP rootNode,
                                                               KisSelectionSP selection,
                                                               KisStrokeUndoFacade *undoFacade, KisUpdatesFacade *updatesFacade)
    : KisStrokeStrategyUndoCommandBased(kundo2_i18n("Transform"), false, undoFacade),
      m_updatesFacade(updatesFacade),
      m_mode(mode),
      m_workRecursively(workRecursively),
      m_filterId(filterId),
      m_forceReset(forceReset),
      m_selection(selection),
      m_rootNode(rootNode)
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(!selection || !dynamic_cast<KisTransformMask*>(rootNode.data()));
    setMacroId(KisCommandUtils::TransformToolId);
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

KisPaintDeviceSP InplaceTransformStrokeStrategy::createDeviceCache(KisPaintDeviceSP dev)
{
    KisPaintDeviceSP cache;

    if (m_selection) {
        QRect srcRect = m_selection->selectedExactRect();

        cache = dev->createCompositionSourceDevice();
        KisPainter gc(cache);
        gc.setSelection(m_selection);
        gc.bitBlt(srcRect.topLeft(), dev, srcRect);
    } else {
        cache = dev->createCompositionSourceDevice(dev);
    }

    return cache;
}

bool InplaceTransformStrokeStrategy::haveDeviceInCache(KisPaintDeviceSP src)
{
    QMutexLocker l(&m_devicesCacheMutex);
    return m_devicesCacheHash.contains(src.data());
}

void InplaceTransformStrokeStrategy::putDeviceCache(KisPaintDeviceSP src, KisPaintDeviceSP cache)
{
    QMutexLocker l(&m_devicesCacheMutex);
    m_devicesCacheHash.insert(src.data(), cache);
}

KisPaintDeviceSP InplaceTransformStrokeStrategy::getDeviceCache(KisPaintDeviceSP src)
{
    QMutexLocker l(&m_devicesCacheMutex);
    KisPaintDeviceSP cache = m_devicesCacheHash.value(src.data());
    if (!cache) {
        warnKrita << "WARNING: Transform Stroke: the device is absent in cache!";
    }

    return cache;
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
         !m_updatesFacade->hasUpdatesRunning())) {

        addMutatedJob(new BarrierUpdateData(forceUpdate));
    }
}

void InplaceTransformStrokeStrategy::doCanvasUpdate(bool forceUpdate)
{
    if (!m_pendingUpdateArgs) return;

    if (!forceUpdate &&
            (m_updateTimer.elapsed() < m_updateInterval ||
             m_updatesFacade->hasUpdatesRunning())) {

        return;
    }

    QVector<KisStrokeJobData *> jobs;

    KritaUtils::addJobBarrier(jobs, [this]() {
        m_updatesFacade->disableDirtyRequests();
        m_updatesDisabled = true;

        for (auto it = std::make_reverse_iterator(m_transformCommands.end());
             it != std::make_reverse_iterator(m_transformCommands.begin());
             ++it) {

            executeCommand(*it, true);
        }
        m_transformCommands.clear();

        KIS_SAFE_ASSERT_RECOVER_RETURN(m_pendingUpdateArgs);
        m_currentTransformArgs = *m_pendingUpdateArgs;
        m_pendingUpdateArgs = boost::none;
    });

    Q_FOREACH (KisNodeSP node, m_processedNodes) {
        KritaUtils::addJobConcurrent(jobs, [this, node]() {
            transformNode(node, m_currentTransformArgs);
        });
    }

    KritaUtils::addJobBarrier(jobs, [this]() {
        QMutexLocker l(&m_dirtyRectsMutex);

        m_updatesFacade->enableDirtyRequests();
        m_updatesDisabled = false;

        Q_FOREACH (KisNodeSP node, m_processedNodes) {
            m_updatesFacade->refreshGraphAsync(node, m_dirtyRects[node] | m_prevDirtyRects[node]);
        }

        m_prevDirtyRects.clear();
        m_dirtyRects.swap(m_prevDirtyRects);
        m_updateTimer.restart();

        // sanity check that no job has been squeezed inbetween
        KIS_SAFE_ASSERT_RECOVER_RETURN(!m_pendingUpdateArgs);
    });

    addMutatedJobs(jobs);
}

void InplaceTransformStrokeStrategy::executeAndAddClearCommand(KUndo2Command *cmd)
{
    QMutexLocker l(&m_commandsMutex);
    KUndo2CommandSP sharedCommand = toQShared(cmd);
    executeCommand(sharedCommand, false);
    m_clearCommands.append(sharedCommand);
}

void InplaceTransformStrokeStrategy::executeAndAddTransformCommand(KUndo2Command *cmd)
{
    QMutexLocker l(&m_commandsMutex);
    KUndo2CommandSP sharedCommand = toQShared(cmd);
    executeCommand(sharedCommand, false);
    m_transformCommands.append(sharedCommand);
}

void InplaceTransformStrokeStrategy::clearSelection(KisPaintDeviceSP device)
{
    KisTransaction transaction(device);
    if (m_selection) {
        device->clearSelection(m_selection);
    } else {
        QRect oldExtent = device->extent();
        device->clear();
        device->setDirty(oldExtent);
    }

    executeAndAddClearCommand(transaction.endAndTake());
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

void InplaceTransformStrokeStrategy::postProcessToplevelCommand(KUndo2Command *command)
{
    TransformExtraData *data = new TransformExtraData();
    data->savedTransformArgs = m_currentTransformArgs;
    data->rootNode = m_rootNode;
    data->transformedNodes = m_processedNodes;

    command->setExtraData(data);

    KisSavedMacroCommand *macroCommand = dynamic_cast<KisSavedMacroCommand*>(command);
    KIS_SAFE_ASSERT_RECOVER_NOOP(macroCommand);

    if (m_overriddenCommand && macroCommand) {
        macroCommand->setOverrideInfo(m_overriddenCommand, m_skippedWhileMergeCommands);
    }

    KisStrokeStrategyUndoCommandBased::postProcessToplevelCommand(command);
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
                                                             QVector<KisStrokeJobData *> *undoJobs)
{
    bool result = false;

    const KUndo2Command *lastCommand = undoFacade()->lastExecutedCommand();
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
            // m_skippedWhileMergeCommands
            command->getCommandExecutionJobs(undoJobs, true, false);
            m_overriddenCommand = command;

            result = true;
        }
    }

    return result;
}

void InplaceTransformStrokeStrategy::initStrokeCallback()
{
    KisStrokeStrategyUndoCommandBased::initStrokeCallback();

    QVector<KisStrokeJobData *> extraInitJobs;

    if (m_selection) {
        m_selection->setVisible(false);
        m_deactivatedSelections.append(m_selection);
    }

    KisSelectionMaskSP overlaySelectionMask =
            dynamic_cast<KisSelectionMask*>(m_rootNode->graphListener()->graphOverlayNode());
    if (overlaySelectionMask) {
        overlaySelectionMask->setDecorationsVisible(false);
        m_deactivatedOverlaySelectionMask = overlaySelectionMask;
    }

    m_processedNodes = fetchNodesList(m_mode, m_rootNode, m_workRecursively);

    bool argsAreInitialized = false;
    QVector<KisStrokeJobData *> lastCommandUndoJobs;

    if (!m_forceReset && tryFetchArgsFromCommandAndUndo(&m_initialTransformArgs,
                                                        m_mode,
                                                        m_rootNode,
                                                        m_processedNodes,
                                                        &lastCommandUndoJobs)) {
        argsAreInitialized = true;
    } else if (!m_forceReset && tryInitArgsFromNode(m_rootNode, &m_initialTransformArgs)) {
        argsAreInitialized = true;
    }

    //extraInitJobs << new Data(new KisHoldUIUpdatesCommand(m_updatesFacade, KisCommandUtils::FlipFlopCommand::INITIALIZING), false, KisStrokeJobData::BARRIER);

    KritaUtils::addJobBarrier(extraInitJobs, [this]() {
        m_updatesFacade->disableDirtyRequests();
        m_updatesDisabled = true;
    });

    extraInitJobs << lastCommandUndoJobs;

    KritaUtils::addJobSequential(extraInitJobs, [this]() {
        /**
             * We must request shape layers to rerender areas outside image bounds
             */
        KisLayerUtils::forceAllHiddenOriginalsUpdate(m_rootNode);
    });

    KritaUtils::addJobBarrier(extraInitJobs, [this]() {
        /**
             * We must ensure that the currently selected subtree
             * has finished all its updates.
             */
        KisLayerUtils::forceAllDelayedNodesUpdate(m_rootNode);
    });

    /// Disable all decorated nodes to generate outline
    /// and preview correctly. We will enable them back
    /// as soon as preview generation is finished.
    KritaUtils::addJobBarrier(extraInitJobs, [this]() {
        Q_FOREACH (KisNodeSP node, m_processedNodes) {
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

        if (m_selection) {
            srcRect = m_selection->selectedExactRect();
        } else {
            srcRect = QRect();
            Q_FOREACH (KisNodeSP node, m_processedNodes) {
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

        TransformTransactionProperties transaction(srcRect, &m_initialTransformArgs, m_rootNode, m_processedNodes);
        if (!argsAreInitialized) {
            m_initialTransformArgs = KisTransformUtils::resetArgsForMode(m_mode, m_filterId, transaction);
        }

        Q_EMIT this->sigTransactionGenerated(transaction, m_initialTransformArgs, this);
    });

    /// recover back visibility of decorated nodes
    KritaUtils::addJobBarrier(extraInitJobs, [this]() {
        Q_FOREACH (KisDecoratedNodeInterface *decoratedNode, m_disabledDecoratedNodes) {
            decoratedNode->setDecorationsVisible(true);
        }
        m_disabledDecoratedNodes.clear();
    });

    Q_FOREACH (KisNodeSP node, m_processedNodes) {
        KritaUtils::addJobConcurrent(extraInitJobs, [this, node]() {
            clearNode(node);
        });
    }

    //extraInitJobs << new Data(toQShared(neHoldUIUpdatesCommand(m_updatesFacade, KisCommandUtils::FlipFlopCommand::FINALIZING)), false, KisStrokeJobData::BARRIER);

    KritaUtils::addJobBarrier(extraInitJobs, [this]() {
        QMutexLocker l(&m_dirtyRectsMutex);

        m_updatesFacade->enableDirtyRequests();
        m_updatesDisabled = false;
        m_updateTimer.start();
    });

    if (!lastCommandUndoJobs.isEmpty()) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(m_overriddenCommand);

        for (auto it = extraInitJobs.begin(); it != extraInitJobs.end(); ++it) {
            (*it)->setCancellable(false);
        }
    }


    addMutatedJobs(extraInitJobs);
}



void InplaceTransformStrokeStrategy::clearNode(KisNodeSP node)
{
    KisPaintDeviceSP device = node->paintDevice();

    if (device) {
        if (!haveDeviceInCache(device)) {
            putDeviceCache(device, createDeviceCache(device));
        }
        clearSelection(device);
    }
}

void InplaceTransformStrokeStrategy::transformNode(KisNodeSP node, const ToolTransformArgs &config)
{
    KisPaintDeviceSP device = node->paintDevice();

    if (device) {
        KisPaintDeviceSP cachedPortion = getDeviceCache(device);
        KIS_SAFE_ASSERT_RECOVER_RETURN(cachedPortion);

        KisPaintDeviceSP src = new KisPaintDevice(*cachedPortion);

        KisTransaction transaction(device);

        KisProcessingVisitor::ProgressHelper helper(node);
        transformAndMergeDevice(config, src,
                                device, &helper);

        executeAndAddTransformCommand(transaction.endAndTake());
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

            executeAndAddTransformCommand(cmd);
            addDirtyRect(node, oldDirtyRect | node->extent());
        }

    } else if (KisTransformMask *transformMask =
               dynamic_cast<KisTransformMask*>(node.data())) {

        const QRect oldDirtyRect = transformMask->extent();

        KUndo2Command *cmd = new KisModifyTransformMaskCommand(transformMask,
                                                               KisTransformMaskParamsInterfaceSP(
                                                                   new KisTransformMaskAdapter(config)));
        executeAndAddTransformCommand(cmd);
        addDirtyRect(node, oldDirtyRect | transformMask->extent());
    }
}

void InplaceTransformStrokeStrategy::finalizeStrokeImpl(QVector<KisStrokeJobData *> &mutatedJobs)
{
    KritaUtils::addJobBarrier(mutatedJobs, [this]() {
        Q_FOREACH (KisSelectionSP selection, m_deactivatedSelections) {
            selection->setVisible(true);
        }

        if (m_deactivatedOverlaySelectionMask) {
            m_deactivatedOverlaySelectionMask->selection()->setVisible(true);
            m_deactivatedOverlaySelectionMask->setDirty();
        }
    });

    KritaUtils::addJobBarrier(mutatedJobs, [this]() {
        Q_FOREACH (KUndo2CommandSP cmd, m_clearCommands) {
            notifyCommandDone(cmd, KisStrokeJobData::CONCURRENT, KisStrokeJobData::NORMAL);
        }

        notifyCommandDone(toQShared(new KUndo2Command()), KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::NORMAL);

        Q_FOREACH (KUndo2CommandSP cmd, m_transformCommands) {
            notifyCommandDone(cmd, KisStrokeJobData::CONCURRENT, KisStrokeJobData::NORMAL);
        }
    });
}

void InplaceTransformStrokeStrategy::finishStrokeCallback()
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

    if (m_finalizingActionsStarted) return;
    m_finalizingActionsStarted = true;

    if (m_currentTransformArgs.isIdentity()) {
        cancelStrokeCallback();
        return;
    }

    QVector<KisStrokeJobData *> mutatedJobs;
    finalizeStrokeImpl(mutatedJobs);

    KritaUtils::addJobBarrier(mutatedJobs, [this]() {
        KisStrokeStrategyUndoCommandBased::finishStrokeCallback();
    });

    addMutatedJobs(mutatedJobs);
}

void InplaceTransformStrokeStrategy::cancelStrokeCallback()
{
    if (m_updatesDisabled) {
        m_updatesFacade->enableDirtyRequests();
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

    if (m_finalizingActionsStarted) return;
    m_finalizingActionsStarted = true;

    QVector<KisStrokeJobData *> mutatedJobs;

    if (m_initialTransformArgs.isIdentity()) {
        for (auto it = std::make_reverse_iterator(m_transformCommands.end());
             it != std::make_reverse_iterator(m_transformCommands.begin());
             ++it) {

            executeCommand(*it, true);
        }
        m_transformCommands.clear();

        for (auto it = std::make_reverse_iterator(m_clearCommands.end());
             it != std::make_reverse_iterator(m_clearCommands.begin());
             ++it) {

            executeCommand(*it, true);
        }
        m_clearCommands.clear();
    } else {
        KritaUtils::addJobBarrier(mutatedJobs, [this]() {
            m_updatesFacade->disableDirtyRequests();
            m_updatesDisabled = true;
        });

        KritaUtils::addJobBarrier(mutatedJobs, [this]() {
            for (auto it = std::make_reverse_iterator(m_transformCommands.end());
                 it != std::make_reverse_iterator(m_transformCommands.begin());
                 ++it) {

                executeCommand(*it, true);
            }
            m_transformCommands.clear();
        });

        Q_FOREACH (KisNodeSP node, m_processedNodes) {
            KritaUtils::addJobConcurrent(mutatedJobs, [this, node]() {
                transformNode(node, m_initialTransformArgs);
            });
        }

        KritaUtils::addJobBarrier(mutatedJobs, [this]() {
            m_updatesFacade->enableDirtyRequests();
            m_updatesDisabled = false;

            Q_FOREACH (KisNodeSP node, m_processedNodes) {
                m_updatesFacade->refreshGraphAsync(node, m_dirtyRects[node] | m_prevDirtyRects[node]);
            }

            m_prevDirtyRects.clear();
            m_dirtyRects.swap(m_prevDirtyRects);
        });
    }

    finalizeStrokeImpl(mutatedJobs);

    KritaUtils::addJobBarrier(mutatedJobs, [this]() {
        if (m_initialTransformArgs.isIdentity()) {
            KisStrokeStrategyUndoCommandBased::cancelStrokeCallback();
        } else {
            KisStrokeStrategyUndoCommandBased::finishStrokeCallback();
        }
    });

    addMutatedJobs(mutatedJobs);
}

InplaceTransformStrokeStrategy::BarrierUpdateData::BarrierUpdateData(bool _forceUpdate)
    : KisAsyncronousStrokeUpdateHelper::UpdateData(_forceUpdate, BARRIER, NORMAL)
{

}
