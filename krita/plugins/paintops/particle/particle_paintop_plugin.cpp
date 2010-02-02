/*
 * Copyright (c) 2010 Lukáš Tvrdý (lukast.dev@gmail.com)
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
#include "particle_paintop_plugin.h"


#include <klocale.h>
#include <kis_debug.h>
#include <kgenericfactory.h>

#include <kis_paintop_registry.h>


#include "kis_particle_paintop.h"
#include "kis_particle_paintop_settings_widget.h"

#include <kis_simple_paintop_factory.h>

#include "kis_global.h"

K_PLUGIN_FACTORY(ParticlePaintOpPluginFactory, registerPlugin<ParticlePaintOpPlugin>();)
K_EXPORT_PLUGIN(ParticlePaintOpPluginFactory("krita"))

ParticlePaintOpPlugin::ParticlePaintOpPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisPaintOpRegistry *r = KisPaintOpRegistry::instance();
    r->add(new KisSimplePaintOpFactory<KisParticlePaintOp, KisParticlePaintOpSettings, KisParticlePaintOpSettingsWidget>("particlebrush", i18n("Particle brush"), "krita-particle.png"));
}

ParticlePaintOpPlugin::~ParticlePaintOpPlugin()
{
}

#include "particle_paintop_plugin.moc"
