/*
 *  SPDX-FileCopyrightText: 2015 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tangent_normal_paintop_plugin.h"

#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include <brushengine/kis_paintop_registry.h>
#include <kis_brush_based_paintop_settings.h>

#include "kis_tangent_normal_paintop.h"
#include "kis_tangent_normal_paintop_settings_widget.h"
#include "kis_simple_paintop_factory.h"

#include "kis_global.h"

K_PLUGIN_FACTORY_WITH_JSON(TangentNormalPaintOpPluginFactory, "kritatangentnormalpaintop.json", registerPlugin<TangentNormalPaintOpPlugin>();)

TangentNormalPaintOpPlugin::TangentNormalPaintOpPlugin(QObject* parent, const QVariantList&):
    QObject(parent)
{
    KisPaintOpRegistry::instance()->add(new KisSimplePaintOpFactory<KisTangentNormalPaintOp, KisBrushBasedPaintOpSettings, KisTangentNormalPaintOpSettingsWidget>(
                                            "tangentnormal", i18n("Tangent Normal"), KisPaintOpFactory::categoryStable(), "krita-tangentnormal.png",
                                            QString(), QStringList(), 16)
                                       );
}

TangentNormalPaintOpPlugin::~TangentNormalPaintOpPlugin() { }

#include "kis_tangent_normal_paintop_plugin.moc"
