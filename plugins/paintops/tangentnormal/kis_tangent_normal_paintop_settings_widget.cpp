/*
 *  SPDX-FileCopyrightText: 2015 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tangent_normal_paintop_settings_widget.h"
#include "kis_brush_based_paintop_settings.h"
#include "kis_tangent_tilt_option.h"

#include <kis_properties_configuration.h>
#include <kis_paintop_settings_widget.h>
#include <kis_curve_option_widget.h>
#include <kis_pressure_rotation_option.h>
#include <kis_pressure_scatter_option_widget.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_flow_option.h>
#include <kis_paint_action_type_option.h>
#include <kis_pressure_size_option.h>
#include <kis_pressure_softness_option.h>
#include <kis_pressure_sharpness_option_widget.h>
#include <kis_airbrush_option_widget.h>
#include <kis_pressure_flow_opacity_option_widget.h>
#include <kis_pressure_spacing_option_widget.h>
#include <kis_pressure_rate_option.h>
#include <kis_compositeop_option.h>
#include "kis_texture_option.h"
#include <kis_pressure_mirror_option_widget.h>
#include "kis_pressure_texture_strength_option.h"


KisTangentNormalPaintOpSettingsWidget::KisTangentNormalPaintOpSettingsWidget(QWidget* parent):
    KisBrushBasedPaintopOptionWidget(parent)
{
    setObjectName("brush option widget");
    setPrecisionEnabled(true);
    setHSLBrushTipEnabled(true);

    addPaintOpOption(new KisCompositeOpOption(true));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureOpacityOption(), i18n("Transparent"), i18n("Opaque")));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureFlowOption(), i18n("0%"), i18n("100%")));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSizeOption(), i18n("0%"), i18n("100%")));

    addPaintOpOption(new KisTangentTiltOption());

    addPaintOpOption(new KisPressureSpacingOptionWidget());
    addPaintOpOption(new KisPressureMirrorOptionWidget());

    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSoftnessOption(), i18n("Soft"), i18n("Hard")));
    addPaintOpOption(new KisPressureSharpnessOptionWidget());
    addPaintOpOption(new KisPressureScatterOptionWidget());
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRotationOption(), i18n("-180°"), i18n("180°")));
    addPaintOpOption(new KisAirbrushOptionWidget(false));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRateOption(), i18n("0%"), i18n("100%")));
    addPaintOpOption(new KisPaintActionTypeOption());

    addPaintOpOption(new KisTextureOption());
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureTextureStrengthOption(), i18n("Weak"), i18n("Strong")));
}

KisTangentNormalPaintOpSettingsWidget::~KisTangentNormalPaintOpSettingsWidget() { }

KisPropertiesConfigurationSP KisTangentNormalPaintOpSettingsWidget::configuration() const
{
    KisBrushBasedPaintOpSettingsSP config = new KisBrushBasedPaintOpSettings(resourcesInterface());
    config->setProperty("paintop", "tangentnormal");
    writeConfiguration(config);
    return config;
}


