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

#include "kis_crop_processing_visitor.h"

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


KisCropProcessingVisitor::KisCropProcessingVisitor(const QRect &rect, bool cropLayers, bool moveLayers)
    : m_rect(rect),
      m_cropLayers(cropLayers),
      m_moveLayers(moveLayers)

{
}

void KisCropProcessingVisitor::visit(KisNode *node, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(node);
    Q_UNUSED(undoAdapter);
}

void KisCropProcessingVisitor::visit(KisCloneLayer *layer, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(layer);
    Q_UNUSED(undoAdapter);
}

void KisCropProcessingVisitor::visit(KisExternalLayer *layer, KisUndoAdapter *undoAdapter)
{
    KUndo2Command* command = layer->crop(m_rect);
    undoAdapter->addCommand(command);
}

void KisCropProcessingVisitor::visit(KisPaintLayer *layer, KisUndoAdapter *undoAdapter)
{
    cropNode(layer, undoAdapter);
}

void KisCropProcessingVisitor::visit(KisGroupLayer *layer, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(undoAdapter);

    layer->resetCache();
}

void KisCropProcessingVisitor::visit(KisAdjustmentLayer *layer, KisUndoAdapter *undoAdapter)
{
    cropNode(layer, undoAdapter);
    layer->resetCache();
}

void KisCropProcessingVisitor::visit(KisGeneratorLayer *layer, KisUndoAdapter *undoAdapter)
{
    cropNode(layer, undoAdapter);
    layer->resetCache();
}

void KisCropProcessingVisitor::visit(KisFilterMask *mask, KisUndoAdapter *undoAdapter)
{
    cropNode(mask, undoAdapter);
}

void KisCropProcessingVisitor::visit(KisTransparencyMask *mask, KisUndoAdapter *undoAdapter)
{
    cropNode(mask, undoAdapter);
}

void KisCropProcessingVisitor::visit(KisSelectionMask *mask, KisUndoAdapter *undoAdapter)
{
    cropNode(mask, undoAdapter);
}

void KisCropProcessingVisitor::cropNode(KisNode *node, KisUndoAdapter *undoAdapter)
{
    /**
     * TODO: implement actual robust cropping of the selections,
     * including the cropping of vector (!) selection.
     */

    if (m_cropLayers) {
        KisTransaction transaction(i18n("Crop"), node->paintDevice());
        node->paintDevice()->crop(m_rect);
        transaction.commit(undoAdapter);
    }

    if (m_moveLayers) {
        QPoint oldPos(node->x(), node->y());
        QPoint newPos(node->x() - m_rect.x(), node->y() - m_rect.y());
        KUndo2Command *command = new KisNodeMoveCommand2(node, oldPos, newPos, undoAdapter);
        undoAdapter->addCommand(command);
    }
}
