/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2008, 2009 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2010 José Luis Vergara Toloza <pentalis@gmail.com>
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

#ifndef KIS_HATCHING_PAINTOP_H_
#define KIS_HATCHING_PAINTOP_H_

#include <brushengine/kis_paintop.h>
#include <kis_brush_based_paintop.h>
#include <kis_types.h>

#include "hatching_brush.h"
#include "kis_hatching_paintop_settings.h"

#include <kis_hatching_pressure_angle_option.h>
#include <kis_hatching_pressure_crosshatching_option.h>
#include <kis_hatching_pressure_separation_option.h>
#include <kis_hatching_pressure_thickness_option.h>

#include <kis_pressure_opacity_option.h>
#include <kis_pressure_size_option.h>


class KisPainter;

class KisHatchingPaintOp : public KisBrushBasedPaintOp
{

public:

    KisHatchingPaintOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisNodeSP node, KisImageSP image);
    ~KisHatchingPaintOp() override;

    /**
     *  Returns a number between -90 and 90, and corresponds to the
     *  angle that results from adding angle 'spin' to 'm_settings->angle',
     *  corrected to coincide with the way the GUI operates.
     */
    double spinAngle(double spin);

protected:
    /**
     *  Paint a hatched dab around the mouse cursor according to
     *  sensor settings and user preferences.
     */
    KisSpacingInformation paintAt(const KisPaintInformation& info) override;

    KisSpacingInformation updateSpacingImpl(const KisPaintInformation &info) const override;

private:
    KisHatchingPaintOpSettingsSP m_settings;
    HatchingBrush *m_hatchingBrush;

    /**
     *  PaintDevice that will be filled with a single pass of
     *  hatching by HatchingBrush::hatch
     */
    KisPaintDeviceSP m_hatchedDab;

    /**
     *  Curve to control the hatching angle
     *  according to user preferences set in the GUI
     */
    KisHatchingPressureAngleOption m_angleOption;

    /**
     *  Curve to control the intensity of crosshatching
     *  according to user preferences set in the GUI
     */
    KisHatchingPressureCrosshatchingOption m_crosshatchingOption;

    /**
     *  Curve to control the dynamics of separation with
     *  device input
     */
    KisHatchingPressureSeparationOption m_separationOption;

    /**
     *  Curve to control the thickness of the hatching lines
     *  with device input
     */
    KisHatchingPressureThicknessOption m_thicknessOption;

    /**
     *  Curve to control the opacity of the entire dab
     *  with device input
     */
    KisPressureOpacityOption m_opacityOption;

    /**
     *  Curve to control the size of the entire dab
     *  with device input
     */
    KisPressureSizeOption m_sizeOption;
};

#endif // KIS_HATCHING_PAINTOP_H_
