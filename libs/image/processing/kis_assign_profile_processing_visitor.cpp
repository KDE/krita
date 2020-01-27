/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_assign_profile_processing_visitor.h"

#include "kis_external_layer_iface.h"

#include "kis_paint_device.h"
#include "kis_transaction.h"
#include "kis_undo_adapter.h"
#include "kis_transform_mask.h"
#include "lazybrush/kis_colorize_mask.h"

#include <KoColorConversionTransformation.h>
#include "kis_projection_leaf.h"
#include "kis_paint_layer.h"
#include "kis_time_range.h"
#include <QSet>


KisAssignProfileProcessingVisitor::KisAssignProfileProcessingVisitor(const KoColorSpace *srcColorSpace,
                                                                     const KoColorSpace *dstColorSpace)
    : m_srcColorSpace(srcColorSpace)
    , m_dstColorSpace(dstColorSpace)

{
}


void KisAssignProfileProcessingVisitor::visitExternalLayer(KisExternalLayer *layer, KisUndoAdapter *undoAdapter)
{
    undoAdapter->addCommand(layer->setProfile(m_dstColorSpace->profile()));
}

void KisAssignProfileProcessingVisitor::visitNodeWithPaintDevice(KisNode *node, KisUndoAdapter *undoAdapter)
{
    if (!node->projectionLeaf()->isLayer()) return;
    if (*m_dstColorSpace == *node->colorSpace()) return;

    QSet<KisPaintDeviceSP> paintDevices;
    paintDevices.insert(node->paintDevice());
    paintDevices.insert(node->original());
    paintDevices.insert(node->projection());

    KUndo2Command *parentConversionCommand = new KUndo2Command();

    Q_FOREACH (KisPaintDeviceSP dev, paintDevices) {
        if (dev->colorSpace()->colorModelId() == m_srcColorSpace->colorModelId()) {
            dev->setProfile(m_dstColorSpace->profile(), parentConversionCommand);
        }
    }

    undoAdapter->addCommand(parentConversionCommand);
    node->invalidateFrames(KisTimeRange::infinite(0), node->extent());
}

void KisAssignProfileProcessingVisitor::visit(KisTransformMask *mask, KisUndoAdapter *undoAdapter)
{
    mask->threadSafeForceStaticImageUpdate();
    KisSimpleProcessingVisitor::visit(mask, undoAdapter);
}

void KisAssignProfileProcessingVisitor::visitColorizeMask(KisColorizeMask *mask, KisUndoAdapter *undoAdapter)
{
    if (mask->colorSpace()->colorModelId() == m_srcColorSpace->colorModelId()) {
        KUndo2Command *parentConversionCommand = new KUndo2Command();
        mask->setProfile(m_dstColorSpace->profile(), parentConversionCommand);
        undoAdapter->addCommand(parentConversionCommand);

        mask->invalidateFrames(KisTimeRange::infinite(0), mask->extent());
    }
}
