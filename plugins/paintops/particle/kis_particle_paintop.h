/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PARTICLE_PAINTOP_H_
#define KIS_PARTICLE_PAINTOP_H_

#include <brushengine/kis_paintop.h>
#include <kis_types.h>
#include <kis_airbrush_option_widget.h>
#include <kis_pressure_rate_option.h>

#include "kis_particle_paintop_settings.h"
#include "particle_brush.h"

class KisPainter;
class KisPaintInformation;

class KisParticlePaintOp : public KisPaintOp
{

public:

    KisParticlePaintOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisNodeSP node, KisImageSP image);
    ~KisParticlePaintOp() override;

    void paintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2, KisDistanceInformation *currentDistance) override;

protected:
    KisSpacingInformation paintAt(const KisPaintInformation& info) override;

    KisSpacingInformation updateSpacingImpl(const KisPaintInformation &info) const override;

    KisTimingInformation updateTimingImpl(const KisPaintInformation &info) const override;

private:
    void doPaintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2);

private:
    KisParticleBrushProperties m_properties;
    KisPaintDeviceSP m_dab;
    ParticleBrush m_particleBrush;
    KisAirbrushOptionProperties m_airbrushOption;
    KisPressureRateOption m_rateOption;
    bool m_first;
};

#endif // KIS_PARTICLE_PAINTOP_H_
