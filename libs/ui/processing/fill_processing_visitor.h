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
    enum ContinuousFillMode
    {
        ContinuousFillMode_DoNotUse,
        ContinuousFillMode_FillAnyRegion,
        ContinuousFillMode_FillSimilarRegions
    };

    FillProcessingVisitor(KisPaintDeviceSP referencePaintDevice,
                          KisSelectionSP selection,
                          KisResourcesSnapshotSP resources);

    void setSeedPoint(const QPoint &seedPoint);
    void setSeedPoints(const QVector<QPoint> &seedPoints);
    void setUseFastMode(bool useFastMode);
    void setUsePattern(bool usePattern);
    void setSelectionOnly(bool selectionOnly);
    void setUseSelectionAsBoundary(bool useSelectionAsBoundary);
    void setAntiAlias(bool antiAlias);
    void setFeather(int feather);
    void setSizeMod(int sizemod);
    void setFillThreshold(int fillThreshold);
    void setOpacitySpread(int opacitySpread);
    void setContinuousFillMode(ContinuousFillMode continuousFillMode);
    void setContinuousFillMask(KisSelectionSP continuousFillMask);
    void setContinuousFillReferenceColor(const KoColor &continuousFillReferenceColor);
    void setUnmerged(bool unmerged);
    void setUseBgColor(bool useBgColor);

private:
    void visitNodeWithPaintDevice(KisNode *node, KisUndoAdapter *undoAdapter) override;
    void visitExternalLayer(KisExternalLayer *layer, KisUndoAdapter *undoAdapter) override;
    void visitColorizeMask(KisColorizeMask *mask, KisUndoAdapter *undoAdapter) override;

    void fillPaintDevice(KisPaintDeviceSP device, KisUndoAdapter *undoAdapter, ProgressHelper &helper);

    void selectionFill(KisPaintDeviceSP device, const QRect &fillRect, KisUndoAdapter *undoAdapter, ProgressHelper &helper);
    void normalFill(KisPaintDeviceSP device, const QRect &fillRect, const QPoint &seedPoint, KisUndoAdapter *undoAdapter, ProgressHelper &helper);
    void continuousFill(KisPaintDeviceSP device, const QRect &fillRect, const QPoint &seedPoint, KisUndoAdapter *undoAdapter, ProgressHelper &helper);

private:
    KisPaintDeviceSP m_refPaintDevice;
    KisSelectionSP m_selection;
    KisResourcesSnapshotSP m_resources;

    QVector<QPoint> m_seedPoints;
    bool m_useFastMode;
    bool m_selectionOnly;
    bool m_useSelectionAsBoundary;
    bool m_usePattern;
    bool m_antiAlias;
    int m_feather;
    int m_sizemod;
    int m_fillThreshold;
    int m_opacitySpread;

    ContinuousFillMode m_continuousFillMode;
    KisSelectionSP m_continuousFillMask;
    KoColor m_continuousFillReferenceColor;

    bool m_unmerged;
    bool m_useBgColor;
};

#endif /* __FILL_PROCESSING_VISITOR_H */
