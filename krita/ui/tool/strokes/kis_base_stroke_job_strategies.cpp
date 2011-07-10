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

#include "kis_base_stroke_job_strategies.h"

#include <KoColorSpace.h>
#include <KoCompositeOp.h>
#include "kis_painter.h"
#include "kis_paint_device.h"
#include "kis_paint_layer.h"
#include "kis_selection.h"


InitStrokeJobStrategy::InitStrokeJobStrategy(bool isExclusive)
    : KisDabProcessingStrategy(true, isExclusive)
{
}

void InitStrokeJobStrategy::processDab(DabProcessingData *data)
{
    Data *d = dynamic_cast<Data*>(data);

    KisResourcesSnapshotSP resources = d->resources;
    KisPainter *painter = d->painter;

    KisNodeSP node = resources->currentNode();
    KisPaintDeviceSP paintDevice = node->paintDevice();
    KisPaintDeviceSP targetDevice = paintDevice;
    bool hasIndirectPainting = d->needsIndirectPainting;

    if (hasIndirectPainting) {
        KisIndirectPaintingSupport *indirect =
            dynamic_cast<KisIndirectPaintingSupport*>(node.data());

        if (indirect) {
            targetDevice = new KisPaintDevice(node, paintDevice->colorSpace());
            indirect->setTemporaryTarget(targetDevice);
            indirect->setTemporaryCompositeOp(resources->compositeOp());
            indirect->setTemporaryOpacity(resources->opacity());

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

    painter->begin(targetDevice, selection);
    resources->setupPainter(painter);

    if(hasIndirectPainting) {
        painter->setCompositeOp(paintDevice->colorSpace()->compositeOp(COMPOSITE_ALPHA_DARKEN));
        painter->setOpacity(OPACITY_OPAQUE_U8);
        painter->setChannelFlags(QBitArray());
    }

    painter->beginTransaction(d->transactionText);
}


FinishStrokeJobStrategy::FinishStrokeJobStrategy(bool isExclusive)
    : KisDabProcessingStrategy(true, isExclusive)
{
}

void FinishStrokeJobStrategy::processDab(DabProcessingData *data)
{
    Data *d = dynamic_cast<Data*>(data);

    KisNodeSP node = d->resources->currentNode();
    KisLayerSP layer = dynamic_cast<KisLayer*>(node.data());
    KisIndirectPaintingSupport *indirect =
        dynamic_cast<KisIndirectPaintingSupport*>(node.data());

    if(layer && indirect && indirect->hasTemporaryTarget()) {
        QString transactionText = d->painter->transactionText();
        d->painter->deleteTransaction();
        indirect->mergeToLayer(layer, transactionText);
    }
    else {
        d->painter->endTransaction(d->resources->image()->undoAdapter());
    }
    delete d->painter;
}


CancelStrokeJobStrategy::CancelStrokeJobStrategy(bool isExclusive)
    : KisDabProcessingStrategy(true, isExclusive)
{
}

void CancelStrokeJobStrategy::processDab(DabProcessingData *data)
{
    Data *d = dynamic_cast<Data*>(data);

    d->painter->revertTransaction();
    delete d->painter;

    KisNodeSP node = d->resources->currentNode();
    KisIndirectPaintingSupport *indirect =
        dynamic_cast<KisIndirectPaintingSupport*>(node.data());

    if(indirect && indirect->hasTemporaryTarget()) {
        indirect->setTemporaryTarget(0);
    }
}
