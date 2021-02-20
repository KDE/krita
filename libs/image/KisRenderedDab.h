/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISRENDEREDDAB_H
#define KISRENDEREDDAB_H

#include "kis_types.h"
#include "kis_fixed_paint_device.h"

struct KisRenderedDab
{
    KisRenderedDab() {}
    KisRenderedDab(KisFixedPaintDeviceSP _device)
        : device(_device),
          offset(_device->bounds().topLeft())
    {
    }

    KisRenderedDab(const KisRenderedDab &/*rhs*/) = default;

    KisFixedPaintDeviceSP device;
    QPoint offset;

    qreal opacity = OPACITY_OPAQUE_F;
    qreal flow = OPACITY_OPAQUE_F;
    qreal averageOpacity = OPACITY_TRANSPARENT_F;

    inline QRect realBounds() const {
        return QRect(offset, device->bounds().size());
    }
};

#endif // KISRENDEREDDAB_H
