/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisColorSmudgeSource.h"

#include "kis_paint_device.h"
#include "kis_image.h"
#include "KisOverlayPaintDeviceWrapper.h"

void KisColorSmudgeSource::readRect(const QRect &rect) {
    readRects({rect});
}

/**********************************************************************************/
/*                 KisColorSmudgeSourcePaintDevice                                */
/**********************************************************************************/

KisColorSmudgeSourcePaintDevice::KisColorSmudgeSourcePaintDevice(KisOverlayPaintDeviceWrapper &overlayDevice, int overlayIndex)
    : m_overlayDevice(overlayDevice),
      m_overlayIndex(overlayIndex)
{
}

void KisColorSmudgeSourcePaintDevice::readRects(const QVector<QRect> &rects) {
    m_overlayDevice.readRects(rects);
}

void KisColorSmudgeSourcePaintDevice::readBytes(quint8 *dstPtr, const QRect &rect) {
    m_overlayDevice.overlay(m_overlayIndex)->readBytes(dstPtr, rect);
}

const KoColorSpace *KisColorSmudgeSourcePaintDevice::colorSpace() const {
    return m_overlayDevice.overlayColorSpace();
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

void KisColorSmudgeSourceImage::readRects(const QVector<QRect> &rects)
{
    m_image->blockUpdates();
    m_overlayDevice.readRects(rects);
    m_image->unblockUpdates();
}

void KisColorSmudgeSourceImage::readBytes(quint8 *dstPtr, const QRect &rect)
{
    m_overlayDevice.overlay()->readBytes(dstPtr, rect);
}

const KoColorSpace *KisColorSmudgeSourceImage::colorSpace() const {
    return m_overlayDevice.overlayColorSpace();
}
