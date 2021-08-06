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
#include "commands_new/KisDisableDirtyRequestsCommand.h"
#include <kis_shape_layer.h>


struct InplaceTransformStrokeStrategy::Private
{
    // initial conditions passed from the tool
    KisUpdatesFacade *updatesFacade;
    KisStrokeUndoFacade *undoFacade;
    ToolTransformArgs::TransformMode mode;
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

    QMutex commandsMutex;

    struct SavedCommand {
        CommandGroup commandGroup;
        KUndo2CommandSP command;
        KisStrokeJobData::Sequentiality sequentiality;
    };

    QVector<SavedCommand> commands;

    QMutex devicesCacheMutex;
    QHash<KisPaintDevice*, KisPaintDeviceSP> devicesCacheHash;
    QHash<KisTransformMask*, KisPaintDeviceSP> transformMaskCacheHash;

    QMutex dirtyRectsMutex;
    KisBatchNodeUpdate dirtyRects;
    KisBatchNodeUpdate prevDirtyRects;

    KisBatchNodeUpdate dirtyPreviewRects;
    KisBatchNodeUpdate prevDirtyPreviewRects;

    inline KisBatchNodeUpdate& effectiveDirtyRects(int levelOfDetail) {
        return levelOfDetail > 0 ? dirtyPreviewRects : dirtyRects;
    }

    inline KisBatchNodeUpdate& effectivePrevDirtyRects(int levelOfDetail) {
        return levelOfDetail > 0 ? prevDirtyPreviewRects : prevDirtyRects;
    }

    // data for asynchronous updates
    boost::optional<ToolTransformArgs> pendingUpdateArgs;
    QElapsedTimer updateTimer;
    const int updateInterval = 30;

    // temporary variable to share data between jobs at the initialization phase
    QVector<KisDecoratedNodeInterface*> disabledDecoratedNodes;

    /**
     * A special cookie-object, which blocks updates in transform mask
     * modification commands until the stroke ends. As soon as the stroke
     * ends, the object is destroyed and the transform mask modification
     * commands start to behave normally.
     */
    QSharedPointer<boost::none_t> commandUpdatesBlockerCookie;

    bool strokeCompletionHasBeenStarted = false;

    KisBatchNodeUpdateSP updateDataForUndo;
    KisBatchNodeUpdate initialUpdatesBeforeClear;
};


