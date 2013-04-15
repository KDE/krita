/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#include "fill_processing_visitor.h"

#include <kis_node.h>
#include <kis_image.h>
#include <kis_fill_painter.h>


FillProcessingVisitor::FillProcessingVisitor(const QPoint &startPoint,
                               KisSelectionSP selection,
                               KisResourcesSnapshotSP resources,
                               bool usePattern,
                               bool selectionOnly,
                               int feather,
                               int sizemod,
                               int fillThreshold,
                               bool unmerged)
    : m_startPoint(startPoint),
      m_selection(selection),
      m_selectionOnly(selectionOnly),
      m_usePattern(usePattern),
      m_resources(resources),
      m_feather(feather),
      m_sizemod(sizemod),
      m_fillThreshold(fillThreshold),
      m_unmerged(unmerged)
{
}

void FillProcessingVisitor::visitExternalLayer(KisExternalLayer *layer, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(layer);
    Q_UNUSED(undoAdapter);
}

void FillProcessingVisitor::visitNodeWithPaintDevice(KisNode *node, KisUndoAdapter *undoAdapter)
{
    KisPaintDeviceSP device = node->paintDevice();
    Q_ASSERT(device);

    ProgressHelper helper(node);
    QRect fillRect = m_resources->image()->bounds();

    if (m_selectionOnly) {
        KisPaintDeviceSP filledDevice = device->createCompositionSourceDevice();
        KisFillPainter fillPainter(filledDevice);
        fillPainter.setProgress(helper.updater());

        if (m_usePattern) {
            fillPainter.fillRect(fillRect, m_resources->currentPattern());
        } else {
            fillPainter.fillRect(fillRect,
                                 m_resources->currentFgColor(),
                                 m_resources->opacity());
        }

        QVector<QRect> dirtyRect = fillPainter.takeDirtyRegion();

        KisPainter *painter = new KisPainter(device, m_selection);
        painter->beginTransaction("");

        m_resources->setupPainter(painter);

        foreach(const QRect &rc, dirtyRect) {
            painter->bitBlt(rc.topLeft(), filledDevice, rc);
        }

        painter->endTransaction(undoAdapter);

    } else {

        KisFillPainter fillPainter(device, m_selection);
        fillPainter.beginTransaction("");

        m_resources->setupPainter(&fillPainter);

        fillPainter.setProgress(helper.updater());
        fillPainter.setSizemod(m_sizemod);
        fillPainter.setFeather(m_feather);
        fillPainter.setFillThreshold(m_fillThreshold);
        fillPainter.setSampleMerged(!m_unmerged);
        fillPainter.setCareForSelection(true);
        fillPainter.setWidth(fillRect.width());
        fillPainter.setHeight(fillRect.height());

        if (m_usePattern) {
            fillPainter.fillPattern(m_startPoint.x(), m_startPoint.y(), m_resources->image()->projection());
        } else {
            fillPainter.fillColor(m_startPoint.x(), m_startPoint.y(), m_resources->image()->projection());
        }

        fillPainter.endTransaction(undoAdapter);
    }
}



