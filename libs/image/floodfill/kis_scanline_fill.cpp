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


template <class BaseClass>
class CopyToSelection : public BaseClass
{
public:
    typedef KisRandomConstAccessorSP SourceAccessorType;

    SourceAccessorType createSourceDeviceAccessor(KisPaintDeviceSP device) {
        return device->createRandomConstAccessorNG();
    }

public:
    void setDestinationSelection(KisPaintDeviceSP pixelSelection) {
        m_pixelSelection = pixelSelection;
        m_it = m_pixelSelection->createRandomAccessorNG();
    }

    ALWAYS_INLINE void fillPixel(quint8 *dstPtr, quint8 opacity, int x, int y) {
        Q_UNUSED(dstPtr);
        m_it->moveTo(x, y);
        *m_it->rawData() = opacity;
    }

private:
    KisPaintDeviceSP m_pixelSelection;
    KisRandomAccessorSP m_it;
};

template <class BaseClass>
class FillWithColor : public BaseClass
{
public:
    typedef KisRandomAccessorSP SourceAccessorType;

    SourceAccessorType createSourceDeviceAccessor(KisPaintDeviceSP device) {
        return device->createRandomAccessorNG();
    }

public:
    FillWithColor() : m_pixelSize(0) {}

    void setFillColor(const KoColor &sourceColor) {
        m_sourceColor = sourceColor;
        m_pixelSize = sourceColor.colorSpace()->pixelSize();
        m_data = m_sourceColor.data();
    }

    ALWAYS_INLINE void fillPixel(quint8 *dstPtr, quint8 opacity, int x, int y) {
        Q_UNUSED(x);
        Q_UNUSED(y);

        if (opacity == MAX_SELECTED) {
            memcpy(dstPtr, m_data, m_pixelSize);
        }
    }

private:
    KoColor m_sourceColor;
    const quint8 *m_data;
    int m_pixelSize;
};

template <class BaseClass>
class FillWithColorExternal : public BaseClass
{
public:
    typedef KisRandomConstAccessorSP SourceAccessorType;

    SourceAccessorType createSourceDeviceAccessor(KisPaintDeviceSP device) {
        return device->createRandomConstAccessorNG();
    }

public:
    void setDestinationDevice(KisPaintDeviceSP device) {
        m_externalDevice = device;
        m_it = m_externalDevice->createRandomAccessorNG();
    }

    void setFillColor(const KoColor &sourceColor) {
        m_sourceColor = sourceColor;
        m_pixelSize = sourceColor.colorSpace()->pixelSize();
        m_data = m_sourceColor.data();
    }

    ALWAYS_INLINE void fillPixel(quint8 *dstPtr, quint8 opacity, int x, int y) {
        Q_UNUSED(dstPtr);

        m_it->moveTo(x, y);
        if (opacity == MAX_SELECTED) {
            memcpy(m_it->rawData(), m_data, m_pixelSize);
        }
    }

private:
    KisPaintDeviceSP m_externalDevice;
    KisRandomAccessorSP m_it;

    KoColor m_sourceColor;
    const quint8 *m_data {0};
    int m_pixelSize {0};
};

class DifferencePolicySlow
{
public:
    ALWAYS_INLINE void initDifferences(KisPaintDeviceSP device, const KoColor &srcPixel, int threshold) {
        m_colorSpace = device->colorSpace();
        m_srcPixel = srcPixel;
        m_srcPixelPtr = m_srcPixel.data();
        m_threshold = threshold;
    }

    ALWAYS_INLINE quint8 calculateDifference(quint8* pixelPtr) {
        if (m_threshold == 1) {
            if (memcmp(m_srcPixelPtr, pixelPtr, m_colorSpace->pixelSize()) == 0) {
                return 0;
            }
            return quint8_MAX;
        }
        else {
            return m_colorSpace->differenceA(m_srcPixelPtr, pixelPtr);
        }
    }

private:
    const KoColorSpace *m_colorSpace;
    KoColor m_srcPixel;
    const quint8 *m_srcPixelPtr;
    int m_threshold;
};

