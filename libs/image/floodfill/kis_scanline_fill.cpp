/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_scanline_fill.h"

#include <KoAlwaysInline.h>

#include <QStack>
#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoCompositeOpRegistry.h>
#include "kis_image.h"
#include "kis_fill_interval_map.h"
#include "kis_pixel_selection.h"
#include "kis_random_accessor_ng.h"
#include "kis_fill_sanity_checks.h"
#include <KisColorSelectionPolicies.h>

class BasePixelAccessPolicy
{
public:
    using SourceAccessorType = KisRandomAccessorSP;

    SourceAccessorType m_srcIt;

    BasePixelAccessPolicy(KisPaintDeviceSP sourceDevice)
        : m_srcIt(sourceDevice->createRandomAccessorNG())
    {}
};

class ConstBasePixelAccessPolicy
{
public:
    using SourceAccessorType = KisRandomConstAccessorSP;

    SourceAccessorType m_srcIt;

    ConstBasePixelAccessPolicy(KisPaintDeviceSP sourceDevice)
        : m_srcIt(sourceDevice->createRandomConstAccessorNG())
    {}
};

class CopyToSelectionPixelAccessPolicy : public ConstBasePixelAccessPolicy
{
public:
    CopyToSelectionPixelAccessPolicy(KisPaintDeviceSP sourceDevice, KisPaintDeviceSP pixelSelection)
        : ConstBasePixelAccessPolicy(sourceDevice)
        , m_pixelSelection(pixelSelection)
        , m_selectionIterator(m_pixelSelection->createRandomAccessorNG())
    {}

    ALWAYS_INLINE void fillPixel(quint8 *dstPtr, quint8 opacity, int x, int y)
    {
        Q_UNUSED(dstPtr);
        m_selectionIterator->moveTo(x, y);
        *m_selectionIterator->rawData() = opacity;
    }

private:
    KisPaintDeviceSP m_pixelSelection;
    KisRandomAccessorSP m_selectionIterator;
};

class FillWithColorPixelAccessPolicy : public BasePixelAccessPolicy
{
public:
    FillWithColorPixelAccessPolicy(KisPaintDeviceSP sourceDevice, const KoColor &fillColor)
        : BasePixelAccessPolicy(sourceDevice)
        , m_fillColor(fillColor)
        , m_fillColorPtr(m_fillColor.data())
        , m_pixelSize(m_fillColor.colorSpace()->pixelSize())
    {}

    ALWAYS_INLINE void fillPixel(quint8 *dstPtr, quint8 opacity, int x, int y)
    {
        Q_UNUSED(x);
        Q_UNUSED(y);

        if (opacity == MAX_SELECTED) {
            memcpy(dstPtr, m_fillColorPtr, m_pixelSize);
        }
    }

private:
    KoColor m_fillColor;
    const quint8 *m_fillColorPtr;
    int m_pixelSize;
};

class FillWithColorExternalPixelAccessPolicy : public ConstBasePixelAccessPolicy
{
public:
    FillWithColorExternalPixelAccessPolicy(KisPaintDeviceSP sourceDevice,
                                     const KoColor &fillColor,
                                     KisPaintDeviceSP externalDevice)
        : ConstBasePixelAccessPolicy(sourceDevice)
        , m_externalDevice(externalDevice)
        , m_externalDeviceIterator(m_externalDevice->createRandomAccessorNG())
        , m_fillColor(fillColor)
        , m_fillColorPtr(m_fillColor.data())
        , m_pixelSize(m_fillColor.colorSpace()->pixelSize())
    {}

    ALWAYS_INLINE void fillPixel(quint8 *dstPtr, quint8 opacity, int x, int y)
    {
        Q_UNUSED(dstPtr);

        m_externalDeviceIterator->moveTo(x, y);
        if (opacity == MAX_SELECTED) {
            memcpy(m_externalDeviceIterator->rawData(), m_fillColorPtr, m_pixelSize);
        }
    }

private:
    KisPaintDeviceSP m_externalDevice;
    KisRandomAccessorSP m_externalDeviceIterator;
    KoColor m_fillColor;
    const quint8 *m_fillColorPtr;
    int m_pixelSize;
};

