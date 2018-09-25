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

#include "kis_global.h"
#include "kis_layer_style_filter_projection_plane.h"
#include "kis_psd_layer_style.h"

#include "kis_ls_drop_shadow_filter.h"
#include "kis_ls_satin_filter.h"
#include "kis_ls_overlay_filter.h"
#include "kis_ls_stroke_filter.h"
#include "kis_ls_bevel_emboss_filter.h"
#include "kis_projection_leaf.h"


struct Q_DECL_HIDDEN KisLayerStyleProjectionPlane::Private
{
    KisAbstractProjectionPlaneWSP sourceProjectionPlane;

    QVector<KisLayerStyleFilterProjectionPlaneSP> stylesBefore;
    QVector<KisLayerStyleFilterProjectionPlaneSP> stylesAfter;

    KisPSDLayerStyleSP style;
    bool canHaveChildNodes = false;
    bool dependsOnLowerNodes = false;

    void initSourcePlane(KisLayer *sourceLayer) {
        KIS_SAFE_ASSERT_RECOVER_RETURN(sourceLayer);
        sourceProjectionPlane = sourceLayer->internalProjectionPlane();
        canHaveChildNodes = sourceLayer->projectionLeaf()->canHaveChildLayers();
        dependsOnLowerNodes = sourceLayer->projectionLeaf()->dependsOnLowerNodes();
    }
};

KisLayerStyleProjectionPlane::KisLayerStyleProjectionPlane(KisLayer *sourceLayer)
    : m_d(new Private)
{
    KisPSDLayerStyleSP style = sourceLayer->layerStyle();

    KIS_ASSERT_RECOVER(style) {
        style = toQShared(new KisPSDLayerStyle());
    }

    init(sourceLayer, style);
}

KisLayerStyleProjectionPlane::KisLayerStyleProjectionPlane(const KisLayerStyleProjectionPlane &rhs, KisLayer *sourceLayer, KisPSDLayerStyleSP clonedStyle)
    : m_d(new Private)
{
    m_d->initSourcePlane(sourceLayer);
    m_d->style = clonedStyle;

    KIS_SAFE_ASSERT_RECOVER(m_d->style) {
        m_d->style = toQShared(new KisPSDLayerStyle());
    }

    Q_FOREACH (KisLayerStyleFilterProjectionPlaneSP plane, rhs.m_d->stylesBefore) {
        m_d->stylesBefore << toQShared(new KisLayerStyleFilterProjectionPlane(*plane, sourceLayer, m_d->style));
    }

    Q_FOREACH (KisLayerStyleFilterProjectionPlaneSP plane, rhs.m_d->stylesAfter) {
        m_d->stylesAfter << toQShared(new KisLayerStyleFilterProjectionPlane(*plane, sourceLayer, m_d->style));
    }
}

// for testing purposes only!
KisLayerStyleProjectionPlane::KisLayerStyleProjectionPlane(KisLayer *sourceLayer, KisPSDLayerStyleSP layerStyle)
    : m_d(new Private)
{
    init(sourceLayer, layerStyle);
}

