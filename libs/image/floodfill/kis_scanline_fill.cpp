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
#include "kis_gap_map.h"
#include <queue>

#define MEASURE_FILL_TIME 0
#if MEASURE_FILL_TIME
#include <QElapsedTimer>
#endif

namespace {

typedef KisSharedPtr<KisGapMap> KisGapMapSP;

/**
 * A work item for the gap closing fill.
 * Can work as a seed point and as a next queued pixel to continue the fill.
 */
struct CloseGapFillPoint
{
    int x;
    int y;

    quint32 distance;   ///< previous pixel's (usually) distance from the nearest lineart gap
    bool allowExpand;   ///< whether the fill at this pixel can "expand", that is fill pixels with greater distance

    // Used with the priority queue.
    // Use the "greater" operator to place the smallest element on top.
    // std::make_tuple instead of std::tie because we need to negate allowExpand.
    friend bool operator>(const CloseGapFillPoint& a, const CloseGapFillPoint& b) {
        return std::make_tuple(!a.allowExpand, a.distance, a.y, a.x) >
               std::make_tuple(!b.allowExpand, b.distance, b.y, b.x);
    }
};

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

} // anonymous namespace

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

    int closeGap;           ///< try to close gaps up to this size in pixels
    KisGapMapSP gapMapSp;   ///< maintains the distance and opacity maps required for the algorithm

    QRect fillExtent;

    // The priority queue is required to correctly handle the fill "expansion" case
    // (starting in a corner and filling towards open areas, where distance is DISTANCE_INFINITE).
    // Holds the next pixel to consider for filling, among with the contextual information.
    template<class T> using GreaterPriorityQueue = std::priority_queue<T, std::vector<T>, std::greater<T>>;
    GreaterPriorityQueue<CloseGapFillPoint> closeGapQueue;

    // Gap closing flood fill must check the currently filled selection in order to finish.
    // Otherwise, it would attempt to fill the same pixels in an infinite loop.
    KisRandomAccessorSP filledSelectionIterator;


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
    m_d->closeGap = 0;
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

void KisScanlineFill::setCloseGap(int closeGap)
{
    m_d->closeGap = closeGap;
}

QRect KisScanlineFill::fillExtent() const
{
    return m_d->fillExtent;
}


/**
 * Used with the scanline fill algorithm.
 *
 * Determine if the (x, y) pixel is near a gap (i.e., has non-INFINITE distance).
 * All gap pixels are pushed onto the gap closing fill queue, while the scanline
 * fill proceeds with regular pixels.
 *
 * @param allowExpand is only true for the initial gap closing seed.
 *
 * @returns true if a gap closing pixel was pushed; the scanline fill must not
 * fill this pixel.
 */
