/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_LAYER_STYLE_PROJECTION_PLANE_H
#define __KIS_LAYER_STYLE_PROJECTION_PLANE_H

#include "kis_abstract_projection_plane.h"

#include <QScopedPointer>

#include "kis_types.h"

#include <kritaimage_export.h>


class KRITAIMAGE_EXPORT KisLayerStyleProjectionPlane : public KisAbstractProjectionPlane
{
public:
    KisLayerStyleProjectionPlane(KisLayer *sourceLayer);
    KisLayerStyleProjectionPlane(const KisLayerStyleProjectionPlane &rhs, KisLayer *sourceLayer, KisPSDLayerStyleSP clonedStyle);

    ~KisLayerStyleProjectionPlane() override;

    QRect recalculate(const QRect& rect, KisNodeSP filthyNode) override;
    void apply(KisPainter *painter, const QRect &rect) override;

    QRect needRect(const QRect &rect, KisLayer::PositionToFilthy pos) const override;
    QRect changeRect(const QRect &rect, KisLayer::PositionToFilthy pos) const override;
    QRect accessRect(const QRect &rect, KisLayer::PositionToFilthy pos) const override;
    QRect needRectForOriginal(const QRect &rect) const override;
    QRect tightUserVisibleBounds() const override;

    KisPaintDeviceList getLodCapableDevices() const override;


    // a method for registering on KisLayerStyleProjectionPlaneFactory
    static KisAbstractProjectionPlaneSP factoryObject(KisLayer *sourceLayer);

private:
    friend class KisLayerStyleProjectionPlaneTest;
    KisLayerStyleProjectionPlane(KisLayer *sourceLayer, KisPSDLayerStyleSP style);

    void init(KisLayer *sourceLayer, KisPSDLayerStyleSP layerStyle);

    QRect stylesNeedRect(const QRect &rect) const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

typedef QSharedPointer<KisLayerStyleProjectionPlane> KisLayerStyleProjectionPlaneSP;
typedef QWeakPointer<KisLayerStyleProjectionPlane> KisLayerStyleProjectionPlaneWSP;

#endif /* __KIS_LAYER_STYLE_PROJECTION_PLANE_H */