template <typename BaseSelectionPolicy>
class SelectionPolicy
{
public:
    SelectionPolicy(BaseSelectionPolicy baseSelectionPolicy)
        : m_baseSelectionPolicy(baseSelectionPolicy)
    {}

    ALWAYS_INLINE quint8 opacityFromDifference(quint8 difference, int x, int y)
    {
        Q_UNUSED(x);
        Q_UNUSED(y);
        
        return m_baseSelectionPolicy.opacityFromDifference(difference);
    }

private:
    BaseSelectionPolicy m_baseSelectionPolicy;
};

template <typename BaseSelectionPolicy>
class MaskedSelectionPolicy
{
public:
    MaskedSelectionPolicy(BaseSelectionPolicy baseSelectionPolicy,
                          KisPaintDeviceSP maskDevice)
        : m_baseSelectionPolicy(baseSelectionPolicy)
        , m_maskIterator(maskDevice->createRandomConstAccessorNG())
    {}

    ALWAYS_INLINE quint8 opacityFromDifference(quint8 difference, int x, int y)
    {
        m_maskIterator->moveTo(x, y);
        const quint8* maskPtr = m_maskIterator->rawDataConst();

        if (*maskPtr == MIN_SELECTED) {
            return MIN_SELECTED;
        }
        
        return m_baseSelectionPolicy.opacityFromDifference(difference);
    }

private:
    BaseSelectionPolicy m_baseSelectionPolicy;
    KisRandomConstAccessorSP m_maskIterator;
};

class GroupSplitDifferencePolicy
{
public:
    GroupSplitDifferencePolicy(int referenceValue)
        : m_referenceValue(referenceValue)
    {}

    ALWAYS_INLINE quint8 difference(const quint8 *colorPtr) const
    {
        return qAbs(*colorPtr - m_referenceValue);
    }

private:
    int m_referenceValue;
};

class GroupSplitSelectionPolicy : public KisColorSelectionPolicies::HardSelectionPolicy
{
public:
    GroupSplitSelectionPolicy(int threshold)
        : KisColorSelectionPolicies::HardSelectionPolicy(threshold)
    {}

    ALWAYS_INLINE quint8 opacityFromDifference(quint8 difference, int x, int y) {
        // TODO: either threshold should always be null, or there should be a special
        //       case for *pixelPtr == 0, which is different from all the other groups,
        //       whatever the threshold is
        Q_UNUSED(x);
        Q_UNUSED(y);
        return KisColorSelectionPolicies::HardSelectionPolicy::opacityFromDifference(difference);
    }
};

class GroupSplitPixelAccessPolicy : public BasePixelAccessPolicy
{
public:
    GroupSplitPixelAccessPolicy(KisPaintDeviceSP scribbleDevice,
                                KisPaintDeviceSP groupMapDevice,
                                qint32 groupIndex)
        : BasePixelAccessPolicy(scribbleDevice)
        , m_groupIndex(groupIndex)
        , m_groupMapIt(groupMapDevice->createRandomAccessorNG())
    {
        KIS_SAFE_ASSERT_RECOVER_NOOP(m_groupIndex > 0);
    }

    ALWAYS_INLINE void fillPixel(quint8 *dstPtr, quint8 opacity, int x, int y)
    {
        Q_UNUSED(opacity);

        // erase the scribble
        *dstPtr = 0;

        // write group index into the map
        m_groupMapIt->moveTo(x, y);
        qint32 *groupMapPtr = reinterpret_cast<qint32*>(m_groupMapIt->rawData());

        if (*groupMapPtr != 0) {
            dbgImage << ppVar(*groupMapPtr) << ppVar(m_groupIndex);
        }

        KIS_SAFE_ASSERT_RECOVER_NOOP(*groupMapPtr == 0);

        *groupMapPtr = m_groupIndex;
    }

private:
    qint32 m_groupIndex;
    KisRandomAccessorSP m_groupMapIt;
};

struct Q_DECL_HIDDEN KisScanlineFill::Private
{
    KisPaintDeviceSP device;
    QPoint startPoint;
    QRect boundingRect;
    int threshold;
    int opacitySpread;

    int rowIncrement;
    KisFillIntervalMap backwardMap;
    QStack<KisFillInterval> forwardStack;