bool KisScanlineFill::tryPushingCloseGapSeed(int x, int y, bool allowExpand)
{
    bool pushed = false;

    if (m_d->gapMapSp && m_d->gapMapSp->gapSize() > 0) {
        const quint32 distance = m_d->gapMapSp->distance(x, y);

        if (distance != KisGapMap::DISTANCE_INFINITE) {
            pushed = true;

            CloseGapFillPoint seed;
            seed.x = x;
            seed.y = y;
            seed.distance = distance;
            seed.allowExpand = allowExpand;

            m_d->closeGapQueue.push(seed);
        }
    }

    return pushed;
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
        const bool stopOnGap = tryPushingCloseGapSeed(x, srcRow, false);

        if (!stopOnGap && opacity) {
            *intervalBorder = x;
            *backwardIntervalBorder = x;
            pixelAccessPolicy.fillPixel(pixelPtr, opacity, x, srcRow);
            if (extendRight) {
                m_d->fillExtent.setRight(qMax(m_d->fillExtent.right(), x));
            } else {
                m_d->fillExtent.setLeft(qMin(m_d->fillExtent.left(), x));
            }
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
        const bool stopOnGap = tryPushingCloseGapSeed(x, row, false);

        if (!stopOnGap && opacity) {
            if (!currentForwardInterval.isValid()) {
                currentForwardInterval.start = x;
                currentForwardInterval.end = x;
                currentForwardInterval.row = nextRow;
            } else {
                currentForwardInterval.end = x;
            }

            pixelAccessPolicy.fillPixel(pixelPtr, opacity, x, row);
            m_d->fillExtent = m_d->fillExtent.united(QRect(x, row, 1, 1));

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
KisFillInterval KisScanlineFill::closeGapPass(DifferencePolicy &differencePolicy,
                                              SelectionPolicy &selectionPolicy,
                                              PixelAccessPolicy &pixelAccessPolicy)
{
    KisFillInterval interval;

    while (!m_d->closeGapQueue.empty()) {
        const CloseGapFillPoint p = m_d->closeGapQueue.top();
        m_d->closeGapQueue.pop();

        // KisGapMap enforces x/y start at (0, 0).
        if ((p.x < 0) || (p.x >= m_d->boundingRect.width()) || (p.y < 0) || (p.y >= m_d->boundingRect.height())) {
            continue;
        }

        pixelAccessPolicy.m_srcIt->moveTo(p.x, p.y);
        quint8 *pixelPtr = const_cast<quint8*>(pixelAccessPolicy.m_srcIt->rawDataConst());
        const quint8 difference = differencePolicy.difference(pixelPtr);
        const quint8 opacity = selectionPolicy.opacityFromDifference(difference, p.x, p.y);

        // The initial pixel (before the fill) is non-opaque, so we try to fill it.
        if (opacity) {
            // However, check if it has already been filled during the ongoing operation.
            // If the test below is true, then the pixel is already in the selection.
            m_d->filledSelectionIterator->moveTo(p.x, p.y);
            if (*m_d->filledSelectionIterator->rawDataConst() == opacity) {
                continue;
            }

            const quint32 previousDistance = p.distance;
            const quint32 distance = m_d->gapMapSp->distance(p.x, p.y);
            const bool allowExpand = p.allowExpand && (distance >= previousDistance);
            const float relativeDiff = static_cast<float>(distance) / previousDistance;

            // This ratio determines whether the fill can spread to pixels with
            // a different gap distance compared to the previous pixel.
            //
            // <1.0 is a pixel closer to a gap, or more in a corner of lineart.
            // =1.0 is the same distance.
            // >1.0 is a pixel that is farther from a gap, or away from tight corners.
            //
            // At high gap sizes, the distance map can vary slightly and the fill could
            // leave out empty pixels. To prevent that, we allow spilling to a more distant
            // pixels as well, up to a given tolerance. If the value is too high,
            // the fill will spill too much.

            static constexpr float SpreadTolerance = 1.3f;

            if ((relativeDiff < SpreadTolerance) || allowExpand) {
                if (distance == KisGapMap::DISTANCE_INFINITE) {
                    // Only return one interval at a time. Otherwise, just skip this pixel.
                    if (!interval.isValid()) {
                        interval.start = p.x;
                        interval.end = p.x;
                        interval.row = p.y;
                    }
                    // We still continue the loop, to process all the pixels we can fill.
                } else {
                    pixelAccessPolicy.fillPixel(pixelPtr, opacity, p.x, p.y);
                    m_d->fillExtent = m_d->fillExtent.united(QRect(p.x, p.y, 1, 1));

                    // Forward the context information to the next pixels.
                    // Spread the fill in four directions: up, down, left, right.

                    CloseGapFillPoint next;
                    next.distance = distance;
                    next.allowExpand = allowExpand;

                    next.x = p.x - 1;
                    next.y = p.y;
                    m_d->closeGapQueue.push(next);

                    next.x = p.x + 1;
                    next.y = p.y;
                    m_d->closeGapQueue.push(next);

                    next.x = p.x;
                    next.y = p.y - 1;
                    m_d->closeGapQueue.push(next);

                    next.x = p.x;
                    next.y = p.y + 1;
                    m_d->closeGapQueue.push(next);
                }
            }
        }
    }

    // If valid, the scanline fill will take over next.
    return interval;
}

template <typename DifferencePolicy, typename SelectionPolicy, typename PixelAccessPolicy>
void KisScanlineFill::runImpl(DifferencePolicy &differencePolicy,
                              SelectionPolicy &selectionPolicy,
                              PixelAccessPolicy &pixelAccessPolicy)
{
    KIS_ASSERT_RECOVER_RETURN(m_d->forwardStack.isEmpty());

    int gapSize = m_d->closeGap;
    KIS_SAFE_ASSERT_RECOVER(gapSize == 0 || (gapSize > 0 && m_d->filledSelectionIterator)) {
        gapSize = 0;
    }

#if MEASURE_FILL_TIME
    QElapsedTimer timerTotal;
    QElapsedTimer timerScanlineFill;
    QElapsedTimer timerGapClosingFill;
    quint64 totalScanlineFillNanos = 0;
    quint64 totalGapClosingFillNanos = 0;

    timerTotal.start();
#endif

    if (gapSize > 0) {
        // We need to reuse the complex policies used by this class and only provide the final
        // "projection" of opacity for the distance map calculation.
        auto opacityFunc = [&](KisPaintDevice* devicePtr, const QRect& rect) {
            return fillOpacity(differencePolicy, selectionPolicy, pixelAccessPolicy, devicePtr, rect);
        };

        // Prime the resources. The computations are made lazily, when distance at a pixel is requested.
        // Resources are freed automatically when the object is destroyed, that is together with the KisScanlineFill object.
        m_d->gapMapSp = KisGapMapSP(new KisGapMap(gapSize, m_d->boundingRect, opacityFunc));
    }

    m_d->fillExtent = QRect();

    KisFillInterval startInterval(m_d->startPoint.x(), m_d->startPoint.x(), m_d->startPoint.y());

    // Decide if we should start with a scanline fill or a gap closing fill.
    if (!tryPushingCloseGapSeed(startInterval.start, startInterval.row, true)) {
        m_d->forwardStack.push(startInterval);
    }

    // The outer loop is to continue scanline filling after a gap closing fill.
    do {
        /**
         * In the end of the first pass we should add an interval
         * containing the starting pixel, but directed into the opposite
         * direction. We cannot do it in the very beginning because the
         * intervals are offset by 1 pixel during every swap operation.
         */
        bool firstPass = true;

#if MEASURE_FILL_TIME
        timerScanlineFill.start();
#endif

        // The scanline fill can only fill the pixels that are "in the open",
        // that is have DISTANCE_INFINITE. Gap pixels will be pushed to
        // the gap closing fill's queue.

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

#if MEASURE_FILL_TIME
        totalScanlineFillNanos += timerScanlineFill.nsecsElapsed();
        timerGapClosingFill.start();
#endif

        // Perform a gap closing pass. This pass can in turn feed the scanline fill's
        // forward stack. It can only fiill pixels with distance < DISTANCE_INFINITE.
        // This way, both passes complement each other to complete the fill.

        if (!m_d->closeGapQueue.empty()) {
            startInterval = closeGapPass(differencePolicy, selectionPolicy, pixelAccessPolicy);
            if (startInterval.isValid()) {
                m_d->forwardStack.push(startInterval);
            }
        }

#if MEASURE_FILL_TIME
        totalGapClosingFillNanos += timerGapClosingFill.nsecsElapsed();
#endif
    } while (!m_d->forwardStack.isEmpty());

#if MEASURE_FILL_TIME
    static constexpr quint64 MillisDivisor = 1000000ull;
    const quint64 totalTime = timerTotal.nsecsElapsed();
    const quint64 overheadTime = (totalTime - totalScanlineFillNanos - totalGapClosingFillNanos);
    qDebug() << "init overhead =" << qSetRealNumberPrecision(3)
              << static_cast<double>(overheadTime) / MillisDivisor << "ms ("
              << static_cast<double>(overheadTime) / totalTime << ")";
    qDebug() << "fill (scanline) =" << (totalScanlineFillNanos / MillisDivisor) << "ms ("
             << qSetRealNumberPrecision(3) << static_cast<double>(totalScanlineFillNanos) / totalTime << ")";
    qDebug() << "fill (gap) =" << (totalGapClosingFillNanos / MillisDivisor) << "ms ("
             << qSetRealNumberPrecision(3) << static_cast<double>(totalGapClosingFillNanos) / totalTime << ")";
    qDebug() << "fill total =" << (totalTime / MillisDivisor) << "ms";
#if KIS_GAP_MAP_MEASURE_ELAPSED_TIME
    if (gapSize > 0) {
        qDebug() << "incl. opacity load" << m_d->gapMapSp->opacityElapsedMillis() << "ms";
        qDebug() << "incl. distance load" << m_d->gapMapSp->distanceElapsedMillis() << "ms";
    }
#endif
    qDebug() << "----------------------------------------";
#endif
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

    if (m_d->closeGap > 0) {
        m_d->filledSelectionIterator = pixelSelection->createRandomAccessorNG();
    }

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

    if (m_d->closeGap > 0) {
        m_d->filledSelectionIterator = pixelSelection->createRandomAccessorNG();
    }

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

    if (m_d->closeGap > 0) {
        m_d->filledSelectionIterator = pixelSelection->createRandomAccessorNG();
    }

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

    if (m_d->closeGap > 0) {
        m_d->filledSelectionIterator = pixelSelection->createRandomAccessorNG();
    }

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

    if (m_d->closeGap > 0) {
        m_d->filledSelectionIterator = pixelSelection->createRandomAccessorNG();
    }

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

    if (m_d->closeGap > 0) {
        m_d->filledSelectionIterator = pixelSelection->createRandomAccessorNG();
    }

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

    if (m_d->closeGap > 0) {
        m_d->filledSelectionIterator = m_d->device->createRandomAccessorNG();
    }

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

/**
 * This opacity-only fill is used by KisGapMap.
 *
 * Using the same policies as the main fill, load the opacity values for
 * a rect of the main filled region. The rect typically corresponds to
 * a tile of an opacity map maintained by KisGapMap, and is loaded on-demand
 * during an ongoing runImpl() loop.
 */
template <typename DifferencePolicy, typename SelectionPolicy, typename PixelAccessPolicy>
bool KisScanlineFill::fillOpacity(DifferencePolicy &differencePolicy,
                                  SelectionPolicy &selectionPolicy,
                                  PixelAccessPolicy &pixelAccessPolicy,
                                  KisPaintDevice* const devicePtr,
                                  const QRect& rect) const
{
#if 0
    // These asserts affect the performance.
    KIS_SAFE_ASSERT_RECOVER_RETURN((m_d->boundingRect.left() == 0) && (m_d->boundingRect.top() == 0) &&
               "FATAL: The fill bounds must start at (0,0)");
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->boundingRect.contains(rect) &&
               "FATAL: The rect is not fully inside the fill bounds");
#endif
    KisRandomAccessorSP accessor = devicePtr->createRandomAccessorNG();

    const int pixelSize = m_d->device->pixelSize();
    bool hasOpaquePixels = false;

    // We are relying on the knowledge of KisGapMap's paint device data organization.
    constexpr int OpacityOffset = 2;

    for (int y = rect.top(); y <= rect.bottom(); ++y) {
        int numPixelsLeft = 0;
        quint8* dataPtr;

        for (int x = rect.left(); x <= rect.right(); ++x) {
            if (numPixelsLeft <= 0) {
                pixelAccessPolicy.m_srcIt->moveTo(x, y);
                numPixelsLeft = pixelAccessPolicy.m_srcIt->numContiguousColumns(x) - 1;
                dataPtr = const_cast<quint8*>(pixelAccessPolicy.m_srcIt->rawDataConst());
            } else {
                numPixelsLeft--;
                dataPtr += pixelSize;
            }

            const quint8 difference = differencePolicy.difference(dataPtr);
            const quint8 opacity = selectionPolicy.opacityFromDifference(difference, x, y);

            if (opacity == MIN_SELECTED) {
                hasOpaquePixels = true;

                accessor->moveTo(x, y);
                quint8* outPtr = accessor->rawData() + OpacityOffset;

                *outPtr = opacity;
            }
        }
    }

    return hasOpaquePixels;
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