template <typename SrcPixelType>
class DifferencePolicyOptimized
{
    typedef SrcPixelType HashKeyType;
    typedef QHash<HashKeyType, quint8> HashType;

public:
    ALWAYS_INLINE void initDifferences(KisPaintDeviceSP device, const KoColor &srcPixel, int threshold) {
        m_colorSpace = device->colorSpace();
        m_srcPixel = srcPixel;
        m_srcPixelPtr = m_srcPixel.data();
        m_threshold = threshold;
    }

    ALWAYS_INLINE quint8 calculateDifference(quint8* pixelPtr) {
        HashKeyType key = *reinterpret_cast<HashKeyType*>(pixelPtr);

        quint8 result;

        typename HashType::iterator it = m_differences.find(key);

        if (it != m_differences.end()) {
            result = *it;
        } else {
            if (m_threshold == 1) {
                if (memcmp(m_srcPixelPtr, pixelPtr, m_colorSpace->pixelSize()) == 0) {
                    result = 0;
                }
                else {
                    result = quint8_MAX;
                }
            }
            else {
                result = m_colorSpace->differenceA(m_srcPixelPtr, pixelPtr);
            }
            m_differences.insert(key, result);
        }

        return result;
    }

private:
    HashType m_differences;

    const KoColorSpace *m_colorSpace;
    KoColor m_srcPixel;
    const quint8 *m_srcPixelPtr;
    int m_threshold;
};

class SelectednessPolicyOptimized
{
    typedef quint8 HashKeyType;
    typedef QHash<HashKeyType, quint8> HashType;

    KisRandomConstAccessorSP m_selectionIt;

public:

    SelectednessPolicyOptimized()
        : m_colorSpace(0)
        , m_threshold(0)
    {
    }

    ALWAYS_INLINE void initSelectedness(KisPaintDeviceSP device, int threshold) {
        m_colorSpace = device->colorSpace();
        m_threshold = threshold;
        m_selectionIt = device->createRandomConstAccessorNG();
    }

    ALWAYS_INLINE quint8 calculateSelectedness(int x, int y) {
        m_selectionIt->moveTo(x, y);
        const quint8* pixelPtr = m_selectionIt->rawDataConst();
        return *pixelPtr;
    }

private:
    HashType m_selectedness;

    const KoColorSpace *m_colorSpace;
    int m_threshold;
};


template <class DifferencePolicy,
          template <class> class PixelFiller>
class HardSelectionPolicy: public PixelFiller<DifferencePolicy>
{
public:
    typename PixelFiller<DifferencePolicy>::SourceAccessorType m_srcIt;

    HardSelectionPolicy(KisPaintDeviceSP device, const KoColor &srcPixel, int threshold)
        : m_threshold(threshold)
    {
        this->initDifferences(device, srcPixel, threshold);
        m_srcIt = this->createSourceDeviceAccessor(device);
    }

    ALWAYS_INLINE quint8 calculateOpacity(quint8* pixelPtr, int, int) {
        return this->calculateDifference(pixelPtr) <= m_threshold ? MAX_SELECTED : MIN_SELECTED;
    }

protected:
    int m_threshold;
};

template <class DifferencePolicy,
          template <class> class PixelFiller>
class SoftSelectionPolicy : public HardSelectionPolicy<DifferencePolicy, PixelFiller>
{
public:
    SoftSelectionPolicy(KisPaintDeviceSP device, const KoColor &srcPixel, int threshold, int softness)
        : HardSelectionPolicy<DifferencePolicy, PixelFiller>(device, srcPixel, threshold)
        , m_softness(softness)
    {}

    ALWAYS_INLINE quint8 calculateOpacity(quint8* pixelPtr, int, int) {
        if (m_threshold == 0) {
            return MIN_SELECTED;
        }
        // Integer version of: (threshold - diff) / (threshold * softness)
        const int diff = this->calculateDifference(pixelPtr);
        if (diff < m_threshold) {
            const int v = (m_threshold - diff) * MAX_SELECTED * 100 / (m_threshold * m_softness);
            return v > MAX_SELECTED ? MAX_SELECTED : v;
        } else {
            return MIN_SELECTED;
        }
    }

protected:
    using HardSelectionPolicy<DifferencePolicy, PixelFiller>::m_threshold;
    int m_softness;
};



template <class DifferencePolicy,
          template <class> class PixelFiller,
          class SelectednessCheckPolicy>
