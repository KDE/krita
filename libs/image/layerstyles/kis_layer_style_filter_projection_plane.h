/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_LAYER_STYLE_FILTER_PROJECTION_PLANE_H
#define __KIS_LAYER_STYLE_FILTER_PROJECTION_PLANE_H

#include "kis_abstract_projection_plane.h"

#include <QScopedPointer>

#include "kis_types.h"

class KisLayerStyleKnockoutBlower;


class KisLayerStyleFilterProjectionPlane : public KisAbstractProjectionPlane
{
public:
    KisLayerStyleFilterProjectionPlane(KisLayer *sourceLayer);
    KisLayerStyleFilterProjectionPlane(const KisLayerStyleFilterProjectionPlane &rhs, KisLayer *sourceLayer, KisPSDLayerStyleSP clonedStyle);
    ~KisLayerStyleFilterProjectionPlane() override;

    void setStyle(KisLayerStyleFilter *filter, KisPSDLayerStyleSP style);

    QRect recalculate(const QRect& rect, KisNodeSP filthyNode) override;
    void apply(KisPainter *painter, const QRect &rect) override;

    QRect needRect(const QRect &rect, KisLayer::PositionToFilthy pos) const override;
    QRect changeRect(const QRect &rect, KisLayer::PositionToFilthy pos) const override;
    QRect accessRect(const QRect &rect, KisLayer::PositionToFilthy pos) const override;
    QRect needRectForOriginal(const QRect &rect) const override;
    QRect tightUserVisibleBounds() const override;

    KisPaintDeviceList getLodCapableDevices() const override;

    /**
     * \returns true if a call to apply() will actually paint anything. Basically,
     * it is a cached version of isEnabled(), though the state may change after calling
     * to recalculate().
     */
    bool isEmpty() const;

    KisLayerStyleKnockoutBlower *knockoutBlower() const;

protected:

    KisLayerStyleFilter* filter() const;
    KisPSDLayerStyleSP style() const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

typedef QSharedPointer<KisLayerStyleFilterProjectionPlane> KisLayerStyleFilterProjectionPlaneSP;
typedef QWeakPointer<KisLayerStyleFilterProjectionPlane> KisLayerStyleFilterProjectionPlaneWSP;

#endif /* __KIS_LAYER_STYLE_FILTER_PROJECTION_PLANE_H */
