/*
 * SPDX-FileCopyrightText: 2008 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "deform_paintop_plugin.h"
#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include <KoCompositeOpRegistry.h>

#include <brushengine/kis_paintop_registry.h>

#include "kis_deform_paintop.h"
#include "kis_global.h"
#include "kis_simple_paintop_factory.h"
#include "kis_deform_paintop_settings_widget.h"

K_PLUGIN_FACTORY_WITH_JSON(DeformPaintOpPluginFactory, "kritadeformpaintop.json", registerPlugin<DeformPaintOpPlugin>();)


DeformPaintOpPlugin::DeformPaintOpPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisPaintOpRegistry *r = KisPaintOpRegistry::instance();
    r->add(new KisSimplePaintOpFactory<KisDeformPaintOp, KisDeformPaintOpSettings, KisDeformPaintOpSettingsWidget>("deformbrush", i18n("Deform"), KisPaintOpFactory::categoryStable(), "krita-deform.png", QString(), QStringList(COMPOSITE_COPY), 16));
}

DeformPaintOpPlugin::~DeformPaintOpPlugin()
{
}

#include "deform_paintop_plugin.moc"
