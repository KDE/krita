/*
 * SPDX-FileCopyrightText: 2010 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "sketch_paintop_plugin.h"


#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include <brushengine/kis_paintop_registry.h>


#include "kis_sketch_paintop.h"
#include "kis_simple_paintop_factory.h"

#include "kis_global.h"

K_PLUGIN_FACTORY_WITH_JSON(SketchPaintOpPluginFactory, "kritasketchpaintop.json", registerPlugin<SketchPaintOpPlugin>();)


SketchPaintOpPlugin::SketchPaintOpPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisPaintOpRegistry *r = KisPaintOpRegistry::instance();
    r->add(new KisSimplePaintOpFactory<KisSketchPaintOp, KisSketchPaintOpSettings, KisSketchPaintOpSettingsWidget>("sketchbrush", i18n("Sketch"), KisPaintOpFactory::categoryStable(), "krita-sketch.png", QString(), QStringList(), 3));

}

SketchPaintOpPlugin::~SketchPaintOpPlugin()
{
}

#include "sketch_paintop_plugin.moc"