void KisLayerStyleProjectionPlane::init(KisLayer *sourceLayer, KisPSDLayerStyleSP style)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(sourceLayer);
    m_d->initSourcePlane(sourceLayer);
    m_d->style = style;

    {
        KisLayerStyleFilterProjectionPlane *dropShadow =
            new KisLayerStyleFilterProjectionPlane(sourceLayer);
        dropShadow->setStyle(new KisLsDropShadowFilter(KisLsDropShadowFilter::DropShadow), style);
        m_d->stylesBefore << toQShared(dropShadow);
    }

    {
        KisLayerStyleFilterProjectionPlane *innerShadow =
            new KisLayerStyleFilterProjectionPlane(sourceLayer);
        innerShadow->setStyle(new KisLsDropShadowFilter(KisLsDropShadowFilter::InnerShadow), style);
        m_d->stylesAfter << toQShared(innerShadow);
    }

    {
        KisLayerStyleFilterProjectionPlane *outerGlow =
            new KisLayerStyleFilterProjectionPlane(sourceLayer);
        outerGlow->setStyle(new KisLsDropShadowFilter(KisLsDropShadowFilter::OuterGlow), style);
        m_d->stylesAfter << toQShared(outerGlow);
    }

    {
        KisLayerStyleFilterProjectionPlane *innerGlow =
            new KisLayerStyleFilterProjectionPlane(sourceLayer);
        innerGlow->setStyle(new KisLsDropShadowFilter(KisLsDropShadowFilter::InnerGlow), style);
        m_d->stylesAfter << toQShared(innerGlow);
    }

    {
        KisLayerStyleFilterProjectionPlane *satin =
            new KisLayerStyleFilterProjectionPlane(sourceLayer);
        satin->setStyle(new KisLsSatinFilter(), style);
        m_d->stylesAfter << toQShared(satin);
    }

    {
        KisLayerStyleFilterProjectionPlane *colorOverlay =
            new KisLayerStyleFilterProjectionPlane(sourceLayer);
        colorOverlay->setStyle(new KisLsOverlayFilter(KisLsOverlayFilter::Color), style);
        m_d->stylesAfter << toQShared(colorOverlay);
    }

    {
        KisLayerStyleFilterProjectionPlane *gradientOverlay =
            new KisLayerStyleFilterProjectionPlane(sourceLayer);
        gradientOverlay->setStyle(new KisLsOverlayFilter(KisLsOverlayFilter::Gradient), style);
        m_d->stylesAfter << toQShared(gradientOverlay);
    }

    {
        KisLayerStyleFilterProjectionPlane *patternOverlay =
            new KisLayerStyleFilterProjectionPlane(sourceLayer);
        patternOverlay->setStyle(new KisLsOverlayFilter(KisLsOverlayFilter::Pattern), style);
        m_d->stylesAfter << toQShared(patternOverlay);
    }

    {
        KisLayerStyleFilterProjectionPlane *stroke =
            new KisLayerStyleFilterProjectionPlane(sourceLayer);
        stroke->setStyle(new KisLsStrokeFilter(), style);
        m_d->stylesAfter << toQShared(stroke);
    }

    {
        KisLayerStyleFilterProjectionPlane *bevelEmboss =
            new KisLayerStyleFilterProjectionPlane(sourceLayer);
        bevelEmboss->setStyle(new KisLsBevelEmbossFilter(), style);
        m_d->stylesAfter << toQShared(bevelEmboss);
    }
}

KisLayerStyleProjectionPlane::~KisLayerStyleProjectionPlane()
{
}

KisAbstractProjectionPlaneSP KisLayerStyleProjectionPlane::factoryObject(KisLayer *sourceLayer)
{
    Q_ASSERT(sourceLayer);
    return toQShared(new KisLayerStyleProjectionPlane(sourceLayer));
}

QRect KisLayerStyleProjectionPlane::recalculate(const QRect& rect, KisNodeSP filthyNode)
{
    KisAbstractProjectionPlaneSP sourcePlane = m_d->sourceProjectionPlane.toStrongRef();
    QRect result = sourcePlane->recalculate(rect, filthyNode);

    if (m_d->style->isEnabled()) {
        Q_FOREACH (const KisAbstractProjectionPlaneSP plane, m_d->stylesBefore) {
            plane->recalculate(rect, filthyNode);
        }

        Q_FOREACH (const KisAbstractProjectionPlaneSP plane, m_d->stylesAfter) {
            plane->recalculate(rect, filthyNode);
        }
    }

    return result;
}

void KisLayerStyleProjectionPlane::apply(KisPainter *painter, const QRect &rect)
{
    KisAbstractProjectionPlaneSP sourcePlane = m_d->sourceProjectionPlane.toStrongRef();

    if (m_d->style->isEnabled()) {
        Q_FOREACH (const KisAbstractProjectionPlaneSP plane, m_d->stylesBefore) {
            plane->apply(painter, rect);
        }

        sourcePlane->apply(painter, rect);

        Q_FOREACH (const KisAbstractProjectionPlaneSP plane, m_d->stylesAfter) {
            plane->apply(painter, rect);
        }
    } else {
        sourcePlane->apply(painter, rect);
    }
}