class HardSelectionPolicyExtended : public HardSelectionPolicy<DifferencePolicy, PixelFiller>
                                  , public SelectednessCheckPolicy
{
public:
    HardSelectionPolicyExtended(KisPaintDeviceSP mainDevice, KisPaintDeviceSP selectionDevice, const KoColor &srcPixel, int threshold)
        : HardSelectionPolicy<DifferencePolicy, PixelFiller>(mainDevice, srcPixel, threshold)
    {
        this->initSelectedness(selectionDevice, threshold);
    }
    
    ALWAYS_INLINE quint8 calculateOpacity(quint8* pixelPtr, int x, int y) {
        return this->calculateDifference(pixelPtr) < m_threshold && this->calculateSelectedness(x, y) > 0 ? MAX_SELECTED : MIN_SELECTED;
    }

protected:
    using HardSelectionPolicy<DifferencePolicy, PixelFiller>::m_threshold;
};

template <class DifferencePolicy,
          template <class> class PixelFiller,
          class SelectednessCheckPolicy>
class SoftSelectionPolicyExtended : public HardSelectionPolicyExtended<DifferencePolicy, PixelFiller, SelectednessCheckPolicy>
{
public:
    SoftSelectionPolicyExtended(KisPaintDeviceSP mainDevice, KisPaintDeviceSP selectionDevice, const KoColor &srcPixel, int threshold, int softness)
        : HardSelectionPolicyExtended<DifferencePolicy, PixelFiller, SelectednessCheckPolicy>(mainDevice, selectionDevice, srcPixel, threshold)
        , m_softness(softness)
    {}
    
    ALWAYS_INLINE quint8 calculateOpacity(quint8* pixelPtr, int x, int y) {
        if (m_threshold == 0) {
            return MIN_SELECTED;
        }
        const quint8 selectedness = this->calculateSelectedness(x, y);
        if (selectedness == 0) {
            return MIN_SELECTED;
        }
        // Integer version of: (threshold - diff) / (threshold * softness)
        const int diff = this->calculateDifference(pixelPtr);
        if (diff < m_threshold) {
            const int v = (m_threshold - diff) * MAX_SELECTED * 100 / (m_threshold * m_softness);
            return v > MAX_SELECTED ? MAX_SELECTED : v;
        } else {
            return MIN_SELECTED;
        }
    }

protected:
    using HardSelectionPolicyExtended<DifferencePolicy, PixelFiller, SelectednessCheckPolicy>::m_threshold;
    int m_softness;
};






class IsNonNullPolicySlow
{
public:
    ALWAYS_INLINE void initDifferences(KisPaintDeviceSP device, const KoColor &srcPixel, int /*threshold*/)
    {
        Q_UNUSED(srcPixel);
        m_pixelSize = device->pixelSize();
        m_testPixel.resize(m_pixelSize);
    }

    ALWAYS_INLINE quint8 calculateDifference(quint8* pixelPtr) {
        if (memcmp(m_testPixel.data(), pixelPtr, m_pixelSize) == 0) {
            return 0;
        }
        return quint8_MAX;
    }

private:
    int m_pixelSize {0};
    QByteArray m_testPixel;
};

template <typename SrcPixelType>
class IsNonNullPolicyOptimized
{
public:
    ALWAYS_INLINE void initDifferences(KisPaintDeviceSP device, const KoColor &srcPixel, int /*threshold*/) {
        Q_UNUSED(device);
        Q_UNUSED(srcPixel);
    }

    ALWAYS_INLINE quint8 calculateDifference(quint8* pixelPtr) {
        SrcPixelType *pixel = reinterpret_cast<SrcPixelType*>(pixelPtr);
        return *pixel == 0;
    }
};

class GroupSplitPolicy
{
public:
    typedef KisRandomConstAccessorSP SourceAccessorType;
    SourceAccessorType m_srcIt;

public:
    GroupSplitPolicy(KisPaintDeviceSP scribbleDevice,
                     KisPaintDeviceSP groupMapDevice,
                     qint32 groupIndex,
                     quint8 referenceValue, int threshold)
        : m_threshold(threshold),
          m_groupIndex(groupIndex),
          m_referenceValue(referenceValue)
    {
        KIS_SAFE_ASSERT_RECOVER_NOOP(m_groupIndex > 0);

        m_srcIt = scribbleDevice->createRandomConstAccessorNG();
        m_groupMapIt = groupMapDevice->createRandomAccessorNG();
    }

