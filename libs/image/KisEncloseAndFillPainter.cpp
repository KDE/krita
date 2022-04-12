/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2022 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <krita_utils.h>
#include <floodfill/kis_scanline_fill.h>
#include <kis_selection_filters.h>
#include <kis_iterator_ng.h>
#include <KoUpdater.h>

#include "KisEncloseAndFillPainter.h"

namespace KisEncloseAndFillPainterDetail {

    struct DifferencePolicyBase
    {
        const KoColorSpace *colorSpace;
        const KoColor color;
    };

    struct TransparentDifferencePolicy : public DifferencePolicyBase
    {
        TransparentDifferencePolicy(const KoColorSpace *colorSpace, const KoColor &color)
            : DifferencePolicyBase{colorSpace, color}
        {}
        // differences in the range [0, 100]
        quint8 getDifferenceFor(const quint8 *srcData) const
        {
            return static_cast<quint8>(colorSpace->opacityU8(srcData) * 100 / quint8_MAX);
        }
    };

    struct SpecificColorDifferencePolicy : public DifferencePolicyBase
    {
        SpecificColorDifferencePolicy(const KoColorSpace *colorSpace, const KoColor &color)
            : DifferencePolicyBase{colorSpace, color.convertedTo(colorSpace)}
        {}
        // differences in the range [0, 100]
        quint8 getDifferenceFor(const quint8 *srcData) const
        {
            return colorSpace->differenceA(srcData, color.data());
        }
    };

    struct TransparentForHalosDifferencePolicy : public DifferencePolicyBase
    {
        TransparentForHalosDifferencePolicy(const KoColorSpace *colorSpace, const KoColor &color)
            : DifferencePolicyBase{colorSpace, color.convertedTo(colorSpace)}
        {}
        // differences in the range [0, 100]
        quint8 getDifferenceFor(const quint8 *srcData) const
        {
            const quint8 opacity = colorSpace->opacityU8(srcData);
            if (opacity == quint8_MAX) {
                return 100;
            }
            const quint8 colorDifference = colorSpace->difference(srcData, color.data());
            const quint8 opacityDifference = opacity * 100 / quint8_MAX;
            return qMin(colorDifference, opacityDifference);
        }
    };

    struct SpecificColorOrTransparentDifferencePolicy : public DifferencePolicyBase
    {
        SpecificColorOrTransparentDifferencePolicy(const KoColorSpace *colorSpace, const KoColor &color)
            : DifferencePolicyBase{colorSpace, color.convertedTo(colorSpace)}
        {}
        // differences in the range [0, 100]
        quint8 getDifferenceFor(const quint8 *srcData) const
        {
            const quint8 colorDifference = colorSpace->difference(srcData, color.data());
            const quint8 opacityDifference = colorSpace->opacityU8(srcData) * 100 / quint8_MAX;
            return qMin(colorDifference, opacityDifference);
        }
    };

    template <typename DifferencePolicy>
    struct HardSelectionPolicy : public DifferencePolicy
    {
        const int threshold;
        HardSelectionPolicy(const KoColorSpace *colorSpace, const KoColor &color, int threshold)
            : DifferencePolicy(colorSpace, color)
            , threshold(threshold)
        {}
        // differences in the range [0, 100]
        quint8 getSelectionFor(const quint8 *srcData) const
        {
            return this->getDifferenceFor(srcData) <= threshold ? MAX_SELECTED : MIN_SELECTED;
        }
    };

    template <typename DifferencePolicy>
    struct SoftSelectionPolicy : public DifferencePolicy
    {
        const int threshold;
        const int softness;
        SoftSelectionPolicy(const KoColorSpace *colorSpace, const KoColor &color, int threshold, int softness)
            : DifferencePolicy(colorSpace, color)
            , threshold(threshold)
            , softness(softness)
        {}
        // differences in the range [0, 100]
        quint8 getSelectionFor(const quint8 *srcData) const
        {
            if (threshold == 0) {
                return MIN_SELECTED;
            }
            // Integer version of: (threshold - diff) / (threshold * softness)
            const int diff = this->getDifferenceFor(srcData);
            if (diff < threshold) {
                const int v = (threshold - diff) * MAX_SELECTED * 100 / (threshold * softness);
                return v > MAX_SELECTED ? MAX_SELECTED : v;
            } else {
                return MIN_SELECTED;
            }
        }
    };
};

class KisEncloseAndFillPainter::Private
{
public:
    KisEncloseAndFillPainter *q {nullptr};
    RegionSelectionMethod regionSelectionMethod {SelectAllRegions};
    KoColor regionSelectionColor;
    bool regionSelectionInvert {false};
    bool regionSelectionIncludeContourRegions {true};
    bool regionSelectionIncludeSurroundingRegions {true};

    Private(KisEncloseAndFillPainter *q) : q(q) {}
    
    void computeEnclosedRegionsMask(KisPixelSelectionSP resultMask,
                                    QRect *resultMaskRect,
                                    KisPixelSelectionSP enclosingMask,
                                    const QRect &enclosingMaskRect,
                                    KisPaintDeviceSP referenceDevice) const;

    void selectAllRegions(KisPixelSelectionSP resultMask,
                          QRect *resultMaskRect,
                          KisPixelSelectionSP enclosingMask,
                          const QRect &enclosingMaskRect,
                          KisPaintDeviceSP referenceDevice) const;

    void selectRegionsFilledWithSpecificColor(KisPixelSelectionSP resultMask,
                                              QRect *resultMaskRect,
                                              KisPixelSelectionSP enclosingMask,
                                              const QRect &enclosingMaskRect,
                                              KisPaintDeviceSP referenceDevice) const;
    void selectRegionsFilledWithTransparent(KisPixelSelectionSP resultMask,
                                            QRect *resultMaskRect,
                                            KisPixelSelectionSP enclosingMask,
                                            const QRect &enclosingMaskRect,
                                            KisPaintDeviceSP referenceDevice) const;
    void selectRegionsFilledWithSpecificColorOrTransparent(KisPixelSelectionSP resultMask,
                                                           QRect *resultMaskRect,
                                                           KisPixelSelectionSP enclosingMask,
                                                           const QRect &enclosingMaskRect,
                                                           KisPaintDeviceSP referenceDevice) const;
    template <typename SelectionPolicy>
    void selectRegionsFilledWithSpecificColorGeneric(KisPixelSelectionSP resultMask,
                                                     QRect *resultMaskRect,
                                                     KisPixelSelectionSP enclosingMask,
                                                     const QRect &enclosingMaskRect,
                                                     KisPaintDeviceSP referenceDevice,
                                                     SelectionPolicy selectionPolicy) const;

