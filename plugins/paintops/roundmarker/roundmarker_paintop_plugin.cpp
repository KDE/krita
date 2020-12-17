/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "roundmarker_paintop_plugin.h"

#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include <brushengine/kis_paintop_registry.h>
#include "kis_roundmarkerop_settings.h"

#include "kis_roundmarkerop.h"
#include "kis_roundmarkerop_settings_widget.h"
#include "kis_simple_paintop_factory.h"

#include "kis_global.h"

K_PLUGIN_FACTORY_WITH_JSON(RoundMarkerPaintOpPluginFactory, "kritaroundmarkerpaintop.json", registerPlugin<RoundMarkerPaintOpPlugin>();)


RoundMarkerPaintOpPlugin::RoundMarkerPaintOpPlugin(QObject* parent, const QVariantList&):
    QObject(parent)
{
    KisPaintOpRegistry::instance()->add(new KisSimplePaintOpFactory<KisRoundMarkerOp, KisRoundMarkerOpSettings, KisRoundMarkerOpSettingsWidget>(
                                            "roundmarker", i18n("Quick Brush"), KisPaintOpFactory::categoryStable(), "krita_roundmarkerop.svg",
                                            QString(), QStringList(), 3)
                                       );
}

RoundMarkerPaintOpPlugin::~RoundMarkerPaintOpPlugin() { }

#include "roundmarker_paintop_plugin.moc"
