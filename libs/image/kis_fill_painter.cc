/*
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2004 Bart Coppens <kde@bartcoppens.be>
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_fill_painter.h"

#include <stdlib.h>
#include <string.h>
#include <cfloat>
#include <stack>

#include <QFontInfo>
#include <QFontMetrics>
#include <QPen>
#include <QMatrix>
#include <QImage>
#include <QMap>
#include <QPainter>
#include <QRect>
#include <QString>

#include <klocalizedstring.h>

#include <KoUpdater.h>

#include "generator/kis_generator.h"
#include "filter/kis_filter_configuration.h"
#include "generator/kis_generator_registry.h"
#include "kis_processing_information.h"
#include "kis_debug.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include <resources/KoPattern.h>
#include "KoColorSpace.h"
#include "kis_transaction.h"
#include "kis_pixel_selection.h"
#include <KoCompositeOpRegistry.h>
#include <floodfill/kis_scanline_fill.h>
#include "kis_selection_filters.h"
#include <kis_perspectivetransform_worker.h>
#include <kis_sequential_iterator.h>
#include <KisColorSelectionPolicies.h>
#include <krita_utils.h>
#include <kis_default_bounds.h>
#include <KisImageResolutionProxy.h>


KisFillPainter::KisFillPainter()
        : KisPainter()
{
    initFillPainter();
}

KisFillPainter::KisFillPainter(KisPaintDeviceSP device)
        : KisPainter(device)
{
    initFillPainter();
}

KisFillPainter::KisFillPainter(KisPaintDeviceSP device, KisSelectionSP selection)
        : KisPainter(device, selection)
{
    initFillPainter();
}

void KisFillPainter::initFillPainter()
{
    m_width = m_height = -1;
    m_careForSelection = false;
    m_sizemod = 0;
    m_feather = 0;
    m_useCompositing = false;
    m_threshold = 0;
    m_opacitySpread = 0;
    m_useSelectionAsBoundary = false;
    m_antiAlias = false;
    m_regionFillingMode = RegionFillingMode_FloodFill;
    m_stopGrowingAtDarkestPixel = false;
}

void KisFillPainter::fillSelection(const QRect &rc, const KoColor &color)
{
    KisPaintDeviceSP fillDevice = new KisPaintDevice(device()->colorSpace());
    fillDevice->setDefaultPixel(color);

    bitBlt(rc.topLeft(), fillDevice, rc);
}

// 'regular' filling
// XXX: This also needs renaming, since filling ought to keep the opacity and the composite op in mind,
//      this is more eraseToColor.
void KisFillPainter::fillRect(qint32 x1, qint32 y1, qint32 w, qint32 h, const KoColor& kc, quint8 opacity)
{
    if (w > 0 && h > 0) {
        // Make sure we're in the right colorspace

        KoColor kc2(kc); // get rid of const
        kc2.convertTo(device()->colorSpace());
        quint8 * data = kc2.data();
        device()->colorSpace()->setOpacity(data, opacity, 1);

        device()->fill(x1, y1, w, h, data);

        addDirtyRect(QRect(x1, y1, w, h));
    }
}

void KisFillPainter::fillRect(const QRect &rc, const KoPatternSP pattern, const QPoint &offset)
{
    fillRect(rc.x(), rc.y(), rc.width(), rc.height(), pattern, offset);
}

void KisFillPainter::fillRect(qint32 x1, qint32 y1, qint32 w, qint32 h, const KoPatternSP pattern, const QPoint &offset)
{
    if (!pattern) return;
    if (!pattern->valid()) return;
    if (!device()) return;
    if (w < 1) return;
    if (h < 1) return;

    KisPaintDeviceSP patternLayer = new KisPaintDevice(device()->compositionSourceColorSpace(), pattern->name());
    patternLayer->convertFromQImage(pattern->pattern(), 0);

    if (!offset.isNull()) {
        patternLayer->moveTo(offset);
    }

    fillRect(x1, y1, w, h, patternLayer, QRect(offset.x(), offset.y(), pattern->width(), pattern->height()));
}

void KisFillPainter::fillRectNoCompose(const QRect &rc, const KoPatternSP pattern, const QTransform transform)
{
    if (!pattern) return;
    if (!pattern->valid()) return;
    if (!device()) return;
    if (rc.width() < 1) return;
    if (rc.height() < 1) return;

    KisPaintDeviceSP patternLayer = new KisPaintDevice(device()->colorSpace(), pattern->name());
    patternLayer->convertFromQImage(pattern->pattern(), 0);

    fillRectNoCompose(rc.x(), rc.y(), rc.width(), rc.height(), patternLayer, QRect(0, 0, pattern->width(), pattern->height()), transform);
}

void KisFillPainter::fillRectNoCompose(qint32 x1, qint32 y1, qint32 w, qint32 h, const KisPaintDeviceSP device, const QRect& deviceRect, const QTransform transform)
{
    /**
     * Since this function doesn't do any kind of compositing, so the pixel size
     * of the source and destination devices must be exactly the same. The color
     * space should ideally be also the same.
     */
    KIS_SAFE_ASSERT_RECOVER_RETURN(device->pixelSize() == this->device()->pixelSize());
    KIS_SAFE_ASSERT_RECOVER_NOOP(*device->colorSpace() == *this->device()->colorSpace());

    KisPaintDeviceSP wrapped = device;
    KisDefaultBoundsBaseSP oldBounds = wrapped->defaultBounds();
    wrapped->setDefaultBounds(new KisWrapAroundBoundsWrapper(oldBounds, deviceRect));
    const bool oldSupportsWrapAroundMode = wrapped->supportsWraproundMode();
    wrapped->setSupportsWraparoundMode(true);


    KisPerspectiveTransformWorker worker(this->device(), transform, false, this->progressUpdater());
    worker.runPartialDst(device, this->device(), QRect(x1, y1, w, h));

    addDirtyRect(QRect(x1, y1, w, h));
    wrapped->setDefaultBounds(oldBounds);
    wrapped->setSupportsWraparoundMode(oldSupportsWrapAroundMode);
}

