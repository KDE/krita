/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_painter_based_stroke_strategy.h"

#include <KoColorSpace.h>
#include <KoCompositeOp.h>
#include "kis_painter.h"
#include "kis_paint_device.h"
#include "kis_paint_layer.h"
#include "kis_selection.h"


KisPainterBasedStrokeStrategy::KisPainterBasedStrokeStrategy(const QString &id,
                                                             const QString &name,
                                                             KisResourcesSnapshotSP resources,
                                                             KisPainter *painter)
    : KisSimpleStrokeStrategy(id, name),
      m_resources(resources),
      m_painter(painter)
{
    enableJob(KisSimpleStrokeStrategy::JOB_INIT);
    enableJob(KisSimpleStrokeStrategy::JOB_FINISH);
    enableJob(KisSimpleStrokeStrategy::JOB_CANCEL);
}

void KisPainterBasedStrokeStrategy::initStrokeCallback()
{
    KisNodeSP node = m_resources->currentNode();
    KisPaintDeviceSP paintDevice = node->paintDevice();
    KisPaintDeviceSP targetDevice = paintDevice;
    bool hasIndirectPainting = needsIndirectPainting();

    if (hasIndirectPainting) {
        KisIndirectPaintingSupport *indirect =
            dynamic_cast<KisIndirectPaintingSupport*>(node.data());

        if (indirect) {
            targetDevice = new KisPaintDevice(node, paintDevice->colorSpace());
            indirect->setTemporaryTarget(targetDevice);
            indirect->setTemporaryCompositeOp(m_resources->compositeOp());
            indirect->setTemporaryOpacity(m_resources->opacity());

            KisPaintLayer *paintLayer = dynamic_cast<KisPaintLayer*>(node.data());
            if(paintLayer) {
                indirect->setTemporaryChannelFlags(paintLayer->channelLockFlags());
            }
        }
        else {
            hasIndirectPainting = false;
        }
    }

    KisSelectionSP selection;
    KisLayerSP layer = dynamic_cast<KisLayer*>(node.data());
    if(layer) {
        selection = layer->selection();
    }

    m_painter->begin(targetDevice, selection);
    m_resources->setupPainter(m_painter);

    if(hasIndirectPainting) {
        m_painter->setCompositeOp(paintDevice->colorSpace()->compositeOp(COMPOSITE_ALPHA_DARKEN));
        m_painter->setOpacity(OPACITY_OPAQUE_U8);
        m_painter->setChannelFlags(QBitArray());
    }

    m_painter->beginTransaction(name());
}

void KisPainterBasedStrokeStrategy::finishStrokeCallback()
{
    KisNodeSP node = m_resources->currentNode();
    KisLayerSP layer = dynamic_cast<KisLayer*>(node.data());
    KisIndirectPaintingSupport *indirect =
        dynamic_cast<KisIndirectPaintingSupport*>(node.data());

    if(layer && indirect && indirect->hasTemporaryTarget()) {
        QString transactionText = m_painter->transactionText();
        m_painter->deleteTransaction();

        indirect->mergeToLayer(layer,
                               m_resources->postExecutionUndoAdapter(),
                               transactionText);
    }
    else {
        m_painter->endTransaction(
            m_resources->postExecutionUndoAdapter());
    }
    delete m_painter;
}

void KisPainterBasedStrokeStrategy::cancelStrokeCallback()
{
    m_painter->revertTransaction();
    delete m_painter;

    KisNodeSP node = m_resources->currentNode();
    KisIndirectPaintingSupport *indirect =
        dynamic_cast<KisIndirectPaintingSupport*>(node.data());

    if(indirect && indirect->hasTemporaryTarget()) {
        indirect->setTemporaryTarget(0);
    }
}
