/*
 * Copyright (c) 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <kis_debug.h>
#include <kis_global.h>
#include <kpluginfactory.h>
#include <klocalizedstring.h>
#include <kis_fixed_paint_device.h>
#include <kis_simple_paintop_factory.h>
#include <brushengine/kis_paintop_registry.h>

#include "kis_my_paintop.h"
#include "my_paintop_plugin.h"
#include "kis_my_paintop_factory.h"
#include "kis_my_paintop_settings.h"
#include "kis_my_paintop_settings_widget.h"

K_PLUGIN_FACTORY_WITH_JSON(MyPaintOpPluginFactory, "kritamypaintop.json", registerPlugin<MyPaintOpPlugin>();)


MyPaintOpPlugin::MyPaintOpPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisPaintOpRegistry *r = KisPaintOpRegistry::instance();
    //r->add(new KisSimplePaintOpFactory<KisMyPaintOp, KisMyPaintOpSettings, KisMyPaintOpSettingsWidget>("mypaintbrush", i18n("MyPaint"), KisPaintOpFactory::categoryStable() , "krita-mypaint.png", QString(), QStringList(), 6));
    r->add(new KisMyPaintOpFactory());
}

MyPaintOpPlugin::~MyPaintOpPlugin()
{
}

#include "my_paintop_plugin.moc"
