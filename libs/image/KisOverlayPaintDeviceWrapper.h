/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISOVERLAYPAINTDEVICEWRAPPER_H
#define KISOVERLAYPAINTDEVICEWRAPPER_H

#include <kis_types.h>
#include "kritaimage_export.h"

class KoColorSpace;


/**
 * A special wrapper class for a paint device that allows working with parts of
 * the source paint device using an overlay, that is without modifying the
 * device itself. The overlay may have higher bit depth if PreciseMode or
 * LazyPreciseMode is used.
 *
 * For example, you have an RGBA8 paint device, but you want all the blending
 * happen in higher bit depth. You wrap your paint device (source paint device)
 * into KisOverlayPaintDeviceWrapper, and the wrapper creates a temporary
 * device (precise overlay paint device) in RGBA16 colorspace. Then you work
 * with this precise paint device as usual, uploading and downloading pixel
 * data to/from the source paint device using readRect() and writeRect()
 *
 * In LazyPreciseMode, if the source device is already "precise", that is
 * having the bit depth higher than 8 bit per channel, no temporary device is
 * created. All the operations are forwarded directly to the source device.
 *
 * In some cases (e.g. when trying to maintain a separate heightmap channel),
 * you may want to have multiple overlays.
 *
 * When doing conversions between U8 and U16 color spaces, the overlay
 * will try to use AVX2-optimized algorithms.
 *
 * Example:
 *
 * \code{.cpp}
 * // initialize the wrapper with the source device,
 * // it creates a precise device if needed
 * KisOverlayPaintDeviceWrapper wrapper(sourceDevice, 1, KisOverlayPaintDeviceWrapper::LazyPreciseMode);
 *
 * // Download the data from the source device. The rects
 * // that have already been read will be cached and will
 * // never be read twice.
 * wrapper.readRect(accessRect);
 *
 * // start modifying the data
 * KisPainter gc(wrapper.overlay());
 *
 * // low opacity might be handled incorrectly in the original
 * // color space, but we work in a precise one!
 * gc.setOpacity(1);
 * gc.bitBlt(accessRect.topLeft(), someOtherDevice, accessRect);
 *
 * // ... repeat multiple times if needed ...
 *
 * // upload the data back to the original source device
 * wrapper.writeRect(accessRect);
 * \endcode
 *
 */

class KRITAIMAGE_EXPORT KisOverlayPaintDeviceWrapper
{
public:
    enum OverlayMode {
        NormalMode = 0,
        PreciseMode,
        LazyPreciseMode
    };

public:
    /**
     * Create an overlay wrapper and attach it to \p device.
     *
     * If \p mode is `NormalMode`, then \p numOverlays are created. All overlays
     * will have the same (possible "imprecise") colorspace as \p source.
     *
     * If \p mode is `PreciseMode`, then \p numOverlays are created. If \p source
     * has "imprecise" color space (U8), then overlays will be upgraded to a
     * "precise" color space (U16).
     *
     * If \p mode is `LazyPreciseMode` **and** \p numOverlays is 1, then the
     * overlay will be created only in case when precise color space is
     * needed. Otherwise, the mode behaves like `PreciseMode`
     *
     * \param device source device
     * \param numOverlays the number of overlays to create
     * \param mode mode to use
     * \param forcedOverlayColorSpace forced color space to use for overlay. Passing non-
     *        null as forcedOverlayColorSpace will override precise color space decision
     *        process. This argument is useful for cases when two overlay devices need to
     *        have exactly the same color space (e.g. in colorsmudge overlay mode).
     */
    KisOverlayPaintDeviceWrapper(KisPaintDeviceSP source, int numOverlays = 1, OverlayMode mode = NormalMode, const KoColorSpace *forcedOverlayColorSpace = nullptr);

    ~KisOverlayPaintDeviceWrapper();

    void setExternalDestination(KisPaintDeviceSP device);
    KisPaintDeviceSP externalDestination() const;

    KisPaintDeviceSP source() const;
    KisPaintDeviceSP overlay(int index = 0) const;

    void readRect(const QRect &rc);
    void writeRect(const QRect &rc, int index = 0);

    void readRects(const QVector<QRect> &rects);
    void writeRects(const QVector<QRect> &rects, int index = 0);

    const KoColorSpace* overlayColorSpace() const;

    /**
     * Create a composite source device for being used over overlay().
     *
     * Please note thate one cannot use
     * overlay()->createCompositeSourceDevice() for this purpose because
     * overlay() is just a copy of sourceDevice() and doesn't have overloaded
     * methods for this color space.
     *
     * TODO: make KisPaintDevice::compositeSourceColorSpace() not a virtual method,
     *       but let is be assigned during the lifetime of the paint device. It'll
     *       let us remove this extra function.
     */
    KisPaintDeviceSP createPreciseCompositionSourceDevice();

    void beginTransaction(KUndo2Command *parent = 0);
    KUndo2Command *endTransaction();

private:

    friend struct KisChangeOverlayWrapperCommand;
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISOVERLAYPAINTDEVICEWRAPPER_H
