/*
  *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
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
#include "DualBrushPaintopPlugin.h"


#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include <brushengine/kis_paintop_registry.h>


#include "DualBrushPaintop.h"
#include "kis_simple_paintop_factory.h"

#include "kis_global.h"

K_PLUGIN_FACTORY_WITH_JSON(DualBrushPaintOpPluginFactory, "kritadualbrushpaintop.json", registerPlugin<DualBrushPaintOpPlugin>();)


DualBrushPaintOpPlugin::DualBrushPaintOpPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisPaintOpRegistry *r = KisPaintOpRegistry::instance();
    r->add(new KisSimplePaintOpFactory<KisDualBrushPaintOp, KisDualBrushPaintOpSettings, KisDualBrushPaintOpSettingsWidget>("DualBrushbrush", i18n("Stacked Brush"),
            KisPaintOpFactory::categoryStable(), "krita-DualBrush.png"));

}

DualBrushPaintOpPlugin::~DualBrushPaintOpPlugin()
{
}

#include "DualBrush_paintop_plugin.moc"
