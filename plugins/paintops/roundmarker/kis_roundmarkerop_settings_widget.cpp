/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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
    KisRoundMarkerOpSettings *config = new KisRoundMarkerOpSettings();
    config->setOptionsWidget(const_cast<KisRoundMarkerOpSettingsWidget*>(this));
    config->setProperty("paintop", "roundmarker");
    writeConfiguration(config);
    return config;
}
