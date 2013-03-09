/*
 *  Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_offset_processing_visitor.h"

#include <klocale.h>

#include "commands_new/kis_node_move_command2.h"

#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include "kis_adjustment_layer.h"
#include "generator/kis_generator_layer.h"

#include "kis_transparency_mask.h"
#include "kis_filter_mask.h"
#include "kis_selection_mask.h"

#include "kis_external_layer_iface.h"

#include "kis_paint_device.h"
#include "kis_transaction.h"
#include "kis_undo_adapter.h"
#include <kis_transform_worker.h>


KisOffsetProcessingVisitor::KisOffsetProcessingVisitor(const QPoint &offsetPoint, const QRect &rect)
    :   m_offset(offsetPoint),
        m_wrapRect(rect)

{
}

void KisOffsetProcessingVisitor::visit(KisNode *node, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(node);
    Q_UNUSED(undoAdapter);
}

void KisOffsetProcessingVisitor::visit(KisCloneLayer *layer, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(layer);
    Q_UNUSED(undoAdapter);
}

void KisOffsetProcessingVisitor::visit(KisExternalLayer *layer, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(layer);
    Q_UNUSED(undoAdapter);
}

void KisOffsetProcessingVisitor::visit(KisPaintLayer *layer, KisUndoAdapter *undoAdapter)
{
    offsetNode(layer, undoAdapter);
}

void KisOffsetProcessingVisitor::visit(KisGroupLayer *layer, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(undoAdapter);

    layer->resetCache();
}

void KisOffsetProcessingVisitor::visit(KisAdjustmentLayer *layer, KisUndoAdapter *undoAdapter)
{
    offsetNode(layer, undoAdapter);
    layer->resetCache();
}

void KisOffsetProcessingVisitor::visit(KisGeneratorLayer *layer, KisUndoAdapter *undoAdapter)
{
    offsetNode(layer, undoAdapter);
    layer->resetCache();
}

void KisOffsetProcessingVisitor::visit(KisFilterMask *mask, KisUndoAdapter *undoAdapter)
{
    offsetNode(mask, undoAdapter);
}

void KisOffsetProcessingVisitor::visit(KisTransparencyMask *mask, KisUndoAdapter *undoAdapter)
{
    offsetNode(mask, undoAdapter);
}

void KisOffsetProcessingVisitor::visit(KisSelectionMask *mask, KisUndoAdapter *undoAdapter)
{
    offsetNode(mask, undoAdapter);
}

void KisOffsetProcessingVisitor::offsetNode(KisNode *node, KisUndoAdapter *undoAdapter)
{
    KisPaintDeviceSP device = node->paintDevice();
    KisTransaction transaction(i18n("Offset"), device);
    KisTransformWorker::offset(device, m_offset, m_wrapRect.size());
    transaction.commit(undoAdapter);
}

