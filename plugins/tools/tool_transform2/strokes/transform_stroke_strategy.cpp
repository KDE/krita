/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "transform_stroke_strategy.h"

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
#include "kis_convex_hull.h"
#include "kis_abstract_projection_plane.h"
#include "kis_recalculate_transform_mask_job.h"
#include "kis_lod_transform.h"

#include "kis_projection_leaf.h"
#include "commands_new/KisSimpleModifyTransformMaskCommand.h"

#include "kis_image_animation_interface.h"
#include "KisAnimAutoKey.h"
#include "kis_sequential_iterator.h"
#include "kis_selection_mask.h"
#include "kis_transparency_mask.h"
#include "kis_filter_mask.h"
#include "kis_image_config.h"
#include "kis_layer_utils.h"
#include <QQueue>
#include <KisDeleteLaterWrapper.h>
#include "transform_transaction_properties.h"
#include "krita_container_utils.h"
#include "commands_new/kis_saved_commands.h"
#include "commands_new/KisLazyCreateTransformMaskKeyframesCommand.h"
#include "kis_command_ids.h"
#include "KisRunnableStrokeJobUtils.h"
#include "commands_new/KisHoldUIUpdatesCommand.h"
#include "KisDecoratedNodeInterface.h"
#include "kis_paint_device_debug_utils.h"
#include "kis_raster_keyframe_channel.h"
#include "kis_layer_utils.h"
#include "KisAnimAutoKey.h"


TransformStrokeStrategy::TransformStrokeStrategy(ToolTransformArgs::TransformMode mode,
                                                 const QString &filterId,
                                                 bool forceReset,
                                                 KisNodeList rootNodes,
                                                 KisSelectionSP selection,
                                                 KisStrokeUndoFacade *undoFacade,
                                                 KisUpdatesFacade *updatesFacade)
    : KisStrokeStrategyUndoCommandBased(kundo2_i18n("Transform"), false, undoFacade),
      m_updatesFacade(updatesFacade),
      m_mode(mode),
      m_filterId(filterId),
      m_forceReset(forceReset),
      m_selection(selection)
{
    if (selection) {
        Q_FOREACH(KisNodeSP node, rootNodes) {
            KIS_SAFE_ASSERT_RECOVER_NOOP(!dynamic_cast<KisTransformMask*>(node.data()));
        }
    }

    m_rootNodes = rootNodes;
    setMacroId(KisCommandUtils::TransformToolId);
}

TransformStrokeStrategy::~TransformStrokeStrategy()
{
}

KisPaintDeviceSP TransformStrokeStrategy::createDeviceCache(KisPaintDeviceSP dev)
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

bool TransformStrokeStrategy::haveDeviceInCache(KisPaintDeviceSP src)
{
    QMutexLocker l(&m_devicesCacheMutex);
    return m_devicesCacheHash.contains(src.data());
}

void TransformStrokeStrategy::putDeviceCache(KisPaintDeviceSP src, KisPaintDeviceSP cache)
{
    QMutexLocker l(&m_devicesCacheMutex);
    m_devicesCacheHash.insert(src.data(), cache);
}

KisPaintDeviceSP TransformStrokeStrategy::getDeviceCache(KisPaintDeviceSP src)
{
    QMutexLocker l(&m_devicesCacheMutex);
    KisPaintDeviceSP cache = m_devicesCacheHash.value(src.data());
    if (!cache) {
        warnKrita << "WARNING: Transform Stroke: the device is absent in cache!";
    }

    return cache;
}

bool TransformStrokeStrategy::checkBelongsToSelection(KisPaintDeviceSP device) const
{
    return m_selection &&
        (device == m_selection->pixelSelection().data() ||
         device == m_selection->projection().data());
}

void TransformStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    TransformData *td = dynamic_cast<TransformData*>(data);
    ClearSelectionData *csd = dynamic_cast<ClearSelectionData*>(data);
    PreparePreviewData *ppd = dynamic_cast<PreparePreviewData*>(data);
    TransformAllData *runAllData = dynamic_cast<TransformAllData*>(data);
    CalculateConvexHullData *cch = dynamic_cast<CalculateConvexHullData*>(data);


    if (runAllData) {
        // here we only save the passed args, actual
        // transformation will be performed during
        // finish job
        m_savedTransformArgs = runAllData->config;
    } else if (ppd) {
        KisNodeSP rootNode = m_rootNodes[0];
        KisNodeList processedNodes = m_processedNodes;
        KisPaintDeviceSP previewDevice;


        if (processedNodes.size() > 1) {
            const QRect bounds = rootNode->image()->bounds();
            const int desiredAnimTime = rootNode->image()->animationInterface()->currentTime();

            KisImageSP clonedImage = new KisImage(0,
                                                  bounds.width(),
                                                  bounds.height(),
                                                  rootNode->image()->colorSpace(),
                                                  "transformed_image");

            // BUG: 413968
            // Workaround: Group layers wouldn't properly render the right frame
            // since `clonedImage` would always have a time value of 0.
            clonedImage->animationInterface()->explicitlySetCurrentTime(desiredAnimTime);

            KisNodeSP cloneRoot = clonedImage->rootLayer();

            Q_FOREACH(KisNodeSP node, processedNodes) {
                // Masks with unselected parents can't be added.
                if (!node->inherits("KisMask")) {
                    clonedImage->addNode(node->clone().data(), cloneRoot);
                }
            }

            clonedImage->refreshGraphAsync();
            KisLayerUtils::refreshHiddenAreaAsync(clonedImage, cloneRoot, clonedImage->bounds());

            KisLayerUtils::forceAllDelayedNodesUpdate(cloneRoot);
            clonedImage->waitForDone();

            previewDevice = createDeviceCache(clonedImage->projection());
            previewDevice->setDefaultBounds(cloneRoot->projection()->defaultBounds());

            // we delete the cloned image in GUI thread to ensure
            // no signals are still pending
            makeKisDeleteLaterWrapper(clonedImage)->deleteLater();
        }
        else if (rootNode->childCount() || !rootNode->paintDevice()) {
            if (KisTransformMask* tmask =
                dynamic_cast<KisTransformMask*>(rootNode.data())) {
                previewDevice = createDeviceCache(tmask->buildPreviewDevice());

                KIS_SAFE_ASSERT_RECOVER(!m_selection) {
                    m_selection = 0;
                }

            } else if (KisGroupLayer *group = dynamic_cast<KisGroupLayer*>(rootNode.data())) {
                const QRect bounds = group->image()->bounds();
                const int desiredAnimTime = group->image()->animationInterface()->currentTime();

                KisImageSP clonedImage = new KisImage(0,
                                                      bounds.width(),
                                                      bounds.height(),
                                                      group->colorSpace(),
                                                      "transformed_image");

                // BUG: 413968
                // Workaround: Group layers wouldn't properly render the right frame
                // since `clonedImage` would always have a time value of 0.
                clonedImage->animationInterface()->explicitlySetCurrentTime(desiredAnimTime);

                KisGroupLayerSP clonedGroup = dynamic_cast<KisGroupLayer*>(group->clone().data());

                // In case the group is pass-through, it needs to be disabled for the preview,
                //   otherwise it will crash (no parent for a preview leaf).
                // Also it needs to be done before setting the root layer for clonedImage.
                // Result: preview for pass-through group is the same as for standard group
                //   (i.e. filter layers in the group won't affect the layer stack for a moment).
                clonedGroup->setPassThroughMode(false);
                clonedImage->setRootLayer(clonedGroup);

                QQueue<KisNodeSP> linearizedSrcNodes;
                KisLayerUtils::recursiveApplyNodes(rootNode, [&linearizedSrcNodes] (KisNodeSP node) {
                    linearizedSrcNodes.enqueue(node);
                });

                KisLayerUtils::recursiveApplyNodes(KisNodeSP(clonedGroup), [&linearizedSrcNodes, processedNodes] (KisNodeSP node) {
                    KisNodeSP srcNode = linearizedSrcNodes.dequeue();

                    if (!processedNodes.contains(srcNode)) {
                        node->setVisible(false);
                    }
                });

                clonedImage->refreshGraphAsync();
                KisLayerUtils::refreshHiddenAreaAsync(clonedImage, clonedGroup, clonedImage->bounds());

                KisLayerUtils::forceAllDelayedNodesUpdate(clonedGroup);
                clonedImage->waitForDone();

                previewDevice = createDeviceCache(clonedImage->projection());
                previewDevice->setDefaultBounds(group->projection()->defaultBounds());

                // we delete the cloned image in GUI thread to ensure
                // no signals are still pending
                makeKisDeleteLaterWrapper(clonedImage)->deleteLater();

            } else {
                rootNode->projectionLeaf()->explicitlyRegeneratePassThroughProjection();
                previewDevice = createDeviceCache(rootNode->projection());
            }



        } else {
            KisPaintDeviceSP cacheDevice = createDeviceCache(rootNode->paintDevice());

            if (dynamic_cast<KisSelectionMask*>(rootNode.data())) {
                KIS_SAFE_ASSERT_RECOVER (cacheDevice->colorSpace()->colorModelId() == GrayAColorModelID &&
                                         cacheDevice->colorSpace()->colorDepthId() == Integer8BitsColorDepthID) {

                    cacheDevice->convertTo(KoColorSpaceRegistry::instance()->colorSpace(GrayAColorModelID.id(), Integer8BitsColorDepthID.id()));
                }

                previewDevice = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
                const QRect srcRect = cacheDevice->exactBounds();

                KisSequentialConstIterator srcIt(cacheDevice, srcRect);
                KisSequentialIterator dstIt(previewDevice, srcRect);

                const int pixelSize = previewDevice->colorSpace()->pixelSize();


                KisImageConfig cfg(true);
                KoColor pixel(cfg.selectionOverlayMaskColor(), previewDevice->colorSpace());

                const qreal coeff = 1.0 / 255.0;
                const qreal baseOpacity = 0.5;

                while (srcIt.nextPixel() && dstIt.nextPixel()) {
                    qreal gray = srcIt.rawDataConst()[0];
                    qreal alpha = srcIt.rawDataConst()[1];

                    pixel.setOpacity(quint8(gray * alpha * baseOpacity * coeff));
                    memcpy(dstIt.rawData(), pixel.data(), pixelSize);
                }

            } else {
                previewDevice = cacheDevice;
            }

            putDeviceCache(rootNode->paintDevice(), cacheDevice);
        }

        Q_EMIT sigPreviewDeviceReady(previewDevice);
    }
    else if (td) {
        if (td->destination == TransformData::PAINT_DEVICE) {
            QRect oldExtent = td->node->projectionPlane()->tightUserVisibleBounds();
            KisPaintDeviceSP device = td->node->paintDevice();

            if (device && !checkBelongsToSelection(device)) {
                KisPaintDeviceSP cachedPortion = getDeviceCache(device);
                Q_ASSERT(cachedPortion);

                KisTransaction transaction(device);

                KisProcessingVisitor::ProgressHelper helper(td->node);
                KisTransformUtils::transformAndMergeDevice(td->config, cachedPortion,
                                                           device, &helper);

                runAndSaveCommand(KUndo2CommandSP(transaction.endAndTake()),
                                  KisStrokeJobData::CONCURRENT,
                                  KisStrokeJobData::NORMAL);

                m_updateData->addUpdate(td->node, cachedPortion->extent() | oldExtent | td->node->projectionPlane()->tightUserVisibleBounds());
            } else if (KisExternalLayer *extLayer =
                  dynamic_cast<KisExternalLayer*>(td->node.data())) {

                if (td->config.mode() == ToolTransformArgs::FREE_TRANSFORM ||
                    (td->config.mode() == ToolTransformArgs::PERSPECTIVE_4POINT &&
                     extLayer->supportsPerspectiveTransform())) {

                    QRect oldDirtyRect = oldExtent | extLayer->theoreticalBoundingRect();

                    KisTransformWorker w = KisTransformUtils::createTransformWorker(td->config, 0, 0);
                    QTransform t = w.transform();

                    runAndSaveCommand(KUndo2CommandSP(extLayer->transform(t)),
                                      KisStrokeJobData::CONCURRENT,
                                      KisStrokeJobData::NORMAL);

                    /**
                     * Shape layer's projection may not be yet ready right
                     * after transformation, because it need to do that in
                     * the GUI thread, so we should approximate that.
                     */
                    const QRect theoreticalNewDirtyRect =
                        kisGrowRect(t.mapRect(oldDirtyRect), 1);

                    m_updateData->addUpdate(td->node, oldDirtyRect | td->node->projectionPlane()->tightUserVisibleBounds() | extLayer->theoreticalBoundingRect() | theoreticalNewDirtyRect);
                }

            } else if (KisTransformMask *transformMask =
                       dynamic_cast<KisTransformMask*>(td->node.data())) {

                runAndSaveCommand(KUndo2CommandSP(
                                      new KisSimpleModifyTransformMaskCommand(transformMask,
                                                                              KisTransformMaskParamsInterfaceSP(
                                                                                  new KisTransformMaskAdapter(td->config)))),
                                  KisStrokeJobData::CONCURRENT,
                                  KisStrokeJobData::NORMAL);

                m_updateData->addUpdate(td->node, oldExtent | td->node->extent());
            }
        } else if (m_selection) {

            /**
             * We use usual transaction here, because we cannot calculate
             * transformation for perspective and warp workers.
             */
            KisTransaction transaction(m_selection->pixelSelection());

            KisProcessingVisitor::ProgressHelper helper(td->node);
            KisTransformUtils::transformDevice(td->config,
                                               m_selection->pixelSelection(),
                                               &helper);

            runAndSaveCommand(KUndo2CommandSP(transaction.endAndTake()),
                              KisStrokeJobData::CONCURRENT,
                              KisStrokeJobData::NORMAL);
        }
    } else if (csd) {
        KisPaintDeviceSP device = csd->node->paintDevice();

        if (device && !checkBelongsToSelection(device)) {
            if (!haveDeviceInCache(device)) {
                putDeviceCache(device, createDeviceCache(device));
            }
            clearSelection(device);

            /**
             * Selection masks might have an overlay enabled, we should disable that
             */
            if (KisSelectionMask *mask = dynamic_cast<KisSelectionMask*>(csd->node.data())) {
                KisSelectionSP selection = mask->selection();
                if (selection) {
                    selection->setVisible(false);
                    m_deactivatedSelections.append(selection);
                }
            }
        } else if (KisExternalLayer *externalLayer = dynamic_cast<KisExternalLayer*>(csd->node.data())) {
            externalLayer->projectionLeaf()->setTemporaryHiddenFromRendering(true);
            m_hiddenProjectionLeaves.append(csd->node);
        } else if (KisTransformMask *transformMask =
                   dynamic_cast<KisTransformMask*>(csd->node.data())) {

            KisTransformMaskParamsInterfaceSP params = transformMask->transformParams();
            params->setHidden(true);

            runAndSaveCommand(KUndo2CommandSP(
                                  new KisSimpleModifyTransformMaskCommand(transformMask,
                                                                          params)),
                                  KisStrokeJobData::SEQUENTIAL,
                                  KisStrokeJobData::NORMAL);
        }
    } else if (cch) {
        if (!m_convexHullHasBeenCalculated) {
            m_convexHullHasBeenCalculated = true;
            QPolygon hull = calculateConvexHull();
            if (!hull.isEmpty()) {
                Q_EMIT sigConvexHullCalculated(hull, this);
            }
        }
    } else {
        KisStrokeStrategyUndoCommandBased::doStrokeCallback(data);
    }
}

