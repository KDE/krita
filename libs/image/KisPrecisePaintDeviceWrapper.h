/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISPRECISEPAINTDEVICEWRAPPER_H
#define KISPRECISEPAINTDEVICEWRAPPER_H

#include <QScopedPointer>

#include "kis_types.h"
#include "kritaimage_export.h"

class KoColorSpace;
class QRegion;

/**
 * A special wrapper class for a paint device that allows working with
 * parts of the source paint device as if it had higher bit depth.
 *
 * For example, you have an RGBA8 paint device, but you want all the
 * blending happen in higher bit depth. You wrap your paint device (source paint
 * device) into KisPrecisePaintDeviceWrapper, and the wrapper creates a temporary
 * device (precise paint device) in RGBA16 colorspace. The you work with this precise
 * paint device as usual, uploading and downloading pixel data to/from the source
 * paint device using readRect() and writeRect()
 *
 * If the source device is already "precise", that is having the bit depth higher than
 * 8 bit per channel, no temporary device is created. All the operations are forwarded
 * directly to the source device
 *
 * Example:
 *
 * \code{.cpp}
 * // initialize the wrapper with the source device,
 * // it creates a precise device if needed
 * KisPrecisePaintDeviceWrapper wrapper(sourceDevice);
 *
 * // download the data from the source device, the operation
 * // might be cached due to keepRectsHistory option
 * wrapper.readRect(accessRect);
 *
 * // start modifying the data
 * KisPainter gc(wrapper.preciseDevice());
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

class KRITAIMAGE_EXPORT KisPrecisePaintDeviceWrapper
{
public:
    /**
     * Create a wrapper, attach it to \p device and create a temporary precise
     * paint device if needed. The temporary device is created iff the source
     * device has 8 bit bit-depth.
     *
     * \param device source device
     * \param keepRectsHistory shown how many rects in readRect() should be cached
     */
    KisPrecisePaintDeviceWrapper(KisPaintDeviceSP device, int keepRectsHistory = 50);
    ~KisPrecisePaintDeviceWrapper();

    /**
     * \return the color space of preciseDevice()
     */
    const KoColorSpace* preciseColorSpace() const;

    /**
     * Create a composite source device for being used over preciseDevice().
     *
     * Please note thate one cannot use
     * preciseDevice()->createCompositeSourceDevice() for this purpose because
     * preciseDevice() is just a copy of sourceDevice() and doesn't have overloaded
     * methods for this color space.
     *
     * TODO: make KisPaintDevice::compositeSourceColorSpace() not a virtual method,
     *       but let is be assigned during the lifetime of the paint device. It'll
     *       let us remove this extra function.
     */
    KisPaintDeviceSP createPreciseCompositionSourceDevice() const;

    /**
     * \return the source device attached to the wrapper
     */
    KisPaintDeviceSP sourceDevice() const;

    /**
     * \return the precise device. If the source device color space is "precise", then
     *         there is no separate precise device, and the original device is returned
     */
    KisPaintDeviceSP preciseDevice() const;

    /**
     * \return the region of the source device that is guaranteed to be cached by
     *         previous calls to readRect(). If one asks for reading a cached rect,
     *         it is not read and just reused.
     */
    QRegion cachedRegion() const;

    /**
     * Reset the region of the cached data from the source paint device
     */
    void resetCachedRegion();

    /**
     * Read rect \p rc from the source device and upload it into the precision device.
     * If rounding correction is not used, the function does nothing.
     */
    void readRect(const QRect &rc);

    /**
     * Write rect \p rc from the precision to source device
     * If rounding correction is not used, the function does nothing.
     */
    void writeRect(const QRect &rc);

    /**
     * \see readRect()
     */
    void readRects(const QVector<QRect> &rects);

    /**
     * \see writeRect()
     */
    void writeRects(const QVector<QRect> &rects);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISPRECISEPAINTDEVICEWRAPPER_H
