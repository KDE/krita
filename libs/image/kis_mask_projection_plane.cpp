/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    // masks don't have any internal rendering subtrees,
    // so just return the extent of the mask
    return m_d->mask->extent();
}

