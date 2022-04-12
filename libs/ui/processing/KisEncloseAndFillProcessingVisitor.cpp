/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2022 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <kis_node.h>
#include <kis_image.h>
#include <kis_wrapped_rect.h>
#include <lazybrush/kis_colorize_mask.h>

#include "KisEncloseAndFillProcessingVisitor.h"

KisEncloseAndFillProcessingVisitor::KisEncloseAndFillProcessingVisitor(
        KisPaintDeviceSP referencePaintDevice,
        KisPixelSelectionSP enclosingMask,
        KisSelectionSP selection,
        KisResourcesSnapshotSP resources,
        KisEncloseAndFillPainter::RegionSelectionMethod regionSelectionMethod,
        const KoColor &regionSelectionColor,
        bool regionSelectionInvert,
        bool regionSelectionIncludeContourRegions,
        bool regionSelectionIncludeSurroundingRegions,
        int fillThreshold,
        int fillOpacitySpread,
        bool antiAlias,
        int expand,
        int feather,
        bool useSelectionAsBoundary,
        bool usePattern,
        bool unmerged,
        bool useBgColor
)
    : m_referencePaintDevice(referencePaintDevice)
    , m_enclosingMask(enclosingMask)
    , m_selection(selection)
    , m_resources(resources)
    , m_regionSelectionMethod(regionSelectionMethod)
    , m_regionSelectionColor(regionSelectionColor)
    , m_regionSelectionInvert(regionSelectionInvert)
    , m_regionSelectionIncludeContourRegions(regionSelectionIncludeContourRegions)
    , m_regionSelectionIncludeSurroundingRegions(regionSelectionIncludeSurroundingRegions)
    , m_fillThreshold(fillThreshold)
    , m_fillOpacitySpread(fillOpacitySpread)
    , m_useSelectionAsBoundary(useSelectionAsBoundary)
    , m_antiAlias(antiAlias)
    , m_expand(expand)
    , m_feather(feather)
    , m_usePattern(usePattern)
    , m_unmerged(unmerged)
    , m_useBgColor(useBgColor)
{}

void KisEncloseAndFillProcessingVisitor::visitExternalLayer(KisExternalLayer *layer, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(layer);
    Q_UNUSED(undoAdapter);
}

void KisEncloseAndFillProcessingVisitor::visitNodeWithPaintDevice(KisNode *node, KisUndoAdapter *undoAdapter)
{
    KisPaintDeviceSP device = node->paintDevice();
    Q_ASSERT(device);
    ProgressHelper helper(node);
    fillPaintDevice(device, undoAdapter, helper);
}

void KisEncloseAndFillProcessingVisitor::fillPaintDevice(KisPaintDeviceSP device, KisUndoAdapter *undoAdapter, ProgressHelper &helper)
{
    Q_ASSERT(m_enclosingMask);

    const QRect fillRect = m_resources->image()->bounds();

    KisEncloseAndFillPainter painter(device, m_selection);
    painter.beginTransaction();

    m_resources->setupPainter(&painter);

    if (m_useBgColor) {
        painter.setPaintColor(painter.backgroundColor());
    }

    painter.setProgress(helper.updater());
    painter.setRegionSelectionMethod(m_regionSelectionMethod);
    painter.setRegionSelectionColor(m_regionSelectionColor);
    painter.setRegionSelectionInvert(m_regionSelectionInvert);
    painter.setRegionSelectionIncludeContourRegions(m_regionSelectionIncludeContourRegions);
    painter.setRegionSelectionIncludeSurroundingRegions(m_regionSelectionIncludeSurroundingRegions);
    painter.setFillThreshold(m_fillThreshold);
    painter.setOpacitySpread(m_fillOpacitySpread);
    painter.setUseSelectionAsBoundary((m_selection.isNull() || !m_selection->hasNonEmptyPixelSelection()) ? false : m_useSelectionAsBoundary);
    painter.setAntiAlias(m_antiAlias);
    painter.setSizemod(m_expand);
    painter.setFeather(m_feather);
    painter.setWidth(fillRect.width());
    painter.setHeight(fillRect.height());

    KisPaintDeviceSP sourceDevice = m_unmerged ? device : m_referencePaintDevice;

    if (m_usePattern) {
        painter.encloseAndFillPattern(m_enclosingMask, sourceDevice, m_resources->fillTransform());
    } else {
        painter.encloseAndFillColor(m_enclosingMask, sourceDevice);
    }

    painter.endTransaction(undoAdapter);
}

void KisEncloseAndFillProcessingVisitor::visitColorizeMask(KisColorizeMask *mask, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(mask);
    Q_UNUSED(undoAdapter);
}
