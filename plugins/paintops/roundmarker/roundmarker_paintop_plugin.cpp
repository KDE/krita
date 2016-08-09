/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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
                                            "roundmarker", i18n("Round Marker"), KisPaintOpFactory::categoryStable(), "krita-colorsmudge.png",
                                            QString(), QStringList(), 0)
                                       );
}

RoundMarkerPaintOpPlugin::~RoundMarkerPaintOpPlugin() { }

#include "roundmarker_paintop_plugin.moc"