void TransformStrokeStrategy::clearSelection(KisPaintDeviceSP device)
{
    KisTransaction transaction(device);
    if (m_selection) {
        device->clearSelection(m_selection);
    } else {
        device->clear();
    }
    runAndSaveCommand(KUndo2CommandSP(transaction.endAndTake()),
                      KisStrokeJobData::SEQUENTIAL,
                      KisStrokeJobData::NORMAL);
}

void TransformStrokeStrategy::postProcessToplevelCommand(KUndo2Command *command)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_savedTransformArgs);

    KisTransformUtils::postProcessToplevelCommand(command,
                                                  *m_savedTransformArgs,
                                                  m_rootNodes,
                                                  m_processedNodes,
                                                  m_currentTime,
                                                  m_overriddenCommand);

    KisStrokeStrategyUndoCommandBased::postProcessToplevelCommand(command);
}

QPolygon TransformStrokeStrategy::calculateConvexHull()
{
    // Best effort attempt to calculate the convex hull, mimicking the
    // approach that computes srcRect in initStrokeCallback below
    QVector<QPoint> points;
    if (m_selection) {
        points = KisConvexHull::findConvexHull(m_selection->pixelSelection());
    } else {
        int numContributions = 0;
        Q_FOREACH (KisNodeSP node, m_processedNodes) {
            if (node->inherits("KisGroupLayer")) continue;

            if (dynamic_cast<const KisTransformMask*>(node.data())) {
                return QPolygon(); // Produce no convex hull if a KisTransformMask is present
            } else {
                KisPaintDeviceSP device;
                if (KisExternalLayer *extLayer = dynamic_cast<KisExternalLayer*>(node.data())) {
                    device = extLayer->projection();
                } else {
                    device = node->paintDevice();
                }
                if (device) {
                    KisPaintDeviceSP toUse;
                    // Use the original device to get the cached device containing the original image data
                    if (haveDeviceInCache(device)) {
                        toUse = getDeviceCache(device);
                    } else {
                        toUse = device;
                    }
                    /* This sometimes does not agree with the original exactBounds
                       because of colorspace changes between the original device
                       and cached. E.g. When the defaultPixel changes as follows it
                       triggers different behavior in calculateExactBounds:
                       KoColor ("ALPHA", "Alpha":0) => KoColor ("GRAYA", "Gray":0, "Alpha":255) 
                    */
                    const bool isConvertedSelection =
                        node->paintDevice() &&
                        node->paintDevice()->colorSpace()->colorModelId() == AlphaColorModelID &&
                        *toUse->colorSpace() == *node->paintDevice()->compositionSourceColorSpace();

                    QPolygon polygon = isConvertedSelection ?
                        KisConvexHull::findConvexHullSelectionLike(toUse) :
                        KisConvexHull::findConvexHull(toUse);

                    points += polygon;
                    numContributions += 1;
                } else {
                    // When can this happen?  Should it continue instead?
                    ENTER_FUNCTION() << "Bailing out, device was null" << ppVar(node);
                    return QPolygon();
                }
            }
        }
        if (numContributions > 1) {
            points = KisConvexHull::findConvexHull(points);
        }
    }
    return QPolygon(points);
}

