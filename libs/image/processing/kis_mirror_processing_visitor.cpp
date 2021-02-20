/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_mirror_processing_visitor.h"

#include "kis_paint_device.h"
#include "kis_transaction.h"
#include "kis_node.h"
#include "kis_image.h"
#include "kis_painter.h"

#include "kis_transform_worker.h"
#include "lazybrush/kis_colorize_mask.h"
#include "processing/kis_transform_processing_visitor.h"

#include "commands_new/kis_transaction_based_command.h"
#include <functional>


KisMirrorProcessingVisitor::KisMirrorProcessingVisitor(const QRect &bounds, Qt::Orientation orientation)
    : m_bounds(bounds),
      m_orientation(orientation),
      m_selectionHelper(0, std::bind(&KisMirrorProcessingVisitor::mirrorDevice, this, std::placeholders::_1))
{
    m_axis = m_orientation == Qt::Horizontal ?
        m_bounds.x() + 0.5 * m_bounds.width() :
        m_bounds.y() + 0.5 * m_bounds.height();
}

KisMirrorProcessingVisitor::KisMirrorProcessingVisitor(KisSelectionSP selection, Qt::Orientation orientation)
    : KisMirrorProcessingVisitor(selection->selectedExactRect(), orientation)
{
    m_selectionHelper.setSelection(selection);
}

KUndo2Command *KisMirrorProcessingVisitor::createInitCommand()
{
    return m_selectionHelper.createInitCommand();
}

void KisMirrorProcessingVisitor::mirrorDevice(KisPaintDeviceSP device)
{
    KisTransformWorker::mirror(device, m_axis, m_orientation);
}

void KisMirrorProcessingVisitor::transformPaintDevice(KisPaintDeviceSP device, KisUndoAdapter *undoAdapter)
{
    m_selectionHelper.transformPaintDevice(device, undoAdapter);
}

void KisMirrorProcessingVisitor::visitNodeWithPaintDevice(KisNode *node, KisUndoAdapter *undoAdapter)
{
    transformPaintDevice(node->paintDevice(), undoAdapter);
}

void KisMirrorProcessingVisitor::visitExternalLayer(KisExternalLayer *layer, KisUndoAdapter *undoAdapter)
{
    if (m_orientation == Qt::Horizontal) {
        KisTransformProcessingVisitor visitor(-1.0, 1.0,
                                              0.0, 0.0,
                                              QPointF(), 0.0,
                                              m_bounds.width(), 0.0,
                                              0);
        visitor.visit(layer, undoAdapter);
    } else {
        KisTransformProcessingVisitor visitor(1.0, -1.0,
                                              0.0, 0.0,
                                              QPointF(), 0.0,
                                              0.0, m_bounds.height(),
                                              0);
        visitor.visit(layer, undoAdapter);
    }
}

void KisMirrorProcessingVisitor::visitColorizeMask(KisColorizeMask *node, KisUndoAdapter *undoAdapter)
{
    QVector<KisPaintDeviceSP> devices = node->allPaintDevices();

    Q_FOREACH (KisPaintDeviceSP device, devices) {
        transformPaintDevice(device, undoAdapter);
    }
}
