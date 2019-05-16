/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_abstract_projection_plane.h"


KisAbstractProjectionPlane::KisAbstractProjectionPlane()
{
}

KisAbstractProjectionPlane::~KisAbstractProjectionPlane()
{
}

QRect KisDumbProjectionPlane::recalculate(const QRect& rect, KisNodeSP filthyNode)
{
    Q_UNUSED(filthyNode);
    return rect;
}

void KisDumbProjectionPlane::apply(KisPainter *painter, const QRect &rect)
{
    Q_UNUSED(painter);
    Q_UNUSED(rect);
}


QRect KisDumbProjectionPlane::needRect(const QRect &rect, KisLayer::PositionToFilthy pos) const
{
    Q_UNUSED(pos);
    return rect;
}

QRect KisDumbProjectionPlane::changeRect(const QRect &rect, KisLayer::PositionToFilthy pos) const
{
    Q_UNUSED(pos);
    return rect;
}

QRect KisDumbProjectionPlane::accessRect(const QRect &rect, KisLayer::PositionToFilthy pos) const
{
    Q_UNUSED(pos);
    return rect;
}

QRect KisDumbProjectionPlane::needRectForOriginal(const QRect &rect) const
{
    return rect;
}

KisPaintDeviceList KisDumbProjectionPlane::getLodCapableDevices() const
{
    // arghm...
    return KisPaintDeviceList();
}
