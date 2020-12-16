/*
 * defaultpaintops_plugin.cc -- Part of Krita
 *
 * SPDX-FileCopyrightText: 2004 Boudewijn Rempt (boud@valdyas.org)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