    void selectAllRegionsExceptFilledWithSpecificColor(KisPixelSelectionSP resultMask,
                                                       QRect *resultMaskRect,
                                                       KisPixelSelectionSP enclosingMask,
                                                       const QRect &enclosingMaskRect,
                                                       KisPaintDeviceSP referenceDevice) const;
    void selectAllRegionsExceptFilledWithTransparent(KisPixelSelectionSP resultMask,
                                                     QRect *resultMaskRect,
                                                     KisPixelSelectionSP enclosingMask,
                                                     const QRect &enclosingMaskRect,
                                                     KisPaintDeviceSP referenceDevice) const;
    void selectAllRegionsExceptFilledWithSpecificColorOrTransparent(KisPixelSelectionSP resultMask,
                                                                    QRect *resultMaskRect,
                                                                    KisPixelSelectionSP enclosingMask,
                                                                    const QRect &enclosingMaskRect,
                                                                    KisPaintDeviceSP referenceDevice) const;
    template <typename SelectionPolicy>
    void selectAllRegionsExceptFilledWithSpecificColorGeneric(KisPixelSelectionSP resultMask,
                                                              QRect *resultMaskRect,
                                                              KisPixelSelectionSP enclosingMask,
                                                              const QRect &enclosingMaskRect,
                                                              KisPaintDeviceSP referenceDevice,
                                                              SelectionPolicy selectionPolicy) const;

    void selectRegionsSurroundedBySpecificColor(KisPixelSelectionSP resultMask,
                                                QRect *resultMaskRect,
                                                KisPixelSelectionSP enclosingMask,
                                                const QRect &enclosingMaskRect,
                                                KisPaintDeviceSP referenceDevice) const;
    void selectRegionsSurroundedByTransparent(KisPixelSelectionSP resultMask,
                                              QRect *resultMaskRect,
                                              KisPixelSelectionSP enclosingMask,
                                              const QRect &enclosingMaskRect,
                                              KisPaintDeviceSP referenceDevice) const;
    void selectRegionsSurroundedBySpecificColorOrTransparent(KisPixelSelectionSP resultMask,
                                                             QRect *resultMaskRect,
                                                             KisPixelSelectionSP enclosingMask,
                                                             const QRect &enclosingMaskRect,
                                                             KisPaintDeviceSP referenceDevice) const;
    template <typename SelectionPolicy>
    void selectRegionsSurroundedBySpecificColorGeneric(KisPixelSelectionSP resultMask,
                                                       QRect *resultMaskRect,
                                                       KisPixelSelectionSP enclosingMask,
                                                       const QRect &enclosingMaskRect,
                                                       KisPaintDeviceSP referenceDevice,
                                                       SelectionPolicy selectionPolicy,
                                                       bool colorOrTransparent = false) const;

    QVector<QPoint> getEnclosingContourPoints(KisPixelSelectionSP enclosingMask,
                                              const QRect &enclosingMaskRect) const;

    void applyPostProcessing(KisPixelSelectionSP mask) const;

    void invertIfNeeded(KisPixelSelectionSP resultMask, KisPixelSelectionSP enclosingMask) const;

    template <typename SelectionPolicy>
    int selectSimilarRegions(KisPixelSelectionSP resultMask,
                             KisPixelSelectionSP enclosingMask,
                             const QRect &enclosingMaskRect,
                             KisPaintDeviceSP referenceDevice,
                             SelectionPolicy selectionPolicy) const;
    template <typename SelectionPolicy>
    int selectDissimilarRegions(KisPixelSelectionSP resultMask,
                                KisPixelSelectionSP enclosingMask,
                                const QRect &enclosingMaskRect,
                                KisPaintDeviceSP referenceDevice,
                                SelectionPolicy selectionPolicy) const;

    void selectRegionsFromContour(KisPixelSelectionSP resultMask,
                                  KisPixelSelectionSP enclosingMask,
                                  const QRect &enclosingMaskRect,
                                  KisPaintDeviceSP referenceDevice) const;
    void selectRegionsFromContour(KisPixelSelectionSP resultMask,
                                  KisPixelSelectionSP enclosingMask,
                                  const QVector<QPoint> &enclosingPoints,
                                  const QRect &enclosingMaskRect,
                                  KisPaintDeviceSP referenceDevice) const;

    void selectRegionsFromContourUntilColor(KisPixelSelectionSP resultMask,
                                            KisPixelSelectionSP enclosingMask,
                                            const QRect &enclosingMaskRect,
                                            KisPaintDeviceSP referenceDevice,
                                            const KoColor &color) const;
    void selectRegionsFromContourUntilColor(KisPixelSelectionSP resultMask,
                                            KisPixelSelectionSP enclosingMask,
                                            const QVector<QPoint> &enclosingPoints,
                                            const QRect &enclosingMaskRect,
                                            KisPaintDeviceSP referenceDevice,
                                            const KoColor &color) const;

    void selectRegionsFromContourUntilColorOrTransparent(KisPixelSelectionSP resultMask,
                                                         KisPixelSelectionSP enclosingMask,
                                                         const QRect &enclosingMaskRect,
                                                         KisPaintDeviceSP referenceDevice,
                                                         const KoColor &color) const;
    void selectRegionsFromContourUntilColorOrTransparent(KisPixelSelectionSP resultMask,
                                                         KisPixelSelectionSP enclosingMask,
                                                         const QVector<QPoint> &enclosingPoints,
                                                         const QRect &enclosingMaskRect,
                                                         KisPaintDeviceSP referenceDevice,
                                                         const KoColor &color) const;

    void removeContourRegions(KisPixelSelectionSP resultMask,
                              KisPixelSelectionSP enclosingMask,
                              const QRect &enclosingMaskRect) const;
    void removeContourRegions(KisPixelSelectionSP resultMask,
                              const QVector<QPoint> &enclosingPoints,
                              const QRect &enclosingMaskRect) const;

    void subtractSelectionsSpecial(KisPixelSelectionSP mask1,
                                   KisPixelSelectionSP mask2,
                                   const QRect &rect) const;
};

KisEncloseAndFillPainter::KisEncloseAndFillPainter()
    : m_d(new Private(this))
{}

KisEncloseAndFillPainter::KisEncloseAndFillPainter(KisPaintDeviceSP device)
    : KisFillPainter(device)
    , m_d(new Private(this))
{}

KisEncloseAndFillPainter::KisEncloseAndFillPainter(KisPaintDeviceSP device, KisSelectionSP selection)
    : KisFillPainter(device, selection)
    , m_d(new Private(this))
{}

KisEncloseAndFillPainter::~KisEncloseAndFillPainter()
{}

