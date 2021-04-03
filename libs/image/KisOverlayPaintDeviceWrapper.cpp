/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisOverlayPaintDeviceWrapper.h"

#include "kis_paint_device.h"
#include <KoColor.h>
#include "KisRectsGrid.h"
#include "kis_painter.h"
#include "KoColorModelStandardIds.h"
#include "KisFastDeviceProcessingUtils.h"
#include "KisRegion.h"
#include "kis_wrapped_rect.h"
#include <KoOptimizedRgbPixelDataScalerU8ToU16Factory.h>

struct KisOverlayPaintDeviceWrapper::Private
{
    KisPaintDeviceSP source;
    QVector<KisPaintDeviceSP> overlays;
    KisRectsGrid grid;
    bool usePreciseMode = false;
    QScopedPointer<KoOptimizedRgbPixelDataScalerU8ToU16Base> scaler;
};


KisOverlayPaintDeviceWrapper::KisOverlayPaintDeviceWrapper(KisPaintDeviceSP source, int numOverlays, OverlayMode mode)
    : m_d(new Private())
{
    m_d->source = source;

    const KoColorSpace *overlayColorSpace = source->colorSpace();

    const bool usePreciseMode = mode == PreciseMode || mode == LazyPreciseMode;

    if (usePreciseMode &&
        overlayColorSpace->colorDepthId() == Integer8BitsColorDepthID) {

        overlayColorSpace =
            KoColorSpaceRegistry::instance()->colorSpace(
                    overlayColorSpace->colorModelId().id(),
                    Integer16BitsColorDepthID.id(),
                    overlayColorSpace->profile());

        if (overlayColorSpace->colorModelId() == RGBAColorModelID) {
            m_d->scaler.reset(KoOptimizedRgbPixelDataScalerU8ToU16Factory::create());
        }

        m_d->usePreciseMode = true;
    }

    if (!m_d->usePreciseMode && mode == LazyPreciseMode && numOverlays == 1) {
        return;
    }

    for (int i = 0; i < numOverlays; i++) {
        KisPaintDeviceSP overlay = new KisPaintDevice(overlayColorSpace);
        overlay->setDefaultPixel(source->defaultPixel().convertedTo(overlayColorSpace));
        overlay->setDefaultBounds(source->defaultBounds());
        overlay->moveTo(source->offset());

        m_d->overlays.append(overlay);
    }
}

KisOverlayPaintDeviceWrapper::~KisOverlayPaintDeviceWrapper()
{
}

KisPaintDeviceSP KisOverlayPaintDeviceWrapper::source() const
{
    return m_d->source;
}

KisPaintDeviceSP KisOverlayPaintDeviceWrapper::overlay(int index) const
{
    return !m_d->overlays.isEmpty() ? m_d->overlays[index] : m_d->source;
}

void KisOverlayPaintDeviceWrapper::readRect(const QRect &rc)
{
    readRects({rc});
}

void KisOverlayPaintDeviceWrapper::writeRect(const QRect &rc, int index)
{
    writeRects({rc}, index);
}

