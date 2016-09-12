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
#include <kis_wrapped_rect.h>
#include "lazybrush/kis_colorize_mask.h"


FillProcessingVisitor::FillProcessingVisitor(const QPoint &startPoint,
                               KisSelectionSP selection,
                               KisResourcesSnapshotSP resources,
                               bool useFastMode,
                               bool usePattern,
                               bool selectionOnly,
                               int feather,
                               int sizemod,
                               int fillThreshold,
                               bool unmerged,
                               bool useBgColor)
    : m_startPoint(startPoint),
      m_selection(selection),
      m_useFastMode(useFastMode),
      m_selectionOnly(selectionOnly),
      m_usePattern(usePattern),
      m_resources(resources),
      m_feather(feather),
      m_sizemod(sizemod),
      m_fillThreshold(fillThreshold),
      m_unmerged(unmerged),
      m_useBgColor(useBgColor)
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
    fillPaintDevice(device, undoAdapter, helper);
}

void FillProcessingVisitor::fillPaintDevice(KisPaintDeviceSP device, KisUndoAdapter *undoAdapter, ProgressHelper &helper)
{
    QRect fillRect = m_resources->image()->bounds();

    if (!device->defaultBounds()->wrapAroundMode() &&
        !fillRect.contains(m_startPoint)) {

        return;
    }

    if (m_selectionOnly) {
        KisPaintDeviceSP filledDevice = device->createCompositionSourceDevice();
        KisFillPainter fillPainter(filledDevice);
        fillPainter.setProgress(helper.updater());

        if (m_usePattern) {
            fillPainter.fillRect(fillRect, m_resources->currentPattern());
        } else if (m_useBgColor) {
            fillPainter.fillRect(fillRect,
                                 m_resources->currentBgColor(),
                                 m_resources->opacity());
        } else {
            fillPainter.fillRect(fillRect,
                                 m_resources->currentFgColor(),
                                 m_resources->opacity());
        }

        QVector<QRect> dirtyRect = fillPainter.takeDirtyRegion();

        KisPainter painter(device, m_selection);
        painter.beginTransaction();

        m_resources->setupPainter(&painter);

        Q_FOREACH (const QRect &rc, dirtyRect) {
            painter.bitBlt(rc.topLeft(), filledDevice, rc);
        }

        painter.endTransaction(undoAdapter);

    } else {

        QPoint startPoint = m_startPoint;
        if (device->defaultBounds()->wrapAroundMode()) {
            startPoint = KisWrappedRect::ptToWrappedPt(startPoint, device->defaultBounds()->bounds());
        }

        KisFillPainter fillPainter(device, m_selection);
        fillPainter.beginTransaction();

        m_resources->setupPainter(&fillPainter);

        fillPainter.setProgress(helper.updater());
        fillPainter.setSizemod(m_sizemod);
        fillPainter.setFeather(m_feather);
        fillPainter.setFillThreshold(m_fillThreshold);
        fillPainter.setCareForSelection(true);
        fillPainter.setWidth(fillRect.width());
        fillPainter.setHeight(fillRect.height());
        fillPainter.setUseCompositioning(!m_useFastMode);

        KisPaintDeviceSP sourceDevice = m_unmerged ? device : m_resources->image()->projection();

        if (m_usePattern) {
            fillPainter.fillPattern(startPoint.x(), startPoint.y(), sourceDevice);
        } else {
            fillPainter.fillColor(startPoint.x(), startPoint.y(), sourceDevice);
        }

        fillPainter.endTransaction(undoAdapter);
    }
}

void FillProcessingVisitor::visitColorizeMask(KisColorizeMask *mask, KisUndoAdapter *undoAdapter)
{
    // we fill only the coloring project so the user can work
    // with the mask like with a usual paint layer
    ProgressHelper helper(mask);
    fillPaintDevice(mask->coloringProjection(), undoAdapter, helper);
}
