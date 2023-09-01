/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "fill_processing_visitor.h"

#include <kis_node.h>
#include <kis_image.h>
#include <kis_wrapped_rect.h>
#include "lazybrush/kis_colorize_mask.h"
#include <kis_assert.h>
#include <KisImageResolutionProxy.h>
#include <KoCompositeOpRegistry.h>
#include "KisAnimAutoKey.h"
#include "kis_undo_adapter.h"


FillProcessingVisitor::FillProcessingVisitor(KisPaintDeviceSP refPaintDevice,
                                             KisSelectionSP selection,
                                             KisResourcesSnapshotSP resources)
    : m_refPaintDevice(refPaintDevice)
    , m_selection(selection)
    , m_resources(resources)
    , m_useFastMode(false)
    , m_selectionOnly(false)
    , m_useSelectionAsBoundary(false)
    , m_usePattern(false)
    , m_antiAlias(false)
    , m_feather(0)
    , m_sizemod(0)
    , m_stopGrowingAtDarkestPixel(false)
    , m_fillThreshold(8)
    , m_opacitySpread(0)
    , m_regionFillingMode(KisFillPainter::RegionFillingMode_FloodFill)
    , m_continuousFillMode(ContinuousFillMode_DoNotUse)
    , m_continuousFillMask(nullptr)
    , m_continuousFillReferenceColor(nullptr)
    , m_unmerged(false)
    , m_useBgColor(false)
    , m_useCustomBlendingOptions(false)
    , m_customOpacity(OPACITY_OPAQUE_U8)
    , m_customCompositeOp(COMPOSITE_OVER)
    , m_progressHelper(nullptr)
{}

void FillProcessingVisitor::visitExternalLayer(KisExternalLayer *layer, KisUndoAdapter *undoAdapter)
{
    Q_UNUSED(layer);
    Q_UNUSED(undoAdapter);
}

void FillProcessingVisitor::visitNodeWithPaintDevice(KisNode *node, KisUndoAdapter *undoAdapter)
{
    KisPaintDeviceSP device = node->paintDevice();
    KIS_ASSERT(device);
    if (!m_progressHelper) {
        m_progressHelper.reset(new ProgressHelper(node));
    }
    fillPaintDevice(device, undoAdapter);
}

void FillProcessingVisitor::visitColorizeMask(KisColorizeMask *mask, KisUndoAdapter *undoAdapter)
{
    // we fill only the coloring project so the user can work
    // with the mask like with a usual paint layer
    if (!m_progressHelper) {
        m_progressHelper.reset(new ProgressHelper(mask));
    }
    fillPaintDevice(mask->coloringProjection(), undoAdapter);
}

void FillProcessingVisitor::fillPaintDevice(KisPaintDeviceSP device, KisUndoAdapter *undoAdapter)
{
    KIS_ASSERT(!m_seedPoints.isEmpty());

    QRect fillRect = m_resources->image()->bounds();

    KUndo2Command *autoKeyframeCommand = KisAutoKey::tryAutoCreateDuplicatedFrame(device);
    if (autoKeyframeCommand) {
        undoAdapter->addCommand(autoKeyframeCommand);
    }

    if (m_selectionOnly) {
        if (device->defaultBounds()->wrapAroundMode()) {
            // Always fill if wrap around mode is on
            selectionFill(device, fillRect, undoAdapter);
        } else {
            // Otherwise fill only if any of the points is inside the rect
            for (const QPoint &seedPoint : m_seedPoints) {
                if (fillRect.contains(seedPoint)) {
                    selectionFill(device, fillRect, undoAdapter);
                    break;
                }
            }
        }
    } else {
        for (QPoint seedPoint : m_seedPoints) {
            if (device->defaultBounds()->wrapAroundMode()) {
                seedPoint = KisWrappedRect::ptToWrappedPt(seedPoint, device->defaultBounds()->imageBorderRect(), device->defaultBounds()->wrapAroundModeAxis());
            }

            if (m_continuousFillMode == ContinuousFillMode_DoNotUse) {
                normalFill(device, fillRect, seedPoint, undoAdapter);
            } else {
                continuousFill(device, fillRect, seedPoint, undoAdapter);
            }
        }
    }
}