void KisEncloseAndFillPainter::encloseAndFillColor(KisPixelSelectionSP enclosingMask, KisPaintDeviceSP referenceDevice)
{
    genericEncloseAndFillStart(enclosingMask, referenceDevice);

    // Now create a layer and fill it
    KisPaintDeviceSP filled = device()->createCompositionSourceDevice();
    const QRect fillRect = currentFillSelection()->selectedExactRect();
    Q_CHECK_PTR(filled);
    KisFillPainter painter(filled);
    painter.fillRect(fillRect, paintColor());
    painter.end();

    genericEncloseAndFillEnd(filled);
}

void KisEncloseAndFillPainter::encloseAndFillPattern(KisPixelSelectionSP enclosingMask,
                                                     KisPaintDeviceSP referenceDevice,
                                                     QTransform patternTransform)
{
    genericEncloseAndFillStart(enclosingMask, referenceDevice);

    // Now create a layer and fill it
    KisPaintDeviceSP filled = device()->createCompositionSourceDevice();
    const QRect fillRect = currentFillSelection()->selectedExactRect();
    Q_CHECK_PTR(filled);
    KisFillPainter painter(filled);
    painter.fillRectNoCompose(fillRect, pattern(), patternTransform);
    painter.end();

    genericEncloseAndFillEnd(filled);
}

void KisEncloseAndFillPainter::genericEncloseAndFillStart(KisPixelSelectionSP enclosingMask, KisPaintDeviceSP referenceDevice)
{
    // Create a selection from the closed regions
    KisPixelSelectionSP pixelSelection = createEncloseAndFillSelection(enclosingMask, referenceDevice,
                                                                       (selection().isNull() ? 0 : selection()->pixelSelection()));
    KisSelectionSP newSelection = new KisSelection(pixelSelection->defaultBounds());
    newSelection->pixelSelection()->applySelection(pixelSelection, SELECTION_REPLACE);
    setCurrentFillSelection(newSelection);
}

void KisEncloseAndFillPainter::genericEncloseAndFillEnd(KisPaintDeviceSP filled)
{
    KisFillPainter::genericFillEnd(filled);
}

KisPixelSelectionSP KisEncloseAndFillPainter::createEncloseAndFillSelection(KisPixelSelectionSP enclosingMask,
                                                                            KisPaintDeviceSP referenceDevice,
                                                                            KisPixelSelectionSP existingSelection)
{
    KisPixelSelectionSP newSelection = new KisPixelSelection(new KisSelectionDefaultBounds(device()));
    return createEncloseAndFillSelection(newSelection, enclosingMask, referenceDevice, existingSelection);
}

KisPixelSelectionSP KisEncloseAndFillPainter::createEncloseAndFillSelection(KisPixelSelectionSP newSelection,
                                                                            KisPixelSelectionSP enclosingMask,
                                                                            KisPaintDeviceSP referenceDevice,
                                                                            KisPixelSelectionSP existingSelection)
{
    Q_ASSERT(newSelection);
    Q_ASSERT(enclosingMask);
    Q_ASSERT(referenceDevice);

    const QRect enclosingMaskRect = enclosingMask->selectedExactRect();
    if (enclosingMaskRect.isEmpty()) {
        return newSelection;
    }
    QRect newSelectionRect;
    // Get the mask that includes all the closed regions inside the enclosing mask
    m_d->computeEnclosedRegionsMask(newSelection, &newSelectionRect, enclosingMask, enclosingMaskRect, referenceDevice);
    if (newSelectionRect.isEmpty()) {
        return newSelection;
    }
    // Invert
    m_d->invertIfNeeded(newSelection, enclosingMask);
    // Intersect the regions mask with the current selection if it should be used as boundary
    if (useSelectionAsBoundary() && existingSelection) {
        newSelection->applySelection(existingSelection, SELECTION_INTERSECT);
    }
    // Post-process
    m_d->applyPostProcessing(newSelection);

    return newSelection;
}

void KisEncloseAndFillPainter::setRegionSelectionMethod(RegionSelectionMethod regionSelectionMethod)
{
    m_d->regionSelectionMethod = regionSelectionMethod;
}

KisEncloseAndFillPainter::RegionSelectionMethod KisEncloseAndFillPainter::regionSelectionMethod() const
{
    return m_d->regionSelectionMethod;
}

void KisEncloseAndFillPainter::setRegionSelectionColor(const KoColor &color)
{
    m_d->regionSelectionColor = color;
}

KoColor KisEncloseAndFillPainter::regionSelectionColor() const
{
    return m_d->regionSelectionColor;
}

void KisEncloseAndFillPainter::setRegionSelectionInvert(bool invert)
{
    m_d->regionSelectionInvert = invert;
}

bool KisEncloseAndFillPainter::regionSelectionInvert() const
{
    return m_d->regionSelectionInvert;
}

void KisEncloseAndFillPainter::setRegionSelectionIncludeContourRegions(bool include)
{
    m_d->regionSelectionIncludeContourRegions = include;
}

bool KisEncloseAndFillPainter::regionSelectionIncludeContourRegions() const
{
    return m_d->regionSelectionIncludeContourRegions;
}

void KisEncloseAndFillPainter::setRegionSelectionIncludeSurroundingRegions(bool include)
{
    m_d->regionSelectionIncludeSurroundingRegions = include;
}

bool KisEncloseAndFillPainter::regionSelectionIncludeSurroundingRegions() const
{
    return m_d->regionSelectionIncludeSurroundingRegions;
}

void KisEncloseAndFillPainter::Private::computeEnclosedRegionsMask(KisPixelSelectionSP resultMask,
                                                                   QRect *resultMaskRect,
                                                                   KisPixelSelectionSP enclosingMask,
                                                                   const QRect &enclosingMaskRect,
                                                                   KisPaintDeviceSP referenceDevice) const
{
    // Create the regions mask
    switch (regionSelectionMethod) {
        case SelectAllRegions:
            selectAllRegions(resultMask, resultMaskRect, enclosingMask, enclosingMaskRect, referenceDevice);
            break;
        case SelectRegionsFilledWithSpecificColor:
            selectRegionsFilledWithSpecificColor(resultMask, resultMaskRect, enclosingMask, enclosingMaskRect, referenceDevice);
            break;
        case SelectRegionsFilledWithTransparent:
            selectRegionsFilledWithTransparent(resultMask, resultMaskRect, enclosingMask, enclosingMaskRect, referenceDevice);
            break;
        case SelectRegionsFilledWithSpecificColorOrTransparent:
            selectRegionsFilledWithSpecificColorOrTransparent(resultMask, resultMaskRect, enclosingMask, enclosingMaskRect, referenceDevice);
            break;
        case SelectAllRegionsExceptFilledWithSpecificColor:
            selectAllRegionsExceptFilledWithSpecificColor(resultMask, resultMaskRect, enclosingMask, enclosingMaskRect, referenceDevice);
            break;
        case SelectAllRegionsExceptFilledWithTransparent:
            selectAllRegionsExceptFilledWithTransparent(resultMask, resultMaskRect, enclosingMask, enclosingMaskRect, referenceDevice);
            break;
        case SelectAllRegionsExceptFilledWithSpecificColorOrTransparent:
            selectAllRegionsExceptFilledWithSpecificColorOrTransparent(resultMask, resultMaskRect, enclosingMask, enclosingMaskRect, referenceDevice);
            break;
        case SelectRegionsSurroundedBySpecificColor:
            selectRegionsSurroundedBySpecificColor(resultMask, resultMaskRect, enclosingMask, enclosingMaskRect, referenceDevice);
            break;
        case SelectRegionsSurroundedByTransparent:
            selectRegionsSurroundedByTransparent(resultMask, resultMaskRect, enclosingMask, enclosingMaskRect, referenceDevice);
            break;
        case SelectRegionsSurroundedBySpecificColorOrTransparent:
            selectRegionsSurroundedBySpecificColorOrTransparent(resultMask, resultMaskRect, enclosingMask, enclosingMaskRect, referenceDevice);
            break;
        default: return;
    }
}

