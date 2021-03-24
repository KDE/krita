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


class KRITAIMAGE_EXPORT KisOverlayPaintDeviceWrapper
{
public:
    enum OverlayMode {
        NormalMode = 0,
        PreciseMode,
        LazyPreciseMode
    };

public:
    KisOverlayPaintDeviceWrapper(KisPaintDeviceSP source, int numOverlays = 1, OverlayMode mode = NormalMode);

    ~KisOverlayPaintDeviceWrapper();

    KisPaintDeviceSP source() const;
    KisPaintDeviceSP overlay(int index = 0) const;

    void readRect(const QRect &rc);
    void writeRect(const QRect &rc, int index = 0);

    void readRects(const QVector<QRect> &rects);
    void writeRects(const QVector<QRect> &rects, int index = 0);

    const KoColorSpace* overlayColorSpace() const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISOVERLAYPAINTDEVICEWRAPPER_H
