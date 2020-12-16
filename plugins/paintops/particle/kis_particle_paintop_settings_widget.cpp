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
    KisParticlePaintOpSettings* config = new KisParticlePaintOpSettings(resourcesInterface());
    config->setOptionsWidget(const_cast<KisParticlePaintOpSettingsWidget*>(this));
    config->setProperty("paintop", "particlebrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}