void KisEncloseAndFillPainter::Private::selectAllRegions(KisPixelSelectionSP resultMask,
                                                         QRect *resultMaskRect,
                                                         KisPixelSelectionSP enclosingMask,
                                                         const QRect &enclosingMaskRect,
                                                         KisPaintDeviceSP referenceDevice) const
{
    // For performance reasons, this function outputs an inverted version of the enclosed regions
    // Here we just fill all the connected regions from the border towards inside
    selectRegionsFromContour(resultMask, enclosingMask, enclosingMaskRect, referenceDevice);
    if (resultMaskRect) {
        *resultMaskRect = resultMask->selectedExactRect();
    }
}

void KisEncloseAndFillPainter::Private::selectRegionsFilledWithSpecificColor(KisPixelSelectionSP resultMask,
                                                                             QRect *resultMaskRect,
                                                                             KisPixelSelectionSP enclosingMask,
                                                                             const QRect &enclosingMaskRect,
                                                                             KisPaintDeviceSP referenceDevice) const
{
    using namespace KisEncloseAndFillPainterDetail;
    const int softness = 100 - q->opacitySpread();
    if (softness == 0) {
        HardSelectionPolicy<SpecificColorDifferencePolicy> policy(referenceDevice->colorSpace(), regionSelectionColor, q->fillThreshold());
        selectRegionsFilledWithSpecificColorGeneric(resultMask, resultMaskRect, enclosingMask, enclosingMaskRect, referenceDevice, policy);
    } else {
        SoftSelectionPolicy<SpecificColorDifferencePolicy> policy(referenceDevice->colorSpace(), regionSelectionColor, q->fillThreshold(), softness);
        selectRegionsFilledWithSpecificColorGeneric(resultMask, resultMaskRect, enclosingMask, enclosingMaskRect, referenceDevice, policy);
    }
}

void KisEncloseAndFillPainter::Private::selectRegionsFilledWithTransparent(KisPixelSelectionSP resultMask,
                                                                           QRect *resultMaskRect,
                                                                           KisPixelSelectionSP enclosingMask,
                                                                           const QRect &enclosingMaskRect,
                                                                           KisPaintDeviceSP referenceDevice) const
{
    using namespace KisEncloseAndFillPainterDetail;
    const int softness = 100 - q->opacitySpread();
    if (softness == 0) {
        HardSelectionPolicy<TransparentDifferencePolicy> policy(referenceDevice->colorSpace(), regionSelectionColor, q->fillThreshold());
        selectRegionsFilledWithSpecificColorGeneric(resultMask, resultMaskRect, enclosingMask, enclosingMaskRect, referenceDevice, policy);
    } else {
        SoftSelectionPolicy<TransparentDifferencePolicy> policy(referenceDevice->colorSpace(), regionSelectionColor, q->fillThreshold(), softness);
        selectRegionsFilledWithSpecificColorGeneric(resultMask, resultMaskRect, enclosingMask, enclosingMaskRect, referenceDevice, policy);
    }
}

void KisEncloseAndFillPainter::Private::selectRegionsFilledWithSpecificColorOrTransparent(KisPixelSelectionSP resultMask,
                                                                                          QRect *resultMaskRect,
                                                                                          KisPixelSelectionSP enclosingMask,
                                                                                          const QRect &enclosingMaskRect,
                                                                                          KisPaintDeviceSP referenceDevice) const
{
    using namespace KisEncloseAndFillPainterDetail;
    const int softness = 100 - q->opacitySpread();
    // Here we must compute the specific color and transparent masks separately
    // so that the contour regions can be removed independently if they are
    // transparent or of the specific color and are connected
    KisPixelSelectionSP resultMaskTransparent = new KisPixelSelection(new KisSelectionDefaultBounds(resultMask));
    if (softness == 0) {
        HardSelectionPolicy<SpecificColorDifferencePolicy> colorPolicy(referenceDevice->colorSpace(), regionSelectionColor, q->fillThreshold());
        selectRegionsFilledWithSpecificColorGeneric(resultMask, nullptr, enclosingMask, enclosingMaskRect, referenceDevice, colorPolicy);
        HardSelectionPolicy<TransparentForHalosDifferencePolicy> transparentPolicy(referenceDevice->colorSpace(), regionSelectionColor, q->fillThreshold());
        selectRegionsFilledWithSpecificColorGeneric(resultMaskTransparent, nullptr, enclosingMask, enclosingMaskRect, referenceDevice, transparentPolicy);
    } else {
        SoftSelectionPolicy<SpecificColorDifferencePolicy> colorPolicy(referenceDevice->colorSpace(), regionSelectionColor, q->fillThreshold(), softness);
        selectRegionsFilledWithSpecificColorGeneric(resultMask, nullptr, enclosingMask, enclosingMaskRect, referenceDevice, colorPolicy);
        HardSelectionPolicy<TransparentForHalosDifferencePolicy> transparentPolicy(referenceDevice->colorSpace(), regionSelectionColor, q->fillThreshold());
        selectRegionsFilledWithSpecificColorGeneric(resultMaskTransparent, nullptr, enclosingMask, enclosingMaskRect, referenceDevice, transparentPolicy);
    }
    // Compose the masks
    resultMask->applySelection(resultMaskTransparent, SELECTION_ADD);
    if (resultMaskRect) {
        *resultMaskRect = resultMask->selectedExactRect();
    }
}

