/*
 *  Copyright (c) 2008,2009 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include "kis_experiment_paintop_settings_widget.h"

#include "kis_experimentop_option.h"
#include "kis_experiment_paintop_settings.h"
#include "kis_experiment_shape_option.h"

#include <kis_color_option.h>
#include <kis_paintop_options_widget.h>
#include <kis_paint_action_type_option.h>

#include <kis_pressure_rotation_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_size_option.h>
#include <kis_curve_option_widget.h>

KisExperimentPaintOpSettingsWidget:: KisExperimentPaintOpSettingsWidget(QWidget* parent)
        : KisPaintOpOptionsWidget(parent)
{
    m_experimentOption =  new KisExperimentOpOption();
    m_experimentShapeOption = new KisExperimentShapeOption();
    m_ColorOption = new KisColorOption();
    
    addPaintOpOption(m_experimentOption);
    addPaintOpOption(m_experimentShapeOption);
    addPaintOpOption(m_ColorOption);
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRotationOption()));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSizeOption()));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureOpacityOption()));
    addPaintOpOption(new KisPaintActionTypeOption());
}

KisExperimentPaintOpSettingsWidget::~ KisExperimentPaintOpSettingsWidget()
{
    delete m_experimentOption;
    delete m_experimentShapeOption;
    delete m_ColorOption;
}

KisPropertiesConfiguration*  KisExperimentPaintOpSettingsWidget::configuration() const
{
    KisExperimentPaintOpSettings* config = new KisExperimentPaintOpSettings();
    config->setOptionsWidget(const_cast<KisExperimentPaintOpSettingsWidget*>(this));
    config->setProperty("paintop", "experimentbrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}
