/*
 * hairy_paintop_plugin.cc -- Part of Krita
 *
 * SPDX-FileCopyrightText: 2008 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "hairy_paintop_plugin.h"
#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include <brushengine/kis_paintop_registry.h>

#include "kis_simple_paintop_factory.h"
#include "kis_hairy_paintop.h"
#include "kis_hairy_paintop_settings_widget.h"
#include "kis_hairy_paintop_settings.h"
#include "kis_global.h"

K_PLUGIN_FACTORY_WITH_JSON(HairyPaintOpPluginFactory, "kritahairypaintop.json", registerPlugin<HairyPaintOpPlugin>();)


HairyPaintOpPlugin::HairyPaintOpPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisPaintOpRegistry *r = KisPaintOpRegistry::instance();
    r->add(new KisSimplePaintOpFactory<KisHairyPaintOp, KisHairyPaintOpSettings, KisHairyPaintOpSettingsWidget>("hairybrush", i18n("Bristle"), KisPaintOpFactory::categoryStable(), "krita-sumi.png", QString(), QStringList(), 4));

}

HairyPaintOpPlugin::~HairyPaintOpPlugin()
{
}

#include "hairy_paintop_plugin.moc"