void KisOverlayPaintDeviceWrapper::readRects(const QVector<QRect> &rects)
{
    if (rects.isEmpty()) return;
    if (m_d->overlays.isEmpty()) return;

    QRect cropRect = m_d->source->extent();
    QVector<QRect> rectsToRead;

    Q_FOREACH (const QRect &rc, rects) {
        if (m_d->source->defaultBounds()->wrapAroundMode()) {
            const QRect wrapRect = m_d->source->defaultBounds()->imageBorderRect();
            KisWrappedRect wrappedRect(rc, wrapRect);
            Q_FOREACH (const QRect &wrc, wrappedRect) {
                rectsToRead += m_d->grid.addRect(wrc);
            }
            cropRect &= wrapRect;
        } else {
            rectsToRead += m_d->grid.addRect(rc);
        }
    }

    KisRegion::makeGridLikeRectsUnique(rectsToRead);

    //TODO: implement synchronization of the offset between the grid and devices

    if (!m_d->scaler) {
        Q_FOREACH (KisPaintDeviceSP overlay, m_d->overlays) {
            Q_FOREACH (const QRect &rect, rectsToRead) {
                const QRect croppedRect = rect & cropRect;
                if (croppedRect.isEmpty()) continue;

                KisPainter::copyAreaOptimized(croppedRect.topLeft(), m_d->source, overlay, croppedRect);
            }
        }
    } else {
        KisPaintDeviceSP overlay = m_d->overlays.first();

        KisRandomConstAccessorSP srcIt = m_d->source->createRandomConstAccessorNG();
        KisRandomAccessorSP dstIt = overlay->createRandomAccessorNG();

        auto rectIter = rectsToRead.begin();
        while (rectIter != rectsToRead.end()) {
            const QRect croppedRect = *rectIter & cropRect;

            if (!croppedRect.isEmpty()) {

                KritaUtils::processTwoDevicesWithStrides(croppedRect,
                                                         srcIt, dstIt,
                    [this] (const quint8 *src, int srcRowStride,
                            quint8 *dst, int dstRowStride,
                            int numRows, int numColumns) {

                    m_d->scaler->convertU8ToU16(src, srcRowStride,
                                                dst, dstRowStride,
                                                numRows, numColumns);
                });

                for (auto it = std::next(m_d->overlays.begin()); it != m_d->overlays.end(); ++it) {
                    KisPaintDeviceSP otherOverlay = *it;
                    KisPainter::copyAreaOptimized(croppedRect.topLeft(), overlay, otherOverlay, croppedRect);
                }
            }

            rectIter++;
        }
    }
}

void KisOverlayPaintDeviceWrapper::writeRects(const QVector<QRect> &rects, int index)
{
    if (rects.isEmpty()) return;
    if (m_d->overlays.isEmpty()) return;

    if (!m_d->scaler) {
        Q_FOREACH (const QRect &rc, rects) {
            KIS_SAFE_ASSERT_RECOVER_NOOP(m_d->grid.contains(rc));
            KisPainter::copyAreaOptimized(rc.topLeft(), m_d->overlays[index], m_d->source, rc);
        }
    } else {
        KisPaintDeviceSP overlay = m_d->overlays[index];


        KisRandomConstAccessorSP srcIt = overlay->createRandomConstAccessorNG();
        KisRandomAccessorSP dstIt = m_d->source->createRandomAccessorNG();

        Q_FOREACH (const QRect &rc, rects) {
            KritaUtils::processTwoDevicesWithStrides(rc,
                                                     srcIt, dstIt,
                [this] (const quint8 *src, int srcRowStride,
                        quint8 *dst, int dstRowStride,
                        int numRows, int numColumns) {

                m_d->scaler->convertU16ToU8(src, srcRowStride,
                                            dst, dstRowStride,
                                            numRows, numColumns);
            });
        }
    }
}

const KoColorSpace *KisOverlayPaintDeviceWrapper::overlayColorSpace() const
{
    return !m_d->overlays.isEmpty() ? m_d->overlays.first()->colorSpace() : m_d->source->colorSpace();
}

KisPaintDeviceSP KisOverlayPaintDeviceWrapper::createPreciseCompositionSourceDevice()
{
    KisPaintDeviceSP result;

    if (!m_d->usePreciseMode) {
        result = source()->createCompositionSourceDevice();
    } else {
        const KoColorSpace *compositionColorSpace =
            m_d->source->compositionSourceColorSpace();

        const KoColorSpace *preciseCompositionColorSpace =
                KoColorSpaceRegistry::instance()->colorSpace(
                    compositionColorSpace->colorModelId().id(),
                    Integer16BitsColorDepthID.id(),
                    compositionColorSpace->profile());

        KisPaintDeviceSP device = new KisPaintDevice(preciseCompositionColorSpace);
        device->setDefaultBounds(m_d->source->defaultBounds());
        result = device;
    }

    return result;
}
