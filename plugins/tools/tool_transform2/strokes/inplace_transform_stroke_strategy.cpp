/*
 *  SPDX-FileCopyrightText: 2013,2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include <boost/optional.hpp>
#include "kis_selection_mask.h"
#include "kis_undo_stores.h"
#include "kis_transparency_mask.h"


struct InplaceTransformStrokeStrategy::Private
{
    // initial conditions passed from the tool
    KisUpdatesFacade *updatesFacade;
    KisStrokeUndoFacade *undoFacade;
    ToolTransformArgs::TransformMode mode;
    bool workRecursively;
    QString filterId;
    bool forceReset;
    KisNodeSP rootNode;
    KisSelectionSP selection;
    KisPaintDeviceSP externalSource;
    KisNodeSP imageRoot;
    int previewLevelOfDetail = -1;
    bool forceLodMode = true;

    // properties filled by initialization/transformation routines
    KisNodeList processedNodes;
    ToolTransformArgs initialTransformArgs;
    ToolTransformArgs currentTransformArgs;

    const KisSavedMacroCommand *overriddenCommand = 0;

    QList<KisSelectionSP> deactivatedSelections;
    KisSelectionMaskSP deactivatedOverlaySelectionMask;
    bool updatesDisabled = false;

    QMutex commandsMutex;
    QVector<std::pair<CommandGroup, KUndo2CommandSP>> commands;

    QMutex devicesCacheMutex;
    QHash<KisPaintDevice*, KisPaintDeviceSP> devicesCacheHash;
    QHash<KisTransformMask*, KisPaintDeviceSP> transformMaskCacheHash;

    QMutex dirtyRectsMutex;
    QHash<KisNodeSP, QRect> dirtyRects;
    QHash<KisNodeSP, QRect> prevDirtyRects;

    QHash<KisNodeSP, QRect> dirtyPreviewRects;
    QHash<KisNodeSP, QRect> prevDirtyPreviewRects;

    inline QHash<KisNodeSP, QRect>& effectiveDirtyRects(int levelOfDetail) {
        return levelOfDetail > 0 ? dirtyPreviewRects : dirtyRects;
    }

    inline QHash<KisNodeSP, QRect>& effectivePrevDirtyRects(int levelOfDetail) {
        return levelOfDetail > 0 ? prevDirtyPreviewRects : prevDirtyRects;
    }

    // a flag to avoid recursion in finish/cancel routines
    bool finalizingActionsStarted = false;

    // data for asynchronous updates
    boost::optional<ToolTransformArgs> pendingUpdateArgs;
    QElapsedTimer updateTimer;
    const int updateInterval = 30;

    // temporary variable to share data betwen jobs at the initialization phase
    QVector<KisDecoratedNodeInterface*> disabledDecoratedNodes;
};


InplaceTransformStrokeStrategy::InplaceTransformStrokeStrategy(ToolTransformArgs::TransformMode mode,
                                                               bool workRecursively,
                                                               const QString &filterId,
                                                               bool forceReset,
                                                               KisNodeSP rootNode,
                                                               KisSelectionSP selection,
                                                               KisPaintDeviceSP externalSource,
                                                               KisStrokeUndoFacade *undoFacade,
                                                               KisUpdatesFacade *updatesFacade,
                                                               KisNodeSP imageRoot,
                                                               bool forceLodMode)
    : KisStrokeStrategyUndoCommandBased(kundo2_i18n("Transform"), false, undoFacade),
      m_d(new Private())
{

    m_d->mode = mode;
    m_d->workRecursively = workRecursively;
    m_d->filterId = filterId;
    m_d->forceReset = forceReset;
    m_d->rootNode = rootNode;
    m_d->selection = selection;
    m_d->externalSource = externalSource;
    m_d->updatesFacade = updatesFacade;
    m_d->undoFacade = undoFacade;
    m_d->imageRoot = imageRoot;
    m_d->forceLodMode = forceLodMode;

    KIS_SAFE_ASSERT_RECOVER_NOOP(!selection || !dynamic_cast<KisTransformMask*>(rootNode.data()));
    setMacroId(KisCommandUtils::TransformToolId);

    // TODO: check if can be relaxed
    setNeedsExplicitCancel(true);
}

InplaceTransformStrokeStrategy::~InplaceTransformStrokeStrategy()
{
}

void InplaceTransformStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    if (UpdateTransformData *upd = dynamic_cast<UpdateTransformData*>(data)) {
        m_d->pendingUpdateArgs = upd->args;
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
    if (!m_d->pendingUpdateArgs) return;

    if (forceUpdate ||
        (m_d->updateTimer.elapsed() > m_d->updateInterval &&
         !m_d->updatesFacade->hasUpdatesRunning())) {

        addMutatedJob(new BarrierUpdateData(forceUpdate));
    }
}

void InplaceTransformStrokeStrategy::doCanvasUpdate(bool forceUpdate)
{
    if (!m_d->pendingUpdateArgs) return;

    if (!forceUpdate &&
            (m_d->updateTimer.elapsed() < m_d->updateInterval ||
             m_d->updatesFacade->hasUpdatesRunning())) {

        return;
    }

    QVector<KisStrokeJobData *> jobs;

    ToolTransformArgs args = *m_d->pendingUpdateArgs;
    m_d->pendingUpdateArgs = boost::none;

    reapplyTransform(args, jobs, m_d->previewLevelOfDetail);

    KritaUtils::addJobBarrier(jobs, [this, args]() {
        m_d->currentTransformArgs = args;
        m_d->updateTimer.restart();
        // sanity check that no job has been squeezed inbetween
        KIS_SAFE_ASSERT_RECOVER_RETURN(!m_d->pendingUpdateArgs);
    });

    addMutatedJobs(jobs);
}

int InplaceTransformStrokeStrategy::calculatePreferredLevelOfDetail(const QRect &srcRect)
{
    KisLodPreferences lodPreferences = this->currentLodPreferences();
    if (!lodPreferences.lodSupported() ||
        !(lodPreferences.lodPreferred() || m_d->forceLodMode)) return -1;

    const int maxSize = 2000;
    const int maxDimension = KisAlgebra2D::maxDimension(srcRect);

    const qreal zoom = qMax(1.0, qreal(maxDimension) / maxSize);

    const int calculatedLod = qCeil(std::log2(zoom));

    return qMax(calculatedLod, lodPreferences.desiredLevelOfDetail());

}


void InplaceTransformStrokeStrategy::postProcessToplevelCommand(KUndo2Command *command)
{
    KisTransformUtils::postProcessToplevelCommand(command,
                                                  m_d->currentTransformArgs,
                                                  m_d->rootNode,
                                                  m_d->processedNodes,
                                                  m_d->overriddenCommand);

    KisStrokeStrategyUndoCommandBased::postProcessToplevelCommand(command);
}



void InplaceTransformStrokeStrategy::initStrokeCallback()
{
    KisStrokeStrategyUndoCommandBased::initStrokeCallback();

    QVector<KisStrokeJobData *> extraInitJobs;

    if (m_d->selection) {
        m_d->selection->setVisible(false);
        m_d->deactivatedSelections.append(m_d->selection);
    }

    KisSelectionMaskSP overlaySelectionMask =
            dynamic_cast<KisSelectionMask*>(m_d->rootNode->graphListener()->graphOverlayNode());
    if (overlaySelectionMask && m_d->rootNode != KisNodeSP(overlaySelectionMask)) {
        overlaySelectionMask->setDecorationsVisible(false);
        m_d->deactivatedOverlaySelectionMask = overlaySelectionMask;
    }


    m_d->rootNode = KisTransformUtils::tryOverrideRootToTransformMask(m_d->rootNode);

    // When placing an external source image, we never work recursively on any layer masks
    m_d->processedNodes = KisTransformUtils::fetchNodesList(m_d->mode, m_d->rootNode, m_d->workRecursively && !m_d->externalSource);

    bool argsAreInitialized = false;
    QVector<KisStrokeJobData *> lastCommandUndoJobs;

    // When externalSource is set, it means that we are initializing a new
    // stroke following a newActivationWithExternalSource, thus we never try
    // to reuse an existing transformation from the undo queue. However, when
    // externalSource is not set, tryFetchArgsFromCommandAndUndo may still
    // recover a previous stroke that referenced an external source.
    if (!m_d->forceReset && !m_d->externalSource) {
        if (KisTransformUtils::tryFetchArgsFromCommandAndUndo(&m_d->initialTransformArgs,
                                                              m_d->mode,
                                                              m_d->rootNode,
                                                              m_d->processedNodes,
                                                              m_d->undoFacade,
                                                              &lastCommandUndoJobs,
                                                              &m_d->overriddenCommand)) {
            argsAreInitialized = true;
        } else if (KisTransformUtils::tryInitArgsFromNode(m_d->rootNode, &m_d->initialTransformArgs)) {
            argsAreInitialized = true;
        }
    }

    KritaUtils::addJobBarrier(extraInitJobs, [this]() {
        m_d->updatesFacade->disableDirtyRequests();
        m_d->updatesDisabled = true;

        Q_FOREACH (KisNodeSP node, m_d->processedNodes) {
            m_d->prevDirtyRects[node] = node->extent();
        }
    });

    extraInitJobs << lastCommandUndoJobs;

    KritaUtils::addJobSequential(extraInitJobs, [this]() {
        /**
          * We must request shape layers to rerender areas outside image bounds
          */
        KisLayerUtils::forceAllHiddenOriginalsUpdate(m_d->rootNode);
    });

    KritaUtils::addJobBarrier(extraInitJobs, [this]() {
        /**
          * We must ensure that the currently selected subtree
          * has finished all its updates.
          */
        KisLayerUtils::forceAllDelayedNodesUpdate(m_d->rootNode);
    });

    /// Disable all decorated nodes to generate outline
    /// and preview correctly. We will enable them back
    /// as soon as preview generation is finished.
    KritaUtils::addJobBarrier(extraInitJobs, [this]() {
        Q_FOREACH (KisNodeSP node, m_d->processedNodes) {
            KisDecoratedNodeInterface *decoratedNode = dynamic_cast<KisDecoratedNodeInterface*>(node.data());
            if (decoratedNode && decoratedNode->decorationsVisible()) {
                decoratedNode->setDecorationsVisible(false);
                m_d->disabledDecoratedNodes << decoratedNode;
            }
        }
    });

    KritaUtils::addJobBarrier(extraInitJobs,
                              [this,
                              argsAreInitialized]() mutable {
        QRect srcRect;

        if (m_d->externalSource) {
            // Start the transformation around the visible pixels of the external image
            srcRect = m_d->externalSource->exactBounds();
        } else if (m_d->selection) {
            srcRect = m_d->selection->selectedExactRect();
        } else {
            srcRect = QRect();
            Q_FOREACH (KisNodeSP node, m_d->processedNodes) {
                // group layers may have a projection of layers
                // that are locked and will not be transformed
                if (node->inherits("KisGroupLayer")) continue;

                if (const KisTransformMask *mask = dynamic_cast<const KisTransformMask*>(node.data())) {
                    srcRect |= mask->sourceDataBounds();
                } else if (const KisSelectionMask *mask = dynamic_cast<const KisSelectionMask*>(node.data())) {
                    srcRect |= mask->selection()->selectedExactRect();
                } else if (const KisTransparencyMask *mask = dynamic_cast<const KisTransparencyMask*>(node.data())) {
                    srcRect |= mask->selection()->selectedExactRect();
                } else {
                    srcRect |= node->exactBounds();
                }
            }
        }

        TransformTransactionProperties transaction(srcRect, &m_d->initialTransformArgs, m_d->rootNode, m_d->processedNodes);
        if (!argsAreInitialized) {
            m_d->initialTransformArgs = KisTransformUtils::resetArgsForMode(m_d->mode, m_d->filterId, transaction);
            m_d->initialTransformArgs.setExternalSource(m_d->externalSource);
        }
        m_d->externalSource.clear();

        const QRect imageBoundsRect = m_d->imageRoot->projection()->defaultBounds()->bounds();
        m_d->previewLevelOfDetail = calculatePreferredLevelOfDetail(srcRect & imageBoundsRect);

        if (m_d->previewLevelOfDetail > 0) {
            for (auto it = m_d->prevDirtyRects.begin(); it != m_d->prevDirtyRects.end(); ++it) {
                KisLodTransform t(m_d->previewLevelOfDetail);
                m_d->prevDirtyPreviewRects[it.key()] = t.map(it.value());
            }
        }

        Q_EMIT sigTransactionGenerated(transaction, m_d->initialTransformArgs, this);
    });

    /// recover back visibility of decorated nodes
    KritaUtils::addJobBarrier(extraInitJobs, [this]() {
        Q_FOREACH (KisDecoratedNodeInterface *decoratedNode, m_d->disabledDecoratedNodes) {
            decoratedNode->setDecorationsVisible(true);
        }
        m_d->disabledDecoratedNodes.clear();
    });

    Q_FOREACH (KisNodeSP node, m_d->processedNodes) {
        KritaUtils::addJobSequential(extraInitJobs, [this, node]() mutable {
            createCacheAndClearNode(node);
        });
    }

    KritaUtils::addJobBarrier(extraInitJobs, [this]() {
        QMutexLocker l(&m_d->dirtyRectsMutex);

        m_d->updatesFacade->enableDirtyRequests();
        m_d->updatesDisabled = false;

        m_d->updateTimer.start();
    });

    if (!lastCommandUndoJobs.isEmpty()) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(m_d->overriddenCommand);

        for (auto it = extraInitJobs.begin(); it != extraInitJobs.end(); ++it) {
            (*it)->setCancellable(false);
        }
    }

    KritaUtils::addJobBarrier(extraInitJobs, [this]() {
        if (m_d->previewLevelOfDetail > 0) {
            QVector<KisStrokeJobData*> lodSyncJobs;
            KisSyncLodCacheStrokeStrategy::createJobsData(lodSyncJobs,
                                                          m_d->imageRoot,
                                                          m_d->previewLevelOfDetail,
                                                          m_d->devicesCacheHash.values() +
                                                          m_d->transformMaskCacheHash.values());

            for (auto it = lodSyncJobs.begin(); it != lodSyncJobs.end(); ++it) {
                (*it)->setLevelOfDetailOverride(m_d->previewLevelOfDetail);
            }

            addMutatedJobs(lodSyncJobs);
        }
    });


    addMutatedJobs(extraInitJobs);
}

