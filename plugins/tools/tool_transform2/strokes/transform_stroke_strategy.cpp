/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

TransformStrokeStrategy::TransformStrokeStrategy(KisNodeSP rootNode,
                                                 KisNodeList processedNodes,
                                                 KisSelectionSP selection,
                                                 KisStrokeUndoFacade *undoFacade)
    : KisStrokeStrategyUndoCommandBased(kundo2_i18n("Transform"), false, undoFacade),
      m_selection(selection)
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(!selection || !dynamic_cast<KisTransformMask*>(rootNode.data()));

    m_savedRootNode = rootNode;
    m_savedProcessedNodes = processedNodes;
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

    if (ppd) {
        KisNodeSP rootNode = m_savedRootNode;
        KisNodeList processedNodes = m_savedProcessedNodes;
        KisPaintDeviceSP previewDevice;


        if (rootNode->childCount() || !rootNode->paintDevice()) {
            if (KisTransformMask* tmask =
                dynamic_cast<KisTransformMask*>(rootNode.data())) {
                previewDevice = createDeviceCache(tmask->buildPreviewDevice());

                KIS_SAFE_ASSERT_RECOVER(!m_selection) {
                    m_selection = 0;
                }

            } else if (KisGroupLayer *group = dynamic_cast<KisGroupLayer*>(rootNode.data())) {
                const QRect bounds = group->image()->bounds();

                KisImageSP clonedImage = new KisImage(0,
                                                      bounds.width(),
                                                      bounds.height(),
                                                      group->colorSpace(),
                                                      "transformed_image");

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

                clonedImage->refreshGraph();

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


        emit sigPreviewDeviceReady(previewDevice);
    } else if(td) {
        m_savedTransformArgs = td->config;

        if (td->destination == TransformData::PAINT_DEVICE) {
            QRect oldExtent = td->node->extent();
            KisPaintDeviceSP device = td->node->paintDevice();

            if (device && !checkBelongsToSelection(device)) {
                KisPaintDeviceSP cachedPortion = getDeviceCache(device);
                Q_ASSERT(cachedPortion);

                KisTransaction transaction(device);

                KisProcessingVisitor::ProgressHelper helper(td->node);
                transformAndMergeDevice(td->config, cachedPortion,
                                        device, &helper);

                runAndSaveCommand(KUndo2CommandSP(transaction.endAndTake()),
                                  KisStrokeJobData::CONCURRENT,
                                  KisStrokeJobData::NORMAL);

                td->node->setDirty(oldExtent | td->node->extent());
            } else if (KisExternalLayer *extLayer =
                  dynamic_cast<KisExternalLayer*>(td->node.data())) {

                if (td->config.mode() == ToolTransformArgs::FREE_TRANSFORM ||
                    (td->config.mode() == ToolTransformArgs::PERSPECTIVE_4POINT &&
                     extLayer->supportsPerspectiveTransform())) {

                    QVector3D transformedCenter;
                    KisTransformWorker w = KisTransformUtils::createTransformWorker(td->config, 0, 0, &transformedCenter);
                    QTransform t = w.transform();

                    runAndSaveCommand(KUndo2CommandSP(extLayer->transform(t)),
                                      KisStrokeJobData::CONCURRENT,
                                      KisStrokeJobData::NORMAL);
                }

            } else if (KisTransformMask *transformMask =
                       dynamic_cast<KisTransformMask*>(td->node.data())) {

                runAndSaveCommand(KUndo2CommandSP(
                                      new KisModifyTransformMaskCommand(transformMask,
                                                                     KisTransformMaskParamsInterfaceSP(
                                                                         new KisTransformMaskAdapter(td->config)))),
                                  KisStrokeJobData::CONCURRENT,
                                  KisStrokeJobData::NORMAL);
            }
        } else if (m_selection) {

            /**
             * We use usual transaction here, because we cannot calsulate
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
                    mask->setDirty();
                }
            }
        } else if (KisExternalLayer *externalLayer = dynamic_cast<KisExternalLayer*>(csd->node.data())) {
            externalLayer->projectionLeaf()->setTemporaryHiddenFromRendering(true);
            externalLayer->setDirty();
            m_hiddenProjectionLeaves.append(csd->node);
        } else if (KisTransformMask *transformMask =
                   dynamic_cast<KisTransformMask*>(csd->node.data())) {

            runAndSaveCommand(KUndo2CommandSP(
                                  new KisModifyTransformMaskCommand(transformMask,
                                                                 KisTransformMaskParamsInterfaceSP(
                                                                     new KisDumbTransformMaskParams(true)))),
                                  KisStrokeJobData::SEQUENTIAL,
                                  KisStrokeJobData::NORMAL);
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
        QRect oldExtent = device->extent();
        device->clear();
        device->setDirty(oldExtent);
    }
    runAndSaveCommand(KUndo2CommandSP(transaction.endAndTake()),
                      KisStrokeJobData::SEQUENTIAL,
                      KisStrokeJobData::NORMAL);
}

void TransformStrokeStrategy::transformAndMergeDevice(const ToolTransformArgs &config,
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
};

void TransformStrokeStrategy::postProcessToplevelCommand(KUndo2Command *command)
{
    TransformExtraData *data = new TransformExtraData();
    data->savedTransformArgs = m_savedTransformArgs;
    data->rootNode = m_savedRootNode;
    data->transformedNodes = m_savedProcessedNodes;

    command->setExtraData(data);
}

bool TransformStrokeStrategy::fetchArgsFromCommand(const KUndo2Command *command, ToolTransformArgs *args, KisNodeSP *rootNode, KisNodeList *transformedNodes)
{
    const TransformExtraData *data = dynamic_cast<const TransformExtraData*>(command->extraData());

    if (data) {
        *args = data->savedTransformArgs;
        *rootNode = data->rootNode;
        *transformedNodes = data->transformedNodes;
    }

    return bool(data);
}

void TransformStrokeStrategy::initStrokeCallback()
{
    KisStrokeStrategyUndoCommandBased::initStrokeCallback();

    if (m_selection) {
        m_selection->setVisible(false);
        m_deactivatedSelections.append(m_selection);
    }
}

void TransformStrokeStrategy::finishStrokeCallback()
{
    Q_FOREACH (KisSelectionSP selection, m_deactivatedSelections) {
        selection->setVisible(true);
    }

    Q_FOREACH (KisNodeSP node, m_hiddenProjectionLeaves) {
        node->projectionLeaf()->setTemporaryHiddenFromRendering(false);
    }

    KisStrokeStrategyUndoCommandBased::finishStrokeCallback();
}

void TransformStrokeStrategy::cancelStrokeCallback()
{
    KisStrokeStrategyUndoCommandBased::cancelStrokeCallback();

    Q_FOREACH (KisSelectionSP selection, m_deactivatedSelections) {
        selection->setVisible(true);
    }

    Q_FOREACH (KisNodeSP node, m_hiddenProjectionLeaves) {
        node->projectionLeaf()->setTemporaryHiddenFromRendering(false);
    }
}
