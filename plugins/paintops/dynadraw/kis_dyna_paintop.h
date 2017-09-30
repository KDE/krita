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

#ifndef KIS_DYNA_PAINTOP_H_
#define KIS_DYNA_PAINTOP_H_

#include <klocalizedstring.h>
#include <brushengine/kis_paintop.h>
#include <kis_types.h>

#include "kis_airbrush_option.h"
#include "kis_pressure_rate_option.h"
#include "dyna_brush.h"


class KisPainter;
class KisDynaPaintOpSettings;

class KisDynaPaintOp : public KisPaintOp
{

public:

    KisDynaPaintOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisNodeSP node, KisImageSP image);
    ~KisDynaPaintOp() override;

    void paintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2, KisDistanceInformation *currentDistance) override;

    virtual bool incremental() const {
        return true;
    }

protected:
    KisSpacingInformation paintAt(const KisPaintInformation& info) override;

    KisSpacingInformation updateSpacingImpl(const KisPaintInformation &info) const override;

    KisTimingInformation updateTimingImpl(const KisPaintInformation &info) const override;

private:
    void doPaintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2);

private:
    KisDynaProperties m_properties;
    KisPaintDeviceSP m_dab;
    DynaBrush m_dynaBrush;
    KisAirbrushOption m_airbrushOption;
    KisPressureRateOption m_rateOption;
};

#endif // KIS_DYNA_PAINTOP_H_