KisPaintDeviceList KisLayerStyleProjectionPlane::getLodCapableDevices() const
{
    KisPaintDeviceList list;
    KisAbstractProjectionPlaneSP sourcePlane = m_d->sourceProjectionPlane.toStrongRef();

    if (m_d->style->isEnabled()) {
        Q_FOREACH (const KisAbstractProjectionPlaneSP plane, m_d->stylesBefore) {
            list << plane->getLodCapableDevices();
        }

        list << sourcePlane->getLodCapableDevices();

        Q_FOREACH (const KisAbstractProjectionPlaneSP plane, m_d->stylesAfter) {
            list << plane->getLodCapableDevices();
        }
    } else {
        list << sourcePlane->getLodCapableDevices();
    }

    return list;
}

QRect KisLayerStyleProjectionPlane::needRect(const QRect &rect, KisLayer::PositionToFilthy pos) const
{
    /**
     * Need rect should also be adjust for the layers that generate their 'original'
     * based on the contents of the underlying/child layers like
     * KisGroupLayer/KisAdjustmentLayer.
     *
     * \see bug 390299
     */

    KisAbstractProjectionPlaneSP sourcePlane = m_d->sourceProjectionPlane.toStrongRef();
    const QRect layerNeedRect = sourcePlane->needRect(rect, pos);
    QRect needRect = layerNeedRect;

    const bool dirtyGroupWithChildren = m_d->canHaveChildNodes && (pos & KisLayer::N_FILTHY);
    const bool adjustmentAboveDirty =
        m_d->canHaveChildNodes &&
        (pos & KisLayer::N_FILTHY || pos & KisLayer::N_ABOVE_FILTHY);

    if (m_d->style->isEnabled() && (dirtyGroupWithChildren || adjustmentAboveDirty)) {

        Q_FOREACH (const KisAbstractProjectionPlaneSP plane, m_d->stylesBefore) {
            needRect |= plane->needRect(layerNeedRect, KisLayer::N_ABOVE_FILTHY);
        }

        Q_FOREACH (const KisAbstractProjectionPlaneSP plane, m_d->stylesAfter) {
            needRect |= plane->needRect(layerNeedRect, KisLayer::N_ABOVE_FILTHY);
        }
    }

    return needRect;
}

QRect KisLayerStyleProjectionPlane::changeRect(const QRect &rect, KisLayer::PositionToFilthy pos) const
{
    KisAbstractProjectionPlaneSP sourcePlane = m_d->sourceProjectionPlane.toStrongRef();
    QRect layerChangeRect = sourcePlane->changeRect(rect, pos);
    QRect changeRect = layerChangeRect;

    if (m_d->style->isEnabled()) {
        Q_FOREACH (const KisAbstractProjectionPlaneSP plane, m_d->stylesBefore) {
            changeRect |= plane->changeRect(layerChangeRect, KisLayer::N_ABOVE_FILTHY);
        }

        Q_FOREACH (const KisAbstractProjectionPlaneSP plane, m_d->stylesAfter) {
            changeRect |= plane->changeRect(layerChangeRect, KisLayer::N_ABOVE_FILTHY);
        }
    }

    return changeRect;
}

QRect KisLayerStyleProjectionPlane::accessRect(const QRect &rect, KisLayer::PositionToFilthy pos) const
{
    KisAbstractProjectionPlaneSP sourcePlane = m_d->sourceProjectionPlane.toStrongRef();
    QRect accessRect = sourcePlane->accessRect(rect, pos);

    if (m_d->style->isEnabled()) {
        Q_FOREACH (const KisAbstractProjectionPlaneSP plane, m_d->stylesBefore) {
            accessRect |= plane->accessRect(rect, KisLayer::N_ABOVE_FILTHY);
        }

        Q_FOREACH (const KisAbstractProjectionPlaneSP plane, m_d->stylesAfter) {
            accessRect |= plane->accessRect(rect, KisLayer::N_ABOVE_FILTHY);
        }
    }

    return accessRect;
}

