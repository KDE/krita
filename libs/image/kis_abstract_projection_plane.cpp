/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

QRect KisDumbProjectionPlane::tightUserVisibleBounds() const
{
    return QRect();
}

KisPaintDeviceList KisDumbProjectionPlane::getLodCapableDevices() const
{
    // arghm...
    return KisPaintDeviceList();
}
