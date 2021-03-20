/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include "kis_time_span.h"
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
    node->invalidateFrames(KisTimeSpan::infinite(0), node->extent());
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

        mask->invalidateFrames(KisTimeSpan::infinite(0), mask->extent());
    }
}
