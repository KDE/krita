/*
 *  Copyright (c) 2008 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_particle_paintop_settings_widget.h"

#include "kis_particleop_option.h"
#include "kis_particle_paintop_settings.h"

#include <kis_paintop_settings_widget.h>
#include <kis_curve_option_widget.h>
#include <kis_paint_action_type_option.h>
#include <kis_airbrush_option_widget.h>
#include <kis_compositeop_option.h>
#include <kis_pressure_rate_option.h>

KisParticlePaintOpSettingsWidget:: KisParticlePaintOpSettingsWidget(QWidget* parent)
    : KisPaintOpSettingsWidget(parent)
{
    m_paintActionTypeOption = new KisPaintActionTypeOption();
    m_particleOption =  new KisParticleOpOption();

    addPaintOpOption(m_particleOption, i18n("Brush size"));
    addPaintOpOption(new KisCompositeOpOption(true), i18n("Blending Mode"));
    addPaintOpOption(new KisAirbrushOptionWidget(false, false), i18n("Airbrush"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRateOption(), i18n("0%"),
                                              i18n("100%")), i18n("Rate"));
    addPaintOpOption(m_paintActionTypeOption, i18n("Painting Mode"));
}

KisParticlePaintOpSettingsWidget::~ KisParticlePaintOpSettingsWidget()
{
}

KisPropertiesConfigurationSP  KisParticlePaintOpSettingsWidget::configuration() const
{
    KisParticlePaintOpSettings* config = new KisParticlePaintOpSettings();
    config->setOptionsWidget(const_cast<KisParticlePaintOpSettingsWidget*>(this));
    config->setProperty("paintop", "particlebrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}
