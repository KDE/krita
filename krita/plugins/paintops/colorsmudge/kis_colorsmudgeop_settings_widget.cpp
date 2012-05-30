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
#include "kis_rate_option_widget.h"
#include "kis_smudge_option_widget.h"

#include <kis_properties_configuration.h>
#include <kis_paintop_options_widget.h>
#include <kis_pressure_size_option.h>
#include <kis_pressure_spacing_option.h>
#include <kis_pressure_rate_option.h>
#include <kis_curve_option_widget.h>
#include <kis_pressure_rotation_option.h>
#include <kis_pressure_scatter_option_widget.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_gradient_option.h>
#include <kis_airbrush_option.h>
#include <kis_compositeop_option.h>

KisColorSmudgeOpSettingsWidget::KisColorSmudgeOpSettingsWidget(QWidget* parent):
    KisBrushBasedPaintopOptionWidget(parent)
{
    setObjectName("brush option widget");
    
//     KisSmudgeOptionWidget* opt = 

    addPaintOpOption(new KisCompositeOpOption(true));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSizeOption()));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureOpacityOption()));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSpacingOption()));
    addPaintOpOption(new KisSmudgeOptionWidget(i18n("Smudge Rate"), i18n("Rate: "), "SmudgeRate", true));
    addPaintOpOption(new KisRateOptionWidget(i18n("Color Rate") , i18n("Rate: "), "ColorRate" , false));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRotationOption()));
    addPaintOpOption(new KisPressureScatterOptionWidget());
//     addPaintOpOption(new KisAirbrushOption(false));
    addPaintOpOption(new KisOverlayModeOptionWidget());
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureGradientOption()));
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


#include "kis_colorsmudgeop_settings_widget.moc"