    inline void swapDirection() {
        rowIncrement *= -1;
        KIS_SAFE_ASSERT_RECOVER_NOOP(forwardStack.isEmpty() &&
                                     "FATAL: the forward stack must be empty "
                                     "on a direction swap");

        forwardStack = QStack<KisFillInterval>(backwardMap.fetchAllIntervals(rowIncrement));
        backwardMap.clear();
    }
};


KisScanlineFill::KisScanlineFill(KisPaintDeviceSP device, const QPoint &startPoint, const QRect &boundingRect)
    : m_d(new Private)
{
    m_d->device = device;
    m_d->startPoint = startPoint;
    m_d->boundingRect = boundingRect;

    m_d->rowIncrement = 1;

    m_d->threshold = 0;
    m_d->opacitySpread = 0;
}

KisScanlineFill::~KisScanlineFill()
{
}

void KisScanlineFill::setThreshold(int threshold)
{
    m_d->threshold = threshold;
}

void KisScanlineFill::setOpacitySpread(int opacitySpread)
{
    m_d->opacitySpread = opacitySpread;
}

template <typename DifferencePolicy, typename SelectionPolicy, typename PixelAccessPolicy>
void KisScanlineFill::extendedPass(KisFillInterval *currentInterval, int srcRow, bool extendRight,
                                   DifferencePolicy &differencePolicy,
                                   SelectionPolicy &selectionPolicy,
                                   PixelAccessPolicy &pixelAccessPolicy)
{
    int x;
    int endX;
    int columnIncrement;
    int *intervalBorder;
    int *backwardIntervalBorder;
    KisFillInterval backwardInterval(currentInterval->start, currentInterval->end, srcRow);

    if (extendRight) {
        x = currentInterval->end;
        endX = m_d->boundingRect.right();
        if (x >= endX) return;
        columnIncrement = 1;
        intervalBorder = &currentInterval->end;

        backwardInterval.start = currentInterval->end + 1;
        backwardIntervalBorder = &backwardInterval.end;
    } else {
        x = currentInterval->start;
        endX = m_d->boundingRect.left();
        if (x <= endX) return;
        columnIncrement = -1;
        intervalBorder = &currentInterval->start;

        backwardInterval.end = currentInterval->start - 1;
        backwardIntervalBorder = &backwardInterval.start;
    }

    do {
        x += columnIncrement;

        pixelAccessPolicy.m_srcIt->moveTo(x, srcRow);
        quint8 *pixelPtr = const_cast<quint8*>(pixelAccessPolicy.m_srcIt->rawDataConst()); // TODO: avoid doing const_cast
        const quint8 difference = differencePolicy.difference(pixelPtr);
        const quint8 opacity = selectionPolicy.opacityFromDifference(difference, x, srcRow);

        if (opacity) {
            *intervalBorder = x;
            *backwardIntervalBorder = x;
            pixelAccessPolicy.fillPixel(pixelPtr, opacity, x, srcRow);
        } else {
            break;
        }
    } while (x != endX);

    if (backwardInterval.isValid()) {
        m_d->backwardMap.insertInterval(backwardInterval);
    }
}

