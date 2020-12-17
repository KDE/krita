/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