void FillProcessingVisitor::selectionFill(KisPaintDeviceSP device, const QRect &fillRect, KisUndoAdapter *undoAdapter)
{
    KisPaintDeviceSP filledDevice = device->createCompositionSourceDevice();
    KisFillPainter fillPainter(filledDevice);
    fillPainter.setProgress(m_progressHelper->updater());

    if (m_usePattern) {
        fillPainter.fillRectNoCompose(fillRect, m_resources->currentPattern(), m_resources->fillTransform());
    } else if (m_useBgColor) {
        fillPainter.fillRect(fillRect,
                             m_resources->currentBgColor(),
                             OPACITY_OPAQUE_U8);
    } else {
        fillPainter.fillRect(fillRect,
                             m_resources->currentFgColor(),
                             OPACITY_OPAQUE_U8);
    }

    QVector<QRect> dirtyRect = fillPainter.takeDirtyRegion();

    KisPainter painter(device, m_selection);
    painter.beginTransaction();

    m_resources->setupPainter(&painter);

    if (m_useCustomBlendingOptions) {
        painter.setOpacity(m_customOpacity);
        painter.setCompositeOpId(m_customCompositeOp);
    }

    Q_FOREACH (const QRect &rc, dirtyRect) {
        painter.bitBlt(rc.topLeft(), filledDevice, rc);
    }

    painter.endTransaction(undoAdapter);
}

void FillProcessingVisitor::normalFill(KisPaintDeviceSP device, const QRect &fillRect, const QPoint &seedPoint, KisUndoAdapter *undoAdapter)
{
    KisFillPainter fillPainter(device, m_selection);
    fillPainter.beginTransaction();

    m_resources->setupPainter(&fillPainter);

    if (m_useBgColor) {
        fillPainter.setPaintColor(fillPainter.backgroundColor());
    }
    fillPainter.setProgress(m_progressHelper->updater());
    fillPainter.setAntiAlias(m_antiAlias);
    fillPainter.setSizemod(m_sizemod);
    fillPainter.setStopGrowingAtDarkestPixel(m_stopGrowingAtDarkestPixel);
    fillPainter.setFeather(m_feather);
    fillPainter.setFillThreshold(m_fillThreshold);
    fillPainter.setOpacitySpread(m_opacitySpread);
    fillPainter.setRegionFillingMode(m_regionFillingMode);
    if (m_regionFillingMode == KisFillPainter::RegionFillingMode_BoundaryFill) {
        fillPainter.setRegionFillingBoundaryColor(m_regionFillingBoundaryColor);
    }
    fillPainter.setCareForSelection(true);
    fillPainter.setUseSelectionAsBoundary((m_selection.isNull() || !m_selection->hasNonEmptyPixelSelection()) ? false : m_useSelectionAsBoundary);
    fillPainter.setWidth(fillRect.width());
    fillPainter.setHeight(fillRect.height());
    fillPainter.setUseCompositing(!m_useFastMode);
    if (m_useCustomBlendingOptions) {
        fillPainter.setOpacity(m_customOpacity);
        fillPainter.setCompositeOpId(m_customCompositeOp);
    }

    KisPaintDeviceSP sourceDevice = m_unmerged ? device : m_refPaintDevice;

    if (m_usePattern) {
        fillPainter.fillPattern(seedPoint.x(), seedPoint.y(), sourceDevice, m_resources->fillTransform());
    } else {
        fillPainter.fillColor(seedPoint.x(), seedPoint.y(), sourceDevice);
    }

    fillPainter.endTransaction(undoAdapter);
}

