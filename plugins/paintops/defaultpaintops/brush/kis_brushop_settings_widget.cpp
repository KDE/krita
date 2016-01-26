/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#include "kis_brushop_settings_widget.h"
#include <kis_brush_based_paintop_settings.h>
#include <kis_pressure_darken_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_flow_option.h>
#include <kis_pressure_size_option.h>
#include <kis_paint_action_type_option.h>
#include <kis_pressure_rotation_option.h>
#include <kis_pressure_mix_option.h>
#include <kis_curve_option_widget.h>
#include <kis_pressure_hsv_option.h>
#include <kis_airbrush_option.h>
#include <kis_pressure_scatter_option_widget.h>
#include <kis_pressure_softness_option.h>
#include <kis_pressure_sharpness_option_widget.h>
#include <kis_color_source_option_widget.h>
#include <kis_compositeop_option.h>
#include <kis_pressure_flow_opacity_option_widget.h>
#include <kis_pressure_spacing_option_widget.h>
#include "kis_texture_option.h"
#include "kis_curve_option_widget.h"
#include <kis_pressure_mirror_option_widget.h>
#include "kis_pressure_texture_strength_option.h"


KisBrushOpSettingsWidget::KisBrushOpSettingsWidget(QWidget* parent)
    : KisBrushBasedPaintopOptionWidget(parent)
{
    setObjectName("brush option widget");
    setPrecisionEnabled(true);

    // Brush tip options
    addPaintOpOption(new KisCompositeOpOption(true), i18n("Blending Mode"));
    addPaintOpOption(new KisFlowOpacityOptionWidget(), i18n("Opacity"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureFlowOption(), i18n("0%"), i18n("100%")), i18n("Flow"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSizeOption(), i18n("0%"), i18n("100%")), i18n("Size"));
    addPaintOpOption(new KisPressureSpacingOptionWidget(), i18n("Spacing"));
    addPaintOpOption(new KisPressureMirrorOptionWidget(), i18n("Mirror"));


    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSoftnessOption(), i18n("Soft"), i18n("Hard")), i18n("Softness"));
    addPaintOpOption(new KisPressureSharpnessOptionWidget(), i18n("Sharpness"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRotationOption(), i18n("0°"), i18n("360°")), i18n("Rotation"));
    addPaintOpOption(new KisPressureScatterOptionWidget(), i18n("Scatter"));

    // Colors options
    addPaintOpOption(new KisColorSourceOptionWidget(), i18n("Source"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureDarkenOption(), i18n("0.0"), i18n("1.0")), i18n("Darken"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureMixOption(), i18n("Foreground"), i18n("Background")), i18n("Mix"));
    addPaintOpOption(new KisCurveOptionWidget(KisPressureHSVOption::createHueOption(), KisPressureHSVOption::hueMinLabel(), KisPressureHSVOption::huemaxLabel()), i18n("Hue"));
    addPaintOpOption(new KisCurveOptionWidget(KisPressureHSVOption::createSaturationOption(), KisPressureHSVOption::saturationMinLabel(), KisPressureHSVOption::saturationmaxLabel()), i18n("Saturation"));
    addPaintOpOption(new KisCurveOptionWidget(KisPressureHSVOption::createValueOption(), KisPressureHSVOption::valueMinLabel(), KisPressureHSVOption::valuemaxLabel()), i18n("Value"));
    addPaintOpOption(new KisAirbrushOption(false), i18n("Airbrush"));
    addPaintOpOption(new KisPaintActionTypeOption(), i18n("Painting Mode"));

    addPaintOpOption(new KisTextureOption(), i18n("Pattern"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureTextureStrengthOption(), i18n("Weak"), i18n("Strong")), i18n("Strength"));

}

KisBrushOpSettingsWidget::~KisBrushOpSettingsWidget()
{
}

KisPropertiesConfiguration* KisBrushOpSettingsWidget::configuration() const
{
    KisBrushBasedPaintOpSettings *config = new KisBrushBasedPaintOpSettings();
    config->setOptionsWidget(const_cast<KisBrushOpSettingsWidget*>(this));
    config->setProperty("paintop", "paintbrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}