    ALWAYS_INLINE quint8 calculateOpacity(quint8* pixelPtr, int x, int y) {
        // TODO: either threshold should always be null, or there should be a special
        //       case for *pixelPtr == 0, which is different from all the other groups,
        //       whatever the threshold is

        Q_UNUSED(x);
        Q_UNUSED(y);
        int diff = qAbs(int(*pixelPtr) - m_referenceValue);
        return diff <= m_threshold ? MAX_SELECTED : MIN_SELECTED;
    }

    ALWAYS_INLINE void fillPixel(quint8 *dstPtr, quint8 opacity, int x, int y) {
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
    int m_threshold;
    qint32 m_groupIndex;
    quint8 m_referenceValue;
    KisRandomAccessorSP m_groupMapIt;
};



struct Q_DECL_HIDDEN KisScanlineFill::Private
{
    KisPaintDeviceSP device;
    QPoint startPoint;
    QRect boundingRect;
    int threshold;
    int softness;

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
    m_d->softness = 100;
}

KisScanlineFill::~KisScanlineFill()
{
}

void KisScanlineFill::setThreshold(int threshold)
{
    m_d->threshold = threshold;
}

void KisScanlineFill::setSoftness(int softness)
{
    m_d->softness = softness;
}

template <class T>
void KisScanlineFill::extendedPass(KisFillInterval *currentInterval, int srcRow, bool extendRight, T &pixelPolicy)
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

        pixelPolicy.m_srcIt->moveTo(x, srcRow);
        quint8 *pixelPtr = const_cast<quint8*>(pixelPolicy.m_srcIt->rawDataConst()); // TODO: avoid doing const_cast
        quint8 opacity = pixelPolicy.calculateOpacity(pixelPtr, x, srcRow);

        if (opacity) {
            *intervalBorder = x;
            *backwardIntervalBorder = x;
            pixelPolicy.fillPixel(pixelPtr, opacity, x, srcRow);
        } else {
            break;
        }
    } while (x != endX);

    if (backwardInterval.isValid()) {
        m_d->backwardMap.insertInterval(backwardInterval);
    }
}

template <class T>
void KisScanlineFill::processLine(KisFillInterval interval, const int rowIncrement, T &pixelPolicy)
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
        // a bit of optimzation for not calling slow random accessor
        // methods too often
        if (numPixelsLeft <= 0) {
            pixelPolicy.m_srcIt->moveTo(x, row);
            numPixelsLeft = pixelPolicy.m_srcIt->numContiguousColumns(x) - 1;
            dataPtr = const_cast<quint8*>(pixelPolicy.m_srcIt->rawDataConst());
        } else {
            numPixelsLeft--;
            dataPtr += pixelSize;
        }

        quint8 *pixelPtr = dataPtr;
        quint8 opacity = pixelPolicy.calculateOpacity(pixelPtr, x, row);

        if (opacity) {
            if (!currentForwardInterval.isValid()) {
                currentForwardInterval.start = x;
                currentForwardInterval.end = x;
                currentForwardInterval.row = nextRow;
            } else {
                currentForwardInterval.end = x;
            }

            pixelPolicy.fillPixel(pixelPtr, opacity, x, row);

            if (x == firstX) {
                extendedPass(&currentForwardInterval, row, false, pixelPolicy);
            }

            if (x == lastX) {
                extendedPass(&currentForwardInterval, row, true, pixelPolicy);
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

template <class T>
void KisScanlineFill::runImpl(T &pixelPolicy)
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

            processLine(interval, m_d->rowIncrement, pixelPolicy);
        }
        m_d->swapDirection();

        if (firstPass) {
            startInterval.row--;
            m_d->forwardStack.push(startInterval);
            firstPass = false;
        }
    }
}

