/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef KIS_HAIRYPAINTOP_H_
#define KIS_HAIRYPAINTOP_H_

#include <klocalizedstring.h>
#include <brushengine/kis_paintop.h>
#include <brushengine/kis_paintop_factory.h>
#include <kis_types.h>

#include "hairy_brush.h"

#include <kis_pressure_size_option.h>
#include <kis_pressure_rotation_option.h>
#include <kis_pressure_opacity_option.h>

class KisPainter;
class KisBrushBasedPaintOpSettings;

class KisHairyPaintOp : public KisPaintOp
{

public:
    KisHairyPaintOp(const KisPaintOpSettingsSP settings, KisPainter *painter, KisNodeSP node, KisImageSP image);

    void paintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2, KisDistanceInformation *currentDistance) override;

protected:
    KisSpacingInformation paintAt(const KisPaintInformation& info) override;

    KisSpacingInformation updateSpacingImpl(const KisPaintInformation &info) const override;

private:
    KisHairyProperties m_properties;

    KisPaintDeviceSP m_dab;
    KisPaintDeviceSP m_dev;
    HairyBrush m_brush;
    KisPressureRotationOption m_rotationOption;
    KisPressureSizeOption m_sizeOption;
    KisPressureOpacityOption m_opacityOption;

    void loadSettings(const KisBrushBasedPaintOpSettings* settings);
};

#endif // KIS_HAIRYPAINTOP_H_
