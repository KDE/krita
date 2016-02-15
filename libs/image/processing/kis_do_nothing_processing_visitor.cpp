/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_do_nothing_processing_visitor.h"

#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include "kis_adjustment_layer.h"
#include "generator/kis_generator_layer.h"

#include "kis_transparency_mask.h"
#include "kis_filter_mask.h"
#include "kis_selection_mask.h"

KisDoNothingProcessingVisitor::~KisDoNothingProcessingVisitor()
{
}

void KisDoNothingProcessingVisitor::visit(KisNode *node, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(node);
    Q_UNUSED(undoAdapter);
}

void KisDoNothingProcessingVisitor::visit(KisCloneLayer *layer, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(layer);
    Q_UNUSED(undoAdapter);
}

void KisDoNothingProcessingVisitor::visit(KisExternalLayer *layer, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(layer);
    Q_UNUSED(undoAdapter);
}

void KisDoNothingProcessingVisitor::visit(KisPaintLayer *layer, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(layer);
    Q_UNUSED(undoAdapter);
}

void KisDoNothingProcessingVisitor::visit(KisGroupLayer *layer, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(layer);
    Q_UNUSED(undoAdapter);
}

void KisDoNothingProcessingVisitor::visit(KisAdjustmentLayer *layer, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(layer);
    Q_UNUSED(undoAdapter);
}

void KisDoNothingProcessingVisitor::visit(KisGeneratorLayer *layer, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(layer);
    Q_UNUSED(undoAdapter);
}

void KisDoNothingProcessingVisitor::visit(KisFilterMask *mask, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(mask);
    Q_UNUSED(undoAdapter);
}

void KisDoNothingProcessingVisitor::visit(KisTransformMask *mask, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(mask);
    Q_UNUSED(undoAdapter);
}

void KisDoNothingProcessingVisitor::visit(KisTransparencyMask *mask, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(mask);
    Q_UNUSED(undoAdapter);
}

void KisDoNothingProcessingVisitor::visit(KisSelectionMask *mask, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(mask);
    Q_UNUSED(undoAdapter);
}
