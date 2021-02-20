/*
 * SPDX-FileCopyrightText: 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "MyPaintPaintOpPlugin.h"

#include <KisResourceLoader.h>
#include <KisResourceLoaderRegistry.h>
#include <brushengine/kis_paintop_registry.h>
#include <kis_debug.h>
#include <kis_fixed_paint_device.h>
#include <kis_global.h>
#include <kis_simple_paintop_factory.h>
#include <klocalizedstring.h>
#include <kpluginfactory.h>

#include "MyPaintPaintOp.h"
#include "MyPaintPaintOpFactory.h"
#include "MyPaintPaintOpPreset.h"
#include "MyPaintPaintOpSettings.h"
#include "MyPaintPaintOpSettingsWidget.h"

K_PLUGIN_FACTORY_WITH_JSON(MyPaintOpPluginFactory, "kritamypaintop.json", registerPlugin<MyPaintOpPlugin>();)


MyPaintOpPlugin::MyPaintOpPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisResourceLoaderRegistry::instance()->registerLoader(new KisResourceLoader<KisMyPaintPaintOpPreset>(ResourceSubType::MyPaintPaintOpPresets, ResourceType::PaintOpPresets
                                                                                                         , i18n("MyPaint Brush Presets")
                                                                                                         , QStringList() << "application/x-mypaint-brush"));
    KisPaintOpRegistry::instance()->add(new KisMyPaintOpFactory());
}

MyPaintOpPlugin::~MyPaintOpPlugin()
{
}

#include "MyPaintPaintOpPlugin.moc"
