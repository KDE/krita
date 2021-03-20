/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_particle_paintop.h"
#include "kis_particle_paintop_settings.h"

#include <cmath>

#include "kis_vec.h"

#include <KoCompositeOp.h>

#include <kis_image.h>
#include <kis_debug.h>

#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_lod_transform.h>
#include <kis_types.h>
#include <kis_paintop_plugin_utils.h>
#include <brushengine/kis_paintop.h>
#include <brushengine/kis_paint_information.h>

#include "kis_particleop_option.h"

#include "particle_brush.h"

KisParticlePaintOp::KisParticlePaintOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisNodeSP node, KisImageSP image)
    : KisPaintOp(painter)
{
    Q_UNUSED(image);
    Q_UNUSED(node);

    m_properties.particleCount = settings->getInt(PARTICLE_COUNT);
    m_properties.iterations = settings->getInt(PARTICLE_ITERATIONS);
    m_properties.gravity = settings->getDouble(PARTICLE_GRAVITY);
    m_properties.weight = settings->getDouble(PARTICLE_WEIGHT);
    m_properties.scale = QPointF(settings->getDouble(PARTICLE_SCALE_X), settings->getDouble(PARTICLE_SCALE_Y));

    m_particleBrush.setProperties(&m_properties);
    m_particleBrush.initParticles();

    m_airbrushOption.readOptionSetting(settings);

    m_rateOption.readOptionSetting(settings);
    m_rateOption.resetAllSensors();

    m_first = true;
}

KisParticlePaintOp::~KisParticlePaintOp()
{
}

KisSpacingInformation KisParticlePaintOp::paintAt(const KisPaintInformation& info)
{
    doPaintLine(info, info);
    return updateSpacingImpl(info);
}

KisSpacingInformation KisParticlePaintOp::updateSpacingImpl(const KisPaintInformation &info) const
{
    return KisPaintOpPluginUtils::effectiveSpacing(0.0, 0.0, true, 0.0, false, 0.0, false, 0.0,
                                                   KisLodTransform::lodToScale(painter()->device()),
                                                   &m_airbrushOption, nullptr, info);
}

KisTimingInformation KisParticlePaintOp::updateTimingImpl(const KisPaintInformation &info) const
{
    return KisPaintOpPluginUtils::effectiveTiming(&m_airbrushOption, &m_rateOption, info);
}

void KisParticlePaintOp::paintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2,
                                   KisDistanceInformation *currentDistance)
{
    // Use superclass behavior for lines of zero length. Otherwise, airbrushing can happen faster
    // than it is supposed to.
    if (pi1.pos() == pi2.pos()) {
        KisPaintOp::paintLine(pi1, pi2, currentDistance);
    } else {
        doPaintLine(pi1, pi2);
    }
}

void KisParticlePaintOp::doPaintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2)
{
    if (!painter()) return;

    if (!m_dab) {
        m_dab = source()->createCompositionSourceDevice();
    }
    else {
        m_dab->clear();
    }


    if (m_first) {
        m_particleBrush.setInitialPosition(pi1.pos());
        m_first = false;
    }

    m_particleBrush.draw(m_dab, painter()->paintColor(), pi2.pos());
    QRect rc = m_dab->extent();

    painter()->bitBlt(rc.x(), rc.y(), m_dab, rc.x(), rc.y(), rc.width(), rc.height());
    painter()->renderMirrorMask(rc, m_dab);
}
