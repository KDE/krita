/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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
        return device->createRandomConstAccessorNG(0, 0);
    }

public:
    void setDestinationSelection(KisPaintDeviceSP pixelSelection) {
        m_pixelSelection = pixelSelection;
        m_it = m_pixelSelection->createRandomAccessorNG(0,0);
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
        return device->createRandomAccessorNG(0, 0);
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
        return device->createRandomConstAccessorNG(0, 0);
    }

public:
    void setDestinationDevice(KisPaintDeviceSP device) {
        m_externalDevice = device;
        m_it = m_externalDevice->createRandomAccessorNG(0,0);
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
    const quint8 *m_data;
    int m_pixelSize;
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
            return m_colorSpace->difference(m_srcPixelPtr, pixelPtr);
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
                result = m_colorSpace->difference(m_srcPixelPtr, pixelPtr);
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

template <bool useSmoothSelection,
          class DifferencePolicy,
          template <class> class PixelFiller>
class SelectionPolicy : public PixelFiller<DifferencePolicy>
{
public:
    typename PixelFiller<DifferencePolicy>::SourceAccessorType m_srcIt;

public:
    SelectionPolicy(KisPaintDeviceSP device, const KoColor &srcPixel, int threshold)
        : m_threshold(threshold)
    {
        this->initDifferences(device, srcPixel, threshold);
        m_srcIt = this->createSourceDeviceAccessor(device);
    }

    ALWAYS_INLINE quint8 calculateOpacity(quint8* pixelPtr) {
        quint8 diff = this->calculateDifference(pixelPtr);

        if (!useSmoothSelection) {
            return diff <= m_threshold ? MAX_SELECTED : MIN_SELECTED;
        } else {
            quint8 selectionValue = qMax(0, m_threshold - diff);

            quint8 result = MIN_SELECTED;

            if (selectionValue > 0) {
                qreal selectionNorm = qreal(selectionValue) / m_threshold;
                result = MAX_SELECTED * selectionNorm;
            }

            return result;
        }
    }

private:
    int m_threshold;
};

class IsNonNullPolicySlow
{
public:
    ALWAYS_INLINE void initDifferences(KisPaintDeviceSP device, const KoColor &srcPixel, int /*threshold*/) {
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
    int m_pixelSize;
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
    typedef KisRandomAccessorSP SourceAccessorType;
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

        m_srcIt = scribbleDevice->createRandomAccessorNG(0,0);
        m_groupMapIt = groupMapDevice->createRandomAccessorNG(0,0);
    }

    ALWAYS_INLINE quint8 calculateOpacity(quint8* pixelPtr) {
        // TODO: either threshold should always be null, or there should be a special
        //       case for *pixelPtr == 0, which is different from all the other groups,
        //       whatever the threshold is

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
    KisRandomAccessorSP it;
    QPoint startPoint;
    QRect boundingRect;
    int threshold;

    int rowIncrement;
    KisFillIntervalMap backwardMap;
    QStack<KisFillInterval> forwardStack;


    inline void swapDirection() {
        rowIncrement *= -1;
        SANITY_ASSERT_MSG(forwardStack.isEmpty(),
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
    m_d->it = device->createRandomAccessorNG(startPoint.x(), startPoint.y());
    m_d->startPoint = startPoint;
    m_d->boundingRect = boundingRect;

    m_d->rowIncrement = 1;

    m_d->threshold = 0;
}

KisScanlineFill::~KisScanlineFill()
{
}

void KisScanlineFill::setThreshold(int threshold)
{
    m_d->threshold = threshold;
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
        quint8 opacity = pixelPolicy.calculateOpacity(pixelPtr);

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
        quint8 opacity = pixelPolicy.calculateOpacity(pixelPtr);

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
    KisRandomConstAccessorSP it = m_d->device->createRandomConstAccessorNG(m_d->startPoint.x(), m_d->startPoint.y());
    KoColor srcColor(it->rawDataConst(), m_d->device->colorSpace());
    KoColor fillColor(originalFillColor);
    fillColor.convertTo(m_d->device->colorSpace());

    const int pixelSize = m_d->device->pixelSize();

    if (pixelSize == 1) {
        SelectionPolicy<false, DifferencePolicyOptimized<quint8>, FillWithColor>
            policy(m_d->device, srcColor, m_d->threshold);
        policy.setFillColor(fillColor);
        runImpl(policy);
    } else if (pixelSize == 2) {
        SelectionPolicy<false, DifferencePolicyOptimized<quint16>, FillWithColor>
            policy(m_d->device, srcColor, m_d->threshold);
        policy.setFillColor(fillColor);
        runImpl(policy);
    } else if (pixelSize == 4) {
        SelectionPolicy<false, DifferencePolicyOptimized<quint32>, FillWithColor>
            policy(m_d->device, srcColor, m_d->threshold);
        policy.setFillColor(fillColor);
        runImpl(policy);
    } else if (pixelSize == 8) {
        SelectionPolicy<false, DifferencePolicyOptimized<quint64>, FillWithColor>
              policy(m_d->device, srcColor, m_d->threshold);
        policy.setFillColor(fillColor);
        runImpl(policy);
    } else {
        SelectionPolicy<false, DifferencePolicySlow, FillWithColor>
              policy(m_d->device, srcColor, m_d->threshold);
        policy.setFillColor(fillColor);
        runImpl(policy);
    }
}

void KisScanlineFill::fillColor(const KoColor &originalFillColor, KisPaintDeviceSP externalDevice)
{
    KisRandomConstAccessorSP it = m_d->device->createRandomConstAccessorNG(m_d->startPoint.x(), m_d->startPoint.y());
    KoColor srcColor(it->rawDataConst(), m_d->device->colorSpace());
    KoColor fillColor(originalFillColor);
    fillColor.convertTo(m_d->device->colorSpace());

    const int pixelSize = m_d->device->pixelSize();

    if (pixelSize == 1) {
        SelectionPolicy<false, DifferencePolicyOptimized<quint8>, FillWithColorExternal>
            policy(m_d->device, srcColor, m_d->threshold);
        policy.setDestinationDevice(externalDevice);
        policy.setFillColor(fillColor);
        runImpl(policy);
    } else if (pixelSize == 2) {
        SelectionPolicy<false, DifferencePolicyOptimized<quint16>, FillWithColorExternal>
            policy(m_d->device, srcColor, m_d->threshold);
        policy.setDestinationDevice(externalDevice);
        policy.setFillColor(fillColor);
        runImpl(policy);
    } else if (pixelSize == 4) {
        SelectionPolicy<false, DifferencePolicyOptimized<quint32>, FillWithColorExternal>
            policy(m_d->device, srcColor, m_d->threshold);
        policy.setDestinationDevice(externalDevice);
        policy.setFillColor(fillColor);
        runImpl(policy);
    } else if (pixelSize == 8) {
        SelectionPolicy<false, DifferencePolicyOptimized<quint64>, FillWithColorExternal>
            policy(m_d->device, srcColor, m_d->threshold);
        policy.setDestinationDevice(externalDevice);
        policy.setFillColor(fillColor);
        runImpl(policy);
    } else {
        SelectionPolicy<false, DifferencePolicySlow, FillWithColorExternal>
            policy(m_d->device, srcColor, m_d->threshold);
        policy.setDestinationDevice(externalDevice);
        policy.setFillColor(fillColor);
        runImpl(policy);
    }
}

void KisScanlineFill::fillSelection(KisPixelSelectionSP pixelSelection)
{
    KisRandomConstAccessorSP it = m_d->device->createRandomConstAccessorNG(m_d->startPoint.x(), m_d->startPoint.y());
    KoColor srcColor(it->rawDataConst(), m_d->device->colorSpace());

    const int pixelSize = m_d->device->pixelSize();

    if (pixelSize == 1) {
        SelectionPolicy<true, DifferencePolicyOptimized<quint8>, CopyToSelection>
            policy(m_d->device, srcColor, m_d->threshold);
        policy.setDestinationSelection(pixelSelection);
        runImpl(policy);
    } else if (pixelSize == 2) {
        SelectionPolicy<true, DifferencePolicyOptimized<quint16>, CopyToSelection>
            policy(m_d->device, srcColor, m_d->threshold);
        policy.setDestinationSelection(pixelSelection);
        runImpl(policy);
    } else if (pixelSize == 4) {
        SelectionPolicy<true, DifferencePolicyOptimized<quint32>, CopyToSelection>
            policy(m_d->device, srcColor, m_d->threshold);
        policy.setDestinationSelection(pixelSelection);
        runImpl(policy);
    } else if (pixelSize == 8) {
        SelectionPolicy<true, DifferencePolicyOptimized<quint64>, CopyToSelection>
              policy(m_d->device, srcColor, m_d->threshold);
        policy.setDestinationSelection(pixelSelection);
        runImpl(policy);
    } else {
        SelectionPolicy<true, DifferencePolicySlow, CopyToSelection>
              policy(m_d->device, srcColor, m_d->threshold);
        policy.setDestinationSelection(pixelSelection);
        runImpl(policy);
    }
}

void KisScanlineFill::clearNonZeroComponent()
{
    const int pixelSize = m_d->device->pixelSize();
    KoColor srcColor(Qt::transparent, m_d->device->colorSpace());

    if (pixelSize == 1) {
        SelectionPolicy<false, IsNonNullPolicyOptimized<quint8>, FillWithColor>
            policy(m_d->device, srcColor, m_d->threshold);
        policy.setFillColor(srcColor);
        runImpl(policy);
    } else if (pixelSize == 2) {
        SelectionPolicy<false, IsNonNullPolicyOptimized<quint16>, FillWithColor>
            policy(m_d->device, srcColor, m_d->threshold);
        policy.setFillColor(srcColor);
        runImpl(policy);
    } else if (pixelSize == 4) {
        SelectionPolicy<false, IsNonNullPolicyOptimized<quint32>, FillWithColor>
            policy(m_d->device, srcColor, m_d->threshold);
        policy.setFillColor(srcColor);
        runImpl(policy);
    } else if (pixelSize == 8) {
        SelectionPolicy<false, IsNonNullPolicyOptimized<quint64>, FillWithColor>
              policy(m_d->device, srcColor, m_d->threshold);
        policy.setFillColor(srcColor);
        runImpl(policy);
    } else {
        SelectionPolicy<false, IsNonNullPolicySlow, FillWithColor>
              policy(m_d->device, srcColor, m_d->threshold);
        policy.setFillColor(srcColor);
        runImpl(policy);
    }
}

void KisScanlineFill::fillContiguousGroup(KisPaintDeviceSP groupMapDevice, qint32 groupIndex)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->device->pixelSize() == 1);
    KIS_SAFE_ASSERT_RECOVER_RETURN(groupMapDevice->pixelSize() == 4);

    KisRandomConstAccessorSP it = m_d->device->createRandomConstAccessorNG(m_d->startPoint.x(), m_d->startPoint.y());
    const quint8 referenceValue = *it->rawDataConst();

    GroupSplitPolicy policy(m_d->device, groupMapDevice, groupIndex, referenceValue, m_d->threshold);
    runImpl(policy);
}

void KisScanlineFill::testingProcessLine(const KisFillInterval &processInterval)
{
    KoColor srcColor(QColor(0,0,0,0), m_d->device->colorSpace());
    KoColor fillColor(QColor(200,200,200,200), m_d->device->colorSpace());

    SelectionPolicy<false, DifferencePolicyOptimized<quint32>, FillWithColor>
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