void KisFillPainter::fillRect(const QRect &rc, const KisPaintDeviceSP device, const QRect& deviceRect)
{
    fillRect(rc.x(), rc.y(), rc.width(), rc.height(), device, deviceRect);
}

void KisFillPainter::fillRect(qint32 x1, qint32 y1, qint32 w, qint32 h, const KisPaintDeviceSP device, const QRect& deviceRect)
{
    const QRect &patternRect = deviceRect;
    const QRect fillRect(x1, y1, w, h);

    auto toPatternLocal = [](int value, int offset, int width) {
        const int normalizedValue = value - offset;
        return offset + (normalizedValue >= 0 ?
                         normalizedValue % width :
                         width - (-normalizedValue - 1) % width - 1);
    };

    int dstY = fillRect.y();
    while (dstY <= fillRect.bottom()) {
        const int dstRowsRemaining = fillRect.bottom() - dstY + 1;

        const int srcY = toPatternLocal(dstY, patternRect.y(), patternRect.height());
        const int height = qMin(patternRect.height() - srcY + patternRect.y(), dstRowsRemaining);

        int dstX = fillRect.x();
        while (dstX <= fillRect.right()) {
            const int dstColumnsRemaining = fillRect.right() - dstX + 1;

            const int srcX = toPatternLocal(dstX, patternRect.x(), patternRect.width());
            const int width = qMin(patternRect.width() - srcX  + patternRect.x(), dstColumnsRemaining);

            bitBlt(dstX, dstY, device, srcX, srcY, width, height);

            dstX += width;
        }
        dstY += height;
    }

    addDirtyRect(QRect(x1, y1, w, h));
}

void KisFillPainter::fillRect(qint32 x1, qint32 y1, qint32 w, qint32 h, const KisFilterConfigurationSP generator)
{
    if (!generator) return;
    KisGeneratorSP g = KisGeneratorRegistry::instance()->value(generator->name());
    if (!device()) return;
    if (w < 1) return;
    if (h < 1) return;

    QRect tmpRc(x1, y1, w, h);

    KisProcessingInformation dstCfg(device(), tmpRc.topLeft(), 0);

    g->generate(dstCfg, tmpRc.size(), generator);

    addDirtyRect(tmpRc);
}

// flood filling

