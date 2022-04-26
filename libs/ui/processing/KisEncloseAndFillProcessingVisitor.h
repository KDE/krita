/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2022 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISENCLOSEANDFILLPROCESSINGVISITOR_H
#define KISENCLOSEANDFILLPROCESSINGVISITOR_H

#include <QPoint>

#include <processing/kis_simple_processing_visitor.h>
#include <kis_selection.h>
#include <kis_resources_snapshot.h>
#include <KisEncloseAndFillPainter.h>
#include <kritaui_export.h>

class KRITAUI_EXPORT KisEncloseAndFillProcessingVisitor : public KisSimpleProcessingVisitor
{
public:
    KisEncloseAndFillProcessingVisitor(
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
    );

private:
    void visitNodeWithPaintDevice(KisNode *node, KisUndoAdapter *undoAdapter) override;
    void visitExternalLayer(KisExternalLayer *layer, KisUndoAdapter *undoAdapter) override;
    void visitColorizeMask(KisColorizeMask *mask, KisUndoAdapter *undoAdapter) override;

    void fillPaintDevice(KisPaintDeviceSP device, KisUndoAdapter *undoAdapter, ProgressHelper &helper);

private:
    KisPaintDeviceSP m_referencePaintDevice {nullptr};
    KisPixelSelectionSP m_enclosingMask {nullptr};
    KisSelectionSP m_selection {nullptr};
    KisResourcesSnapshotSP m_resources {nullptr};
    KisEncloseAndFillPainter::RegionSelectionMethod m_regionSelectionMethod {KisEncloseAndFillPainter::SelectAllRegions};
    KoColor m_regionSelectionColor;
    bool m_regionSelectionInvert {false};
    bool m_regionSelectionIncludeContourRegions {true};
    bool m_regionSelectionIncludeSurroundingRegions {true};
    int m_fillThreshold {8};
    int m_fillOpacitySpread {100};
    bool m_useSelectionAsBoundary {true};
    bool m_antiAlias {false};
    int m_expand {0};
    int m_feather {0};
    bool m_usePattern {false};
    bool m_unmerged {false};
    bool m_useBgColor {false};
};

#endif