template <typename DifferencePolicy, typename SelectionPolicy, typename PixelAccessPolicy>
void KisScanlineFill::processLine(KisFillInterval interval, const int rowIncrement,
                                  DifferencePolicy &differencePolicy,
                                  SelectionPolicy &selectionPolicy,
                                  PixelAccessPolicy &pixelAccessPolicy)
{
    m_d->backwardMap.cropInterval(&interval);

    if (!interval.isValid()) return;

    int firstX = interval.start;
    int lastX = interval.end;
    int x = firstX;
    int row = interval.row;
    int nextRow = row + rowIncrement;

    KisFillInterval currentForwardInterval;

    int numPixelsLeft = 0;
    quint8 *dataPtr = 0;
    const int pixelSize = m_d->device->pixelSize();

    while(x <= lastX) {
        // a bit of optimization for not calling slow random accessor
        // methods too often
        if (numPixelsLeft <= 0) {
            pixelAccessPolicy.m_srcIt->moveTo(x, row);
            numPixelsLeft = pixelAccessPolicy.m_srcIt->numContiguousColumns(x) - 1;
            dataPtr = const_cast<quint8*>(pixelAccessPolicy.m_srcIt->rawDataConst());
        } else {
            numPixelsLeft--;
            dataPtr += pixelSize;
        }

        quint8 *pixelPtr = dataPtr;
        const quint8 difference = differencePolicy.difference(pixelPtr);
        const quint8 opacity = selectionPolicy.opacityFromDifference(difference, x, row);

        if (opacity) {
            if (!currentForwardInterval.isValid()) {
                currentForwardInterval.start = x;
                currentForwardInterval.end = x;
                currentForwardInterval.row = nextRow;
            } else {
                currentForwardInterval.end = x;
            }

            pixelAccessPolicy.fillPixel(pixelPtr, opacity, x, row);

            if (x == firstX) {
                extendedPass(&currentForwardInterval, row, false,
                             differencePolicy, selectionPolicy, pixelAccessPolicy);
            }

            if (x == lastX) {
                extendedPass(&currentForwardInterval, row, true,
                             differencePolicy, selectionPolicy, pixelAccessPolicy);
            }

        } else {
            if (currentForwardInterval.isValid()) {
                m_d->forwardStack.push(currentForwardInterval);
                currentForwardInterval.invalidate();
            }
        }

        x++;
    }

    if (currentForwardInterval.isValid()) {
        m_d->forwardStack.push(currentForwardInterval);
    }
}

template <typename DifferencePolicy, typename SelectionPolicy, typename PixelAccessPolicy>
void KisScanlineFill::runImpl(DifferencePolicy &differencePolicy,
                              SelectionPolicy &selectionPolicy,
                              PixelAccessPolicy &pixelAccessPolicy)
{
    KIS_ASSERT_RECOVER_RETURN(m_d->forwardStack.isEmpty());

    KisFillInterval startInterval(m_d->startPoint.x(), m_d->startPoint.x(), m_d->startPoint.y());
    m_d->forwardStack.push(startInterval);

    /**
     * In the end of the first pass we should add an interval
     * containing the starting pixel, but directed into the opposite
     * direction. We cannot do it in the very beginning because the
     * intervals are offset by 1 pixel during every swap operation.
     */
    bool firstPass = true;

    while (!m_d->forwardStack.isEmpty()) {
        while (!m_d->forwardStack.isEmpty()) {
            KisFillInterval interval = m_d->forwardStack.pop();

            if (interval.row > m_d->boundingRect.bottom() ||
                interval.row < m_d->boundingRect.top()) {

                continue;
            }

            processLine(interval, m_d->rowIncrement, differencePolicy, selectionPolicy, pixelAccessPolicy);
        }
        m_d->swapDirection();

        if (firstPass) {
            startInterval.row--;
            m_d->forwardStack.push(startInterval);
            firstPass = false;
        }
    }
}

template <template <typename SrcPixelType> typename OptimizedDifferencePolicy,
          typename SlowDifferencePolicy,
          typename SelectionPolicy, typename PixelAccessPolicy>
void KisScanlineFill::selectDifferencePolicyAndRun(const KoColor &srcColor,
                                                   SelectionPolicy &selectionPolicy,
                                                   PixelAccessPolicy &pixelAccessPolicy)
{
    const int pixelSize = srcColor.colorSpace()->pixelSize();

    if (pixelSize == 1) {
        OptimizedDifferencePolicy<quint8> dp(srcColor, m_d->threshold);
        runImpl(dp, selectionPolicy, pixelAccessPolicy);
    } else if (pixelSize == 2) {
        OptimizedDifferencePolicy<quint16> dp(srcColor, m_d->threshold);
        runImpl(dp, selectionPolicy, pixelAccessPolicy);
    } else if (pixelSize == 4) {
        OptimizedDifferencePolicy<quint32> dp(srcColor, m_d->threshold);
        runImpl(dp, selectionPolicy, pixelAccessPolicy);
    } else if (pixelSize == 8) {
        OptimizedDifferencePolicy<quint64> dp(srcColor, m_d->threshold);
        runImpl(dp, selectionPolicy, pixelAccessPolicy);
    } else {
        SlowDifferencePolicy dp(srcColor, m_d->threshold);
        runImpl(dp, selectionPolicy, pixelAccessPolicy);
    }
}