void TransformStrokeStrategy::initStrokeCallback()
{
    KisStrokeStrategyUndoCommandBased::initStrokeCallback();

    m_currentTime = KisTransformUtils::fetchCurrentImageTime(m_rootNodes);

    if (m_selection) {
        m_selection->setVisible(false);
        m_deactivatedSelections.append(m_selection);
    }

    Q_FOREACH(KisNodeSP node, m_rootNodes) {
        KisSelectionMaskSP overlaySelectionMask =
                dynamic_cast<KisSelectionMask*>(node->graphListener()->graphOverlayNode());
        if (overlaySelectionMask) {
            overlaySelectionMask->setDecorationsVisible(false);
            m_deactivatedOverlaySelectionMasks.append(overlaySelectionMask);
        }
    }

    if (m_rootNodes.size() == 1){
        KisNodeSP rootNode = m_rootNodes[0];
        rootNode = KisTransformUtils::tryOverrideRootToTransformMask(rootNode);

        if (rootNode->inherits("KisTransformMask") && rootNode->projectionLeaf()->isDroppedNode()) {
            rootNode.clear();
            m_processedNodes.clear();

            TransformTransactionProperties transaction(QRect(), &m_initialTransformArgs, m_rootNodes, m_processedNodes);
            Q_EMIT sigTransactionGenerated(transaction, m_initialTransformArgs, this);
            return;
        }
    }

    ToolTransformArgs initialTransformArgs;
    bool isExternalSourcePresent = false;
    m_processedNodes = KisTransformUtils::fetchNodesList(m_mode, m_rootNodes, isExternalSourcePresent, m_selection);

    bool argsAreInitialized = false;
    QVector<KisStrokeJobData *> lastCommandUndoJobs;

    if (!m_forceReset && KisTransformUtils::tryFetchArgsFromCommandAndUndo(&initialTransformArgs,
                                                                           m_mode,
                                                                           m_rootNodes,
                                                                           m_processedNodes,
                                                                           undoFacade(),
                                                                           m_currentTime,
                                                                           &lastCommandUndoJobs,
                                                                           &m_overriddenCommand)) {
        argsAreInitialized = true;
    } else if (!m_forceReset && KisTransformUtils::tryInitArgsFromNode(m_rootNodes, &initialTransformArgs)) {
        argsAreInitialized = true;
    }

    QVector<KisStrokeJobData *> extraInitJobs;

    extraInitJobs << new Data(new KisHoldUIUpdatesCommand(m_updatesFacade, KisCommandUtils::FlipFlopCommand::INITIALIZING), false, KisStrokeJobData::BARRIER);

    extraInitJobs << lastCommandUndoJobs;

    KritaUtils::addJobSequential(extraInitJobs, [this]() {
        // When dealing with animated transform mask layers, create keyframe and save the command for undo.
        // NOTE: for transform masks we create a keyframe no matter what the user
        //       settings are
        Q_FOREACH (KisNodeSP node, m_processedNodes) {
            if (KisTransformMask* transformMask = dynamic_cast<KisTransformMask*>(node.data())) {
                if (KisLazyCreateTransformMaskKeyframesCommand::maskHasAnimation(transformMask)) {
                    runAndSaveCommand(toQShared(new KisLazyCreateTransformMaskKeyframesCommand(transformMask)), KisStrokeJobData::BARRIER, KisStrokeJobData::NORMAL);
                }
            } else if (KisAutoKey::activeMode() > KisAutoKey::NONE &&
                       node->hasEditablePaintDevice()){

                KUndo2Command *autoKeyframeCommand =
                        KisAutoKey::tryAutoCreateDuplicatedFrame(node->paintDevice(),
                                                                 KisAutoKey::SupportsLod);
                if (autoKeyframeCommand) {
                    runAndSaveCommand(toQShared(autoKeyframeCommand), KisStrokeJobData::BARRIER, KisStrokeJobData::NORMAL);
                }
            }
        }
    });

    KritaUtils::addJobSequential(extraInitJobs, [this]() {
        /**
         * We must request shape layers to rerender areas outside image bounds
         */
        Q_FOREACH(KisNodeSP node, m_rootNodes) {
            KisLayerUtils::forceAllHiddenOriginalsUpdate(node);
        }
    });

    KritaUtils::addJobBarrier(extraInitJobs, [this]() {
        /**
         * We must ensure that the currently selected subtree
         * has finished all its updates.
         */
        Q_FOREACH(KisNodeSP node, m_rootNodes) {
            KisLayerUtils::forceAllDelayedNodesUpdate(node);
        }
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

    KritaUtils::addJobBarrier(extraInitJobs, [this, initialTransformArgs, argsAreInitialized]() mutable {
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
                } else if (const KisTransparencyMask *mask = dynamic_cast<const KisTransparencyMask*>(node.data())) {
                    srcRect |= mask->selection()->selectedExactRect();
                } else if (const KisFilterMask *mask = dynamic_cast<const KisFilterMask*>(node.data())) {
                    /// Filter masks have special handling of transparency. Their filter
                    /// may declare if they affect transparent pixels or not. In case of
                    /// transformations we don't care about that, we should just transform
                    /// non-default area of the mask.
                    if (mask->paintDevice()) {
                        srcRect |= mask->paintDevice()->nonDefaultPixelArea();
                    }
                } else {
                    /// We shouldn't include masks or layer styles into the handles rect,
                    /// in the end, we process the paint device only
                    srcRect |= node->paintDevice() ? node->paintDevice()->exactBounds() : node->exactBounds();
                }
            }
        }

        TransformTransactionProperties transaction(srcRect, &initialTransformArgs, m_rootNodes, m_processedNodes);
        if (!argsAreInitialized) {
            initialTransformArgs = KisTransformUtils::resetArgsForMode(m_mode, m_filterId, transaction, 0);
        }

        this->m_initialTransformArgs = initialTransformArgs;
        if (transaction.currentConfig()->boundsRotation() != 0.0) {
            this->m_convexHullHasBeenCalculated = true;
            transaction.setConvexHull(calculateConvexHull());
            transaction.setConvexHullHasBeenRequested(true);
        }
        Q_EMIT this->sigTransactionGenerated(transaction, initialTransformArgs, this);
    });

    extraInitJobs << new PreparePreviewData();

    KisBatchNodeUpdateSP sharedData(new KisBatchNodeUpdate());

    KritaUtils::addJobBarrier(extraInitJobs, [this, sharedData]() {
        KisNodeList filteredRoots = KisLayerUtils::sortAndFilterMergeableInternalNodes(m_processedNodes, true);
        Q_FOREACH (KisNodeSP root, filteredRoots) {
            sharedData->addUpdate(root, root->projectionPlane()->tightUserVisibleBounds());
        }
    });

    extraInitJobs << new Data(new KisUpdateCommandEx(sharedData, m_updatesFacade, KisUpdateCommandEx::INITIALIZING), false, Data::BARRIER);

    Q_FOREACH (KisNodeSP node, m_processedNodes) {
        extraInitJobs << new ClearSelectionData(node);
    }

    extraInitJobs << new Data(new KisUpdateCommandEx(sharedData, m_updatesFacade, KisUpdateCommandEx::FINALIZING), false, Data::BARRIER);

    /// recover back visibility of decorated nodes
    KritaUtils::addJobBarrier(extraInitJobs, [this]() {
        Q_FOREACH (KisDecoratedNodeInterface *decoratedNode, m_disabledDecoratedNodes) {
            decoratedNode->setDecorationsVisible(true);
        }
        m_disabledDecoratedNodes.clear();
    });

    extraInitJobs << new Data(toQShared(new KisHoldUIUpdatesCommand(m_updatesFacade, KisCommandUtils::FlipFlopCommand::FINALIZING)), false, KisStrokeJobData::BARRIER);

    if (!lastCommandUndoJobs.isEmpty()) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(m_overriddenCommand);

        for (auto it = extraInitJobs.begin(); it != extraInitJobs.end(); ++it) {
            (*it)->setCancellable(false);
        }
    }

    addMutatedJobs(extraInitJobs);
}