void KisScanlineFill::fillColor(const KoColor &originalFillColor)
{
    KoColor srcColor(m_d->device->pixel(m_d->startPoint));
    KoColor fillColor(originalFillColor);
    fillColor.convertTo(m_d->device->colorSpace());

    const int pixelSize = m_d->device->pixelSize();

    if (pixelSize == 1) {
        HardSelectionPolicy<DifferencePolicyOptimized<quint8>, FillWithColor>
            policy(m_d->device, srcColor, m_d->threshold);
        policy.setFillColor(fillColor);
        runImpl(policy);
    } else if (pixelSize == 2) {
        HardSelectionPolicy<DifferencePolicyOptimized<quint16>, FillWithColor>
            policy(m_d->device, srcColor, m_d->threshold);
        policy.setFillColor(fillColor);
        runImpl(policy);
    } else if (pixelSize == 4) {
        HardSelectionPolicy<DifferencePolicyOptimized<quint32>, FillWithColor>
            policy(m_d->device, srcColor, m_d->threshold);
        policy.setFillColor(fillColor);
        runImpl(policy);
    } else if (pixelSize == 8) {
        HardSelectionPolicy<DifferencePolicyOptimized<quint64>, FillWithColor>
            policy(m_d->device, srcColor, m_d->threshold);
        policy.setFillColor(fillColor);
        runImpl(policy);
    } else {
        HardSelectionPolicy<DifferencePolicySlow, FillWithColor>
            policy(m_d->device, srcColor, m_d->threshold);
        policy.setFillColor(fillColor);
        runImpl(policy);
    }
}

void KisScanlineFill::fillColor(const KoColor &originalFillColor, KisPaintDeviceSP externalDevice)
{
    KoColor srcColor(m_d->device->pixel(m_d->startPoint));
    KoColor fillColor(originalFillColor);
    fillColor.convertTo(m_d->device->colorSpace());

    const int pixelSize = m_d->device->pixelSize();

    if (pixelSize == 1) {
        HardSelectionPolicy<DifferencePolicyOptimized<quint8>, FillWithColorExternal>
            policy(m_d->device, srcColor, m_d->threshold);
        policy.setDestinationDevice(externalDevice);
        policy.setFillColor(fillColor);
        runImpl(policy);
    } else if (pixelSize == 2) {
        HardSelectionPolicy<DifferencePolicyOptimized<quint16>, FillWithColorExternal>
            policy(m_d->device, srcColor, m_d->threshold);
        policy.setDestinationDevice(externalDevice);
        policy.setFillColor(fillColor);
        runImpl(policy);
    } else if (pixelSize == 4) {
        HardSelectionPolicy<DifferencePolicyOptimized<quint32>, FillWithColorExternal>
            policy(m_d->device, srcColor, m_d->threshold);
        policy.setDestinationDevice(externalDevice);
        policy.setFillColor(fillColor);
        runImpl(policy);
    } else if (pixelSize == 8) {
        HardSelectionPolicy<DifferencePolicyOptimized<quint64>, FillWithColorExternal>
            policy(m_d->device, srcColor, m_d->threshold);
        policy.setDestinationDevice(externalDevice);
        policy.setFillColor(fillColor);
        runImpl(policy);
    } else {
        HardSelectionPolicy<DifferencePolicySlow, FillWithColorExternal>
            policy(m_d->device, srcColor, m_d->threshold);
        policy.setDestinationDevice(externalDevice);
        policy.setFillColor(fillColor);
        runImpl(policy);
    }
}

