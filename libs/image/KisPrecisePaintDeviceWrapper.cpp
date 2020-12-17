/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisPrecisePaintDeviceWrapper.h"

#include <QRegion>
#include "kis_paint_device.h"
#include "kis_wrapped_rect.h"
#include "KisFastDeviceProcessingUtils.h"

#include <KoColor.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>

#include <KoColorSpaceMaths.h>

struct KisPrecisePaintDeviceWrapper::Private
{
    KisPaintDeviceSP srcDevice;
    KisPaintDeviceSP precDevice;

    QRegion preparedRegion;
    const KoColorSpace *precColorSpace = 0;

    int keepRectsHistory = 50;
};


KisPrecisePaintDeviceWrapper::KisPrecisePaintDeviceWrapper(KisPaintDeviceSP device, int keepRectsHistory)
    : m_d(new Private)
{
    m_d->srcDevice = device;
    m_d->keepRectsHistory = keepRectsHistory;

    const KoColorSpace *baseSpace = device->colorSpace();
    const bool useRoundingCorrection = baseSpace->colorDepthId() == Integer8BitsColorDepthID;

    if (useRoundingCorrection) {
        m_d->precColorSpace =
                KoColorSpaceRegistry::instance()->colorSpace(
                baseSpace->colorModelId().id(),
                Integer16BitsColorDepthID.id(),
                baseSpace->profile());
        m_d->precDevice = new KisPaintDevice(m_d->precColorSpace);
        m_d->precDevice->setDefaultPixel(device->defaultPixel().convertedTo(m_d->precColorSpace));
        m_d->precDevice->setDefaultBounds(device->defaultBounds());
        m_d->precDevice->moveTo(device->offset());

    } else {
        // just use source device as a precise operation device
        m_d->precDevice = device;
        m_d->precColorSpace = device->colorSpace();
    }
}

KisPrecisePaintDeviceWrapper::~KisPrecisePaintDeviceWrapper()
{
}

const KoColorSpace *KisPrecisePaintDeviceWrapper::preciseColorSpace() const
{
    return m_d->precColorSpace;
}

KisPaintDeviceSP KisPrecisePaintDeviceWrapper::createPreciseCompositionSourceDevice() const
{
    KisPaintDeviceSP result;

    if (m_d->precDevice == m_d->srcDevice) {
        result = m_d->srcDevice->createCompositionSourceDevice();
    } else {
        const KoColorSpace *compositionColorSpace =
            m_d->srcDevice->compositionSourceColorSpace();

        const KoColorSpace *preciseCompositionColorSpace =
                KoColorSpaceRegistry::instance()->colorSpace(
                    compositionColorSpace->colorModelId().id(),
                    Integer16BitsColorDepthID.id(),
                    compositionColorSpace->profile());

        KisPaintDeviceSP device = new KisPaintDevice(preciseCompositionColorSpace);
        device->setDefaultBounds(m_d->srcDevice->defaultBounds());
        result = device;
    }

    return result;
}

KisPaintDeviceSP KisPrecisePaintDeviceWrapper::sourceDevice() const
{
    return m_d->srcDevice;
}

KisPaintDeviceSP KisPrecisePaintDeviceWrapper::preciseDevice() const
{
    return m_d->precDevice;
}

QRegion KisPrecisePaintDeviceWrapper::cachedRegion() const
{
    return m_d->precDevice == m_d->srcDevice ? m_d->srcDevice->extent() : m_d->preparedRegion;
}

void KisPrecisePaintDeviceWrapper::resetCachedRegion()
{
    m_d->preparedRegion = QRegion();
}

void KisPrecisePaintDeviceWrapper::readRect(const QRect &rect)
{
    readRects({rect});
}

void KisPrecisePaintDeviceWrapper::writeRect(const QRect &rc)
{
    writeRects({rc});
}

namespace {

struct WriteProcessor {
    WriteProcessor(int _channelCount) : channelCount(_channelCount) {}

    ALWAYS_INLINE
    void operator()(const quint8 *srcPtr, quint8 *dstPtr) {
        const quint16 *srcChannel = reinterpret_cast<const quint16*>(srcPtr);
        quint8 *dstChannel = reinterpret_cast<quint8*>(dstPtr);

        for (int k = 0; k < channelCount; k++) {
            *(dstChannel + k) = KoColorSpaceMaths<quint16, quint8>::scaleToA(*(srcChannel + k));
        }
    }

    const int channelCount;
};

struct ReadProcessor {
    ReadProcessor(int _channelCount) : m_channelCount(_channelCount) {}

    ALWAYS_INLINE
    void operator()(const quint8 *srcPtr, quint8 *dstPtr) {
        const quint8 *srcChannel = reinterpret_cast<const quint8*>(srcPtr);
        quint16 *dstChannel = reinterpret_cast<quint16*>(dstPtr);

        for (int k = 0; k < m_channelCount; k++) {
            *(dstChannel + k) = KoColorSpaceMaths<quint8, quint16>::scaleToA(*(srcChannel + k));
        }
    }

    const int m_channelCount;
};

}

void KisPrecisePaintDeviceWrapper::readRects(const QVector<QRect> &rects)
{
    if (m_d->precDevice == m_d->srcDevice) return;

    const QRect srcExtent = m_d->srcDevice->extent();

    QRegion requestedRects;
    Q_FOREACH (const QRect &rc, rects) {
        if (m_d->srcDevice->defaultBounds()->wrapAroundMode()) {
            const QRect wrapRect = m_d->srcDevice->defaultBounds()->imageBorderRect();
            KisWrappedRect wrappedRect(rc, wrapRect);
            Q_FOREACH (const QRect &wrc, wrappedRect) {
                const QRect croppedRect = wrc & srcExtent;

                requestedRects += croppedRect;
            }
        } else {
            const QRect croppedRect = rc & srcExtent;

            requestedRects += croppedRect;
        }
    }

    QRegion diff(requestedRects);
    diff -= m_d->preparedRegion;

    if (rects.isEmpty()) return;
    const int channelCount = m_d->precColorSpace->channelCount();

    KisRandomConstAccessorSP srcIt = m_d->srcDevice->createRandomConstAccessorNG();
    KisRandomAccessorSP dstIt = m_d->precDevice->createRandomAccessorNG();

    auto rectIter = diff.begin();
    while (rectIter != diff.end()) {
        KritaUtils::processTwoDevices(*rectIter,
                                      srcIt, dstIt,
                                      m_d->srcDevice->pixelSize(),
                                      m_d->precDevice->pixelSize(),
                                      ReadProcessor(channelCount));
        rectIter++;
    }

    /**
     * Don't let the region grow too much. When the region has too many
     * rects, it becomes really slow
     */
    if (m_d->preparedRegion.rectCount() > m_d->keepRectsHistory) {
        m_d->preparedRegion = requestedRects;
    } else {
        m_d->preparedRegion += requestedRects;
    }
}

void KisPrecisePaintDeviceWrapper::writeRects(const QVector<QRect> &rects)
{
    if (m_d->precDevice == m_d->srcDevice) return;

    if (rects.isEmpty()) return;
    const int channelCount = m_d->precColorSpace->channelCount();

    KisRandomConstAccessorSP srcIt = m_d->precDevice->createRandomConstAccessorNG();
    KisRandomAccessorSP dstIt = m_d->srcDevice->createRandomAccessorNG();

    Q_FOREACH (const QRect &rc, rects) {
        KritaUtils::processTwoDevices(rc,
                                      srcIt, dstIt,
                                      m_d->precDevice->pixelSize(),
                                      m_d->srcDevice->pixelSize(),
                                      WriteProcessor(channelCount));
    }
}
