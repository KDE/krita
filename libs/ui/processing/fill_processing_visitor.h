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
#include <kis_fill_painter.h>

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
    void setStopGrowingAtDarkestPixel(bool stopGrowingAtDarkestPixel);
    void setFillThreshold(int fillThreshold);
    void setOpacitySpread(int opacitySpread);
    void setCloseGap(int gap);
    void setRegionFillingMode(KisFillPainter::RegionFillingMode regionFillingMode);
    void setRegionFillingBoundaryColor(const KoColor &regionFillingBoundaryColor);
    void setContinuousFillMode(ContinuousFillMode continuousFillMode);
    void setContinuousFillMask(KisSelectionSP continuousFillMask);
    void setContinuousFillReferenceColor(const QSharedPointer<KoColor> continuousFillReferenceColor);
    void setUnmerged(bool unmerged);
    void setUseBgColor(bool useBgColor);
    void setUseCustomBlendingOptions(bool useCustomBlendingOptions);
    void setCustomOpacity(int customOpacity);
    void setCustomCompositeOp(const QString &customCompositeOp);
    void setOutDirtyRect(QSharedPointer<QRect> outDirtyRect);
    void setProgressHelper(QSharedPointer<ProgressHelper> progressHelper);

private:
    void visitNodeWithPaintDevice(KisNode *node, KisUndoAdapter *undoAdapter) override;
    void visitExternalLayer(KisExternalLayer *layer, KisUndoAdapter *undoAdapter) override;
    void visitColorizeMask(KisColorizeMask *mask, KisUndoAdapter *undoAdapter) override;

    void fillPaintDevice(KisPaintDeviceSP device, KisUndoAdapter *undoAdapter);

    void selectionFill(KisPaintDeviceSP device, const QRect &fillRect, KisUndoAdapter *undoAdapter);
    void normalFill(KisPaintDeviceSP device, const QRect &fillRect, const QPoint &seedPoint, KisUndoAdapter *undoAdapter);
    void continuousFill(KisPaintDeviceSP device, const QRect &fillRect, const QPoint &seedPoint, KisUndoAdapter *undoAdapter);

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
    bool m_stopGrowingAtDarkestPixel;
    int m_fillThreshold;
    int m_opacitySpread;
    int m_closeGap;
    KisFillPainter::RegionFillingMode m_regionFillingMode;
    KoColor m_regionFillingBoundaryColor;

    ContinuousFillMode m_continuousFillMode;
    KisSelectionSP m_continuousFillMask;
    QSharedPointer<KoColor> m_continuousFillReferenceColor {nullptr};

    bool m_unmerged;
    bool m_useBgColor;

    bool m_useCustomBlendingOptions;
    int m_customOpacity;
    QString m_customCompositeOp;

    QSharedPointer<QRect> m_outDirtyRect;

    QSharedPointer<ProgressHelper> m_progressHelper {nullptr};
};

#endif /* __FILL_PROCESSING_VISITOR_H */
