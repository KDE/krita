/*
 * SPDX-FileCopyrightText: 2008 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "hatching_paintop_plugin.h"


#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include <brushengine/kis_paintop_registry.h>


#include "kis_hatching_paintop.h"
#include "kis_simple_paintop_factory.h"

#include "kis_global.h"

K_PLUGIN_FACTORY_WITH_JSON(HatchingPaintOpPluginFactory, "kritahatchingpaintop.json", registerPlugin<HatchingPaintOpPlugin>();)


HatchingPaintOpPlugin::HatchingPaintOpPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisPaintOpRegistry *r = KisPaintOpRegistry::instance();
    r->add(new KisSimplePaintOpFactory<KisHatchingPaintOp, KisHatchingPaintOpSettings, KisHatchingPaintOpSettingsWidget>("hatchingbrush", i18n("Hatching"),
                                                                                                                         KisPaintOpFactory::categoryStable() , "krita-hatching.png", QString(), QStringList(), 7));

}

HatchingPaintOpPlugin::~HatchingPaintOpPlugin()
{
}

#include "hatching_paintop_plugin.moc"
