/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_LAYER_PROJECTION_PLANE_H
#define __KIS_LAYER_PROJECTION_PLANE_H

#include "kis_abstract_projection_plane.h"

#include <QScopedPointer>


/**
 * An implementation of the KisAbstractProjectionPlane interface for a
 * layer object
 */
class KisLayerProjectionPlane : public KisAbstractProjectionPlane
{
public:
    KisLayerProjectionPlane(KisLayer *layer);
    ~KisLayerProjectionPlane() override;

    QRect recalculate(const QRect& rect, KisNodeSP filthyNode) override;
    void apply(KisPainter *painter, const QRect &rect) override;
    void applyMaxOutAlpha(KisPainter *painter, const QRect &rect);

    QRect needRect(const QRect &rect, KisLayer::PositionToFilthy pos) const override;
    QRect changeRect(const QRect &rect, KisLayer::PositionToFilthy pos) const override;
    QRect accessRect(const QRect &rect, KisLayer::PositionToFilthy pos) const override;
    QRect needRectForOriginal(const QRect &rect) const override;
    QRect tightUserVisibleBounds() const override;

    KisPaintDeviceList getLodCapableDevices() const override;

private:
    void applyImpl(KisPainter *painter, const QRect &rect, bool maxOutAlpha);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

typedef QSharedPointer<KisLayerProjectionPlane> KisLayerProjectionPlaneSP;
typedef QWeakPointer<KisLayerProjectionPlane> KisLayerProjectionPlaneWSP;


#endif /* __KIS_LAYER_PROJECTION_PLANE_H */