void KisFillPainter::fillColor(int startX, int startY, KisPaintDeviceSP sourceDevice)
{
    if (!m_useCompositing) {
        if (m_sizemod || m_feather ||
            compositeOpId() != COMPOSITE_OVER ||
            opacity() != MAX_SELECTED ||
            sourceDevice != device()) {

            warnKrita << "WARNING: Fast Flood Fill (no compositing mode)"
                       << "does not support compositeOps, opacity, "
                       << "selection enhancements and separate source "
                       << "devices";
        }

        QRect fillBoundsRect(0, 0, m_width, m_height);
        QPoint startPoint(startX, startY);

        if (!fillBoundsRect.contains(startPoint)) return;

        KisScanlineFill gc(device(), startPoint, fillBoundsRect);
        gc.setThreshold(m_threshold);
        if (m_regionFillingMode == RegionFillingMode_FloodFill) {
            gc.fill(paintColor());
        } else {
            gc.fillUntilColor(paintColor(), m_regionFillingBoundaryColor);
        }

    } else {
        genericFillStart(startX, startY, sourceDevice);

        // Now create a layer and fill it
        KisPaintDeviceSP filled = device()->createCompositionSourceDevice();
        Q_CHECK_PTR(filled);
        KisFillPainter painter(filled);
        painter.fillRect(0, 0, m_width, m_height, paintColor());
        painter.end();

        genericFillEnd(filled);
    }
}

void KisFillPainter::fillPattern(int startX, int startY, KisPaintDeviceSP sourceDevice, QTransform patternTransform)
{
    genericFillStart(startX, startY, sourceDevice);

    // Now create a layer and fill it
    KisPaintDeviceSP filled = device()->createCompositionSourceDevice();
    Q_CHECK_PTR(filled);
    KisFillPainter painter(filled);
    painter.fillRectNoCompose(QRect(0, 0, m_width, m_height), pattern(), patternTransform);
    painter.end();

    genericFillEnd(filled);
}

void KisFillPainter::genericFillStart(int startX, int startY, KisPaintDeviceSP sourceDevice)
{
    Q_ASSERT(m_width > 0);
    Q_ASSERT(m_height > 0);

    // Create a selection from the surrounding area

    KisPixelSelectionSP pixelSelection = createFloodSelection(startX, startY, sourceDevice,
                                                              (selection().isNull() ? 0 : selection()->pixelSelection()));
    KisSelectionSP newSelection = new KisSelection(pixelSelection->defaultBounds(),
                                                   selection() ? selection()->resolutionProxy() : KisImageResolutionProxy::identity());
    newSelection->pixelSelection()->applySelection(pixelSelection, SELECTION_REPLACE);
    m_fillSelection = newSelection;
}

void KisFillPainter::genericFillEnd(KisPaintDeviceSP filled)
{
    if (progressUpdater() && progressUpdater()->interrupted()) {
        m_width = m_height = -1;
        return;
    }

//  TODO: filling using the correct bound of the selection would be better, *but*
//  the selection is limited to the exact bound of a layer, while in reality, we don't
//  want that, since we want a transparent layer to be completely filled
//     QRect rc = m_fillSelection->selectedExactRect();


    /**
     * Apply the real selection to a filled one
     */
    KisSelectionSP realSelection = selection();
    QRect rc;

    if (realSelection) {
        rc = m_fillSelection->selectedExactRect().intersected(realSelection->projection()->selectedExactRect());
        m_fillSelection->pixelSelection()->applySelection(
            realSelection->projection(), SELECTION_INTERSECT);
    } else {
        rc = m_fillSelection->selectedExactRect();
    }

    setSelection(m_fillSelection);
    bitBlt(rc.topLeft(), filled, rc);
    setSelection(realSelection);

    if (progressUpdater()) progressUpdater()->setProgress(100);

    m_width = m_height = -1;
}

KisPixelSelectionSP KisFillPainter::createFloodSelection(int startX, int startY, KisPaintDeviceSP sourceDevice,
                                                         KisPaintDeviceSP existingSelection)
{
    KisPixelSelectionSP newSelection = new KisPixelSelection(new KisSelectionDefaultBounds(device()));
    return createFloodSelection(newSelection, startX, startY, sourceDevice, existingSelection);
}

