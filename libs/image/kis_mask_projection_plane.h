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

#ifndef __KIS_MASK_PROJECTION_PLANE_H
#define __KIS_MASK_PROJECTION_PLANE_H

#include "kis_abstract_projection_plane.h"

#include <QScopedPointer>

/**
 * An implementation of the KisAbstractProjectionPlane interface for a
 * layer object.
 *
 * Please note that recalculate() and apply() methods are not defined
 * for masks, because the KisLayer code still uses traditional
 * methods of KisMask directly.
 */
class KisMaskProjectionPlane : public KisAbstractProjectionPlane
{
public:
    KisMaskProjectionPlane(KisMask *mask);
    ~KisMaskProjectionPlane() override;

    QRect recalculate(const QRect& rect, KisNodeSP filthyNode) override;
    void apply(KisPainter *painter, const QRect &rect) override;

    QRect needRect(const QRect &rect, KisNode::PositionToFilthy pos) const override;
    QRect changeRect(const QRect &rect, KisNode::PositionToFilthy pos) const override;
    QRect accessRect(const QRect &rect, KisNode::PositionToFilthy pos) const override;
    QRect needRectForOriginal(const QRect &rect) const override;
    QRect tightUserVisibleBounds() const override;

    KisPaintDeviceList getLodCapableDevices() const override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_MASK_PROJECTION_PLANE_H */
