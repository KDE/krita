/*
 * SPDX-FileCopyrightText: 2009 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