KisPixelSelectionSP KisFillPainter::createFloodSelection(KisPixelSelectionSP pixelSelection, int startX, int startY,
                                                         KisPaintDeviceSP sourceDevice, KisPaintDeviceSP existingSelection)
{

    if (m_width < 0 || m_height < 0) {
        if (selection() && m_careForSelection) {
            QRect rc = selection()->selectedExactRect();
            m_width = rc.width() - (startX - rc.x());
            m_height = rc.height() - (startY - rc.y());
        }
    }
    dbgImage << "Width: " << m_width << " Height: " << m_height;
    // Otherwise the width and height should have been set
    Q_ASSERT(m_width > 0 && m_height > 0);

    QRect fillBoundsRect(0, 0, m_width, m_height);
    QPoint startPoint(startX, startY);

    if (!fillBoundsRect.contains(startPoint)) {
        return pixelSelection;
    }

    KisScanlineFill gc(sourceDevice, startPoint, fillBoundsRect);
    gc.setThreshold(m_threshold);
    gc.setOpacitySpread(m_useCompositing ? m_opacitySpread : 100);
    if (m_regionFillingMode == RegionFillingMode_FloodFill) {
        if (m_useSelectionAsBoundary && !pixelSelection.isNull()) {
            gc.fillSelection(pixelSelection, existingSelection);
        } else {
            gc.fillSelection(pixelSelection);
        }
    } else {
        if (m_useSelectionAsBoundary && !pixelSelection.isNull()) {
            gc.fillSelectionUntilColor(pixelSelection, m_regionFillingBoundaryColor, existingSelection);
        } else {
            gc.fillSelectionUntilColor(pixelSelection, m_regionFillingBoundaryColor);
        }
    }

    if (m_useCompositing) {
        if (m_sizemod > 0) {
            if (m_stopGrowingAtDarkestPixel) {
                KisGrowUntilDarkestPixelSelectionFilter biggy(m_sizemod, sourceDevice);
                biggy.process(pixelSelection, pixelSelection->selectedRect().adjusted(-m_sizemod, -m_sizemod, m_sizemod, m_sizemod));
            } else {
                KisGrowSelectionFilter biggy(m_sizemod, m_sizemod);
                biggy.process(pixelSelection, pixelSelection->selectedRect().adjusted(-m_sizemod, -m_sizemod, m_sizemod, m_sizemod));
            }
        }
        else if (m_sizemod < 0) {
            KisShrinkSelectionFilter tiny(-m_sizemod, -m_sizemod, false);
            tiny.process(pixelSelection, pixelSelection->selectedRect());
        }
        // Since the feathering already smooths the selection, the antiAlias
        // is not applied if we must feather
        if (m_feather > 0) {
            KisFeatherSelectionFilter feathery(m_feather);
            feathery.process(pixelSelection, pixelSelection->selectedRect().adjusted(-m_feather, -m_feather, m_feather, m_feather));
        } else if (m_antiAlias) {
            KisAntiAliasSelectionFilter antiAliasFilter;
            antiAliasFilter.process(pixelSelection, pixelSelection->selectedRect());
        }
    }

    return pixelSelection;
}

template <typename DifferencePolicy, typename SelectionPolicy>
void createSimilarColorsSelectionImpl(KisPixelSelectionSP outSelection,
                                      KisPaintDeviceSP referenceDevice,
                                      const QRect &rect,
                                      KisPixelSelectionSP mask,
                                      DifferencePolicy differencePolicy,
                                      SelectionPolicy selectionPolicy,
                                      KoUpdater *updater = nullptr)
{
    KisSequentialConstIterator referenceDeviceIterator(referenceDevice, rect);
    KisSequentialIterator outSelectionIterator(outSelection, rect);

    const int totalNumberOfPixels = rect.width() * rect.height();
    const int numberOfUpdates = 4;
    const int numberOfPixelsPerUpdate = totalNumberOfPixels / numberOfUpdates;
    const int progressIncrement = 100 / numberOfUpdates;
    int numberOfPixelsProcessed = 0;

    if (mask) {
        KisSequentialConstIterator maskIterator(mask, rect);
        while (referenceDeviceIterator.nextPixel() &&
               outSelectionIterator.nextPixel() &&
               maskIterator.nextPixel()) {
            if (*maskIterator.rawDataConst() != MIN_SELECTED) {
                *outSelectionIterator.rawData() =
                    selectionPolicy.opacityFromDifference(
                        differencePolicy.difference(referenceDeviceIterator.rawDataConst())
                    );
            }
            if (updater) {
                ++numberOfPixelsProcessed;
                if (numberOfPixelsProcessed > numberOfPixelsPerUpdate) {
                    numberOfPixelsProcessed = 0;
                    updater->setProgress(updater->progress() + progressIncrement);
                }
            }
        }
    } else {
        while (referenceDeviceIterator.nextPixel() &&
               outSelectionIterator.nextPixel()) {
            *outSelectionIterator.rawData() =
                selectionPolicy.opacityFromDifference(
                    differencePolicy.difference(referenceDeviceIterator.rawDataConst())
                );
            if (updater) {
                ++numberOfPixelsProcessed;
                if (numberOfPixelsProcessed > numberOfPixelsPerUpdate) {
                    numberOfPixelsProcessed = 0;
                    updater->setProgress(updater->progress() + progressIncrement);
                }
            }
        }
    }
    if (updater) {
        updater->setProgress(100);
    }
}