template <typename SelectionPolicy>
void KisEncloseAndFillPainter::Private::selectRegionsFilledWithSpecificColorGeneric(KisPixelSelectionSP resultMask,
                                                                                    QRect *resultMaskRect,
                                                                                    KisPixelSelectionSP enclosingMask,
                                                                                    const QRect &enclosingMaskRect,
                                                                                    KisPaintDeviceSP referenceDevice,
                                                                                    SelectionPolicy selectionPolicy) const
{
    // Select all the pixels using the given selection policy and
    // return if there are no selected pixels
    if (selectSimilarRegions(resultMask, enclosingMask, enclosingMaskRect, referenceDevice, selectionPolicy) == 0) {
        if (resultMaskRect) {
            *resultMaskRect = QRect();
        }
        return;
    }
    // Remove the regions that touch the enclosing area
    if (!regionSelectionIncludeContourRegions) {
        removeContourRegions(resultMask, enclosingMask, enclosingMaskRect);
    }
    if (resultMaskRect) {
        *resultMaskRect = resultMask->selectedExactRect();
    }
}

void KisEncloseAndFillPainter::Private::selectAllRegionsExceptFilledWithSpecificColor(KisPixelSelectionSP resultMask,
                                                                                      QRect *resultMaskRect,
                                                                                      KisPixelSelectionSP enclosingMask,
                                                                                      const QRect &enclosingMaskRect,
                                                                                      KisPaintDeviceSP referenceDevice) const
{
    using namespace KisEncloseAndFillPainterDetail;
    const int softness = 100 - q->opacitySpread();
    if (softness == 0) {
        HardSelectionPolicy<SpecificColorDifferencePolicy> policy(referenceDevice->colorSpace(), regionSelectionColor, q->fillThreshold());
        selectAllRegionsExceptFilledWithSpecificColorGeneric(resultMask, resultMaskRect, enclosingMask, enclosingMaskRect, referenceDevice, policy);
    } else {
        SoftSelectionPolicy<SpecificColorDifferencePolicy> policy(referenceDevice->colorSpace(), regionSelectionColor, q->fillThreshold(), softness);
        selectAllRegionsExceptFilledWithSpecificColorGeneric(resultMask, resultMaskRect, enclosingMask, enclosingMaskRect, referenceDevice, policy);
    }
}

void KisEncloseAndFillPainter::Private::selectAllRegionsExceptFilledWithTransparent(KisPixelSelectionSP resultMask,
                                                                                    QRect *resultMaskRect,
                                                                                    KisPixelSelectionSP enclosingMask,
                                                                                    const QRect &enclosingMaskRect,
                                                                                    KisPaintDeviceSP referenceDevice) const
{
    using namespace KisEncloseAndFillPainterDetail;
    const int softness = 100 - q->opacitySpread();
    if (softness == 0) {
        HardSelectionPolicy<TransparentDifferencePolicy> policy(referenceDevice->colorSpace(), KoColor(QColor(Qt::transparent), referenceDevice->colorSpace()), q->fillThreshold());
        selectAllRegionsExceptFilledWithSpecificColorGeneric(resultMask, resultMaskRect, enclosingMask, enclosingMaskRect, referenceDevice, policy);
    } else {
        SoftSelectionPolicy<TransparentDifferencePolicy> policy(referenceDevice->colorSpace(), KoColor(QColor(Qt::transparent), referenceDevice->colorSpace()), q->fillThreshold(), softness);
        selectAllRegionsExceptFilledWithSpecificColorGeneric(resultMask, resultMaskRect, enclosingMask, enclosingMaskRect, referenceDevice, policy);
    }
}

void KisEncloseAndFillPainter::Private::selectAllRegionsExceptFilledWithSpecificColorOrTransparent(KisPixelSelectionSP resultMask,
                                                                                                   QRect *resultMaskRect,
                                                                                                   KisPixelSelectionSP enclosingMask,
                                                                                                   const QRect &enclosingMaskRect,
                                                                                                   KisPaintDeviceSP referenceDevice) const
{
    using namespace KisEncloseAndFillPainterDetail;
    const int softness = 100 - q->opacitySpread();
    // Here we must compute the specific color and transparent masks separately
    if (softness == 0) {
        HardSelectionPolicy<SpecificColorOrTransparentDifferencePolicy> policy(referenceDevice->colorSpace(), regionSelectionColor, q->fillThreshold());
        selectAllRegionsExceptFilledWithSpecificColorGeneric(resultMask, resultMaskRect, enclosingMask, enclosingMaskRect, referenceDevice, policy);
    } else {
        SoftSelectionPolicy<SpecificColorOrTransparentDifferencePolicy> policy(referenceDevice->colorSpace(), regionSelectionColor, q->fillThreshold(), softness);
        selectAllRegionsExceptFilledWithSpecificColorGeneric(resultMask, resultMaskRect, enclosingMask, enclosingMaskRect, referenceDevice, policy);
    }
}

template <typename SelectionPolicy>
void KisEncloseAndFillPainter::Private::selectAllRegionsExceptFilledWithSpecificColorGeneric(KisPixelSelectionSP resultMask,
                                                                                             QRect *resultMaskRect,
                                                                                             KisPixelSelectionSP enclosingMask,
                                                                                             const QRect &enclosingMaskRect,
                                                                                             KisPaintDeviceSP referenceDevice,
                                                                                             SelectionPolicy selectionPolicy) const
{
    const QVector<QPoint> enclosingPoints = getEnclosingContourPoints(enclosingMask, enclosingMaskRect);
    // Remove the regions that touch the enclosing area
    if (selectDissimilarRegions(resultMask, enclosingMask, enclosingMaskRect, referenceDevice, selectionPolicy) == 0) {
        if (resultMaskRect) {
            *resultMaskRect = QRect();
        }
        return;
    }
    if (!regionSelectionIncludeContourRegions) {
        // Here we don't use removeContourRegions because the contour regions
        // in the mask may include multiple connected color regions
        KisPixelSelectionSP mask = new KisPixelSelection(new KisSelectionDefaultBounds(enclosingMask));
        selectRegionsFromContour(mask, enclosingMask, enclosingPoints, enclosingMaskRect, referenceDevice);
        resultMask->applySelection(mask, SELECTION_SUBTRACT);
    }
    if (resultMaskRect) {
        *resultMaskRect = resultMask->selectedExactRect();
    }
}

void KisEncloseAndFillPainter::Private::selectRegionsSurroundedBySpecificColor(KisPixelSelectionSP resultMask,
                                                                               QRect *resultMaskRect,
                                                                               KisPixelSelectionSP enclosingMask,
                                                                               const QRect &enclosingMaskRect,
                                                                               KisPaintDeviceSP referenceDevice) const
{
    using namespace KisEncloseAndFillPainterDetail;
    const int softness = 100 - q->opacitySpread();
    if (softness == 0) {
        HardSelectionPolicy<SpecificColorDifferencePolicy> policy(referenceDevice->colorSpace(), regionSelectionColor, q->fillThreshold());
        selectRegionsSurroundedBySpecificColorGeneric(resultMask, resultMaskRect, enclosingMask, enclosingMaskRect, referenceDevice, policy);
    } else {
        SoftSelectionPolicy<SpecificColorDifferencePolicy> policy(referenceDevice->colorSpace(), regionSelectionColor, q->fillThreshold(), softness);
        selectRegionsSurroundedBySpecificColorGeneric(resultMask, resultMaskRect, enclosingMask, enclosingMaskRect, referenceDevice, policy);
    }
}

