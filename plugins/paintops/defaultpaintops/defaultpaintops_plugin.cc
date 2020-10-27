/*
 * defaultpaintops_plugin.cc -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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

#include "defaultpaintops_plugin.h"
#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include <KoCompositeOpRegistry.h>

#include "kis_simple_paintop_factory.h"
#include "kis_brushop.h"
#include "kis_brushop_settings_widget.h"
#include "kis_duplicateop.h"
#include "kis_duplicateop_settings.h"
#include "kis_global.h"
#include <brushengine/kis_paintop_registry.h>
#include "KisBrushOpSettings.h"
#include "KisBrushServerProvider.h"
#include "kis_duplicateop_settings_widget.h"

K_PLUGIN_FACTORY_WITH_JSON(DefaultPaintOpsPluginFactory, "kritadefaultpaintops.json", registerPlugin<DefaultPaintOpsPlugin>();)


DefaultPaintOpsPlugin::DefaultPaintOpsPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisPaintOpRegistry *r = KisPaintOpRegistry::instance();
    r->add(new KisSimplePaintOpFactory<KisBrushOp, KisBrushOpSettings, KisBrushOpSettingsWidget>("paintbrush", i18nc("Pixel paintbrush", "Pixel"), KisPaintOpFactory::categoryStable(), "krita-paintbrush.png", QString(), QStringList(), 1));
    r->add(new KisSimplePaintOpFactory<KisDuplicateOp, KisDuplicateOpSettings, KisDuplicateOpSettingsWidget>("duplicate", i18nc("clone paintbrush (previously \"Duplicate\")", "Clone"), KisPaintOpFactory::categoryStable(), "krita-duplicate.png", QString(), QStringList(COMPOSITE_COPY), 15));
}

DefaultPaintOpsPlugin::~DefaultPaintOpsPlugin()
{
}

#include "defaultpaintops_plugin.moc"
