/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
