/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KisPrecisePaintDeviceWrapper.h"

#include <QRegion>
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_sequential_iterator.h"

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
    if (m_d->precDevice == m_d->srcDevice) return;
    const int channelCount = m_d->precColorSpace->channelCount();

    KisSequentialIterator srcIt(m_d->srcDevice, rc);
    KisSequentialConstIterator precIt(m_d->precDevice, rc);

    while (srcIt.nextPixel() && precIt.nextPixel()) {
        quint8 *srcPtr = reinterpret_cast<quint8*>(srcIt.rawData());
        const quint16 *precPtr = reinterpret_cast<const quint16*>(precIt.rawDataConst());

        for (int i = 0; i < channelCount; i++) {
            *(srcPtr + i) = KoColorSpaceMaths<quint16, quint8>::scaleToA(*(precPtr + i));
        }
    }
}

void KisPrecisePaintDeviceWrapper::readRects(const QVector<QRect> &rects)
{
    if (m_d->precDevice == m_d->srcDevice) return;

    QRegion requestedRects;
    Q_FOREACH (const QRect &rc, rects) {
        requestedRects += rc;
    }

    QRegion diff(requestedRects);
    diff -= m_d->preparedRegion;

    const int channelCount = m_d->precColorSpace->channelCount();

    Q_FOREACH (const QRect &rc, diff.rects()) {
        KisSequentialConstIterator srcIt(m_d->srcDevice, rc);
        KisSequentialIterator precIt(m_d->precDevice, rc);

        while (srcIt.nextPixel() && precIt.nextPixel()) {
            const quint8 *srcPtr = reinterpret_cast<const quint8*>(srcIt.rawDataConst());
            quint16 *precPtr = reinterpret_cast<quint16*>(precIt.rawData());

            for (int i = 0; i < channelCount; i++) {
                *(precPtr + i) = KoColorSpaceMaths<quint8, quint16>::scaleToA(*(srcPtr + i));
            }
        }
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

    Q_FOREACH (const QRect &rc, rects) {
        writeRect(rc);
    }
}

