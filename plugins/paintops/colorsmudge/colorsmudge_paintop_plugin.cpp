/*
 *  SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "colorsmudge_paintop_plugin.h"

#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include <brushengine/kis_paintop_registry.h>
#include "kis_colorsmudgeop_settings.h"

#include "kis_colorsmudgeop.h"
#include "kis_colorsmudgeop_settings_widget.h"
#include "kis_simple_paintop_factory.h"

#include "kis_global.h"

K_PLUGIN_FACTORY_WITH_JSON(ColorSmudgePaintOpPluginFactory, "kritacolorsmudgepaintop.json", registerPlugin<ColorSmudgePaintOpPlugin>();)


ColorSmudgePaintOpPlugin::ColorSmudgePaintOpPlugin(QObject* parent, const QVariantList&):
    QObject(parent)
{
    KisPaintOpRegistry::instance()->add(new KisSimplePaintOpFactory<KisColorSmudgeOp, KisColorSmudgeOpSettings, KisColorSmudgeOpSettingsWidget>(
                                            "colorsmudge", i18n("Color Smudge"), KisPaintOpFactory::categoryStable(), "krita-colorsmudge.png",
                                            QString(), QStringList(), 2)
                                       );
}

ColorSmudgePaintOpPlugin::~ColorSmudgePaintOpPlugin() { }

#include "colorsmudge_paintop_plugin.moc"
