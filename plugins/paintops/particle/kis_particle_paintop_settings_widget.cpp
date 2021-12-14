/*
 *  SPDX-FileCopyrightText: 2008 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    addPaintOpOption(new KisParticleOpOption());
    addPaintOpOption(new KisCompositeOpOption(true));
    addPaintOpOption(new KisAirbrushOptionWidget(false, false));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRateOption(), i18n("0%"), i18n("100%")));
    addPaintOpOption(new KisPaintActionTypeOption());
}

KisParticlePaintOpSettingsWidget::~ KisParticlePaintOpSettingsWidget()
{
}

KisPropertiesConfigurationSP  KisParticlePaintOpSettingsWidget::configuration() const
{
    KisParticlePaintOpSettings* config = new KisParticlePaintOpSettings(resourcesInterface());
    config->setProperty("paintop", "particlebrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}