void FillProcessingVisitor::continuousFill(KisPaintDeviceSP device, const QRect &fillRect, const QPoint &seedPoint, KisUndoAdapter *undoAdapter)
{
    // In continuous filling we use a selection mask that represents the
    // cumulated regions already filled. Being able to discard filling
    // operations based on if they were already filled the area under
    // the cursor speeds up greatly the continuous fill operation

    KIS_ASSERT(m_continuousFillMask);
    KIS_SAFE_ASSERT_RECOVER_NOOP(m_continuousFillReferenceColor);

    // The following checks are useful in the continuous fill.
    if (m_useSelectionAsBoundary && m_selection) {
        // If we must use the current selection as boundary we must discard
        // the continuous fill points that lie outside of it.
        // This avoids unnecessary and expensive flood fill operations
        // when the user drags the mouse outside the selection.
        if (!m_selection->selectedRect().contains(seedPoint)) {
            return;
        }
        const quint8 opacity = m_selection->projection()->pixel(seedPoint).opacityU8();
        if (opacity == OPACITY_TRANSPARENT_U8) {
            return;
        }
    }
    // Filling a pixel is likely to fill nearby pixels with the same color,
    // so this check should reduce the number of flood fill operations
    // considerably
    {
        // If the color in the m_continuous fill mask under the start point
        // equals white we return early and don't fill. This means the area
        // under the start point was already filled
        const quint8 opacity = m_continuousFillMask->pixelSelection()->pixel(seedPoint).opacityU8();
        if (opacity == OPACITY_OPAQUE_U8) {
            return;
        }
    }
    if (m_continuousFillMode == ContinuousFillMode_FillSimilarRegions) {
        // If the color in the reference device under the start point
        // differs from the reference color we return early and don't fill
        const KoColor referenceColor = m_continuousFillReferenceColor->convertedTo(m_refPaintDevice->colorSpace());
        const KoColor referenceDeviceColor = m_refPaintDevice->pixel(seedPoint);
        if (referenceColor != referenceDeviceColor) {
            return;
        }
    }

    KisSelectionSP newFillSelection;
    KisPaintDeviceSP sourceDevice = m_unmerged ? device : m_refPaintDevice;
    // First we get the new region mask
    {
        KisFillPainter painter;

        painter.setProgress(m_progressHelper->updater());
        painter.setSizemod(m_sizemod);
        painter.setStopGrowingAtDarkestPixel(m_stopGrowingAtDarkestPixel);
        painter.setAntiAlias(m_antiAlias);
        painter.setFeather(m_feather);
        painter.setFillThreshold(m_fillThreshold);
        painter.setOpacitySpread(m_opacitySpread);
        painter.setRegionFillingMode(m_regionFillingMode);
        if (m_regionFillingMode == KisFillPainter::RegionFillingMode_BoundaryFill) {
            painter.setRegionFillingBoundaryColor(m_regionFillingBoundaryColor);
        }
        painter.setCareForSelection(true);
        painter.setUseSelectionAsBoundary((m_selection.isNull() || !m_selection->hasNonEmptyPixelSelection()) ? false : m_useSelectionAsBoundary);
        painter.setWidth(fillRect.width());
        painter.setHeight(fillRect.height());
        painter.setUseCompositing(!m_useFastMode);

        KisPixelSelectionSP pixelSelection = painter.createFloodSelection(seedPoint.x(),
                                                                          seedPoint.y(),
                                                                          sourceDevice,
                                                                          m_selection.isNull() ? 0 : m_selection->pixelSelection());

        newFillSelection = new KisSelection(pixelSelection->defaultBounds(),
                                            m_selection ? m_selection->resolutionProxy() : KisImageResolutionProxy::identity());
        newFillSelection->pixelSelection()->applySelection(pixelSelection, SELECTION_REPLACE);
    }
    // Now we actually fill the destination device
    // If there is an active selection, we use a trimmed version of the mask to
    // fill. We cannot trim the obtained mask since that would delete the not
    // selected areas from it and those are need to be part of the continuous
    // fill mask. This avoids unnecessary and expensive flood fill operations
    // when the user drags the mouse outside the selection.
    {
        KisSelectionSP trimmedFillSelection;
        if (m_selection) {
            trimmedFillSelection = new KisSelection(newFillSelection->pixelSelection()->defaultBounds(), newFillSelection->resolutionProxy());
            trimmedFillSelection->pixelSelection()->applySelection(newFillSelection->pixelSelection(), SELECTION_REPLACE);
            trimmedFillSelection->pixelSelection()->applySelection(m_selection->projection(), SELECTION_INTERSECT);
        } else {
            trimmedFillSelection = newFillSelection;
        }
        KisSelectionSP tmpSelection = m_selection;
        m_selection = trimmedFillSelection;
        selectionFill(device, fillRect, undoAdapter);
        m_selection = tmpSelection;
    }
    // Now we update the continuous fill mask with the new mask
    {
        m_continuousFillMask->pixelSelection()->applySelection(newFillSelection->pixelSelection(), SELECTION_ADD);
    }
}

