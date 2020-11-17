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
                                                               InplaceTransformStrokeStrategy::SharedStateSP sharedState,
                                                               KisStrokeUndoFacade *undoFacade, KisUpdatesFacade *updatesFacade)
    : KisStrokeStrategyUndoCommandBased(kundo2_i18n("Transform"), false, undoFacade),
      m_updatesFacade(updatesFacade),
      m_mode(mode),
      m_workRecursively(workRecursively),
      m_filterId(filterId),
      m_forceReset(forceReset),
      m_selection(selection),
      m_rootNode(rootNode),
      m_sharedState(sharedState)
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(!selection || !dynamic_cast<KisTransformMask*>(rootNode.data()));
    setMacroId(KisCommandUtils::TransformToolId);
    setNeedsExplicitCancel(true);
}

InplaceTransformStrokeStrategy::InplaceTransformStrokeStrategy(const ToolTransformArgs &args, InplaceTransformStrokeStrategy::SharedStateSP sharedState, KisStrokeUndoFacade *undoFacade, KisUpdatesFacade *updatesFacade)
    : InplaceTransformStrokeStrategy(sharedState->args.mode(),
                                     sharedState->props.transformedNodes().size() > 1,
                                     sharedState->args.filterId(),
                                     false,
                                     sharedState->props.rootNode(),
                                     sharedState->selection,
                                     sharedState,
                                     undoFacade,
                                     updatesFacade)
{
    m_sharedState->nextInitializationArgs = args;
    m_processedNodes = sharedState->props.transformedNodes();
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
    QMutexLocker l(&m_sharedState->devicesCacheMutex);
    return m_sharedState->devicesCacheHash.contains(src.data());
}

void InplaceTransformStrokeStrategy::putDeviceCache(KisPaintDeviceSP src, KisPaintDeviceSP cache)
{
    QMutexLocker l(&m_sharedState->devicesCacheMutex);
    m_sharedState->devicesCacheHash.insert(src.data(), cache);
}

KisPaintDeviceSP InplaceTransformStrokeStrategy::getDeviceCache(KisPaintDeviceSP src)
{
    QMutexLocker l(&m_sharedState->devicesCacheMutex);
    KisPaintDeviceSP cache = m_sharedState->devicesCacheHash.value(src.data());
    if (!cache) {
        warnKrita << "WARNING: Transform Stroke: the device is absent in cache!";
    }

    return cache;
}

void InplaceTransformStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    KisStrokeStrategyUndoCommandBased::doStrokeCallback(data);
}

void InplaceTransformStrokeStrategy::clearSelection(KisPaintDeviceSP device)
{
    KisTransaction transaction(device);
    if (m_sharedState->selection) {
        device->clearSelection(m_sharedState->selection);
    } else {
        QRect oldExtent = device->extent();
        device->clear();
        device->setDirty(oldExtent);
    }

    {
        QMutexLocker l(&m_sharedState->commandsMutex);
        m_sharedState->clearCommands.append(toQShared(transaction.endAndTake()));
    }
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
    data->savedTransformArgs = m_sharedState->args;
    data->rootNode = m_sharedState->props.rootNode();
    data->transformedNodes = m_sharedState->props.transformedNodes();

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

    QMutexLocker l(&m_sharedState->initializationMutex);
    m_sharedState->processingStarted = true;


    KIS_SAFE_ASSERT_RECOVER_NOOP(!m_sharedState->isInitialized ||
                                 m_selection == m_sharedState->selection);

    KIS_SAFE_ASSERT_RECOVER_NOOP(!m_sharedState->isInitialized || !m_forceReset);

    QVector<KisStrokeJobData *> extraInitJobs;

    if (!m_sharedState->isInitialized) {

        if (m_selection) {
            m_selection->setVisible(false);
            m_sharedState->deactivatedSelections.append(m_selection);
        }

        KisSelectionMaskSP overlaySelectionMask =
                dynamic_cast<KisSelectionMask*>(m_rootNode->graphListener()->graphOverlayNode());
        if (overlaySelectionMask) {
            overlaySelectionMask->setDecorationsVisible(false);
            m_sharedState->deactivatedOverlaySelectionMask = overlaySelectionMask;
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

            m_sharedState->isInitialized = true;
            m_sharedState->args = m_initialTransformArgs;
            m_sharedState->props = transaction;
            m_sharedState->selection = m_selection;
            m_sharedState->initialTransformArgs = m_initialTransformArgs;

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

        if (!lastCommandUndoJobs.isEmpty()) {
            KIS_SAFE_ASSERT_RECOVER_NOOP(m_overriddenCommand);

            for (auto it = extraInitJobs.begin(); it != extraInitJobs.end(); ++it) {
                (*it)->setCancellable(false);
            }
        }
    } else {
        KritaUtils::addJobBarrier(extraInitJobs, [this]() {
            m_updatesFacade->disableDirtyRequests();
            m_updatesDisabled = true;

            for (auto it = std::make_reverse_iterator(m_sharedState->transformCommands.end());
                 it != std::make_reverse_iterator(m_sharedState->transformCommands.begin());
                 ++it) {

                executeCommand(*it, true);
            }
            m_sharedState->transformCommands.clear();
            m_sharedState->args = m_sharedState->nextInitializationArgs;
        });
    }

    if (!m_sharedState->args.isIdentity()) {
        Q_FOREACH (KisNodeSP node, m_processedNodes) {
            KritaUtils::addJobConcurrent(extraInitJobs, [this, node]() {
                transformNode(node, m_sharedState->args);
            });
        }
    }

    KritaUtils::addJobBarrier(extraInitJobs, [this]() {
        QMutexLocker l(&m_sharedState->dirtyRectsMutex);

        m_updatesFacade->enableDirtyRequests();
        m_updatesDisabled = false;

        Q_FOREACH (KisNodeSP node, m_sharedState->props.transformedNodes()) {
            m_updatesFacade->refreshGraphAsync(node, m_sharedState->dirtyRects[node]);
        }

        m_sharedState->dirtyRects.clear();
    });

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

        {
            QMutexLocker l(&m_sharedState->commandsMutex);
            // TODO: add cmd->redo();
            m_sharedState->transformCommands.append(toQShared(transaction.endAndTake()));
        }
        m_sharedState->addDirtyRect(node, cachedPortion->extent() | device->extent());
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

            {
                QMutexLocker l(&m_sharedState->commandsMutex);
                // TODO: add cmd->redo();
                m_sharedState->transformCommands.append(toQShared(cmd));
            }

            m_sharedState->addDirtyRect(node, oldDirtyRect | node->extent());
        }

    } else if (KisTransformMask *transformMask =
               dynamic_cast<KisTransformMask*>(node.data())) {

        const QRect oldDirtyRect = transformMask->extent();

        KUndo2Command *cmd = new KisModifyTransformMaskCommand(transformMask,
                                                               KisTransformMaskParamsInterfaceSP(
                                                                   new KisTransformMaskAdapter(config)));
        {
            QMutexLocker l(&m_sharedState->commandsMutex);
            // TODO: add cmd->redo();
            m_sharedState->transformCommands.append(toQShared(cmd));
        }

        m_sharedState->addDirtyRect(node, oldDirtyRect | transformMask->extent());
    }
}

