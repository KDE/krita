/*
 * Copyright (c) 2009 Lukáš Tvrdý (lukast.dev@gmail.com)
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
#include "grid_paintop_plugin.h"

#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include <brushengine/kis_paintop_registry.h>

#include "kis_grid_paintop.h"
#include "kis_simple_paintop_factory.h"

#include "kis_global.h"

K_PLUGIN_FACTORY_WITH_JSON(GridPaintOpPluginFactory, "kritagridpaintop.json", registerPlugin<GridPaintOpPlugin>();)


GridPaintOpPlugin::GridPaintOpPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisPaintOpRegistry *r = KisPaintOpRegistry::instance();
    r->add(new KisSimplePaintOpFactory<KisGridPaintOp, KisGridPaintOpSettings, KisGridPaintOpSettingsWidget>("gridbrush", i18nc("type of a brush engine, shown in the list of brush engines", "Grid"),
                                                                                                             KisPaintOpFactory::categoryStable(), "krita-grid.png", QString(), QStringList(), 8));

}

GridPaintOpPlugin::~GridPaintOpPlugin()
{
}

#include "grid_paintop_plugin.moc"
