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
#include "kis_layer_projection_plane.h"
#include "kis_psd_layer_style.h"

#include "kis_ls_drop_shadow_filter.h"
#include "kis_ls_satin_filter.h"
#include "kis_ls_overlay_filter.h"
#include "kis_ls_stroke_filter.h"
#include "kis_ls_bevel_emboss_filter.h"
#include "kis_projection_leaf.h"
#include "kis_cached_paint_device.h"
#include "kis_painter.h"
#include "kis_ls_utils.h"
#include "KisLayerStyleKnockoutBlower.h"


struct Q_DECL_HIDDEN KisLayerStyleProjectionPlane::Private
{
    KisLayerProjectionPlaneWSP sourceProjectionPlane;

    QVector<KisLayerStyleFilterProjectionPlaneSP> stylesBefore;
    QVector<KisLayerStyleFilterProjectionPlaneSP> stylesAfter;
    QVector<KisLayerStyleFilterProjectionPlaneSP> stylesOverlay;

    KisCachedPaintDevice cachedPaintDevice;
    KisCachedSelection cachedSelection;
    KisLayer *sourceLayer = 0;


    KisPSDLayerStyleSP style;
    bool canHaveChildNodes = false;
    bool dependsOnLowerNodes = false;

    void initSourcePlane(KisLayer *sourceLayer) {
        KIS_SAFE_ASSERT_RECOVER_RETURN(sourceLayer);
        sourceProjectionPlane = sourceLayer->internalProjectionPlane();
        canHaveChildNodes = sourceLayer->projectionLeaf()->canHaveChildLayers();
        dependsOnLowerNodes = sourceLayer->projectionLeaf()->dependsOnLowerNodes();
        this->sourceLayer = sourceLayer;
    }

    QVector<KisLayerStyleFilterProjectionPlaneSP> allStyles() const {
        return stylesBefore + stylesOverlay + stylesAfter;
    }

    bool hasOverlayStyles() const {
        Q_FOREACH (KisLayerStyleFilterProjectionPlaneSP plane, stylesOverlay) {
            if (!plane->isEmpty()) return true;
        }

        return false;
    }

    bool hasKnockoutStyles() const {
        Q_FOREACH (KisLayerStyleFilterProjectionPlaneSP plane, stylesBefore) {
            if (!plane->knockoutBlower()->isEmpty()) return true;
        }

        Q_FOREACH (KisLayerStyleFilterProjectionPlaneSP plane, stylesAfter) {
            if (!plane->knockoutBlower()->isEmpty()) return true;
        }

        return false;
    }

    void applyComplexPlane(KisPainter *painter,
                           KisLayerStyleFilterProjectionPlaneSP plane,
                           const QRect &rect,
                           KisPaintDeviceSP originalClone);
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