void InplaceTransformStrokeStrategy::finalizeStrokeImpl(QVector<KisStrokeJobData *> &mutatedJobs)
{
    KritaUtils::addJobBarrier(mutatedJobs, [this]() {
        Q_FOREACH (KisSelectionSP selection, m_sharedState->deactivatedSelections) {
            selection->setVisible(true);
        }

        if (m_sharedState->deactivatedOverlaySelectionMask) {
            m_sharedState->deactivatedOverlaySelectionMask->selection()->setVisible(true);
            m_sharedState->deactivatedOverlaySelectionMask->setDirty();
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

    if (m_sharedState->args.isIdentity()) {
        cancelStrokeCallback();
        return;
    }

    Q_FOREACH (KUndo2CommandSP cmd, m_sharedState->clearCommands) {
        notifyCommandDone(cmd, KisStrokeJobData::CONCURRENT, KisStrokeJobData::NORMAL);
    }

    notifyCommandDone(toQShared(new KUndo2Command()), KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::NORMAL);

    Q_FOREACH (KUndo2CommandSP cmd, m_sharedState->transformCommands) {
        notifyCommandDone(cmd, KisStrokeJobData::CONCURRENT, KisStrokeJobData::NORMAL);
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

    const int value = m_sharedState->skipCancellationMarker.fetchAndAddOrdered(-1);
    if (value >= 0) return;

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
        for (auto it = std::make_reverse_iterator(m_sharedState->transformCommands.end());
             it != std::make_reverse_iterator(m_sharedState->transformCommands.begin());
             ++it) {

            executeCommand(*it, true);
        }
        m_sharedState->transformCommands.clear();

        for (auto it = std::make_reverse_iterator(m_sharedState->clearCommands.end());
             it != std::make_reverse_iterator(m_sharedState->clearCommands.begin());
             ++it) {

            executeCommand(*it, true);
        }
        m_sharedState->clearCommands.clear();
    } else {
        for (auto it = std::make_reverse_iterator(m_sharedState->transformCommands.end());
             it != std::make_reverse_iterator(m_sharedState->transformCommands.begin());
             ++it) {

            executeCommand(*it, true);
        }
        m_sharedState->transformCommands.clear();

        Q_FOREACH (KisNodeSP node, m_sharedState->props.transformedNodes()) {
            KritaUtils::addJobConcurrent(mutatedJobs, [this, node]() {
                transformNode(node, m_sharedState->initialTransformArgs);
            });
        }
    }

    finalizeStrokeImpl(mutatedJobs);

    KritaUtils::addJobBarrier(mutatedJobs, [this]() {
        KisStrokeStrategyUndoCommandBased::cancelStrokeCallback();
    });

    addMutatedJobs(mutatedJobs);
}
