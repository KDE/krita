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

#include "kis_layer_style_projection_plane.h"

#include "filter/kis_filter_configuration.h"
#include "kis_layer_style_filter_projection_plane.h"


struct KisLayerStyleProjectionPlane::Private
{
    KisAbstractProjectionPlane *sourceProjectionPlane;

    QVector<KisAbstractProjectionPlaneSP> stylesBefore;
    QVector<KisAbstractProjectionPlaneSP> stylesAfter;

};

KisLayerStyleProjectionPlane::KisLayerStyleProjectionPlane(KisLayerSP sourceLayer)
    : m_d(new Private)
{
    m_d->sourceProjectionPlane = sourceLayer->projectionPlane();

    KisLayerStyleFilterProjectionPlane *dropShadow =
        new KisLayerStyleFilterProjectionPlane(sourceLayer);

    KisFilterConfiguration* kfc = new KisFilterConfiguration("lsdropshadow", 1);

    kfc->setProperty("drop_shadow/distance", 20);
    kfc->setProperty("context/global_angle", 135);
    kfc->setProperty("drop_shadow/spread", 50);
    kfc->setProperty("drop_shadow/size", 10);
    kfc->setProperty("drop_shadow/noise", 30);
    kfc->setProperty("drop_shadow/knocks_out", false);
    kfc->setProperty("drop_shadow/opacity", 50);
    kfc->setProperty("context/keep_original", false);

    dropShadow->setFilter(toQShared(kfc));

    m_d->stylesBefore << toQShared(dropShadow);
}

KisLayerStyleProjectionPlane::~KisLayerStyleProjectionPlane()
{
}

KisAbstractProjectionPlane* KisLayerStyleProjectionPlane::factoryObject(KisLayerSP sourceLayer)
{
    return new KisLayerStyleProjectionPlane(sourceLayer);
}

QRect KisLayerStyleProjectionPlane::recalculate(const QRect& rect, KisNodeSP filthyNode)
{
    return m_d->sourceProjectionPlane->recalculate(rect, filthyNode);
}

void KisLayerStyleProjectionPlane::apply(KisPainter *painter, const QRect &rect)
{
    foreach (const KisAbstractProjectionPlaneSP plane, m_d->stylesBefore) {
        plane->apply(painter, rect);
    }

    m_d->sourceProjectionPlane->apply(painter, rect);

    foreach (const KisAbstractProjectionPlaneSP plane, m_d->stylesAfter) {
        plane->apply(painter, rect);
    }
}

QRect KisLayerStyleProjectionPlane::needRect(const QRect &rect, KisLayer::PositionToFilthy pos) const
{
    return m_d->sourceProjectionPlane->needRect(rect, pos);
}

QRect KisLayerStyleProjectionPlane::changeRect(const QRect &rect, KisLayer::PositionToFilthy pos) const
{
    QRect layerChangeRect = m_d->sourceProjectionPlane->changeRect(rect, pos);
    QRect changeRect = layerChangeRect;

    foreach (const KisAbstractProjectionPlaneSP plane, m_d->stylesBefore) {
        changeRect |= plane->changeRect(layerChangeRect, KisLayer::N_ABOVE_FILTHY);
    }

    foreach (const KisAbstractProjectionPlaneSP plane, m_d->stylesAfter) {
        changeRect |= plane->changeRect(layerChangeRect, KisLayer::N_ABOVE_FILTHY);
    }

    return changeRect;
}

QRect KisLayerStyleProjectionPlane::accessRect(const QRect &rect, KisLayer::PositionToFilthy pos) const
{
    QRect accessRect = m_d->sourceProjectionPlane->accessRect(rect, pos);

    foreach (const KisAbstractProjectionPlaneSP plane, m_d->stylesBefore) {
        accessRect |= plane->accessRect(rect, KisLayer::N_ABOVE_FILTHY);
    }

    foreach (const KisAbstractProjectionPlaneSP plane, m_d->stylesAfter) {
        accessRect |= plane->accessRect(rect, KisLayer::N_ABOVE_FILTHY);
    }

    return accessRect;
}

