/*
 *  Copyright (c) 2020 Boudewijn Rempt <boud@valdyas.org>
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
#include "KisTrimProcessingVisitor.h"


#include "kis_external_layer_iface.h"

#include "kis_paint_device.h"
#include "kis_image.h"
#include "kis_transaction.h"
#include "kis_undo_adapter.h"
#include "kis_transform_mask.h"
#include "lazybrush/kis_colorize_mask.h"

#include <klocalizedstring.h>

KisTrimProcessingVisitor::KisTrimProcessingVisitor(const QRect &imageBounds)
    : m_imageBounds(imageBounds)
{
}

void KisTrimProcessingVisitor::visitExternalLayer(KisExternalLayer *layer, KisUndoAdapter *undoAdapter)
{
    QRect rc = layer->exactBounds();
    rc = rc.intersected(m_imageBounds);

    KUndo2Command* command = layer->crop(rc);
    undoAdapter->addCommand(command);
}

void KisTrimProcessingVisitor::trimPaintDeviceImpl(KisPaintDeviceSP device, KisUndoAdapter *undoAdapter)
{
    KisTransaction transaction(kundo2_noi18n("trim"), device);

    QRect rc = device->exactBounds();
    rc = rc.intersected(m_imageBounds);

    device->crop(rc);
    transaction.commit(undoAdapter);
}

void KisTrimProcessingVisitor::visitNodeWithPaintDevice(KisNode *node, KisUndoAdapter *undoAdapter)
{
    trimPaintDeviceImpl(node->paintDevice(), undoAdapter);
}

void KisTrimProcessingVisitor::visit(KisTransformMask *node, KisUndoAdapter *undoAdapter)
{
    KisSimpleProcessingVisitor::visit(node, undoAdapter);
}

void KisTrimProcessingVisitor::visitColorizeMask(KisColorizeMask *node, KisUndoAdapter *undoAdapter)
{
    QVector<KisPaintDeviceSP> devices = node->allPaintDevices();

    Q_FOREACH (KisPaintDeviceSP device, devices) {
        trimPaintDeviceImpl(device, undoAdapter);
    }
}
