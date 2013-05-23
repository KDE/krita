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
#include <kis_transaction.h>
#include <kis_painter.h>
#include <kis_transform_worker.h>
#include <kis_perspectivetransform_worker.h>
#include <kis_warptransform_worker.h>


TransformStrokeStrategy::TransformStrokeStrategy(KisNodeSP rootNode,
                                                 KisSelectionSP selection,
                                                 KisPostExecutionUndoAdapter *undoAdapter,
                                                 KisUndoAdapter *legacyUndoAdapter)
    : KisStrokeStrategyUndoCommandBased(i18n("Transform Stroke"), false, undoAdapter),
      m_selection(selection),
      m_legacyUndoAdapter(legacyUndoAdapter)
{
    if (rootNode->childCount() || !rootNode->paintDevice()) {
        m_previewDevice = createDeviceCache(rootNode->projection());
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

                KisTransaction transaction("Transform Device", device);

                KisProcessingVisitor::ProgressHelper helper(td->node);
                transformAndMergeDevice(td->config, cachedPortion,
                                        device, &helper);

                runAndSaveCommand(KUndo2CommandSP(transaction.endAndTake()),
                                  KisStrokeJobData::CONCURRENT,
                                  KisStrokeJobData::NORMAL);

                td->node->setDirty(oldExtent | td->node->extent());
            }
        } else if (m_selection) {
            // FIXME: do it undoable
            m_selection->flatten();
            Q_ASSERT(m_selection->pixelSelection());

            KisSelectionTransaction transaction("Transform Selection", m_legacyUndoAdapter, m_selection);

            KisProcessingVisitor::ProgressHelper helper(td->node);
            transformDevice(td->config,
                            m_selection->pixelSelection(),
                            &helper);

            runAndSaveCommand(KUndo2CommandSP(transaction.endAndTake()),
                              KisStrokeJobData::CONCURRENT,
                              KisStrokeJobData::NORMAL);

            m_legacyUndoAdapter->emitSelectionChanged();
        }
    } else if (csd) {
        KisPaintDeviceSP device = csd->node->paintDevice();
        if (device && !checkBelongsToSelection(device)) {
            if (!haveDeviceInCache(device)) {
                putDeviceCache(device, createDeviceCache(device));
            }
            clearSelection(device);
        }
    } else {
        KisStrokeStrategyUndoCommandBased::doStrokeCallback(data);
    }
}

void TransformStrokeStrategy::clearSelection(KisPaintDeviceSP device)
{
    KisTransaction transaction("Clear Selection", device);
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

    transformDevice(config, src, helper);
    if (src != dst) {
        QRect mergeRect = src->extent();
        KisPainter painter(dst);
        painter.setProgress(mergeUpdater);
        painter.bitBlt(mergeRect.topLeft(), src, mergeRect);
        painter.end();
    }
}

void TransformStrokeStrategy::transformDevice(const ToolTransformArgs &config,
                                              KisPaintDeviceSP device,
                                              KisProcessingVisitor::ProgressHelper *helper)
{
    if (config.mode() == ToolTransformArgs::WARP) {
        KoUpdaterPtr updater = helper->updater();

        KisWarpTransformWorker worker(config.warpType(),
                                      device,
                                      config.origPoints(),
                                      config.transfPoints(),
                                      config.alpha(),
                                      updater);
        worker.run();
    } else {
        QVector3D transformedCenter;

        {
            KisTransformWorker t(0,
                                 config.scaleX(), config.scaleY(),
                                 config.shearX(), config.shearY(),
                                 config.originalCenter().x(),
                                 config.originalCenter().y(),
                                 config.aZ(),
                                 0, // set X and Y translation
                                 0, // to null for calculation
                                 0,
                                 config.filter());

            transformedCenter = QVector3D(t.transform().map(config.originalCenter()));
        }

        QPointF translation = config.transformedCenter() - transformedCenter.toPointF();

        KoUpdaterPtr updater1 = helper->updater();
        KoUpdaterPtr updater2 = helper->updater();

        KisTransformWorker transformWorker(device,
                                           config.scaleX(), config.scaleY(),
                                           config.shearX(), config.shearY(),
                                           config.originalCenter().x(),
                                           config.originalCenter().y(),
                                           config.aZ(),
                                           (int)(translation.x()),
                                           (int)(translation.y()),
                                           updater1,
                                           config.filter());
        transformWorker.run();

        KisPerspectiveTransformWorker perspectiveWorker(device,
                                                        config.transformedCenter(),
                                                        config.aX(),
                                                        config.aY(),
                                                        config.cameraPos().z(),
                                                        updater2);
        perspectiveWorker.run();
    }
}