void InplaceTransformStrokeStrategy::finishStrokeCallback()
{
    QVector<KisStrokeJobData *> mutatedJobs;

    finishAction(mutatedJobs);

    if (!mutatedJobs.isEmpty()) {
        addMutatedJobs(mutatedJobs);
    }
}

void InplaceTransformStrokeStrategy::cancelStrokeCallback()
{
    QVector<KisStrokeJobData *> mutatedJobs;

    cancelAction(mutatedJobs);

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

void InplaceTransformStrokeStrategy::executeAndAddCommand(KUndo2Command *cmd, InplaceTransformStrokeStrategy::CommandGroup group)
{
    QMutexLocker l(&m_d->commandsMutex);
    KUndo2CommandSP sharedCommand = toQShared(cmd);
    executeCommand(sharedCommand, false);
    m_d->commands.append(std::make_pair(group, sharedCommand));
}

void InplaceTransformStrokeStrategy::notifyAllCommandsDone()
{
    for (auto it = m_d->commands.begin(); it != m_d->commands.end(); ++it) {
        if (it->first == Clear) {
            notifyCommandDone(it->second, KisStrokeJobData::CONCURRENT, KisStrokeJobData::NORMAL);
        }
    }

    notifyCommandDone(toQShared(new KUndo2Command()), KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::NORMAL);

    for (auto it = m_d->commands.begin(); it != m_d->commands.end(); ++it) {
        if (it->first == Transform) {
            notifyCommandDone(it->second, KisStrokeJobData::CONCURRENT, KisStrokeJobData::NORMAL);
        }
    }
}

void InplaceTransformStrokeStrategy::undoAllCommands()
{
    for (auto it = std::make_reverse_iterator(m_d->commands.end());
         it != std::make_reverse_iterator(m_d->commands.begin());
         ++it) {

        executeCommand(it->second, true);
    }

    m_d->commands.clear();
}

void InplaceTransformStrokeStrategy::undoTransformCommands(int levelOfDetail)
{
    for (auto it = std::make_reverse_iterator(m_d->commands.end());
         it != std::make_reverse_iterator(m_d->commands.begin());) {

        if ((levelOfDetail > 0 &&
             (it->first == TransformLod || it->first == TransformLodTemporary)) ||
            (levelOfDetail <= 0 &&
             (it->first == Transform || it->first == TransformTemporary))) {

            executeCommand(it->second, true);
            it = std::make_reverse_iterator(m_d->commands.erase(std::next(it).base()));
        } else {
            ++it;
        }
    }
}

void InplaceTransformStrokeStrategy::postAllUpdates(int levelOfDetail)
{
    QHash<KisNodeSP, QRect> &dirtyRects = m_d->effectiveDirtyRects(levelOfDetail);
    QHash<KisNodeSP, QRect> &prevDirtyRects = m_d->effectivePrevDirtyRects(levelOfDetail);

    Q_FOREACH (KisNodeSP node, m_d->processedNodes) {
        const QRect dirtyRect = dirtyRects[node] | prevDirtyRects[node];
        if (dirtyRect.isEmpty()) continue;

        /**
         * When transforming transform masks in non-lod mode, the projection is
         * jenerated by KisRecalculateTransformMaskJob, which is forced by the
         * undo command. We shouldn't try to start the update after a command.
         */
        if (dynamic_cast<KisTransformMask*>(node.data()) && levelOfDetail <= 0) continue;

        m_d->updatesFacade->refreshGraphAsync(node, dirtyRect);
    }

    prevDirtyRects.clear();
    dirtyRects.swap(prevDirtyRects);
}

void InplaceTransformStrokeStrategy::transformNode(KisNodeSP node, const ToolTransformArgs &config, int levelOfDetail)
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

                    executeAndAddCommand(cmd, Transform);
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
            QMutexLocker l(&m_d->devicesCacheMutex);
            cachedPortion = m_d->devicesCacheHash[device.data()];
        }

        KIS_SAFE_ASSERT_RECOVER_RETURN(cachedPortion);

        KisTransaction transaction(device);

        KisProcessingVisitor::ProgressHelper helper(node);
        KisTransformUtils::transformAndMergeDevice(config, cachedPortion,
                                                   device, &helper);

        executeAndAddCommand(transaction.endAndTake(), commandGroup);
        addDirtyRect(node, cachedPortion->extent() | device->extent(), levelOfDetail);

    } else if (KisTransformMask *transformMask =
               dynamic_cast<KisTransformMask*>(node.data())) {

        const QRect oldDirtyRect = transformMask->extent();

        if (levelOfDetail <= 0) {

            KUndo2Command *cmd = new KisModifyTransformMaskCommand(transformMask,
                                                                   KisTransformMaskParamsInterfaceSP(
                                                                       new KisTransformMaskAdapter(config)));
            executeAndAddCommand(cmd, Transform);
        } else {
            KisPaintDeviceSP cachedPortion;

            {
                QMutexLocker l(&m_d->devicesCacheMutex);
                cachedPortion = m_d->transformMaskCacheHash[transformMask];
            }

            KIS_SAFE_ASSERT_RECOVER_RETURN(cachedPortion);

            KisPaintDeviceSP dst = new KisPaintDevice(cachedPortion->colorSpace());
            dst->prepareClone(cachedPortion);

            KisProcessingVisitor::ProgressHelper helper(node);
            KisTransformUtils::transformAndMergeDevice(config, cachedPortion,
                                                       dst, &helper);

            transformMask->overrideStaticCacheDevice(dst);
            KUndo2Command *cmd = new KisModifyTransformMaskCommand(transformMask,
                                                                   KisTransformMaskParamsInterfaceSP(
                                                                       new KisTransformMaskAdapter(config)),
                                                                   true);
            executeAndAddCommand(cmd, commandGroup);

            addDirtyRect(node, oldDirtyRect | transformMask->extent(), TransformLod);
        }
    }
}