void KisEncloseAndFillPainter::Private::selectRegionsSurroundedByTransparent(KisPixelSelectionSP resultMask,
                                                                             QRect *resultMaskRect,
                                                                             KisPixelSelectionSP enclosingMask,
                                                                             const QRect &enclosingMaskRect,
                                                                             KisPaintDeviceSP referenceDevice) const
{
    using namespace KisEncloseAndFillPainterDetail;
    const int softness = 100 - q->opacitySpread();
    if (softness == 0) {
        HardSelectionPolicy<TransparentDifferencePolicy> policy(referenceDevice->colorSpace(), KoColor(QColor(Qt::transparent), referenceDevice->colorSpace()), q->fillThreshold());
        selectRegionsSurroundedBySpecificColorGeneric(resultMask, resultMaskRect, enclosingMask, enclosingMaskRect, referenceDevice, policy);
    } else {
        SoftSelectionPolicy<TransparentDifferencePolicy> policy(referenceDevice->colorSpace(), KoColor(QColor(Qt::transparent), referenceDevice->colorSpace()), q->fillThreshold(), softness);
        selectRegionsSurroundedBySpecificColorGeneric(resultMask, resultMaskRect, enclosingMask, enclosingMaskRect, referenceDevice, policy);
    }
}

void KisEncloseAndFillPainter::Private::selectRegionsSurroundedBySpecificColorOrTransparent(KisPixelSelectionSP resultMask,
                                                                                            QRect *resultMaskRect,
                                                                                            KisPixelSelectionSP enclosingMask,
                                                                                            const QRect &enclosingMaskRect,
                                                                                            KisPaintDeviceSP referenceDevice) const
{
    using namespace KisEncloseAndFillPainterDetail;
    const int softness = 100 - q->opacitySpread();
    // Here we must compute the specific color and transparent masks separately
    KisPixelSelectionSP resultMaskTransparent = new KisPixelSelection(new KisSelectionDefaultBounds(resultMask));
    if (softness == 0) {
        HardSelectionPolicy<SpecificColorOrTransparentDifferencePolicy> colorPolicy(referenceDevice->colorSpace(), regionSelectionColor, q->fillThreshold());
        selectRegionsSurroundedBySpecificColorGeneric(resultMask, resultMaskRect, enclosingMask, enclosingMaskRect, referenceDevice, colorPolicy, true);
    } else {
        SoftSelectionPolicy<SpecificColorOrTransparentDifferencePolicy> colorPolicy(referenceDevice->colorSpace(), regionSelectionColor, q->fillThreshold(), softness);
        selectRegionsSurroundedBySpecificColorGeneric(resultMask, resultMaskRect, enclosingMask, enclosingMaskRect, referenceDevice, colorPolicy, true);
    }
}

template <typename SelectionPolicy>
void KisEncloseAndFillPainter::Private::selectRegionsSurroundedBySpecificColorGeneric(KisPixelSelectionSP resultMask,
                                                                                      QRect *resultMaskRect,
                                                                                      KisPixelSelectionSP enclosingMask,
                                                                                      const QRect &enclosingMaskRect,
                                                                                      KisPaintDeviceSP referenceDevice,
                                                                                      SelectionPolicy selectionPolicy,
                                                                                      bool colorOrTransparent) const
{
    // Get the enclosing mask contour points
    const QVector<QPoint> enclosingPoints = getEnclosingContourPoints(enclosingMask, enclosingMaskRect);
    if (enclosingPoints.isEmpty()) {
        return;
    }
    // Here we just fill all the areas from the border towards inside until the specific color
    if (colorOrTransparent) {
        selectRegionsFromContourUntilColorOrTransparent(resultMask, enclosingMask, enclosingPoints, enclosingMaskRect, referenceDevice, selectionPolicy.color);
    } else {
        selectRegionsFromContourUntilColor(resultMask, enclosingMask, enclosingPoints, enclosingMaskRect, referenceDevice, selectionPolicy.color);
    }
    // Invert the mask since it contains the regions surrounding the regions we want
    resultMask->invert();
    // Since, after inverting, the mask includes the region outside the enclosing
    // mask, we must intersect the current mask with the enclosing mask. The result
    // is a mask that includes all the closed regions inside the enclosing mask
    resultMask->applySelection(enclosingMask, SELECTION_INTERSECT);
    // Remove the surrounding regions, if needed
    if (!regionSelectionIncludeSurroundingRegions) {
        KisPixelSelectionSP mask = new KisPixelSelection(new KisSelectionDefaultBounds(enclosingMask));
        selectSimilarRegions(mask, enclosingMask, enclosingMaskRect, referenceDevice, selectionPolicy);
        resultMask->applySelection(mask, SELECTION_SUBTRACT);
    }
    // Remove the regions that touch the enclosing area
    removeContourRegions(resultMask, enclosingPoints, enclosingMaskRect);
    if (resultMaskRect) {
        *resultMaskRect = resultMask->selectedExactRect();
    }
}