void KisScanlineFill::fillSelectionWithBoundary(KisPixelSelectionSP pixelSelection, KisPaintDeviceSP existingSelection)
{
    KoColor srcColor(m_d->device->pixel(m_d->startPoint));

    const int pixelSize = m_d->device->pixelSize();

    if (m_d->softness == 0) {
        if (pixelSize == 1) {
            HardSelectionPolicyExtended<DifferencePolicyOptimized<quint8>, CopyToSelection, SelectednessPolicyOptimized>
                policy(m_d->device, existingSelection, srcColor, m_d->threshold);
            policy.setDestinationSelection(pixelSelection);
            runImpl(policy);
        } else if (pixelSize == 2) {
            HardSelectionPolicyExtended<DifferencePolicyOptimized<quint16>, CopyToSelection, SelectednessPolicyOptimized>
                policy(m_d->device, existingSelection, srcColor, m_d->threshold);
            policy.setDestinationSelection(pixelSelection);
            runImpl(policy);
        } else if (pixelSize == 4) {
            HardSelectionPolicyExtended<DifferencePolicyOptimized<quint32>, CopyToSelection, SelectednessPolicyOptimized>
                policy(m_d->device, existingSelection, srcColor, m_d->threshold);
            policy.setDestinationSelection(pixelSelection);
            runImpl(policy);

        } else if (pixelSize == 8) {
            HardSelectionPolicyExtended<DifferencePolicyOptimized<quint64>, CopyToSelection, SelectednessPolicyOptimized>
                policy(m_d->device, existingSelection, srcColor, m_d->threshold);
            policy.setDestinationSelection(pixelSelection);
            runImpl(policy);
        } else {
            HardSelectionPolicyExtended<DifferencePolicySlow, CopyToSelection, SelectednessPolicyOptimized>
                policy(m_d->device, existingSelection, srcColor, m_d->threshold);
            policy.setDestinationSelection(pixelSelection);
            runImpl(policy);
        }
    } else {
        if (pixelSize == 1) {
            SoftSelectionPolicyExtended<DifferencePolicyOptimized<quint8>, CopyToSelection, SelectednessPolicyOptimized>
                policy(m_d->device, existingSelection, srcColor, m_d->threshold, m_d->softness);
            policy.setDestinationSelection(pixelSelection);
            runImpl(policy);
        } else if (pixelSize == 2) {
            SoftSelectionPolicyExtended<DifferencePolicyOptimized<quint16>, CopyToSelection, SelectednessPolicyOptimized>
                policy(m_d->device, existingSelection, srcColor, m_d->threshold, m_d->softness);
            policy.setDestinationSelection(pixelSelection);
            runImpl(policy);
        } else if (pixelSize == 4) {
            SoftSelectionPolicyExtended<DifferencePolicyOptimized<quint32>, CopyToSelection, SelectednessPolicyOptimized>
                policy(m_d->device, existingSelection, srcColor, m_d->threshold, m_d->softness);
            policy.setDestinationSelection(pixelSelection);
            runImpl(policy);

        } else if (pixelSize == 8) {
            SoftSelectionPolicyExtended<DifferencePolicyOptimized<quint64>, CopyToSelection, SelectednessPolicyOptimized>
                policy(m_d->device, existingSelection, srcColor, m_d->threshold, m_d->softness);
            policy.setDestinationSelection(pixelSelection);
            runImpl(policy);
        } else {
            SoftSelectionPolicyExtended<DifferencePolicySlow, CopyToSelection, SelectednessPolicyOptimized>
                policy(m_d->device, existingSelection, srcColor, m_d->threshold, m_d->softness);
            policy.setDestinationSelection(pixelSelection);
            runImpl(policy);
        }
    }
}

void KisScanlineFill::fillSelection(KisPixelSelectionSP pixelSelection)
{
    KoColor srcColor(m_d->device->pixel(m_d->startPoint));

    const int pixelSize = m_d->device->pixelSize();

    if (m_d->softness == 0) {
        if (pixelSize == 1) {
            HardSelectionPolicy<DifferencePolicyOptimized<quint8>, CopyToSelection>
                policy(m_d->device, srcColor, m_d->threshold);
            policy.setDestinationSelection(pixelSelection);
            runImpl(policy);
        } else if (pixelSize == 2) {
            HardSelectionPolicy<DifferencePolicyOptimized<quint16>, CopyToSelection>
                policy(m_d->device, srcColor, m_d->threshold);
            policy.setDestinationSelection(pixelSelection);
            runImpl(policy);
        } else if (pixelSize == 4) {
            HardSelectionPolicy<DifferencePolicyOptimized<quint32>, CopyToSelection>
                policy(m_d->device, srcColor, m_d->threshold);
            policy.setDestinationSelection(pixelSelection);
            runImpl(policy);
        } else if (pixelSize == 8) {
            HardSelectionPolicy<DifferencePolicyOptimized<quint64>, CopyToSelection>
                policy(m_d->device, srcColor, m_d->threshold);
            policy.setDestinationSelection(pixelSelection);
            runImpl(policy);
        } else {
            HardSelectionPolicy<DifferencePolicySlow, CopyToSelection>
                policy(m_d->device, srcColor, m_d->threshold);
            policy.setDestinationSelection(pixelSelection);
            runImpl(policy);
        }
    } else {
        if (pixelSize == 1) {
            SoftSelectionPolicy<DifferencePolicyOptimized<quint8>, CopyToSelection>
                policy(m_d->device, srcColor, m_d->threshold, m_d->softness);
            policy.setDestinationSelection(pixelSelection);
            runImpl(policy);
        } else if (pixelSize == 2) {
            SoftSelectionPolicy<DifferencePolicyOptimized<quint16>, CopyToSelection>
                policy(m_d->device, srcColor, m_d->threshold, m_d->softness);
            policy.setDestinationSelection(pixelSelection);
            runImpl(policy);
        } else if (pixelSize == 4) {
            SoftSelectionPolicy<DifferencePolicyOptimized<quint32>, CopyToSelection>
                policy(m_d->device, srcColor, m_d->threshold, m_d->softness);
            policy.setDestinationSelection(pixelSelection);
            runImpl(policy);
        } else if (pixelSize == 8) {
            SoftSelectionPolicy<DifferencePolicyOptimized<quint64>, CopyToSelection>
                policy(m_d->device, srcColor, m_d->threshold, m_d->softness);
            policy.setDestinationSelection(pixelSelection);
            runImpl(policy);
        } else {
            SoftSelectionPolicy<DifferencePolicySlow, CopyToSelection>
                policy(m_d->device, srcColor, m_d->threshold, m_d->softness);
            policy.setDestinationSelection(pixelSelection);
            runImpl(policy);
        }
    }
}