void KisFillPainter::createSimilarColorsSelection(KisPixelSelectionSP outSelection,
                                                  const KoColor &referenceColor,
                                                  KisPaintDeviceSP referenceDevice,
                                                  const QRect &rect,
                                                  KisPixelSelectionSP mask)
{
    if (rect.isEmpty()) {
        return;
    }

    KoColor srcColor(referenceColor);
    srcColor.convertTo(referenceDevice->colorSpace());

    const int pixelSize = referenceDevice->pixelSize();
    const int softness = 100 - opacitySpread();

    using namespace KisColorSelectionPolicies;

    if (softness == 0) {
        HardSelectionPolicy sp(fillThreshold());
        if (pixelSize == 1) {
            OptimizedDifferencePolicy<quint8> dp(srcColor, fillThreshold());
            createSimilarColorsSelectionImpl(outSelection, referenceDevice, rect, mask, dp, sp);
        } else if (pixelSize == 2) {
            OptimizedDifferencePolicy<quint16> dp(srcColor, fillThreshold());
            createSimilarColorsSelectionImpl(outSelection, referenceDevice, rect, mask, dp, sp);
        } else if (pixelSize == 4) {
            OptimizedDifferencePolicy<quint32> dp(srcColor, fillThreshold());
            createSimilarColorsSelectionImpl(outSelection, referenceDevice, rect, mask, dp, sp);
        } else if (pixelSize == 8) {
            OptimizedDifferencePolicy<quint64> dp(srcColor, fillThreshold());
            createSimilarColorsSelectionImpl(outSelection, referenceDevice, rect, mask, dp, sp);
        } else {
            SlowDifferencePolicy dp(srcColor, fillThreshold());
            createSimilarColorsSelectionImpl(outSelection, referenceDevice, rect, mask, dp, sp);
        }
    } else {
        SoftSelectionPolicy sp(fillThreshold(), softness);
        if (pixelSize == 1) {
            OptimizedDifferencePolicy<quint8> dp(srcColor, fillThreshold());
            createSimilarColorsSelectionImpl(outSelection, referenceDevice, rect, mask, dp, sp);
        } else if (pixelSize == 2) {
            OptimizedDifferencePolicy<quint16> dp(srcColor, fillThreshold());
            createSimilarColorsSelectionImpl(outSelection, referenceDevice, rect, mask, dp, sp);
        } else if (pixelSize == 4) {
            OptimizedDifferencePolicy<quint32> dp(srcColor, fillThreshold());
            createSimilarColorsSelectionImpl(outSelection, referenceDevice, rect, mask, dp, sp);
        } else if (pixelSize == 8) {
            OptimizedDifferencePolicy<quint64> dp(srcColor, fillThreshold());
            createSimilarColorsSelectionImpl(outSelection, referenceDevice, rect, mask, dp, sp);
        } else {
            SlowDifferencePolicy dp(srcColor, fillThreshold());
            createSimilarColorsSelectionImpl(outSelection, referenceDevice, rect, mask, dp, sp);
        }
    }
}

