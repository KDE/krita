/*
 *  SPDX-FileCopyrightText: 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "MyPaintPaintOp.h"

#include <KoColorConversions.h>
#include <KoCompositeOpRegistry.h>
#include <KoToolManager.h>
#include <QDebug>
#include <QtMath>
#include <kis_brush_based_paintop_settings.h>
#include <kis_image.h>
#include <kis_node.h>
#include <kis_paint_information.h>
#include <kis_painter.h>
#include <kis_paintop.h>
#include <kis_paintop_plugin_utils.h>
#include <kis_paintop_settings.h>
#include <kis_spacing_information.h>
#include <libmypaint/mypaint-brush.h>

#include "MyPaintPaintOpOption.h"

KisMyPaintPaintOp::KisMyPaintPaintOp(const KisPaintOpSettingsSP settings, KisPainter *painter, KisNodeSP /*node*/, KisImageSP image)
    : KisPaintOp (painter) {

    m_image = image;

    m_brush.reset(new KisMyPaintPaintOpPreset());
    m_surface.reset(new KisMyPaintSurface(this->painter(), nullptr, m_image));

    m_brush->apply(settings);

    if (!qRound(settings->getFloat(MYPAINT_ERASER)) && settings->getBool("EraserMode")) {

        mypaint_brush_from_defaults(m_brush->brush());
        mypaint_brush_set_base_value(m_brush->brush(), MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC, log(settings->getFloat(MYPAINT_DIAMETER)/2));
        painter->setCompositeOp(COMPOSITE_ERASE);
        mypaint_brush_set_base_value(m_brush->brush(), MYPAINT_BRUSH_SETTING_ERASER, false);
    }
    else if (qRound(settings->getFloat(MYPAINT_ERASER)) && settings->getBool("EraserMode")) {

        painter->setCompositeOp(COMPOSITE_ERASE);
        mypaint_brush_set_base_value(m_brush->brush(), MYPAINT_BRUSH_SETTING_ERASER, false);
    }

    m_brush->setColor(this->painter()->paintColor(), painter->device()->colorSpace());

    if (KoToolManager::instance()->activeToolId() != "KritaShape/KisToolBrush") {
        mypaint_brush_set_base_value(m_brush->brush(), MYPAINT_BRUSH_SETTING_SLOW_TRACKING, 0.0);
    }

    m_settings = settings;
    m_airBrushOption.readOptionSetting(m_settings);

    m_dtime = -1;
    m_isStrokeStarted = false;
    m_radius = settings->getFloat(MYPAINT_DIAMETER)/2;
}

KisMyPaintPaintOp::~KisMyPaintPaintOp() {
}

KisSpacingInformation KisMyPaintPaintOp::paintAt(const KisPaintInformation& info) {

    if (!painter()) {
        return KisSpacingInformation(1.0);
    }

    const qreal lodScale = KisLodTransform::lodToScale(painter()->device());
    qreal radius = m_radius * lodScale;

    mypaint_brush_set_base_value(m_brush->brush(), MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC, log(radius));

    m_isStrokeStarted = mypaint_brush_get_state(m_brush->brush(), MYPAINT_BRUSH_STATE_STROKE_STARTED);
    if (!m_isStrokeStarted) {

        mypaint_brush_stroke_to(m_brush->brush(), m_surface->surface(), info.pos().x(), info.pos().y(), info.pressure(),
                               info.xTilt(), info.yTilt(), 1.0f);

        m_dtime = 0.015;
    }
    else {
        m_dtime = abs(info.currentTime() - m_previousTime)*0.001;
    }

    mypaint_brush_stroke_to(m_brush->brush(), m_surface->surface(), info.pos().x(), info.pos().y(), info.pressure(),
                           info.xTilt(), info.yTilt(), m_dtime);

    m_previousTime = info.currentTime();

    return computeSpacing(info, lodScale);
}

KisSpacingInformation KisMyPaintPaintOp::updateSpacingImpl(const KisPaintInformation &info) const
{
    KisSpacingInformation spacingInfo = computeSpacing(info, KisLodTransform::lodToScale(painter()->device()));
    return spacingInfo;
}

KisTimingInformation KisMyPaintPaintOp::updateTimingImpl(const KisPaintInformation &info) const {

    return KisPaintOpPluginUtils::effectiveTiming(&m_airBrushOption, nullptr, info);
}

KisSpacingInformation KisMyPaintPaintOp::computeSpacing(const KisPaintInformation &info, qreal lodScale) const {

    return KisPaintOpPluginUtils::effectiveSpacing(m_radius*2, m_radius*2,
                                                   false, 0.0, false, m_radius*2,
                                                   true, 1, lodScale, &m_airBrushOption, nullptr, info);
}