void KisScanlineFill::fill(const KoColor &originalFillColor)
{
    KoColor srcColor(m_d->device->pixel(m_d->startPoint));
    KoColor fillColor(originalFillColor);
    fillColor.convertTo(m_d->device->colorSpace());

    using namespace KisColorSelectionPolicies;

    SelectionPolicy<HardSelectionPolicy> sp(HardSelectionPolicy(m_d->threshold));
    FillWithColorPixelAccessPolicy pap(m_d->device, fillColor);

    selectDifferencePolicyAndRun<OptimizedDifferencePolicy, SlowDifferencePolicy>
                                (srcColor, sp, pap);
}

void KisScanlineFill::fillUntilColor(const KoColor &originalFillColor, const KoColor &boundaryColor)
{
    KoColor srcColor(boundaryColor);
    srcColor.convertTo(m_d->device->colorSpace());
    KoColor fillColor(originalFillColor);
    fillColor.convertTo(m_d->device->colorSpace());

    using namespace KisColorSelectionPolicies;
    
    SelectionPolicy<SelectAllUntilColorHardSelectionPolicy> sp(SelectAllUntilColorHardSelectionPolicy(m_d->threshold));
    FillWithColorPixelAccessPolicy pap(m_d->device, fillColor);

    selectDifferencePolicyAndRun<OptimizedDifferencePolicy, SlowDifferencePolicy>
                                (srcColor, sp, pap);
}

void KisScanlineFill::fill(const KoColor &originalFillColor, KisPaintDeviceSP externalDevice)
{
    KoColor srcColor(m_d->device->pixel(m_d->startPoint));
    KoColor fillColor(originalFillColor);
    fillColor.convertTo(m_d->device->colorSpace());

    using namespace KisColorSelectionPolicies;

    SelectionPolicy<HardSelectionPolicy> sp(HardSelectionPolicy(m_d->threshold));
    FillWithColorExternalPixelAccessPolicy pap(m_d->device, fillColor, externalDevice);

    selectDifferencePolicyAndRun<OptimizedDifferencePolicy, SlowDifferencePolicy>
                                (srcColor, sp, pap);
}

void KisScanlineFill::fillUntilColor(const KoColor &originalFillColor, const KoColor &boundaryColor, KisPaintDeviceSP externalDevice)
{
    KoColor srcColor(boundaryColor);
    srcColor.convertTo(m_d->device->colorSpace());
    KoColor fillColor(originalFillColor);
    fillColor.convertTo(m_d->device->colorSpace());

    using namespace KisColorSelectionPolicies;

    SelectionPolicy<SelectAllUntilColorHardSelectionPolicy> sp(SelectAllUntilColorHardSelectionPolicy(m_d->threshold));
    FillWithColorExternalPixelAccessPolicy pap(m_d->device, fillColor, externalDevice);

    selectDifferencePolicyAndRun<OptimizedDifferencePolicy, SlowDifferencePolicy>
                                (srcColor, sp, pap);
}

void KisScanlineFill::fillSelection(KisPixelSelectionSP pixelSelection, KisPaintDeviceSP boundarySelection)
{
    KoColor srcColor(m_d->device->pixel(m_d->startPoint));

    const int softness = 100 - m_d->opacitySpread;

    using namespace KisColorSelectionPolicies;

    CopyToSelectionPixelAccessPolicy pap(m_d->device, pixelSelection);

    if (softness == 0) {
        MaskedSelectionPolicy<HardSelectionPolicy>
            sp(HardSelectionPolicy(m_d->threshold), boundarySelection);
        selectDifferencePolicyAndRun<OptimizedDifferencePolicy, SlowDifferencePolicy>
                                    (srcColor, sp, pap);
    } else {
        MaskedSelectionPolicy<SoftSelectionPolicy> 
            sp(SoftSelectionPolicy(m_d->threshold, softness), boundarySelection);
        selectDifferencePolicyAndRun<OptimizedDifferencePolicy, SlowDifferencePolicy>
                                    (srcColor, sp, pap);
    }
}

