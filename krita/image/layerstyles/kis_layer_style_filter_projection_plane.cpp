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

#include "kis_layer_style_filter_projection_plane.h"

#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "kis_layer_style_filter.h"
#include "kis_layer_style_filter_environment.h"
#include "kis_psd_layer_style.h"


#include "kis_painter.h"


struct KisLayerStyleFilterProjectionPlane::Private
{
    KisLayer *sourceLayer;

    QScopedPointer<KisLayerStyleFilter> filter;
    KisPSDLayerStyleSP style;
    QScopedPointer<KisLayerStyleFilterEnvironment> environment;
};

KisLayerStyleFilterProjectionPlane::
KisLayerStyleFilterProjectionPlane(KisLayer *sourceLayer)
    : m_d(new Private)
{
    Q_ASSERT(sourceLayer);
    m_d->sourceLayer = sourceLayer;
    m_d->environment.reset(new KisLayerStyleFilterEnvironment(sourceLayer));
}

KisLayerStyleFilterProjectionPlane::~KisLayerStyleFilterProjectionPlane()
{
}

void KisLayerStyleFilterProjectionPlane::setStyle(KisLayerStyleFilter *filter, KisPSDLayerStyleSP style)
{
    m_d->filter.reset(filter);
    m_d->style = style;
}

QRect KisLayerStyleFilterProjectionPlane::recalculate(const QRect& rect, KisNodeSP filthyNode)
{
    Q_UNUSED(rect);
    Q_UNUSED(filthyNode);

    /// do nothing
    return QRect();
}

void KisLayerStyleFilterProjectionPlane::apply(KisPainter *painter, const QRect &rect)
{
    if (!m_d->sourceLayer || !m_d->filter) {
        warnKrita << "KisLayerStyleFilterProjectionPlane::apply(): [BUG] is not initialized";
        return;
    }

    m_d->filter->processDirectly(m_d->sourceLayer->projection(),
                                 painter->device(),
                                 rect,
                                 m_d->style,
                                 m_d->environment.data());
}


QRect KisLayerStyleFilterProjectionPlane::needRect(const QRect &rect, KisLayer::PositionToFilthy pos) const
{
    if (!m_d->sourceLayer || !m_d->filter) {
        warnKrita << "KisLayerStyleFilterProjectionPlane::needRect(): [BUG] is not initialized";
        return rect;
    }

    KIS_ASSERT_RECOVER_NOOP(pos == KisLayer::N_ABOVE_FILTHY);
    return m_d->filter->neededRect(rect, m_d->style);
}

QRect KisLayerStyleFilterProjectionPlane::changeRect(const QRect &rect, KisLayer::PositionToFilthy pos) const
{
    if (!m_d->sourceLayer || !m_d->filter) {
        warnKrita << "KisLayerStyleFilterProjectionPlane::changeRect(): [BUG] is not initialized";
        return rect;
    }

    KIS_ASSERT_RECOVER_NOOP(pos == KisLayer::N_ABOVE_FILTHY);
    return m_d->filter->changedRect(rect, m_d->style);
}

QRect KisLayerStyleFilterProjectionPlane::accessRect(const QRect &rect, KisLayer::PositionToFilthy pos) const
{
    return needRect(rect, pos);
}

