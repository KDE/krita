/*
 * SPDX-FileCopyrightText: 2008 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "spray_paintop_plugin.h"

#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include <brushengine/kis_paintop_registry.h>
#include <kis_fixed_paint_device.h>
#include "kis_spray_paintop.h"
#include "kis_simple_paintop_factory.h"

#include "kis_global.h"

K_PLUGIN_FACTORY_WITH_JSON(SprayPaintOpPluginFactory, "kritaspraypaintop.json", registerPlugin<SprayPaintOpPlugin>();)


SprayPaintOpPlugin::SprayPaintOpPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisPaintOpRegistry *r = KisPaintOpRegistry::instance();
    r->add(new KisSimplePaintOpFactory<KisSprayPaintOp, KisSprayPaintOpSettings, KisSprayPaintOpSettingsWidget>("spraybrush", i18n("Spray"), KisPaintOpFactory::categoryStable() , "krita-spray.png", QString(), QStringList(), 6));

}

SprayPaintOpPlugin::~SprayPaintOpPlugin()
{
}

#include "spray_paintop_plugin.moc"