void KisScanlineFill::fillSelection(KisPixelSelectionSP pixelSelection)
{
    KoColor srcColor(m_d->device->pixel(m_d->startPoint));

    const int softness = 100 - m_d->opacitySpread;

    using namespace KisColorSelectionPolicies;
    
    CopyToSelectionPixelAccessPolicy pap(m_d->device, pixelSelection);

    if (softness == 0) {
        SelectionPolicy<HardSelectionPolicy> sp(HardSelectionPolicy(m_d->threshold));
        selectDifferencePolicyAndRun<OptimizedDifferencePolicy, SlowDifferencePolicy>
                                    (srcColor, sp, pap);
    } else {
        SelectionPolicy<SoftSelectionPolicy> sp(SoftSelectionPolicy(m_d->threshold, softness));
        selectDifferencePolicyAndRun<OptimizedDifferencePolicy, SlowDifferencePolicy>
                                    (srcColor, sp, pap);
    }
}

void KisScanlineFill::fillSelectionUntilColor(KisPixelSelectionSP pixelSelection, const KoColor &boundaryColor, KisPaintDeviceSP boundarySelection)
{
    KoColor srcColor(boundaryColor);
    srcColor.convertTo(m_d->device->colorSpace());

    const int softness = 100 - m_d->opacitySpread;

    using namespace KisColorSelectionPolicies;
    
    CopyToSelectionPixelAccessPolicy pap(m_d->device, pixelSelection);

    if (softness == 0) {
        MaskedSelectionPolicy<SelectAllUntilColorHardSelectionPolicy>
            sp(SelectAllUntilColorHardSelectionPolicy(m_d->threshold), boundarySelection);
        selectDifferencePolicyAndRun<OptimizedDifferencePolicy, SlowDifferencePolicy>
                                    (srcColor, sp, pap);
    } else {
        MaskedSelectionPolicy<SelectAllUntilColorSoftSelectionPolicy> 
            sp(SelectAllUntilColorSoftSelectionPolicy(m_d->threshold, softness), boundarySelection);
        selectDifferencePolicyAndRun<OptimizedDifferencePolicy, SlowDifferencePolicy>
                                    (srcColor, sp, pap);
    }
}

void KisScanlineFill::fillSelectionUntilColor(KisPixelSelectionSP pixelSelection, const KoColor &boundaryColor)
{
    KoColor srcColor(boundaryColor);
    srcColor.convertTo(m_d->device->colorSpace());

    const int softness = 100 - m_d->opacitySpread;

    using namespace KisColorSelectionPolicies;
    
    CopyToSelectionPixelAccessPolicy pap(m_d->device, pixelSelection);

    if (softness == 0) {
        SelectionPolicy<SelectAllUntilColorHardSelectionPolicy>
            sp(SelectAllUntilColorHardSelectionPolicy(m_d->threshold));
        selectDifferencePolicyAndRun<OptimizedDifferencePolicy, SlowDifferencePolicy>
                                    (srcColor, sp, pap);
    } else {
        SelectionPolicy<SelectAllUntilColorSoftSelectionPolicy> 
            sp(SelectAllUntilColorSoftSelectionPolicy(m_d->threshold, softness));
        selectDifferencePolicyAndRun<OptimizedDifferencePolicy, SlowDifferencePolicy>
                                    (srcColor, sp, pap);
    }
}

void KisScanlineFill::fillSelectionUntilColorOrTransparent(KisPixelSelectionSP pixelSelection, const KoColor &boundaryColor, KisPaintDeviceSP boundarySelection)
{
    KoColor srcColor(boundaryColor);
    srcColor.convertTo(m_d->device->colorSpace());

    const int softness = 100 - m_d->opacitySpread;

    using namespace KisColorSelectionPolicies;
    
    CopyToSelectionPixelAccessPolicy pap(m_d->device, pixelSelection);

    if (softness == 0) {
        MaskedSelectionPolicy<SelectAllUntilColorHardSelectionPolicy>
            sp(SelectAllUntilColorHardSelectionPolicy(m_d->threshold), boundarySelection);
        selectDifferencePolicyAndRun<OptimizedColorOrTransparentDifferencePolicy,
                                     SlowColorOrTransparentDifferencePolicy>
                                    (srcColor, sp, pap);
    } else {
        MaskedSelectionPolicy<SelectAllUntilColorSoftSelectionPolicy> 
            sp(SelectAllUntilColorSoftSelectionPolicy(m_d->threshold, softness), boundarySelection);
        selectDifferencePolicyAndRun<OptimizedColorOrTransparentDifferencePolicy,
                                     SlowColorOrTransparentDifferencePolicy>
                                    (srcColor, sp, pap);
    }
}

