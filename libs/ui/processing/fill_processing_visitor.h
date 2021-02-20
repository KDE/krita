/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __FILL_PROCESSING_VISITOR_H
#define __FILL_PROCESSING_VISITOR_H

#include <processing/kis_simple_processing_visitor.h>

#include <QPoint>
#include <kis_selection.h>
#include <kis_resources_snapshot.h>
#include <kritaui_export.h>


class KRITAUI_EXPORT FillProcessingVisitor : public KisSimpleProcessingVisitor
{
public:
    FillProcessingVisitor(
                   KisPaintDeviceSP referencePaintDevice,
                   const QPoint &startPoint,
                   KisSelectionSP selection,
                   KisResourcesSnapshotSP resources,
                   bool useFastMode,
                   bool usePattern,
                   bool selectionOnly,
                   bool useSelectionAsBoundary,
                   int feather,
                   int sizemod,
                   int fillThreshold,
                   bool unmerged,
                   bool m_useBgColor);

private:
    void visitNodeWithPaintDevice(KisNode *node, KisUndoAdapter *undoAdapter) override;
    void visitExternalLayer(KisExternalLayer *layer, KisUndoAdapter *undoAdapter) override;
    void visitColorizeMask(KisColorizeMask *mask, KisUndoAdapter *undoAdapter) override;

    void fillPaintDevice(KisPaintDeviceSP device, KisUndoAdapter *undoAdapter, ProgressHelper &helper);

private:
    KisPaintDeviceSP m_refPaintDevice;
    QPoint m_startPoint;
    KisSelectionSP m_selection;
    bool m_useFastMode;
    bool m_selectionOnly;
    bool m_useSelectionAsBoundary;
    bool m_usePattern;
    KisResourcesSnapshotSP m_resources;

    int m_feather;
    int m_sizemod;
    int m_fillThreshold;
    bool m_unmerged;
    bool m_useBgColor;
};

#endif /* __FILL_PROCESSING_VISITOR_H */
