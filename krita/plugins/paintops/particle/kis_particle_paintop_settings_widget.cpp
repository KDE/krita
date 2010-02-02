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

#include <kis_paintop_options_widget.h>
#include <kis_paint_action_type_option.h>

KisParticlePaintOpSettingsWidget:: KisParticlePaintOpSettingsWidget(QWidget* parent)
    : KisPaintOpOptionsWidget(parent)
{
    m_paintActionTypeOption = new KisPaintActionTypeOption();
    m_particleOption =  new KisParticleOpOption();

    addPaintOpOption(m_particleOption);
    addPaintOpOption(m_paintActionTypeOption);
}

KisParticlePaintOpSettingsWidget::~ KisParticlePaintOpSettingsWidget()
{
    delete m_particleOption;
    delete m_paintActionTypeOption;
}

KisPropertiesConfiguration*  KisParticlePaintOpSettingsWidget::configuration() const
{
    KisParticlePaintOpSettings* config = new KisParticlePaintOpSettings();
    config->setOptionsWidget( const_cast<KisParticlePaintOpSettingsWidget*>( this ) );
    config->setProperty("paintop", "particlebrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}
