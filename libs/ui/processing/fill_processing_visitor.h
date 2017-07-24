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
    FillProcessingVisitor(const QPoint &startPoint,
                   KisSelectionSP selection,
                   KisResourcesSnapshotSP resources,
                   bool useFastMode,
                   bool usePattern,
                   bool selectionOnly,
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
    QPoint m_startPoint;
    KisSelectionSP m_selection;
    bool m_useFastMode;
    bool m_selectionOnly;
    bool m_usePattern;
    KisResourcesSnapshotSP m_resources;

    int m_feather;
    int m_sizemod;
    int m_fillThreshold;
    bool m_unmerged;
    bool m_useBgColor;
};

#endif /* __FILL_PROCESSING_VISITOR_H */
