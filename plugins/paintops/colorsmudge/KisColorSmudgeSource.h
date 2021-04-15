/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KRITA_KISCOLORSMUDGESOURCE_H
#define KRITA_KISCOLORSMUDGESOURCE_H

#include <QtGlobal>
#include <kis_types.h>

class KoColorSpace;
class QRect;
class KisOverlayPaintDeviceWrapper;

class KisColorSmudgeSource {
public:
    virtual ~KisColorSmudgeSource() = default;
    virtual void readBytes(quint8 *dstPtr, const QRect &rect) = 0;
    virtual const KoColorSpace* colorSpace() const = 0;
};

using KisColorSmudgeSourceSP = QSharedPointer<KisColorSmudgeSource>;

struct KisColorSmudgeSourcePaintDevice : public KisColorSmudgeSource
{
    KisColorSmudgeSourcePaintDevice(KisPaintDeviceSP sourceDevice);
    void readBytes(quint8 *dstPtr, const QRect &rect) override;
    const KoColorSpace* colorSpace() const override;

private:
    KisPaintDeviceSP m_sourceDevice;
};

struct KisColorSmudgeSourceImage : public KisColorSmudgeSource
{
    KisColorSmudgeSourceImage(KisImageSP image,
                              KisOverlayPaintDeviceWrapper &overlayDevice);
    void readBytes(quint8 *dstPtr, const QRect &rect) override;
    const KoColorSpace* colorSpace() const override;

private:
    KisImageSP m_image;
    KisOverlayPaintDeviceWrapper &m_overlayDevice;
};


#endif //KRITA_KISCOLORSMUDGESOURCE_H
