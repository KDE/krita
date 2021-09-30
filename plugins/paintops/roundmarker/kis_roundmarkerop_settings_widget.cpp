/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_roundmarkerop_settings_widget.h"
#include "kis_brush_based_paintop_settings.h"

#include <kis_properties_configuration.h>
#include <kis_paintop_settings_widget.h>
#include <kis_pressure_size_option.h>
#include <kis_curve_option_widget.h>
#include <kis_compositeop_option.h>
#include <kis_pressure_spacing_option_widget.h>
//#include "kis_texture_option.h"
//#include "kis_pressure_texture_strength_option.h"
#include "kis_roundmarkerop_settings.h"
#include "kis_roundmarker_option.h"


KisRoundMarkerOpSettingsWidget::KisRoundMarkerOpSettingsWidget(QWidget* parent)
    : KisPaintOpSettingsWidget(parent)
{
    setObjectName("roundmarker option widget");
    //setPrecisionEnabled(true);

    addPaintOpOption(new KisRoundMarkerOption(), i18n("Brush"));
    addPaintOpOption(new KisCompositeOpOption(true), i18n("Blending Mode"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSizeOption(), i18n("0%"), i18n("100%")), i18n("Size"));
    addPaintOpOption(new KisPressureSpacingOptionWidget(), i18n("Spacing"));

    //addPaintOpOption(new KisTextureOption(), i18n("Pattern"));
    //addPaintOpOption(new KisCurveOptionWidget(new KisPressureTextureStrengthOption(), i18n("Weak"), i18n("Strong")), i18n("Strength"));
}

KisRoundMarkerOpSettingsWidget::~KisRoundMarkerOpSettingsWidget() { }

KisPropertiesConfigurationSP KisRoundMarkerOpSettingsWidget::configuration() const
{
    KisRoundMarkerOpSettings *config = new KisRoundMarkerOpSettings(resourcesInterface());
    config->setProperty("paintop", "roundmarker");
    writeConfiguration(config);
    return config;
}