void InplaceTransformStrokeStrategy::createCacheAndClearNode(KisNodeSP node)
{
    KisPaintDeviceSP device;
    CommandGroup commandGroup = Clear;

    if (KisExternalLayer *extLayer =
                   dynamic_cast<KisExternalLayer*>(node.data())) {

        if (m_d->mode == ToolTransformArgs::FREE_TRANSFORM ||
            (m_d->mode == ToolTransformArgs::PERSPECTIVE_4POINT &&
             extLayer->supportsPerspectiveTransform())) {

            device = node->projection();
            commandGroup = ClearTemporary;
        }
    } else if (KisTransformMask *mask = dynamic_cast<KisTransformMask*>(node.data())) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(!m_d->selection);

        // NOTE: this action should be either sequential or barrier
        QMutexLocker l(&m_d->devicesCacheMutex);
        if (!m_d->transformMaskCacheHash.contains(mask)) {
            KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->updatesDisabled);

            KisPaintDeviceSP dev = mask->buildSourcePreviewDevice();
            m_d->transformMaskCacheHash.insert(mask, new KisPaintDevice(*dev));

            return;
        }

    } else {
        device = node->paintDevice();
    }

    if (device) {

        {
            QMutexLocker l(&m_d->devicesCacheMutex);

            if (!m_d->devicesCacheHash.contains(device.data())) {
                KisPaintDeviceSP cache;

                // The image that will be transformed is linked to the original
                // layer. We copy existing pixels or use an external source.
                if (m_d->initialTransformArgs.externalSource()) {
                    cache = device->createCompositionSourceDevice(m_d->initialTransformArgs.externalSource());
                } else if (m_d->selection) {
                    QRect srcRect = m_d->selection->selectedExactRect();

                    cache = device->createCompositionSourceDevice();
                    KisPainter gc(cache);
                    gc.setSelection(m_d->selection);
                    gc.bitBlt(srcRect.topLeft(), device, srcRect);
                } else {
                    cache = device->createCompositionSourceDevice(device);
                }

                m_d->devicesCacheHash.insert(device.data(), cache);
            }
        }

        // Don't clear the selection or layer when the source is external
        if (m_d->initialTransformArgs.externalSource()) return;

        KisTransaction transaction(device);
        if (m_d->selection) {
            device->clearSelection(m_d->selection);
        } else {
            QRect oldExtent = device->extent();
            device->clear();
            device->setDirty(oldExtent);
        }

        executeAndAddCommand(transaction.endAndTake(), commandGroup);
    }
}