void KisScanlineFill::clearNonZeroComponent()
{
    const int pixelSize = m_d->device->pixelSize();
    KoColor srcColor(Qt::transparent, m_d->device->colorSpace());

    if (pixelSize == 1) {
        HardSelectionPolicy<IsNonNullPolicyOptimized<quint8>, FillWithColor>
            policy(m_d->device, srcColor, m_d->threshold);
        policy.setFillColor(srcColor);
        runImpl(policy);
    } else if (pixelSize == 2) {
        HardSelectionPolicy<IsNonNullPolicyOptimized<quint16>, FillWithColor>
            policy(m_d->device, srcColor, m_d->threshold);
        policy.setFillColor(srcColor);
        runImpl(policy);
    } else if (pixelSize == 4) {
        HardSelectionPolicy<IsNonNullPolicyOptimized<quint32>, FillWithColor>
            policy(m_d->device, srcColor, m_d->threshold);
        policy.setFillColor(srcColor);
        runImpl(policy);
    } else if (pixelSize == 8) {
        HardSelectionPolicy<IsNonNullPolicyOptimized<quint64>, FillWithColor>
              policy(m_d->device, srcColor, m_d->threshold);
        policy.setFillColor(srcColor);
        runImpl(policy);
    } else {
        HardSelectionPolicy<IsNonNullPolicySlow, FillWithColor>
              policy(m_d->device, srcColor, m_d->threshold);
        policy.setFillColor(srcColor);
        runImpl(policy);
    }
}

void KisScanlineFill::fillContiguousGroup(KisPaintDeviceSP groupMapDevice, qint32 groupIndex)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->device->pixelSize() == 1);
    KIS_SAFE_ASSERT_RECOVER_RETURN(groupMapDevice->pixelSize() == 4);

    const quint8 referenceValue = *m_d->device->pixel(m_d->startPoint).data();

    GroupSplitPolicy policy(m_d->device, groupMapDevice, groupIndex, referenceValue, m_d->threshold);
    runImpl(policy);
}

void KisScanlineFill::testingProcessLine(const KisFillInterval &processInterval)
{
    KoColor srcColor(QColor(0,0,0,0), m_d->device->colorSpace());
    KoColor fillColor(QColor(200,200,200,200), m_d->device->colorSpace());

    HardSelectionPolicy<DifferencePolicyOptimized<quint32>, FillWithColor>
        policy(m_d->device, srcColor, m_d->threshold);

    policy.setFillColor(fillColor);

    processLine(processInterval, 1, policy);
}

QVector<KisFillInterval> KisScanlineFill::testingGetForwardIntervals() const
{
    return QVector<KisFillInterval>(m_d->forwardStack);
}

KisFillIntervalMap* KisScanlineFill::testingGetBackwardIntervals() const
{
    return &m_d->backwardMap;
}
