/*
 *  Copyright (c) 2009,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include "soft_paintop_plugin.h"


#include <klocale.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kpluginfactory.h>

#include <kis_paintop_registry.h>
#include <kis_simple_paintop_factory.h>

#include "kis_soft_paintop.h"

#include "kis_global.h"

K_PLUGIN_FACTORY(SoftPaintOpPluginFactory, registerPlugin<SoftPaintOpPlugin>();)
K_EXPORT_PLUGIN(SoftPaintOpPluginFactory("krita"))


SoftPaintOpPlugin::SoftPaintOpPlugin(QObject *parent, const QVariantList &)
        : KParts::Plugin(parent)
{
    setComponentData(SoftPaintOpPluginFactory::componentData());
    KisPaintOpRegistry *r = KisPaintOpRegistry::instance();
    r->add(new KisSimplePaintOpFactory<KisSoftPaintOp, KisSoftPaintOpSettings, KisSoftPaintOpSettingsWidget>("softbrush", i18n("Soft brush"), "krita-soft.png"));
}

SoftPaintOpPlugin::~SoftPaintOpPlugin()
{
}

#include "soft_paintop_plugin.moc"
