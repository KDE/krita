/*
 * curvepaintop_plugin.cc -- Part of Krita
 *
 * SPDX-FileCopyrightText: 2008 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "curve_paintop_plugin.h"
#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include <brushengine/kis_paintop_registry.h>

#include "kis_curve_paintop.h"
#include "kis_curve_paintop_settings_widget.h"
#include "kis_simple_paintop_factory.h"
#include "kis_global.h"

K_PLUGIN_FACTORY_WITH_JSON(CurvePaintOpPluginFactory, "kritacurvepaintop.json", registerPlugin<CurvePaintOpPlugin>();)


CurvePaintOpPlugin::CurvePaintOpPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisPaintOpRegistry *r = KisPaintOpRegistry::instance();
    r->add(new KisSimplePaintOpFactory<KisCurvePaintOp, KisCurvePaintOpSettings, KisCurvePaintOpSettingsWidget>("curvebrush", i18n("Curve"), KisPaintOpFactory::categoryStable(), "krita-curve.png", QString(), QStringList(), 9));

}

CurvePaintOpPlugin::~CurvePaintOpPlugin()
{
}

#include "curve_paintop_plugin.moc"
