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
#include "kis_transaction.h"
#include "kis_image.h"
#include "kis_distance_information.h"


KisPainterBasedStrokeStrategy::PainterInfo::PainterInfo(KisPainter *_painter, KisDistanceInformation *_dragDistance)
    : painter(_painter), dragDistance(_dragDistance)
{
}

KisPainterBasedStrokeStrategy::PainterInfo::~PainterInfo()
{
    delete(painter);
    delete(dragDistance);
}

KisPainterBasedStrokeStrategy::KisPainterBasedStrokeStrategy(const QString &id,
                                                             const QString &name,
                                                             KisResourcesSnapshotSP resources,
                                                             QVector<PainterInfo*> painterInfos)
    : KisSimpleStrokeStrategy(id, name),
      m_resources(resources),
      m_painterInfos(painterInfos),
      m_transaction(0)
{
    init();
}

KisPainterBasedStrokeStrategy::KisPainterBasedStrokeStrategy(const QString &id,
                                                             const QString &name,
                                                             KisResourcesSnapshotSP resources,
                                                             PainterInfo *painterInfo)
    : KisSimpleStrokeStrategy(id, name),
      m_resources(resources),
      m_painterInfos(QVector<PainterInfo*>() <<  painterInfo),
      m_transaction(0)
{
    init();
}

void KisPainterBasedStrokeStrategy::init()
{
    enableJob(KisSimpleStrokeStrategy::JOB_INIT);
    enableJob(KisSimpleStrokeStrategy::JOB_FINISH);
    enableJob(KisSimpleStrokeStrategy::JOB_CANCEL);
}

void KisPainterBasedStrokeStrategy::initPainters(KisPaintDeviceSP targetDevice,
                                                 KisSelectionSP selection,
                                                 bool hasIndirectPainting)
{
    foreach(PainterInfo *info, m_painterInfos) {
        KisPainter *painter = info->painter;

        painter->begin(targetDevice, selection);
        m_resources->setupPainter(painter);

        if(hasIndirectPainting) {
            painter->setCompositeOp(targetDevice->colorSpace()->compositeOp(COMPOSITE_ALPHA_DARKEN));
            painter->setOpacity(OPACITY_OPAQUE_U8);
            painter->setChannelFlags(QBitArray());
        }
    }
}

void KisPainterBasedStrokeStrategy::deletePainters()
{
    foreach(PainterInfo *info, m_painterInfos) {
        delete info;
    }

    m_painterInfos.clear();
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
    } else {
        selection = m_resources->image()->globalSelection();
    }

    m_transaction = new KisTransaction(name(), targetDevice);

    initPainters(targetDevice, selection, hasIndirectPainting);
}

void KisPainterBasedStrokeStrategy::finishStrokeCallback()
{
    KisNodeSP node = m_resources->currentNode();
    KisLayerSP layer = dynamic_cast<KisLayer*>(node.data());
    KisIndirectPaintingSupport *indirect =
        dynamic_cast<KisIndirectPaintingSupport*>(node.data());

    if(layer && indirect && indirect->hasTemporaryTarget()) {
        QString transactionText = m_transaction->text();
        m_transaction->end();

        indirect->mergeToLayer(layer,
                               m_resources->postExecutionUndoAdapter(),
                               transactionText);
    }
    else {
        m_transaction->commit(m_resources->postExecutionUndoAdapter());
    }
    delete m_transaction;
    deletePainters();
}

void KisPainterBasedStrokeStrategy::cancelStrokeCallback()
{
    m_transaction->revert();
    delete m_transaction;
    deletePainters();

    KisNodeSP node = m_resources->currentNode();
    KisIndirectPaintingSupport *indirect =
        dynamic_cast<KisIndirectPaintingSupport*>(node.data());

    if(indirect && indirect->hasTemporaryTarget()) {
        indirect->setTemporaryTarget(0);
    }
}
