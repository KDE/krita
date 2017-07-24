/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2008, 2009 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef KIS_CHALK_PAINTOP_H_
#define KIS_CHALK_PAINTOP_H_

#include <brushengine/kis_paintop.h>
#include <kis_types.h>

#include "chalk_brush.h"
#include "kis_chalk_paintop_settings.h"
#include "kis_airbrush_option.h"
#include "kis_pressure_rate_option.h"

class KisPainter;

class KisChalkPaintOp : public KisPaintOp
{

public:

    KisChalkPaintOp(const KisPaintOpSettingsSP settings, KisPainter *painter, KisNodeSP node, KisImageSP image);
    ~KisChalkPaintOp() override;

protected:

    KisSpacingInformation paintAt(const KisPaintInformation& info) override;

    KisSpacingInformation updateSpacingImpl(const KisPaintInformation &info) const override;

    KisTimingInformation updateTimingImpl(const KisPaintInformation &info) const override;

private:
    KisPaintDeviceSP m_dab;
    ChalkBrush * m_chalkBrush;
    KisAirbrushOption m_airbrushOption;
    KisPressureOpacityOption m_opacityOption;
    KisPressureRateOption m_rateOption;
    ChalkProperties m_properties;
};

#endif // KIS_CHALK_PAINTOP_H_
