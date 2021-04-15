/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisColorSmudgeSource.h"

#include "kis_paint_device.h"
#include "kis_image.h"
#include "KisOverlayPaintDeviceWrapper.h"

/**********************************************************************************/
/*                 KisColorSmudgeSourcePaintDevice                                */
/**********************************************************************************/

KisColorSmudgeSourcePaintDevice::KisColorSmudgeSourcePaintDevice(KisPaintDeviceSP sourceDevice)
        : m_sourceDevice(sourceDevice)
{
}

void KisColorSmudgeSourcePaintDevice::readBytes(quint8 *dstPtr, const QRect &rect) {
    m_sourceDevice->readBytes(dstPtr, rect);
}

const KoColorSpace *KisColorSmudgeSourcePaintDevice::colorSpace() const {
    return m_sourceDevice->colorSpace();
}

/**********************************************************************************/
/*                 KisColorSmudgeSourceImage                                      */
/**********************************************************************************/

KisColorSmudgeSourceImage::KisColorSmudgeSourceImage(KisImageSP image, KisOverlayPaintDeviceWrapper &overlayDevice)
        : m_image(image),
          m_overlayDevice(overlayDevice)
{
    KIS_ASSERT(m_image->projection() == m_overlayDevice.source());
}

void KisColorSmudgeSourceImage::readBytes(quint8 *dstPtr, const QRect &rect) {
    m_image->blockUpdates();
    m_overlayDevice.readRect(rect);
    m_image->unblockUpdates();
    m_overlayDevice.overlay()->readBytes(dstPtr, rect);
}

const KoColorSpace *KisColorSmudgeSourceImage::colorSpace() const {
    return m_overlayDevice.overlayColorSpace();
}