InplaceTransformStrokeStrategy::InplaceTransformStrokeStrategy(ToolTransformArgs::TransformMode mode,
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
    m_d->filterId = filterId;
    m_d->forceReset = forceReset;
    m_d->rootNode = rootNode;
    m_d->selection = selection;
    m_d->externalSource = externalSource;
    m_d->updatesFacade = updatesFacade;
    m_d->undoFacade = undoFacade;
    m_d->imageRoot = imageRoot;
    m_d->forceLodMode = forceLodMode;
    m_d->commandUpdatesBlockerCookie.reset(new boost::none_t(boost::none));

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
        if (upd->destination == UpdateTransformData::PAINT_DEVICE) {
            m_d->pendingUpdateArgs = upd->args;
            tryPostUpdateJob(false);
        } else if (m_d->selection) {
            // NOTE: selection is hidden during the transformation, so we
            //       don't have to do any preview for that. We transform
            //       that in one go in the end of the stroke.

            KisTransaction transaction(m_d->selection->pixelSelection());

            KisProcessingVisitor::ProgressHelper helper(m_d->imageRoot.data());
            KisTransformUtils::transformDevice(upd->args,
                                               m_d->selection->pixelSelection(), &helper);

            runAndSaveCommand(KUndo2CommandSP(transaction.endAndTake()),
                              KisStrokeJobData::CONCURRENT, KisStrokeJobData::NORMAL);
        }

    } else if (BarrierUpdateData *barrierData = dynamic_cast<BarrierUpdateData *>(data)) {

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

    reapplyTransform(args, jobs, m_d->previewLevelOfDetail, false);

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
    m_d->processedNodes = KisTransformUtils::fetchNodesList(m_d->mode, m_d->rootNode, m_d->externalSource);

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
        Q_FOREACH (KisNodeSP node, m_d->processedNodes) {
            m_d->prevDirtyRects.addUpdate(node, node->projectionPlane()->tightUserVisibleBounds());
        }

        m_d->initialUpdatesBeforeClear = m_d->prevDirtyRects.compressed();
        m_d->updateDataForUndo.reset(new KisBatchNodeUpdate(m_d->initialUpdatesBeforeClear));

        executeAndAddCommand(new KisUpdateCommandEx(m_d->updateDataForUndo, m_d->updatesFacade, KisUpdateCommandEx::INITIALIZING, m_d->commandUpdatesBlockerCookie), Clear, KisStrokeJobData::BARRIER);
        executeAndAddCommand(new KisDisableDirtyRequestsCommand(m_d->updatesFacade, KisDisableDirtyRequestsCommand::INITIALIZING), Clear, KisStrokeJobData::BARRIER);
    });

    extraInitJobs << lastCommandUndoJobs;

    if (!lastCommandUndoJobs.isEmpty()) {
        KritaUtils::addJobBarrier(extraInitJobs, [this]() {
            /**
             * In case we are doing a continued action, we need to
             * set initial update to the "very initial" state that
             * was present before the previous stroke. So here we
             * just override the rect calculated before
             */
            KisBatchNodeUpdate updates;

            Q_FOREACH (KisNodeSP node, m_d->processedNodes) {
                updates.addUpdate(node, node->projectionPlane()->tightUserVisibleBounds());
            }

            m_d->initialUpdatesBeforeClear = updates.compressed();
            *m_d->updateDataForUndo = m_d->initialUpdatesBeforeClear;
        });
    }

    KritaUtils::addJobSequential(extraInitJobs, [this]() {
        // When dealing with animated transform mask layers, create keyframe and save the command for undo.
        Q_FOREACH (KisNodeSP node, m_d->processedNodes) {
            if (KisTransformMask* transformMask = dynamic_cast<KisTransformMask*>(node.data())) {
                QSharedPointer<KisInitializeTransformMaskKeyframesCommand> addKeyCommand(new KisInitializeTransformMaskKeyframesCommand(transformMask,
                                                                                                                                        KisTransformMaskParamsInterfaceSP(
                                                                                                                                            new KisTransformMaskAdapter(m_d->initialTransformArgs))));
                runAndSaveCommand( addKeyCommand, KisStrokeJobData::CONCURRENT, KisStrokeJobData::NORMAL);
            }
        }
    });

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

        KisPaintDeviceSP externalSource =
            m_d->externalSource ? m_d->externalSource :
            (argsAreInitialized && m_d->initialTransformArgs.externalSource()) ?
                m_d->initialTransformArgs.externalSource() : 0;

        if (externalSource) {
            // Start the transformation around the visible pixels of the external image
            srcRect = externalSource->exactBounds();
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
                    /// We shouldn't include masks or layer styles into the handles rect,
                    /// in the end, we process the paint device only
                    srcRect |= node->paintDevice() ? node->paintDevice()->exactBounds() : node->exactBounds();
                }
            }
        }

        TransformTransactionProperties transaction(srcRect, &m_d->initialTransformArgs, m_d->rootNode, m_d->processedNodes);
        if (!argsAreInitialized) {
            m_d->initialTransformArgs = KisTransformUtils::resetArgsForMode(m_d->mode, m_d->filterId, transaction, m_d->externalSource);
        }
        m_d->externalSource.clear();

        const QRect imageBoundsRect = m_d->imageRoot->projection()->defaultBounds()->bounds();
        m_d->previewLevelOfDetail = calculatePreferredLevelOfDetail(srcRect & imageBoundsRect);

        if (m_d->previewLevelOfDetail > 0) {
            for (auto it = m_d->prevDirtyRects.begin(); it != m_d->prevDirtyRects.end(); ++it) {
                KisLodTransform t(m_d->previewLevelOfDetail);
                m_d->prevDirtyPreviewRects.addUpdate(it->first, t.map(it->second));
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

        executeAndAddCommand(new KisDisableDirtyRequestsCommand(m_d->updatesFacade, KisDisableDirtyRequestsCommand::FINALIZING), Clear, KisStrokeJobData::BARRIER);

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

void InplaceTransformStrokeStrategy::executeAndAddCommand(KUndo2Command *cmd, InplaceTransformStrokeStrategy::CommandGroup group, KisStrokeJobData::Sequentiality seq)
{
    QMutexLocker l(&m_d->commandsMutex);
    KUndo2CommandSP sharedCommand = toQShared(cmd);
    executeCommand(sharedCommand, false);
    m_d->commands.append({group, sharedCommand, seq});
}

void InplaceTransformStrokeStrategy::notifyAllCommandsDone()
{
    for (auto it = m_d->commands.begin(); it != m_d->commands.end(); ++it) {
        if (it->commandGroup == Clear) {
            notifyCommandDone(it->command, it->sequentiality, KisStrokeJobData::NORMAL);
        }
    }

    notifyCommandDone(toQShared(new KUndo2Command()), KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::NORMAL);

    for (auto it = m_d->commands.begin(); it != m_d->commands.end(); ++it) {
        if (it->commandGroup == Transform) {
            notifyCommandDone(it->command, it->sequentiality, KisStrokeJobData::NORMAL);
        }
    }
}

void InplaceTransformStrokeStrategy::undoAllCommands()
{
    for (auto it = std::make_reverse_iterator(m_d->commands.end());
         it != std::make_reverse_iterator(m_d->commands.begin());
         ++it) {

        executeCommand(it->command, true);
    }

    m_d->commands.clear();
}

void InplaceTransformStrokeStrategy::undoTransformCommands(int levelOfDetail)
{
    for (auto it = std::make_reverse_iterator(m_d->commands.end());
         it != std::make_reverse_iterator(m_d->commands.begin());) {

        if ((levelOfDetail > 0 &&
             (it->commandGroup == TransformLod || it->commandGroup == TransformLodTemporary)) ||
            (levelOfDetail <= 0 &&
             (it->commandGroup == Transform || it->commandGroup == TransformTemporary))) {

            executeCommand(it->command, true);
            it = std::make_reverse_iterator(m_d->commands.erase(std::next(it).base()));
        } else {
            ++it;
        }
    }
}

void InplaceTransformStrokeStrategy::fetchAllUpdateRequests(int levelOfDetail, KisBatchNodeUpdateSP updateData)
{
    KisBatchNodeUpdate &dirtyRects = m_d->effectiveDirtyRects(levelOfDetail);
    KisBatchNodeUpdate &prevDirtyRects = m_d->effectivePrevDirtyRects(levelOfDetail);

    *updateData = (prevDirtyRects | dirtyRects).compressed();

    KisBatchNodeUpdate savedUndoRects = dirtyRects;

    if (levelOfDetail > 0) {

        for (auto it = savedUndoRects.begin(); it != savedUndoRects.end(); ++it) {
            it->second = KisLodTransform::upscaledRect(it->second, levelOfDetail);
        }
    }

    *m_d->updateDataForUndo = (m_d->initialUpdatesBeforeClear | savedUndoRects).compressed();
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
                    const QRect oldDirtyRect = extLayer->projectionPlane()->tightUserVisibleBounds() | extLayer->theoreticalBoundingRect();

                    QVector3D transformedCenter;
                    KisTransformWorker w = KisTransformUtils::createTransformWorker(config, 0, 0, &transformedCenter);
                    QTransform t = w.transform();
                    KUndo2Command *cmd = extLayer->transform(t);

                    executeAndAddCommand(cmd, Transform, KisStrokeJobData::CONCURRENT);

                    /// we should make sure that the asynchronous shape regeneration
                    /// has completed before we issue the updates a bit later
                    if (KisShapeLayer *shapeLayer = dynamic_cast<KisShapeLayer*>(extLayer)) {
                        shapeLayer->forceUpdateTimedNode();
                    }

                    /**
                     * Shape layer's projection may not be yet ready right
                     * after transformation, because it need to do that in
                     * the GUI thread, so we should approximate that.
                     */
                    const QRect theoreticalNewDirtyRect =
                        kisGrowRect(t.mapRect(oldDirtyRect), 1);

                    addDirtyRect(node,
                                 oldDirtyRect |
                                 theoreticalNewDirtyRect |
                                 extLayer->projectionPlane()->tightUserVisibleBounds() |
                                 extLayer->theoreticalBoundingRect(), 0);
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

        executeAndAddCommand(transaction.endAndTake(), commandGroup, KisStrokeJobData::CONCURRENT);
        addDirtyRect(node, cachedPortion->extent() | node->projectionPlane()->tightUserVisibleBounds(), levelOfDetail);

    } else if (KisTransformMask *transformMask =
               dynamic_cast<KisTransformMask*>(node.data())) {

        const QRect oldDirtyRect = transformMask->extent();

        if (levelOfDetail > 0) {
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
        }


        if (commandGroup == Transform) { // Set Keyframe Data.
            ToolTransformArgs unscaled = ToolTransformArgs(config);

            if (levelOfDetail > 0) {
                unscaled.scale3dSrcAndDst(KisLodTransform::lodToInvScale(levelOfDetail));
            }

            KUndo2Command* cmd = new KisSetTransformMaskKeyframesCommand(transformMask,
                                                          KisTransformMaskParamsInterfaceSP(
                                                                new KisTransformMaskAdapter(unscaled)));
            executeAndAddCommand(cmd, commandGroup, KisStrokeJobData::BARRIER);
        }


        {
            KUndo2Command *cmd = new KisModifyTransformMaskCommand(transformMask,
                                                                   KisTransformMaskParamsInterfaceSP(
                                                                       new KisTransformMaskAdapter(config)),
                                                                   m_d->commandUpdatesBlockerCookie);
            executeAndAddCommand(cmd, commandGroup, KisStrokeJobData::CONCURRENT);
            addDirtyRect(node, oldDirtyRect | transformMask->extent(), levelOfDetail);
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

        executeAndAddCommand(transaction.endAndTake(), commandGroup, KisStrokeJobData::CONCURRENT);
    }
}

void InplaceTransformStrokeStrategy::reapplyTransform(ToolTransformArgs args,
                                                      QVector<KisStrokeJobData *> &mutatedJobs,
                                                      int levelOfDetail,
                                                      bool useHoldUI)
{
    if (levelOfDetail > 0) {
        args.scale3dSrcAndDst(KisLodTransform::lodToScale(levelOfDetail));
    }

    KisBatchNodeUpdateSP updateData(new KisBatchNodeUpdate());

    CommandGroup commandGroup =
        levelOfDetail > 0 ? TransformLod : Transform;

    KritaUtils::addJobBarrier(mutatedJobs, levelOfDetail,
                              [this, args, levelOfDetail, updateData, useHoldUI, commandGroup]() {

        // it has its own dirty requests blocking inside
        undoTransformCommands(levelOfDetail);

        if (useHoldUI) {
            executeAndAddCommand(new KisHoldUIUpdatesCommand(m_d->updatesFacade, KisCommandUtils::FlipFlopCommand::INITIALIZING), commandGroup, KisStrokeJobData::BARRIER);
        }

        executeAndAddCommand(new KisDisableDirtyRequestsCommand(m_d->updatesFacade, KisUpdateCommandEx::INITIALIZING), commandGroup, KisStrokeJobData::BARRIER);
    });

    Q_FOREACH (KisNodeSP node, m_d->processedNodes) {
        KritaUtils::addJobConcurrent(mutatedJobs, levelOfDetail,
                                     [this, node, args, levelOfDetail]() {
            transformNode(node, args, levelOfDetail);
        });
    }

    KritaUtils::addJobBarrier(mutatedJobs, levelOfDetail,
                              [this, levelOfDetail, updateData, useHoldUI, commandGroup]() {

        fetchAllUpdateRequests(levelOfDetail, updateData);

        executeAndAddCommand(new KisDisableDirtyRequestsCommand(m_d->updatesFacade, KisUpdateCommandEx::FINALIZING), commandGroup, KisStrokeJobData::BARRIER);
        executeAndAddCommand(new KisUpdateCommandEx(m_d->updateDataForUndo, m_d->updatesFacade, KisUpdateCommandEx::FINALIZING, m_d->commandUpdatesBlockerCookie), commandGroup, KisStrokeJobData::BARRIER);

        if (useHoldUI) {
            executeAndAddCommand(new KisHoldUIUpdatesCommand(m_d->updatesFacade, KisCommandUtils::FlipFlopCommand::FINALIZING), commandGroup, KisStrokeJobData::BARRIER);
        }

        /**
         * We also emit the updates manually, because KisUpdateCommandEx is
         * still blocked by m_d->commandUpdatesBlockerCookie (for easy undo
         * purposes)
         */
        for (auto it = updateData->begin(); it != updateData->end(); ++it) {
            KisTransformMask *transformMask = dynamic_cast<KisTransformMask*>(it->first.data());

            if (transformMask &&
                    ((levelOfDetail <= 0 && !transformMask->transformParams()->isAffine()) ||
                     (levelOfDetail <= 0 && m_d->previewLevelOfDetail > 0))) {
                transformMask->threadSafeForceStaticImageUpdate();
            } else {
                m_d->updatesFacade->refreshGraphAsync(it->first, it->second);
            }

        }
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

        m_d->commandUpdatesBlockerCookie.reset();
    });


    if (saveCommands) {
        KritaUtils::addJobBarrier(mutatedJobs, [this]() {
            notifyAllCommandsDone();
            m_d->commands.clear();
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

    if (m_d->previewLevelOfDetail > 0) {
        /**
         * Update jobs from level of detail updates may cause dirtying
         * of the transform mask's static cache device. Therefore we must
         * ensure that final update of the mask happens strictly after
         * them.
         */
        KritaUtils::addJobBarrier(mutatedJobs, [this]() { Q_UNUSED(this) });

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
                undoTransformCommands(m_d->previewLevelOfDetail);
            });
        }

        reapplyTransform(m_d->currentTransformArgs, mutatedJobs, 0, true);
    }

    mutatedJobs << new UpdateTransformData(m_d->currentTransformArgs,
                                           UpdateTransformData::SELECTION);

    // the rest of the transform finishing work cannot be cancelled...
    KritaUtils::addJobBarrier(mutatedJobs, [this]() {
        m_d->strokeCompletionHasBeenStarted = true;

        QVector<KisStrokeJobData *> nonCancellableFinishJobs;

        finalizeStrokeImpl(nonCancellableFinishJobs, true);

        KritaUtils::addJobBarrier(nonCancellableFinishJobs, [this]() {
            KisStrokeStrategyUndoCommandBased::finishStrokeCallback();
        });

        for (auto it = nonCancellableFinishJobs.begin(); it != nonCancellableFinishJobs.end(); ++it) {
            (*it)->setCancellable(false);
        }

        this->addMutatedJobs(nonCancellableFinishJobs);

    });
}

void InplaceTransformStrokeStrategy::cancelAction(QVector<KisStrokeJobData *> &mutatedJobs)
{
    /**
     * It is too late to cancel anything, the transformation has been completed
     * and its commands have been pushed into the undo adapter, so there is no
     * way to stop that.
     */
    if (m_d->strokeCompletionHasBeenStarted) return;


    KIS_SAFE_ASSERT_RECOVER_NOOP(m_d->transformMaskCacheHash.isEmpty() ||
                                 (m_d->transformMaskCacheHash.size() == 1 && m_d->processedNodes.size() == 1));

    const bool isChangingTransformMask = !m_d->transformMaskCacheHash.isEmpty();

    if (m_d->initialTransformArgs.isIdentity()) {
        KritaUtils::addJobBarrier(mutatedJobs, [this]() {
            m_d->commandUpdatesBlockerCookie.reset();
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

        reapplyTransform(m_d->initialTransformArgs, mutatedJobs, 0, true);

        mutatedJobs << new UpdateTransformData(m_d->initialTransformArgs,
                                               UpdateTransformData::SELECTION);

        finalizeStrokeImpl(mutatedJobs, bool(m_d->overriddenCommand));

        KritaUtils::addJobSequential(mutatedJobs, [this]() {
            Q_FOREACH (KisTransformMask *mask, m_d->transformMaskCacheHash.keys()) {
                mask->threadSafeForceStaticImageUpdate();
            }
        });

        if (m_d->overriddenCommand) {
            KritaUtils::addJobBarrier(mutatedJobs, [this]() {
                m_d->currentTransformArgs = m_d->initialTransformArgs;
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
    m_d->effectiveDirtyRects(levelOfDetail).addUpdate(node, rect);
}
