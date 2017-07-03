/* This file is part of the KDE project
 *
 * Copyright (C) 2017 Grigory Tantsevov <tantsevov@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "watercolor_paintop_plugin.h"

#include <kpluginfactory.h>

#include <brushengine/kis_paintop_registry.h>
#include "kis_simple_paintop_factory.h"

#include "kis_watercolor_paintop.h"
#include "kis_watercolor_paintop_settings.h"
#include "kis_watercolor_paintop_settings_widget.h"

K_PLUGIN_FACTORY_WITH_JSON(WatercolorPaintOpPluginFactory, "kritawatercolorpaintop.json", registerPlugin<WatercolorPaintOpPlugin>();)

WatercolorPaintOpPlugin::WatercolorPaintOpPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisPaintOpRegistry *r = KisPaintOpRegistry::instance();
    r->add(new KisSimplePaintOpFactory<KisWatercolorPaintOp,
           KisWatercolorPaintOpSettings,
           KisWatercolorPaintOpSettingsWidget>("watercolorbrush",
                                               i18n("Watercolor"),
                                               KisPaintOpFactory::categoryStable(),
                                               "",                                  /// Image of brush
                                               QString(), QStringList(), 5));
}

WatercolorPaintOpPlugin::~WatercolorPaintOpPlugin()
{

}

#include "watercolor_paintop_plugin.moc"