void FillProcessingVisitor::setSeedPoint(const QPoint &seedPoint)
{
    m_seedPoints.clear();
    m_seedPoints.append(seedPoint);
}

void FillProcessingVisitor::setSeedPoints(const QVector<QPoint> &seedPoints)
{
    m_seedPoints = seedPoints;
}

void FillProcessingVisitor::setUseFastMode(bool useFastMode)
{
    m_useFastMode = useFastMode;
}

void FillProcessingVisitor::setUsePattern(bool usePattern)
{
    m_usePattern = usePattern;
}

void FillProcessingVisitor::setSelectionOnly(bool selectionOnly)
{
    m_selectionOnly = selectionOnly;
}

void FillProcessingVisitor::setUseSelectionAsBoundary(bool useSelectionAsBoundary)
{
    m_useSelectionAsBoundary = useSelectionAsBoundary;
}

void FillProcessingVisitor::setAntiAlias(bool antiAlias)
{
    m_antiAlias = antiAlias;
}

void FillProcessingVisitor::setFeather(int feather)
{
    m_feather = feather;
}

void FillProcessingVisitor::setSizeMod(int sizemod)
{
    m_sizemod = sizemod;
}

void FillProcessingVisitor::setStopGrowingAtDarkestPixel(bool stopGrowingAtDarkestPixel)
{
    m_stopGrowingAtDarkestPixel = stopGrowingAtDarkestPixel;
}

void FillProcessingVisitor::setFillThreshold(int fillThreshold)
{
    m_fillThreshold = fillThreshold;
}

void FillProcessingVisitor::setOpacitySpread(int opacitySpread)
{
    m_opacitySpread = opacitySpread;
}

void FillProcessingVisitor::setRegionFillingMode(KisFillPainter::RegionFillingMode regionFillingMode)
{
    m_regionFillingMode = regionFillingMode;
}

void FillProcessingVisitor::setRegionFillingBoundaryColor(const KoColor &regionFillingBoundaryColor)
{
    m_regionFillingBoundaryColor = regionFillingBoundaryColor;
}

void FillProcessingVisitor::setContinuousFillMode(ContinuousFillMode continuousFillMode)
{
    m_continuousFillMode = continuousFillMode;
}

void FillProcessingVisitor::setContinuousFillMask(KisSelectionSP continuousFillMask)
{
    m_continuousFillMask = continuousFillMask;
}

void FillProcessingVisitor::setContinuousFillReferenceColor(const QSharedPointer<KoColor> continuousFillReferenceColor)
{
    m_continuousFillReferenceColor = continuousFillReferenceColor;
}

void FillProcessingVisitor::setUnmerged(bool unmerged)
{
    m_unmerged = unmerged;
}

void FillProcessingVisitor::setUseBgColor(bool useBgColor)
{
    m_useBgColor = useBgColor;
}

void FillProcessingVisitor::setUseCustomBlendingOptions(bool useCustomBlendingOptions)
{
    m_useCustomBlendingOptions = useCustomBlendingOptions;
}

void FillProcessingVisitor::setCustomOpacity(int customOpacity)
{
    m_customOpacity = customOpacity;
}

void FillProcessingVisitor::setCustomCompositeOp(const QString &customCompositeOp)
{
    m_customCompositeOp = customCompositeOp;
}

void FillProcessingVisitor::setProgressHelper(QSharedPointer<ProgressHelper> progressHelper)
{
    m_progressHelper = progressHelper;
}
