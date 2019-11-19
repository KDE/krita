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

#include "kis_mask_projection_plane.h"

#include <QBitArray>
#include <KoColorSpace.h>
#include <KoChannelInfo.h>
#include "kis_painter.h"
#include "kis_mask.h"


struct KisMaskProjectionPlane::Private
{
    KisMask *mask;
};


KisMaskProjectionPlane::KisMaskProjectionPlane(KisMask *mask)
    : m_d(new Private)
{
    m_d->mask = mask;
}

KisMaskProjectionPlane::~KisMaskProjectionPlane()
{
}

QRect KisMaskProjectionPlane::recalculate(const QRect& rect, KisNodeSP filthyNode)
{
    Q_UNUSED(filthyNode);

    KIS_ASSERT_RECOVER_NOOP(0 && "KisMaskProjectionPlane::recalculate() is not defined!");

    return rect;
}

void KisMaskProjectionPlane::apply(KisPainter *painter, const QRect &rect)
{
    Q_UNUSED(painter);
    Q_UNUSED(rect);

    KIS_ASSERT_RECOVER_NOOP(0 && "KisMaskProjectionPlane::apply() is not defined!");
}

KisPaintDeviceList KisMaskProjectionPlane::getLodCapableDevices() const
{
    // masks have no projection
    return KisPaintDeviceList();
}

QRect KisMaskProjectionPlane::needRect(const QRect &rect, KisNode::PositionToFilthy pos) const
{
    return m_d->mask->needRect(rect, pos);
}

QRect KisMaskProjectionPlane::changeRect(const QRect &rect, KisNode::PositionToFilthy pos) const
{
    return m_d->mask->changeRect(rect, pos);
}

QRect KisMaskProjectionPlane::accessRect(const QRect &rect, KisNode::PositionToFilthy pos) const
{
    return m_d->mask->accessRect(rect, pos);
}

QRect KisMaskProjectionPlane::needRectForOriginal(const QRect &rect) const
{
    return rect;
}

QRect KisMaskProjectionPlane::tightUserVisibleBounds() const
{
    return QRect();
}

