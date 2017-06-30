/*
 *  Copyright (c) 2009-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_dyna_paintop.h"
#include "kis_dyna_paintop_settings.h"

#include <cmath>

#include <QRect>

#include <kis_image.h>
#include <kis_debug.h>

#include <kis_vec.h>
#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <brushengine/kis_paint_information.h>
#include <kis_types.h>
#include <brushengine/kis_paintop.h>
#include <kis_selection.h>
#include <kis_random_accessor_ng.h>
#include <kis_paintop_plugin_utils.h>
#include <kis_lod_transform.h>

#include "kis_dynaop_option.h"

#include "filter.h"

KisDynaPaintOp::KisDynaPaintOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisNodeSP node, KisImageSP image)
    : KisPaintOp(painter)
{
    Q_UNUSED(node);

    if (image) {
        m_dynaBrush.setCanvasSize(image->width(), image->height());
    }
    else {
        // some dummy values for scratchpad
        m_dynaBrush.setCanvasSize(1000, 1000);
    }

    m_properties.initWidth = settings->getDouble(DYNA_WIDTH);
    m_properties.action = settings->getDouble(DYNA_ACTION);
    m_properties.mass = settings->getDouble(DYNA_MASS);
    m_properties.drag = settings->getDouble(DYNA_DRAG);

    double angle = settings->getDouble(DYNA_ANGLE);
    m_properties.xAngle = cos(angle * M_PI / 180.0);
    m_properties.yAngle = sin(angle * M_PI / 180.0);

    m_properties.widthRange = settings->getDouble(DYNA_WIDTH_RANGE);
    m_properties.diameter = settings->getInt(DYNA_DIAMETER);
    m_properties.lineCount = settings->getInt(DYNA_LINE_COUNT);
    m_properties.lineSpacing = settings->getDouble(DYNA_LINE_SPACING);
    m_properties.enableLine = settings->getBool(DYNA_ENABLE_LINE);
    m_properties.useTwoCircles = settings->getBool(DYNA_USE_TWO_CIRCLES);
    m_properties.useFixedAngle = settings->getBool(DYNA_USE_FIXED_ANGLE);

    m_dynaBrush.setProperties(&m_properties);

    m_airbrushOption.readOptionSetting(settings);

    m_rateOption.readOptionSetting(settings);
    m_rateOption.resetAllSensors();
}

KisDynaPaintOp::~KisDynaPaintOp()
{
}

void KisDynaPaintOp::paintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2,
                                 KisDistanceInformation *currentDistance)
{
    // Use superclass behavior for lines of zero length. Otherwise, airbrushing can happen faster
    // than it is supposed to.
    if (pi1.pos() == pi2.pos()) {
        KisPaintOp::paintLine(pi1, pi2, currentDistance);
    }
    else {
        doPaintLine(pi1, pi2);
    }
}

void KisDynaPaintOp::doPaintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2)
{
    Q_UNUSED(pi2);
    if (!painter()) return;

    if (!m_dab) {
        m_dab = source()->createCompositionSourceDevice();
    }
    else {
        m_dab->clear();
    }

    qreal x1, y1;

    x1 = pi1.pos().x();
    y1 = pi1.pos().y();

    m_dynaBrush.updateCursorPosition(pi1.pos());
    m_dynaBrush.paint(m_dab, x1, y1, painter()->paintColor());

    QRect rc = m_dab->extent();

    painter()->bitBlt(rc.topLeft(), m_dab, rc);
    painter()->renderMirrorMask(rc, m_dab);
}

KisSpacingInformation KisDynaPaintOp::paintAt(const KisPaintInformation& info)
{
    doPaintLine(info, info);
    return updateSpacingImpl(info);
}

KisSpacingInformation KisDynaPaintOp::updateSpacingImpl(const KisPaintInformation &info) const
{
    return KisPaintOpPluginUtils::effectiveSpacing(0.0, 0.0, true, 0.0, false, 0.0, false, 0.0,
                                                   KisLodTransform::lodToScale(painter()->device()),
                                                   &m_airbrushOption, nullptr, info);
}

KisTimingInformation KisDynaPaintOp::updateTimingImpl(const KisPaintInformation &info) const
{
    return KisPaintOpPluginUtils::effectiveTiming(&m_airbrushOption, &m_rateOption, info);
}