void InplaceTransformStrokeStrategy::reapplyTransform(ToolTransformArgs args,
                                                      QVector<KisStrokeJobData *> &mutatedJobs,
                                                      int levelOfDetail)
{
    if (levelOfDetail > 0) {
        args.scaleSrcAndDst(KisLodTransform::lodToScale(levelOfDetail));
    }

    KritaUtils::addJobBarrier(mutatedJobs, levelOfDetail,
                              [this, args, levelOfDetail]() {
        m_d->updatesFacade->disableDirtyRequests();
        m_d->updatesDisabled = true;
        undoTransformCommands(levelOfDetail);
    });

    Q_FOREACH (KisNodeSP node, m_d->processedNodes) {
        KritaUtils::addJobConcurrent(mutatedJobs, levelOfDetail,
                                     [this, node, args, levelOfDetail]() {
            transformNode(node, args, levelOfDetail);
        });
    }

    KritaUtils::addJobBarrier(mutatedJobs, levelOfDetail, [this, levelOfDetail]() {
        m_d->updatesFacade->enableDirtyRequests();
        m_d->updatesDisabled = false;
        postAllUpdates(levelOfDetail);
    });
}

void InplaceTransformStrokeStrategy::finalizeStrokeImpl(QVector<KisStrokeJobData *> &mutatedJobs, bool saveCommands)
{
    KritaUtils::addJobBarrier(mutatedJobs, [this]() {
        Q_FOREACH (KisSelectionSP selection, m_d->deactivatedSelections) {
            selection->setVisible(true);
        }

        if (m_d->deactivatedOverlaySelectionMask) {
            m_d->deactivatedOverlaySelectionMask->selection()->setVisible(true);
            m_d->deactivatedOverlaySelectionMask->setDirty();
        }
    });


    if (saveCommands) {
        KritaUtils::addJobBarrier(mutatedJobs, [this]() {
            notifyAllCommandsDone();
        });
    }
}