QVector<KisStrokeJobData*> KisFillPainter::createSimilarColorsSelectionJobs(
    KisPixelSelectionSP outSelection,
    const QSharedPointer<KoColor> referenceColor,
    KisPaintDeviceSP referenceDevice,
    const QRect &rect,
    KisPixelSelectionSP mask,
    QSharedPointer<KisProcessingVisitor::ProgressHelper> progressHelper
)
{
    if (rect.isEmpty()) {
        return {};
    }

    QVector<KisStrokeJobData*> jobsData;
    QVector<QRect> fillPatches =
        KritaUtils::splitRectIntoPatches(rect, KritaUtils::optimalPatchSize());
    const int threshold = fillThreshold();
    const int softness = 100 - opacitySpread();
    const int sizemod = this->sizemod();
    const bool stopGrowingAtDarkestPixel = this->stopGrowingAtDarkestPixel();
    const int feather = this->feather();
    const bool antiAlias = this->antiAlias();

    KritaUtils::addJobBarrier(jobsData, nullptr);

    for (const QRect &patch : fillPatches) {
        KritaUtils::addJobConcurrent(
            jobsData,
            [referenceDevice, outSelection, mask, referenceColor,
             threshold, softness, patch, progressHelper]() mutable
            {
                if (patch.isEmpty()) {
                    return;
                }

                KoUpdater *updater = progressHelper ? progressHelper->updater() : nullptr;

                using namespace KisColorSelectionPolicies;

                const int pixelSize = referenceDevice->pixelSize();
                KoColor srcColor(*referenceColor);
                srcColor.convertTo(referenceDevice->colorSpace());

                if (softness == 0) {
                    HardSelectionPolicy sp(threshold);
                    if (pixelSize == 1) {
                        OptimizedDifferencePolicy<quint8> dp(srcColor, threshold);
                        createSimilarColorsSelectionImpl(outSelection, referenceDevice, patch, mask, dp, sp, updater);
                    } else if (pixelSize == 2) {
                        OptimizedDifferencePolicy<quint16> dp(srcColor, threshold);
                        createSimilarColorsSelectionImpl(outSelection, referenceDevice, patch, mask, dp, sp, updater);
                    } else if (pixelSize == 4) {
                        OptimizedDifferencePolicy<quint32> dp(srcColor, threshold);
                        createSimilarColorsSelectionImpl(outSelection, referenceDevice, patch, mask, dp, sp, updater);
                    } else if (pixelSize == 8) {
                        OptimizedDifferencePolicy<quint64> dp(srcColor, threshold);
                        createSimilarColorsSelectionImpl(outSelection, referenceDevice, patch, mask, dp, sp, updater);
                    } else {
                        SlowDifferencePolicy dp(srcColor, threshold);
                        createSimilarColorsSelectionImpl(outSelection, referenceDevice, patch, mask, dp, sp, updater);
                    }
                } else {
                    SoftSelectionPolicy sp(threshold, softness);
                    if (pixelSize == 1) {
                        OptimizedDifferencePolicy<quint8> dp(srcColor, threshold);
                        createSimilarColorsSelectionImpl(outSelection, referenceDevice, patch, mask, dp, sp, updater);
                    } else if (pixelSize == 2) {
                        OptimizedDifferencePolicy<quint16> dp(srcColor, threshold);
                        createSimilarColorsSelectionImpl(outSelection, referenceDevice, patch, mask, dp, sp, updater);
                    } else if (pixelSize == 4) {
                        OptimizedDifferencePolicy<quint32> dp(srcColor, threshold);
                        createSimilarColorsSelectionImpl(outSelection, referenceDevice, patch, mask, dp, sp, updater);
                    } else if (pixelSize == 8) {
                        OptimizedDifferencePolicy<quint64> dp(srcColor, threshold);
                        createSimilarColorsSelectionImpl(outSelection, referenceDevice, patch, mask, dp, sp, updater);
                    } else {
                        SlowDifferencePolicy dp(srcColor, threshold);
                        createSimilarColorsSelectionImpl(outSelection, referenceDevice, patch, mask, dp, sp, updater);
                    }
                }
            }
        );
    }

    KritaUtils::addJobSequential(
        jobsData,
        [outSelection, referenceDevice, mask,
         sizemod, stopGrowingAtDarkestPixel, feather, antiAlias, progressHelper]() mutable
        {
            KoUpdater *updater = progressHelper ? progressHelper->updater() : nullptr;

            if (sizemod > 0) {
                if (stopGrowingAtDarkestPixel) {
                    KisGrowUntilDarkestPixelSelectionFilter biggy(sizemod, referenceDevice);
                    biggy.process(outSelection, outSelection->selectedRect().adjusted(-sizemod, -sizemod, sizemod, sizemod));
                } else {
                    KisGrowSelectionFilter biggy(sizemod, sizemod);
                    biggy.process(outSelection, outSelection->selectedRect().adjusted(-sizemod, -sizemod, sizemod, sizemod));
                }
            } else if (sizemod < 0) {
                KisShrinkSelectionFilter tiny(-sizemod, -sizemod, false);
                tiny.process(outSelection, outSelection->selectedRect());
            }
            if (updater) {
                updater->setProgress(33);
            }

            // Since the feathering already smooths the selection, the antiAlias
            // is not applied if we must feather
            if (feather > 0) {
                KisFeatherSelectionFilter feathery(feather);
                feathery.process(outSelection, outSelection->selectedRect().adjusted(-feather, -feather, feather, feather));
            } else if (antiAlias) {
                KisAntiAliasSelectionFilter antiAliasFilter;
                antiAliasFilter.process(outSelection, outSelection->selectedRect());
            }
            if (updater) {
                updater->setProgress(66);
            }

            if (mask) {
                outSelection->applySelection(mask, SELECTION_INTERSECT);
            }
            if (updater) {
                updater->setProgress(100);
            }
        }
    );

    return jobsData;
}
