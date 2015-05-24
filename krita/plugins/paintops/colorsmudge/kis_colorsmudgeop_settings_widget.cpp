/*
 *  Copyright (C) 2011 Silvio Heinrich <plassy@web.de>
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

#include "kis_colorsmudgeop_settings_widget.h"
#include "kis_brush_based_paintop_settings.h"
#include "kis_overlay_mode_option.h"
#include "kis_rate_option.h"
#include "kis_smudge_option_widget.h"
#include "kis_smudge_radius_option.h"

#include <kis_properties_configuration.h>
#include <kis_paintop_settings_widget.h>
#include <kis_pressure_size_option.h>
#include <kis_curve_option_widget.h>
#include <kis_pressure_rotation_option.h>
#include <kis_pressure_scatter_option_widget.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_gradient_option.h>
#include <kis_airbrush_option.h>
#include <kis_compositeop_option.h>
#include <kis_pressure_spacing_option_widget.h>
#include "kis_texture_option.h"
#include "kis_curve_option_widget.h"
#include <kis_pressure_mirror_option_widget.h>
#include "kis_pressure_texture_strength_option.h"

KisColorSmudgeOpSettingsWidget::KisColorSmudgeOpSettingsWidget(QWidget* parent):
    KisBrushBasedPaintopOptionWidget(parent)
{
    setObjectName("brush option widget");
    setPrecisionEnabled(true);

    addPaintOpOption(new KisCompositeOpOption(true), i18n("Blending Mode"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureOpacityOption(), i18n("Transparent"), i18n("Opaque")), i18n("Opacity"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSizeOption(), i18n("0%"), i18n("100%")), i18n("Size"));
    addPaintOpOption(new KisPressureSpacingOptionWidget(), i18n("Spacing"));
    addPaintOpOption(new KisPressureMirrorOptionWidget(), i18n("Mirror"));

    addPaintOpOption(new KisSmudgeOptionWidget(), i18n("Smudge Length"));
    addPaintOpOption(new KisCurveOptionWidget(new KisSmudgeRadiusOption(), i18n("0.0"), i18n("1.0")), i18n("Smudge Radius"));
    addPaintOpOption(new KisCurveOptionWidget(new KisRateOption("ColorRate", KisPaintOpOption::GENERAL, false), i18n("0.0"), i18n("1.0")), i18n("Color Rate"));

    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRotationOption(), i18n("0°"), i18n("360°")), i18n("Rotation"));
    addPaintOpOption(new KisPressureScatterOptionWidget(), i18n("Scatter"));
    addPaintOpOption(new KisOverlayModeOptionWidget(), i18n("Overlay Mode"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureGradientOption(), i18n("0%"), i18n("100%")), i18n("Gradient"));

    addPaintOpOption(new KisTextureOption(), i18n("Pattern"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureTextureStrengthOption(), i18n("Weak"), i18n("Strong")), i18n("Strength"));

}

KisColorSmudgeOpSettingsWidget::~KisColorSmudgeOpSettingsWidget() { }

KisPropertiesConfiguration* KisColorSmudgeOpSettingsWidget::configuration() const
{
    KisBrushBasedPaintOpSettings *config = new KisBrushBasedPaintOpSettings();
    config->setOptionsWidget(const_cast<KisColorSmudgeOpSettingsWidget*>(this));
    config->setProperty("paintop", "colorsmudge");
    writeConfiguration(config);
    return config;
}