void KisScanlineFill::fillSelectionUntilColorOrTransparent(KisPixelSelectionSP pixelSelection, const KoColor &boundaryColor)
{
    KoColor srcColor(boundaryColor);
    srcColor.convertTo(m_d->device->colorSpace());

    const int softness = 100 - m_d->opacitySpread;

    using namespace KisColorSelectionPolicies;
    
    CopyToSelectionPixelAccessPolicy pap(m_d->device, pixelSelection);

    if (softness == 0) {
        SelectionPolicy<SelectAllUntilColorHardSelectionPolicy>
            sp(SelectAllUntilColorHardSelectionPolicy(m_d->threshold));
        selectDifferencePolicyAndRun<OptimizedColorOrTransparentDifferencePolicy,
                                     SlowColorOrTransparentDifferencePolicy>
                                    (srcColor, sp, pap);
    } else {
        SelectionPolicy<SelectAllUntilColorSoftSelectionPolicy> 
            sp(SelectAllUntilColorSoftSelectionPolicy(m_d->threshold, softness));
        selectDifferencePolicyAndRun<OptimizedColorOrTransparentDifferencePolicy,
                                     SlowColorOrTransparentDifferencePolicy>
                                    (srcColor, sp, pap);
    }
}

void KisScanlineFill::clearNonZeroComponent()
{
    const int pixelSize = m_d->device->pixelSize();
    KoColor srcColor = KoColor::createTransparent(m_d->device->colorSpace());

    using namespace KisColorSelectionPolicies;
    
    FillWithColorPixelAccessPolicy pap(m_d->device, srcColor);
    SelectionPolicy<HardSelectionPolicy> sp(HardSelectionPolicy(m_d->threshold));

    if (pixelSize == 1) {
        OptimizedIsNonNullDifferencePolicy<quint8> dp;
        runImpl(dp, sp, pap);
    } else if (pixelSize == 2) {
        OptimizedIsNonNullDifferencePolicy<quint16> dp;
        runImpl(dp, sp, pap);
    } else if (pixelSize == 4) {
        OptimizedIsNonNullDifferencePolicy<quint32> dp;
        runImpl(dp, sp, pap);
    } else if (pixelSize == 8) {
        OptimizedIsNonNullDifferencePolicy<quint64> dp;
        runImpl(dp, sp, pap);
    } else {
        SlowIsNonNullDifferencePolicy dp(pixelSize);
        runImpl(dp, sp, pap);
    }
}

void KisScanlineFill::fillContiguousGroup(KisPaintDeviceSP groupMapDevice, qint32 groupIndex)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->device->pixelSize() == 1);
    KIS_SAFE_ASSERT_RECOVER_RETURN(groupMapDevice->pixelSize() == 4);

    const quint8 referenceValue = *m_d->device->pixel(m_d->startPoint).data();

    using namespace KisColorSelectionPolicies;

    GroupSplitDifferencePolicy dp(referenceValue);
    GroupSplitSelectionPolicy sp(m_d->threshold);
    GroupSplitPixelAccessPolicy pap(m_d->device, groupMapDevice, groupIndex);

    runImpl(dp, sp, pap);
}

void KisScanlineFill::testingProcessLine(const KisFillInterval &processInterval)
{
    KoColor srcColor(QColor(0,0,0,0), m_d->device->colorSpace());
    KoColor fillColor(QColor(200,200,200,200), m_d->device->colorSpace());

    using namespace KisColorSelectionPolicies;

    OptimizedDifferencePolicy<quint32> dp(srcColor, m_d->threshold);
    SelectionPolicy<HardSelectionPolicy> sp(HardSelectionPolicy(m_d->threshold));
    FillWithColorPixelAccessPolicy pap(m_d->device, fillColor);

    processLine(processInterval, 1, dp, sp, pap);
}

QVector<KisFillInterval> KisScanlineFill::testingGetForwardIntervals() const
{
    return QVector<KisFillInterval>(m_d->forwardStack);
}

KisFillIntervalMap* KisScanlineFill::testingGetBackwardIntervals() const
{
    return &m_d->backwardMap;
}
