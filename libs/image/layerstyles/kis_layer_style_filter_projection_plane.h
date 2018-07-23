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

#ifndef __KIS_LAYER_STYLE_FILTER_PROJECTION_PLANE_H
#define __KIS_LAYER_STYLE_FILTER_PROJECTION_PLANE_H

#include "kis_abstract_projection_plane.h"

#include <QScopedPointer>

#include "kis_types.h"


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

    KisPaintDeviceList getLodCapableDevices() const override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

typedef QSharedPointer<KisLayerStyleFilterProjectionPlane> KisLayerStyleFilterProjectionPlaneSP;
typedef QWeakPointer<KisLayerStyleFilterProjectionPlane> KisLayerStyleFilterProjectionPlaneWSP;

#endif /* __KIS_LAYER_STYLE_FILTER_PROJECTION_PLANE_H */
