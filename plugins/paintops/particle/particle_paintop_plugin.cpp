/*
 * SPDX-FileCopyrightText: 2010 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "particle_paintop_plugin.h"


#include <klocalizedstring.h>
#include <kis_debug.h>
#include <kpluginfactory.h>

#include <brushengine/kis_paintop_registry.h>


#include "kis_particle_paintop.h"
#include "kis_particle_paintop_settings_widget.h"

#include <kis_simple_paintop_factory.h>

#include "kis_global.h"

K_PLUGIN_FACTORY_WITH_JSON(ParticlePaintOpPluginFactory, "kritaparticlepaintop.json", registerPlugin<ParticlePaintOpPlugin>();)


ParticlePaintOpPlugin::ParticlePaintOpPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisPaintOpRegistry *r = KisPaintOpRegistry::instance();
    r->add(new KisSimplePaintOpFactory<KisParticlePaintOp, KisParticlePaintOpSettings, KisParticlePaintOpSettingsWidget>("particlebrush", i18n("Particle"), KisPaintOpFactory::categoryStable(), "krita-particle.png", QString(), QStringList(), 11));
}

ParticlePaintOpPlugin::~ParticlePaintOpPlugin()
{
}

#include "particle_paintop_plugin.moc"