    Q_FOREACH (KisLayerStyleFilterProjectionPlaneSP plane, rhs.m_d->allStyles()) {
        m_d->stylesBefore << toQShared(new KisLayerStyleFilterProjectionPlane(*plane, sourceLayer, m_d->style));
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
        KisLayerStyleFilterProjectionPlane *outerGlow =
            new KisLayerStyleFilterProjectionPlane(sourceLayer);
        outerGlow->setStyle(new KisLsDropShadowFilter(KisLsDropShadowFilter::OuterGlow), style);
        m_d->stylesAfter << toQShared(outerGlow);
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

    {
        KisLayerStyleFilterProjectionPlane *patternOverlay =
            new KisLayerStyleFilterProjectionPlane(sourceLayer);
        patternOverlay->setStyle(new KisLsOverlayFilter(KisLsOverlayFilter::Pattern), style);
        m_d->stylesOverlay << toQShared(patternOverlay);
    }

    {
        KisLayerStyleFilterProjectionPlane *gradientOverlay =
            new KisLayerStyleFilterProjectionPlane(sourceLayer);
        gradientOverlay->setStyle(new KisLsOverlayFilter(KisLsOverlayFilter::Gradient), style);
        m_d->stylesOverlay << toQShared(gradientOverlay);
    }

    {
        KisLayerStyleFilterProjectionPlane *colorOverlay =
            new KisLayerStyleFilterProjectionPlane(sourceLayer);
        colorOverlay->setStyle(new KisLsOverlayFilter(KisLsOverlayFilter::Color), style);
        m_d->stylesOverlay << toQShared(colorOverlay);
    }

    {
        KisLayerStyleFilterProjectionPlane *satin =
            new KisLayerStyleFilterProjectionPlane(sourceLayer);
        satin->setStyle(new KisLsSatinFilter(), style);
        m_d->stylesOverlay << toQShared(satin);
    }

    {
        KisLayerStyleFilterProjectionPlane *innerGlow =
            new KisLayerStyleFilterProjectionPlane(sourceLayer);
        innerGlow->setStyle(new KisLsDropShadowFilter(KisLsDropShadowFilter::InnerGlow), style);
        m_d->stylesOverlay << toQShared(innerGlow);
    }

    {
        KisLayerStyleFilterProjectionPlane *innerShadow =
            new KisLayerStyleFilterProjectionPlane(sourceLayer);
        innerShadow->setStyle(new KisLsDropShadowFilter(KisLsDropShadowFilter::InnerShadow), style);
        m_d->stylesOverlay << toQShared(innerShadow);
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
    QRect result = rect;

    if (m_d->style->isEnabled()) {
        result = sourcePlane->recalculate(stylesNeedRect(rect), filthyNode);

        Q_FOREACH (const KisAbstractProjectionPlaneSP plane, m_d->allStyles()) {
            plane->recalculate(rect, filthyNode);
        }
    } else {
        result = sourcePlane->recalculate(rect, filthyNode);
    }

    return result;
}

void KisLayerStyleProjectionPlane::Private::applyComplexPlane(KisPainter *painter,
                                                              KisLayerStyleFilterProjectionPlaneSP plane,
                                                              const QRect &rect,
                                                              KisPaintDeviceSP originalClone)
{
    if (plane->isEmpty()) return;

    if (!plane->knockoutBlower()->isEmpty()) {
        KisCachedPaintDevice::Guard d1(originalClone, cachedPaintDevice);
        KisPaintDeviceSP mergedStyle = d1.device();
        mergedStyle->makeCloneFromRough(originalClone, rect);

        KisPainter overlayPainter(mergedStyle);
        plane->apply(&overlayPainter, rect);
        plane->knockoutBlower()->apply(painter, mergedStyle, rect);

    } else {
        plane->apply(painter, rect);
    }
}

void KisLayerStyleProjectionPlane::apply(KisPainter *painter, const QRect &rect)
{
    KisLayerProjectionPlaneSP sourcePlane = m_d->sourceProjectionPlane.toStrongRef();

    if (m_d->style->isEnabled()) {
        if (m_d->hasOverlayStyles() || m_d->hasKnockoutStyles()) {
            KisCachedPaintDevice::Guard d1(painter->device(), m_d->cachedPaintDevice);
            KisPaintDeviceSP originalClone = d1.device();
            originalClone->makeCloneFromRough(painter->device(), rect);

            Q_FOREACH (const KisLayerStyleFilterProjectionPlaneSP plane, m_d->stylesBefore) {
                m_d->applyComplexPlane(painter, plane, rect, originalClone);
            }

            if (m_d->hasOverlayStyles()) {
                KisCachedSelection::Guard s1(m_d->cachedSelection);
                KisSelectionSP knockoutSelection = s1.selection();
                KisLsUtils::selectionFromAlphaChannel(m_d->sourceLayer->projection(), knockoutSelection, rect);

                KisCachedPaintDevice::Guard d2(painter->device(), m_d->cachedPaintDevice);
                KisPaintDeviceSP sourceProjection = d2.device();
                sourceProjection->makeCloneFromRough(painter->device(), rect);

                {
                    KisPainter overlayPainter(sourceProjection);
                    sourcePlane->applyMaxOutAlpha(&overlayPainter, rect);

                    Q_FOREACH (const KisAbstractProjectionPlaneSP plane, m_d->stylesOverlay) {
                        plane->apply(&overlayPainter, rect);
                    }
                }

                KisLayerStyleKnockoutBlower blower;
                blower.setKnockoutSelection(knockoutSelection);
                blower.apply(painter, sourceProjection, rect);

                blower.resetKnockoutSelection();
            } else {
                sourcePlane->apply(painter, rect);
            }

            Q_FOREACH (KisLayerStyleFilterProjectionPlaneSP plane, m_d->stylesAfter) {
                m_d->applyComplexPlane(painter, plane, rect, originalClone);
            }
        } else {
            Q_FOREACH (const KisAbstractProjectionPlaneSP plane, m_d->stylesBefore) {
                plane->apply(painter, rect);
            }

            sourcePlane->apply(painter, rect);

            Q_FOREACH (const KisAbstractProjectionPlaneSP plane, m_d->stylesAfter) {
                plane->apply(painter, rect);
            }
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
        Q_FOREACH (const KisAbstractProjectionPlaneSP plane, m_d->allStyles()) {
            list << plane->getLodCapableDevices();
        }

        list << sourcePlane->getLodCapableDevices();
    } else {
        list << sourcePlane->getLodCapableDevices();
    }

    return list;
}

QRect KisLayerStyleProjectionPlane::needRect(const QRect &rect, KisLayer::PositionToFilthy pos) const
{
    /**
     * Need rect should also be adjust for the layers that generate their 'original'
     * based on the contents of the underlying layers like KisAdjustmentLayer
     *
     * \see bug 390299
     */

    QRect needRect = rect;

    const bool adjustmentAboveDirty = m_d->dependsOnLowerNodes &&
        (pos & KisLayer::N_FILTHY || pos & KisLayer::N_ABOVE_FILTHY);

    if (m_d->style->isEnabled() && adjustmentAboveDirty) {
        needRect |= stylesNeedRect(rect);
    }

    KisAbstractProjectionPlaneSP sourcePlane = m_d->sourceProjectionPlane.toStrongRef();
    needRect = sourcePlane->needRect(needRect, pos);

    return needRect;
}

QRect KisLayerStyleProjectionPlane::changeRect(const QRect &rect, KisLayer::PositionToFilthy pos) const
{
    KisAbstractProjectionPlaneSP sourcePlane = m_d->sourceProjectionPlane.toStrongRef();
    QRect layerChangeRect = sourcePlane->changeRect(rect, pos);
    QRect changeRect = layerChangeRect;

    if (m_d->style->isEnabled()) {
        Q_FOREACH (const KisAbstractProjectionPlaneSP plane, m_d->allStyles()) {
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
        Q_FOREACH (const KisAbstractProjectionPlaneSP plane, m_d->allStyles()) {
            accessRect |= plane->accessRect(rect, KisLayer::N_ABOVE_FILTHY);
        }
    }

    return accessRect;
}

QRect KisLayerStyleProjectionPlane::needRectForOriginal(const QRect &rect) const
{
    /**
     * Need rect should also be adjust for the layers that generate their 'original'
     * based on the contents of the child layers like KisGroupLayer
     *
     * \see bug 366419
     */

    QRect needRect = rect;

    if (m_d->style->isEnabled()) {
        needRect |= stylesNeedRect(needRect);
    }

    KisAbstractProjectionPlaneSP sourcePlane = m_d->sourceProjectionPlane.toStrongRef();
    needRect = sourcePlane->needRectForOriginal(needRect);

    return needRect;
}

QRect KisLayerStyleProjectionPlane::stylesNeedRect(const QRect &rect) const
{
    QRect needRect = rect;

    Q_FOREACH (const KisAbstractProjectionPlaneSP plane, m_d->allStyles()) {
        needRect |= plane->needRect(rect, KisLayer::N_ABOVE_FILTHY);
    }

    return needRect;
}
