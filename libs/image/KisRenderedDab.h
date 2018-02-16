/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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