QVector<QPoint> KisEncloseAndFillPainter::Private::getEnclosingContourPoints(KisPixelSelectionSP enclosingMask,
                                                                             const QRect &enclosingMaskRect) const
{
    QVector<QPoint> enclosingPoints;
    const int scanlineWidth = enclosingMaskRect.width() + 2;
    QVector<quint8> buffer(scanlineWidth * 3);
    quint8 *scanlines[3] = {buffer.data(), buffer.data() + scanlineWidth, buffer.data() + scanlineWidth * 2};
    // Initialize the buffer
    // Top, outside row
    memset(scanlines[0], MIN_SELECTED, scanlineWidth);
    // Middle row
    // Left, outside pixel
    *(scanlines[1]) = MIN_SELECTED;
    // Middle, inside pixels
    enclosingMask->readBytes(scanlines[1] + 1, enclosingMaskRect.x(), enclosingMaskRect.y(), enclosingMaskRect.width(), 1);
    // Right, outside pixel
    *(scanlines[2] - 1) = MIN_SELECTED;
    // Bottom row
    if (enclosingMaskRect.height() == 1) {
        // Bottom, outside row
        memset(scanlines[2], MIN_SELECTED, scanlineWidth);
    } else {
        // Left, outside pixel
        *(scanlines[2]) = MIN_SELECTED;
        // Middle, inside pixels
        enclosingMask->readBytes(scanlines[2] + 1, enclosingMaskRect.x(), enclosingMaskRect.y() + 1, enclosingMaskRect.width(), 1);
        // Right, outside pixel
        *(scanlines[2] + scanlineWidth - 1) = MIN_SELECTED;
    }

    for (int y = 0; y < enclosingMaskRect.height(); ++y) {
        if (y > 0) {
            // Rotate pointers
            quint8 *tmp = scanlines[0];
            scanlines[0] = scanlines[1];
            scanlines[1] = scanlines[2];
            scanlines[2] = tmp;
            // Read new row
            if (y == enclosingMaskRect.height() - 1) {
                // Bottom, outside row
                memset(scanlines[2], MIN_SELECTED, scanlineWidth);
            } else {
                // Left, outside pixel
                *(scanlines[2]) = MIN_SELECTED;
                // Middle, inside pixels
                enclosingMask->readBytes(scanlines[2] + 1, enclosingMaskRect.x(), enclosingMaskRect.y() + y + 1, enclosingMaskRect.width(), 1);
                // Right, outside pixel
                *(scanlines[2] + scanlineWidth - 1) = MIN_SELECTED;
            }
        }
        const quint8 *topPixel = scanlines[0] + 1;
        const quint8 *middlePixel = scanlines[1] + 1;
        const quint8 *bottomPixel = scanlines[2] + 1;
        for (int x = 0; x < enclosingMaskRect.width(); ++x, ++topPixel, ++middlePixel, ++bottomPixel) {
            // Continue if the current pixel is not in the selection
            if (*middlePixel == MIN_SELECTED) {
                continue;
            }
            // Get all eight neighbor pixels. If at least one of them is not
            // in the selection then the current pixel is a border pixel and
            // we add it to the list
            quint8 neighbors = 0;
            neighbors |= (*(topPixel    - 1) == MIN_SELECTED) << 7;
            neighbors |= (*(topPixel       ) == MIN_SELECTED) << 6;
            neighbors |= (*(topPixel    + 1) == MIN_SELECTED) << 5;
            neighbors |= (*(middlePixel - 1) == MIN_SELECTED) << 4;
            neighbors |= (*(middlePixel + 1) == MIN_SELECTED) << 3;
            neighbors |= (*(bottomPixel - 1) == MIN_SELECTED) << 2;
            neighbors |= (*(bottomPixel    ) == MIN_SELECTED) << 1;
            neighbors |= (*(bottomPixel + 1) == MIN_SELECTED) << 0;
            if (neighbors != 0) {
                enclosingPoints.push_back(QPoint(enclosingMaskRect.x() + x, enclosingMaskRect.y() + y));
            }
        }
    }

    return enclosingPoints;
}

void KisEncloseAndFillPainter::Private::applyPostProcessing(KisPixelSelectionSP mask) const
{
    if (q->sizemod() > 0) {
        KisGrowSelectionFilter biggy(q->sizemod(), q->sizemod());
        biggy.process(mask, mask->selectedRect().adjusted(-q->sizemod(), -q->sizemod(), q->sizemod(), q->sizemod()));
    } else if (q->sizemod() < 0) {
        KisShrinkSelectionFilter tiny(-q->sizemod(), -q->sizemod(), false);
        tiny.process(mask, mask->selectedRect());
    }
    // Since the feathering already smooths the selection, the antiAlias
    // is not applied if we must feather
    if (q->feather() > 0) {
        KisFeatherSelectionFilter feathery(q->feather());
        feathery.process(mask, mask->selectedRect().adjusted(-q->feather(), -q->feather(), q->feather(), q->feather()));
    } else if (q->antiAlias()) {
        KisAntiAliasSelectionFilter antiAliasFilter;
        antiAliasFilter.process(mask, mask->selectedRect());
    }
}

void KisEncloseAndFillPainter::Private::invertIfNeeded(KisPixelSelectionSP resultMask, KisPixelSelectionSP enclosingMask) const
{
    if (regionSelectionMethod == SelectAllRegions) {
        // Return if the mask should be inverted since here it is already inverted
        if (regionSelectionInvert) {
            return;
        }
    } else if (!regionSelectionInvert) {
        return;
    }
    resultMask->invert();
    // Since, after inverting, the mask includes the region outside the enclosing
    // mask, we must intersect the current mask with the enclosing mask. The result
    // is a mask that includes all the closed regions inside the enclosing mask
    resultMask->applySelection(enclosingMask, SELECTION_INTERSECT);
}

template <typename SelectionPolicy>
int KisEncloseAndFillPainter::Private::selectSimilarRegions(KisPixelSelectionSP resultMask,
                                                            KisPixelSelectionSP enclosingMask,
                                                            const QRect &enclosingMaskRect,
                                                            KisPaintDeviceSP referenceDevice,
                                                            SelectionPolicy selectionPolicy) const
{
    KisSequentialIterator resultMaskIterator(resultMask, enclosingMaskRect);
    KisSequentialConstIterator enclosingMaskIterator(enclosingMask, enclosingMaskRect);
    KisSequentialConstIterator referenceDeviceIterator(referenceDevice, enclosingMaskRect);
    int nPixels = 0;
    // Select all the pixels using the given selection policy
    while (resultMaskIterator.nextPixel() && enclosingMaskIterator.nextPixel() && referenceDeviceIterator.nextPixel()) {
        if (*enclosingMaskIterator.oldRawData() == MIN_SELECTED) {
            continue;
        }
        const quint8 selection = selectionPolicy.getSelectionFor(referenceDeviceIterator.oldRawData());
        if (selection > MIN_SELECTED) {
            *resultMaskIterator.rawData() = selection;
            ++nPixels;
        }
    }
    return nPixels;
}

template <typename SelectionPolicy>
int KisEncloseAndFillPainter::Private::selectDissimilarRegions(KisPixelSelectionSP resultMask,
                                                               KisPixelSelectionSP enclosingMask,
                                                               const QRect &enclosingMaskRect,
                                                               KisPaintDeviceSP referenceDevice,
                                                               SelectionPolicy selectionPolicy) const
{
    KisSequentialIterator resultMaskIterator(resultMask, enclosingMaskRect);
    KisSequentialConstIterator enclosingMaskIterator(enclosingMask, enclosingMaskRect);
    KisSequentialConstIterator referenceDeviceIterator(referenceDevice, enclosingMaskRect);
    int nPixels = 0;
    // Select all the pixels using the given selection policy
    while (resultMaskIterator.nextPixel() && enclosingMaskIterator.nextPixel() && referenceDeviceIterator.nextPixel()) {
        if (*enclosingMaskIterator.oldRawData() == MIN_SELECTED) {
            continue;
        }
        const quint8 selection = MAX_SELECTED - selectionPolicy.getSelectionFor(referenceDeviceIterator.oldRawData());
        if (selection > MIN_SELECTED) {
            *resultMaskIterator.rawData() = selection;
            ++nPixels;
        }
    }
    return nPixels;
}

