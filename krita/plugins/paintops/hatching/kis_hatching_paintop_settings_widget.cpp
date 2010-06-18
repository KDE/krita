/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2010 José Luis Vergara <pentalis@gmail.com>
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

#include "kis_hatching_paintop_settings_widget.h"

#include "kis_hatching_options.h"
#include "kis_hatching_preferences.h"
#include "kis_hatching_paintop_settings.h"

#include "kis_hatching_pressure_crosshatching_option.h"
#include "kis_hatching_pressure_separation_option.h"
#include "kis_hatching_pressure_thickness_option.h"

#include <kis_curve_option_widget.h>
#include <kis_pressure_opacity_option.h>

#include <kis_paintop_options_widget.h>
#include <kis_paint_action_type_option.h>

KisHatchingPaintOpSettingsWidget:: KisHatchingPaintOpSettingsWidget(QWidget* parent)
        : KisBrushBasedPaintopOptionWidget(parent)
{
    addPaintOpOption(new KisHatchingOptions());
    addPaintOpOption(new KisHatchingPreferences());
    //addPaintOpOption(new KisCurveOptionWidget(new KisPressureOpacityOption()));
    addPaintOpOption(new KisPaintActionTypeOption());
    addPaintOpOption(new KisCurveOptionWidget(new KisHatchingPressureSeparationOption()));
    addPaintOpOption(new KisCurveOptionWidget(new KisHatchingPressureThicknessOption()));
    addPaintOpOption(new KisCurveOptionWidget(new KisHatchingPressureCrosshatchingOption()));
}

KisHatchingPaintOpSettingsWidget::~ KisHatchingPaintOpSettingsWidget()
{
}

KisPropertiesConfiguration*  KisHatchingPaintOpSettingsWidget::configuration() const
{
    KisHatchingPaintOpSettings* config = new KisHatchingPaintOpSettings();
    config->setOptionsWidget(const_cast<KisHatchingPaintOpSettingsWidget*>(this));
    config->setProperty("paintop", "hatchingbrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}