void TransformStrokeStrategy::finishStrokeImpl(bool applyTransform, const ToolTransformArgs &args)
{
    /**
     * Since our finishStrokeCallback() initiates new jobs,
     * cancellation request may come even after
     * finishStrokeCallback() (cancellations may be called
     * until there are no jobs left in the stroke's queue).
     *
     * Therefore we should check for double-entry here and
     * make sure the finalizing jobs are no cancellable.
     */

    if (m_finalizingActionsStarted) return;
    m_finalizingActionsStarted = true;

    QVector<KisStrokeJobData *> mutatedJobs;

    auto restoreTemporaryHiddenNodes = [this] () {
        Q_FOREACH (KisNodeSP node, m_hiddenProjectionLeaves) {
            node->projectionLeaf()->setTemporaryHiddenFromRendering(false);
            if (KisDelayedUpdateNodeInterface *delayedNode = dynamic_cast<KisDelayedUpdateNodeInterface*>(node.data())) {
                delayedNode->forceUpdateTimedNode();
            } else {
                node->setDirty();
            }
        }
    };

    if (applyTransform) {
        m_savedTransformArgs = args;

        m_updateData.reset(new KisBatchNodeUpdate());

        KritaUtils::addJobBarrier(mutatedJobs, [this] () {
            runAndSaveCommand(toQShared(new KisUpdateCommandEx(m_updateData, m_updatesFacade, KisUpdateCommandEx::INITIALIZING)), KisStrokeJobData::BARRIER, KisStrokeJobData::NORMAL);
            m_updatesDisabled = true;
            m_updatesFacade->disableDirtyRequests();
        });

        Q_FOREACH (KisNodeSP node, m_processedNodes) {
            mutatedJobs << new TransformData(TransformData::PAINT_DEVICE,
                                             args,
                                             node);
        }
        mutatedJobs << new TransformData(TransformData::SELECTION,
                                         args,
                                         m_rootNodes[0]);

        KritaUtils::addJobBarrier(mutatedJobs, restoreTemporaryHiddenNodes);

        KritaUtils::addJobBarrier(mutatedJobs, [this] () {
            m_updatesFacade->enableDirtyRequests();
            m_updatesDisabled = false;

            m_updateData->compress();
            runAndSaveCommand(toQShared(new KisUpdateCommandEx(m_updateData, m_updatesFacade, KisUpdateCommandEx::FINALIZING)), KisStrokeJobData::BARRIER, KisStrokeJobData::NORMAL);
        });
    } else {
        KritaUtils::addJobBarrier(mutatedJobs, restoreTemporaryHiddenNodes);
    }

    KritaUtils::addJobBarrier(mutatedJobs, [this, applyTransform]() {
        Q_FOREACH (KisSelectionSP selection, m_deactivatedSelections) {
            selection->setVisible(true);
        }

        Q_FOREACH(KisSelectionMaskSP deactivatedOverlaySelectionMask, m_deactivatedOverlaySelectionMasks) {
            deactivatedOverlaySelectionMask->selection()->setVisible(true);
            deactivatedOverlaySelectionMask->setDirty();
        }

        if (applyTransform) {
            KisStrokeStrategyUndoCommandBased::finishStrokeCallback();
        } else {
            KisStrokeStrategyUndoCommandBased::cancelStrokeCallback();
        }
    });

    for (auto it = mutatedJobs.begin(); it != mutatedJobs.end(); ++it) {
        (*it)->setCancellable(false);
    }

    addMutatedJobs(mutatedJobs);
}

void TransformStrokeStrategy::finishStrokeCallback()
{
    if (!m_savedTransformArgs || m_savedTransformArgs->isUnchanging()) {
        cancelStrokeCallback();
        return;
    }

    finishStrokeImpl(true, *m_savedTransformArgs);
}

void TransformStrokeStrategy::cancelStrokeCallback()
{
    if (m_updatesDisabled) {
        m_updatesFacade->enableDirtyRequests();
    }

    finishStrokeImpl(!m_initialTransformArgs.isUnchanging(), m_initialTransformArgs);
}