void InplaceTransformStrokeStrategy::finishAction(QVector<KisStrokeJobData *> &mutatedJobs)
{
    /**
     * * Forward to cancelling should happen before the guard for
     *   finalizingActionsStarted.
     *
     * * Transform masks may switch mode and become identity, that
     *   shouldn't be cancelled.
     */
    if (m_d->currentTransformArgs.isUnchanging() &&
        m_d->transformMaskCacheHash.isEmpty() &&
        !m_d->overriddenCommand) {

        cancelAction(mutatedJobs);
        return;
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

    if (m_d->finalizingActionsStarted) return;
    m_d->finalizingActionsStarted = true;

    if (m_d->previewLevelOfDetail > 0) {
        /**
         * Update jobs from level of detail updates may cause dirtying
         * of the transform mask's static cache device. Therefore we must
         * ensure that final update of the mask happens strictly after
         * them.
         */
        KritaUtils::addJobBarrier(mutatedJobs, [this]() { Q_UNUSED(this) });

        mutatedJobs << new Data(new KisHoldUIUpdatesCommand(m_d->updatesFacade, KisCommandUtils::FlipFlopCommand::INITIALIZING), false, KisStrokeJobData::BARRIER);

        if (!m_d->transformMaskCacheHash.isEmpty()) {
            KritaUtils::addJobSequential(mutatedJobs, [this]() {
                Q_FOREACH (KisTransformMask *mask, m_d->transformMaskCacheHash.keys()) {
                    mask->overrideStaticCacheDevice(0);
                }

                /**
                 * Transform masks don't have internal state switch for LoD mode,
                 * therefore all the preview transformations must be cancelled
                 * before applying the final command
                 */
                m_d->updatesFacade->disableDirtyRequests();
                m_d->updatesDisabled = true;
                undoTransformCommands(m_d->previewLevelOfDetail);
                m_d->updatesFacade->enableDirtyRequests();
                m_d->updatesDisabled = false;
            });
        }

        reapplyTransform(m_d->currentTransformArgs, mutatedJobs, 0);
        mutatedJobs << new Data(new KisHoldUIUpdatesCommand(m_d->updatesFacade, KisCommandUtils::FlipFlopCommand::FINALIZING), false, KisStrokeJobData::BARRIER);
    }

    finalizeStrokeImpl(mutatedJobs, true);

    KritaUtils::addJobBarrier(mutatedJobs, [this]() {
        KisStrokeStrategyUndoCommandBased::finishStrokeCallback();
    });
}

void InplaceTransformStrokeStrategy::cancelAction(QVector<KisStrokeJobData *> &mutatedJobs)
{
    if (m_d->updatesDisabled) {
        m_d->updatesFacade->enableDirtyRequests();
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

    if (m_d->finalizingActionsStarted) return;
    m_d->finalizingActionsStarted = true;

    KIS_SAFE_ASSERT_RECOVER_NOOP(m_d->transformMaskCacheHash.isEmpty() ||
                                 (m_d->transformMaskCacheHash.size() == 1 && m_d->processedNodes.size() == 1));

    const bool isChangingTransformMask = !m_d->transformMaskCacheHash.isEmpty();

    if (m_d->initialTransformArgs.isIdentity()) {
        KritaUtils::addJobBarrier(mutatedJobs, [this]() {
            undoTransformCommands(0);
            undoAllCommands();
        });
        finalizeStrokeImpl(mutatedJobs, false);

        KritaUtils::addJobSequential(mutatedJobs, [this]() {
            Q_FOREACH (KisTransformMask *mask, m_d->transformMaskCacheHash.keys()) {
                mask->overrideStaticCacheDevice(0);
                mask->threadSafeForceStaticImageUpdate();
            }
        });

        KritaUtils::addJobBarrier(mutatedJobs, [this]() {
            KisStrokeStrategyUndoCommandBased::cancelStrokeCallback();
        });
    } else {
        KIS_SAFE_ASSERT_RECOVER_NOOP(isChangingTransformMask || m_d->overriddenCommand);

        KritaUtils::addJobSequential(mutatedJobs, [this]() {
            Q_FOREACH (KisTransformMask *mask, m_d->transformMaskCacheHash.keys()) {
                mask->overrideStaticCacheDevice(0);
            }
        });

        reapplyTransform(m_d->initialTransformArgs, mutatedJobs, 0);
        finalizeStrokeImpl(mutatedJobs, bool(m_d->overriddenCommand));

        KritaUtils::addJobSequential(mutatedJobs, [this]() {
            Q_FOREACH (KisTransformMask *mask, m_d->transformMaskCacheHash.keys()) {
                mask->threadSafeForceStaticImageUpdate();
            }
        });

        if (m_d->overriddenCommand) {
            KritaUtils::addJobBarrier(mutatedJobs, [this]() {
                KisStrokeStrategyUndoCommandBased::finishStrokeCallback();
            });
        } else {
            KritaUtils::addJobBarrier(mutatedJobs, [this]() {
                KisStrokeStrategyUndoCommandBased::cancelStrokeCallback();
            });
        }
    }
}

void InplaceTransformStrokeStrategy::addDirtyRect(KisNodeSP node, const QRect &rect, int levelOfDetail) {
    QMutexLocker l(&m_d->dirtyRectsMutex);
    m_d->effectiveDirtyRects(levelOfDetail)[node] |= rect;
}

#include "inplace_transform_stroke_strategy.moc"
