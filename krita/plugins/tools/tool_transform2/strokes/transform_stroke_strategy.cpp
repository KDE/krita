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


TransformStrokeStrategy::TransformStrokeStrategy(KisNodeSP node,
                                                 KisSelectionSP selection,
                                                 KisPaintDeviceSP selectedPortionCache,
                                                 KisPostExecutionUndoAdapter *undoAdapter,
                                                 KisUndoAdapter *legacyUndoAdapter)
    : KisStrokeStrategyUndoCommandBased(i18n("Transform Stroke"), false, undoAdapter),
      m_node(node),
      m_selection(selection),
      m_selectedPortionCache(selectedPortionCache),
      m_legacyUndoAdapter(legacyUndoAdapter),
      m_progressUpdater(0)
{
    Q_ASSERT(m_node);
}

TransformStrokeStrategy::~TransformStrokeStrategy()
{
    if (m_progressUpdater) {
        m_progressUpdater->deleteLater();
    }
}

void TransformStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    TransformData *td = dynamic_cast<TransformData*>(data);
    ClearSelectionData *csd = dynamic_cast<ClearSelectionData*>(data);

    if(td) {
        if (td->destination == TransformData::PAINT_DEVICE) {
            QRect oldExtent = m_node->extent();

            KisPaintDeviceSP device = m_node->paintDevice();

            KisTransaction transaction("Transform Device", device);

            transformAndMergeDevice(td->config, m_selectedPortionCache,
                                    device);

            runAndSaveCommand(KUndo2CommandSP(transaction.endAndTake()),
                              KisStrokeJobData::CONCURRENT,
                              KisStrokeJobData::NORMAL);

            m_node->setDirty(oldExtent | m_node->extent());
        } else if (m_selection) {
            // FIXME: do it undoable
            m_selection->flatten();
            Q_ASSERT(m_selection->pixelSelection());

            KisSelectionTransaction transaction("Transform Selection", m_legacyUndoAdapter, m_selection);

            transformDevice(td->config,
                            m_selection->pixelSelection());

            runAndSaveCommand(KUndo2CommandSP(transaction.endAndTake()),
                              KisStrokeJobData::CONCURRENT,
                              KisStrokeJobData::NORMAL);

            m_legacyUndoAdapter->emitSelectionChanged();
        }
    } else if (csd) {
        clearSelection();
    } else {
        KisStrokeStrategyUndoCommandBased::doStrokeCallback(data);
    }
}

void TransformStrokeStrategy::clearSelection()
{
    KisPaintDeviceSP device = m_node->paintDevice();
    Q_ASSERT(device);

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
                                                      KisPaintDeviceSP dst)
{
    KoUpdaterPtr mergeUpdater = src != dst ? fetchUpdater() : 0;

    transformDevice(config, src);
    if (src != dst) {
        QRect mergeRect = src->extent();
        KisPainter painter(dst);
        painter.setProgress(mergeUpdater);
        painter.bitBlt(mergeRect.topLeft(), src, mergeRect);
        painter.end();
    }
}

KoUpdaterPtr TransformStrokeStrategy::fetchUpdater()
{
    QMutexLocker l(&m_progressMutex);

    if (!m_progressUpdater) {
        KisNodeProgressProxy *progressProxy = m_node->nodeProgressProxy();
        m_progressUpdater = new KoProgressUpdater(progressProxy);
        m_progressUpdater->moveToThread(m_node->thread());
    }

    return m_progressUpdater->startSubtask();
}

void TransformStrokeStrategy::transformDevice(const ToolTransformArgs &config,
                                              KisPaintDeviceSP device)
{
    if (config.mode() == ToolTransformArgs::WARP) {
        KoUpdaterPtr updater = fetchUpdater();

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

        KoUpdaterPtr updater1 = fetchUpdater();
        KoUpdaterPtr updater2 = fetchUpdater();

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
