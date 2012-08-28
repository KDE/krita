/*
 *  Copyright (c) 2010 José Luis Vergara <pentalis@gmail.com>
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
#include "waterymix_paintop_plugin.h"


#include <klocale.h>
#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kpluginfactory.h>

#include <kis_paintop_registry.h>


#include "kis_waterymix_paintop.h"
#include "kis_simple_paintop_factory.h"

#include "kis_global.h"

K_PLUGIN_FACTORY(WateryMixPaintOpPluginFactory, registerPlugin<WateryMixPaintOpPlugin>();)
K_EXPORT_PLUGIN(WateryMixPaintOpPluginFactory("krita"))


WateryMixPaintOpPlugin::WateryMixPaintOpPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisPaintOpRegistry *r = KisPaintOpRegistry::instance();
    r->add(new KisSimplePaintOpFactory<KisWateryMixPaintOp, KisWateryMixPaintOpSettings, KisWateryMixPaintOpSettingsWidget>("waterymixbrush", i18n("Watery Mixbrush"), KisPaintOpFactory::categoryExperimental(),"krita-waterymix.png"));

}

WateryMixPaintOpPlugin::~WateryMixPaintOpPlugin()
{
}

#include "waterymix_paintop_plugin.moc"