void KisEncloseAndFillPainter::Private::selectRegionsFromContour(KisPixelSelectionSP resultMask,
                                                                 KisPixelSelectionSP enclosingMask,
                                                                 const QRect &enclosingMaskRect,
                                                                 KisPaintDeviceSP referenceDevice) const
{
    const QVector<QPoint> enclosingPoints = getEnclosingContourPoints(enclosingMask, enclosingMaskRect);
    selectRegionsFromContour(resultMask, enclosingMask, enclosingPoints, enclosingMaskRect, referenceDevice);
}

void KisEncloseAndFillPainter::Private::selectRegionsFromContour(KisPixelSelectionSP resultMask,
                                                                 KisPixelSelectionSP enclosingMask,
                                                                 const QVector<QPoint> &enclosingPoints,
                                                                 const QRect &enclosingMaskRect,
                                                                 KisPaintDeviceSP referenceDevice) const
{
    if (enclosingPoints.isEmpty()) {
        return;
    }
    // Here we just fill all the areas from the border towards inside
    for (const QPoint &point : enclosingPoints) {
        // Continue if the region under the point was already filled
        if (*(resultMask->pixel(point).data()) == MAX_SELECTED) {
            continue;
        }
        KisPixelSelectionSP mask = new KisPixelSelection(new KisSelectionDefaultBounds(resultMask));
        KisScanlineFill gc(referenceDevice, point, enclosingMaskRect);
        gc.setThreshold(q->fillThreshold());
        gc.setOpacitySpread(q->opacitySpread());
        // Use the enclosing mask as boundary so that we don't fill
        // potentially large regions on the outside
        gc.fillSelectionWithBoundary(mask, enclosingMask);
        resultMask->applySelection(mask, SELECTION_ADD);
    }
}

void KisEncloseAndFillPainter::Private::selectRegionsFromContourUntilColor(KisPixelSelectionSP resultMask,
                                                                           KisPixelSelectionSP enclosingMask,
                                                                           const QRect &enclosingMaskRect,
                                                                           KisPaintDeviceSP referenceDevice,
                                                                           const KoColor &color) const
{
    const QVector<QPoint> enclosingPoints = getEnclosingContourPoints(enclosingMask, enclosingMaskRect);
    selectRegionsFromContourUntilColor(resultMask, enclosingMask, enclosingPoints, enclosingMaskRect, referenceDevice, color);
}

void KisEncloseAndFillPainter::Private::selectRegionsFromContourUntilColor(KisPixelSelectionSP resultMask,
                                                                           KisPixelSelectionSP enclosingMask,
                                                                           const QVector<QPoint> &enclosingPoints,
                                                                           const QRect &enclosingMaskRect,
                                                                           KisPaintDeviceSP referenceDevice,
                                                                           const KoColor &color) const
{
    if (enclosingPoints.isEmpty()) {
        return;
    }
    // Here we just fill all the areas from the border towards inside until the specific color
    for (const QPoint &point : enclosingPoints) {
        // Continue if the region under the point was already filled
        if (*(resultMask->pixel(point).data()) == MAX_SELECTED) {
            continue;
        }
        KisPixelSelectionSP mask = new KisPixelSelection(new KisSelectionDefaultBounds(resultMask));
        KisScanlineFill gc(referenceDevice, point, enclosingMaskRect);
        gc.setThreshold(q->fillThreshold());
        gc.setOpacitySpread(q->opacitySpread());
        // Use the enclosing mask as boundary so that we don't fill
        // potentially large regions in the outside
        gc.fillSelectionUntilColorWithBoundary(mask, color, enclosingMask);
        resultMask->applySelection(mask, SELECTION_ADD);
    }
}

void KisEncloseAndFillPainter::Private::selectRegionsFromContourUntilColorOrTransparent(KisPixelSelectionSP resultMask,
                                                                                        KisPixelSelectionSP enclosingMask,
                                                                                        const QRect &enclosingMaskRect,
                                                                                        KisPaintDeviceSP referenceDevice,
                                                                                        const KoColor &color) const
{
    const QVector<QPoint> enclosingPoints = getEnclosingContourPoints(enclosingMask, enclosingMaskRect);
    selectRegionsFromContourUntilColorOrTransparent(resultMask, enclosingMask, enclosingPoints, enclosingMaskRect, referenceDevice, color);
}

void KisEncloseAndFillPainter::Private::selectRegionsFromContourUntilColorOrTransparent(KisPixelSelectionSP resultMask,
                                                                                        KisPixelSelectionSP enclosingMask,
                                                                                        const QVector<QPoint> &enclosingPoints,
                                                                                        const QRect &enclosingMaskRect,
                                                                                        KisPaintDeviceSP referenceDevice,
                                                                                        const KoColor &color) const
{
    if (enclosingPoints.isEmpty()) {
        return;
    }
    // Here we just fill all the areas from the border towards inside until the specific color
    for (const QPoint &point : enclosingPoints) {
        // Continue if the region under the point was already filled
        if (*(resultMask->pixel(point).data()) == MAX_SELECTED) {
            continue;
        }
        KisPixelSelectionSP mask = new KisPixelSelection(new KisSelectionDefaultBounds(resultMask));
        KisScanlineFill gc(referenceDevice, point, enclosingMaskRect);
        gc.setThreshold(q->fillThreshold());
        gc.setOpacitySpread(q->opacitySpread());
        // Use the enclosing mask as boundary so that we don't fill
        // potentially large regions in the outside
        gc.fillSelectionUntilColorOrTransparentWithBoundary(mask, color, enclosingMask);
        resultMask->applySelection(mask, SELECTION_ADD);
    }
}

void KisEncloseAndFillPainter::Private::removeContourRegions(KisPixelSelectionSP resultMask,
                                                             KisPixelSelectionSP enclosingMask,
                                                             const QRect &enclosingMaskRect) const
{
    const QVector<QPoint> enclosingPoints = getEnclosingContourPoints(enclosingMask, enclosingMaskRect);
    removeContourRegions(resultMask, enclosingPoints, enclosingMaskRect);
}

void KisEncloseAndFillPainter::Private::removeContourRegions(KisPixelSelectionSP resultMask,
                                                             const QVector<QPoint> &enclosingPoints,
                                                             const QRect &enclosingMaskRect) const
{
    if (enclosingPoints.isEmpty()) {
        return;
    }
    // Here we just fill all the non-zero areas from the border towards inside
    for (const QPoint &point : enclosingPoints) {
        // Continue if the region under the point was already filled
        if (*(resultMask->pixel(point).data()) == MIN_SELECTED) {
            continue;
        }
        KisScanlineFill gc(resultMask, point, enclosingMaskRect);
        gc.clearNonZeroComponent();
    }
}
