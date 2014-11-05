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

#include <KoProgressUpdater.h>
#include "kis_node_progress_proxy.h"

#include <klocale.h>
#include <kis_node.h>
#include <kis_external_layer_iface.h>
#include <kis_transaction.h>
#include <kis_painter.h>
#include <kis_transform_worker.h>
#include <kis_transform_mask.h>
#include "kis_transform_mask_adapter.h"
#include "kis_transform_utils.h"



class ModifyTransformMaskCommand : public KUndo2Command {
public:
    ModifyTransformMaskCommand(KisTransformMaskSP mask, KisTransformMaskParamsInterfaceSP params)
        : m_mask(mask),
          m_params(params),
          m_oldParams(m_mask->transformParams())
        {
        }

    void redo() {
        m_mask->setTransformParams(m_params);

        /**
         * NOTE: this code "duplicates" the functionality provided
         * by KisRecalculateTransformMaskJob, but there is not much
         * reason for starting a separate stroke when a transformation
         * has happened
         */

        m_mask->recaclulateStaticImage();
        updateMask();
    }

    void undo() {
        m_mask->setTransformParams(m_oldParams);

        m_mask->recaclulateStaticImage();
        updateMask();
    }

private:
    void updateMask() {
        QRect updateRect = m_mask->extent();

        KisNodeSP parent = m_mask->parent();
        if (parent && parent->original()) {
            updateRect |= parent->original()->defaultBounds()->bounds();
        }

        m_mask->setDirty(updateRect);
    }

private:
    KisTransformMaskSP m_mask;
    KisTransformMaskParamsInterfaceSP m_params;
    KisTransformMaskParamsInterfaceSP m_oldParams;
};


TransformStrokeStrategy::TransformStrokeStrategy(KisNodeSP rootNode,
                                                 KisSelectionSP selection,
                                                 KisPostExecutionUndoAdapter *undoAdapter)
    : KisStrokeStrategyUndoCommandBased(kundo2_i18n("Transform"), false, undoAdapter),
      m_selection(selection)
{
    if (rootNode->childCount() || !rootNode->paintDevice()) {
        KisPaintDeviceSP device;

        if (dynamic_cast<KisTransformMask*>(rootNode.data())) {
            KisNodeSP parentNode = rootNode->parent();
            device = parentNode->paintDevice();

            if (!device) {
                device = parentNode->original();
            }

        } else {
            device = rootNode->projection();
        }

        m_previewDevice = createDeviceCache(device);

    } else {
        m_previewDevice = createDeviceCache(rootNode->paintDevice());
        putDeviceCache(rootNode->paintDevice(), m_previewDevice);
    }

    Q_ASSERT(m_previewDevice);
}

TransformStrokeStrategy::~TransformStrokeStrategy()
{
}

KisPaintDeviceSP TransformStrokeStrategy::previewDevice() const
{
    return m_previewDevice;
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
        qWarning() << "WARNING: Transform Stroke: the device is absent in cache!";
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

    if(td) {
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
            } if (KisExternalLayer *extLayer =
                  dynamic_cast<KisExternalLayer*>(td->node.data())) {

                if (td->config.mode() == ToolTransformArgs::FREE_TRANSFORM ||
                    td->config.mode() == ToolTransformArgs::PERSPECTIVE_4POINT) {

                    if (td->config.aX() || td->config.aY()) {
                        qWarning() << "Perspective transform of an external layer is not supported:" << extLayer->name();
                    }

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
                                      new ModifyTransformMaskCommand(transformMask,
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
        } else if (KisTransformMask *transformMask =
                   dynamic_cast<KisTransformMask*>(csd->node.data())) {

            runAndSaveCommand(KUndo2CommandSP(
                                  new ModifyTransformMaskCommand(transformMask,
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
