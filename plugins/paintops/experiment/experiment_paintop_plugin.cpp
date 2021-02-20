/*
 * SPDX-FileCopyrightText: 2008 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "experiment_paintop_plugin.h"

#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include <brushengine/kis_paintop_registry.h>

#include "kis_experiment_paintop.h"
#include "kis_experiment_paintop_settings_widget.h"
#include "kis_simple_paintop_factory.h"

#include "kis_global.h"

K_PLUGIN_FACTORY_WITH_JSON(ExperimentPaintOpPluginFactory, "kritaexperimentpaintop.json", registerPlugin<ExperimentPaintOpPlugin>();)


ExperimentPaintOpPlugin::ExperimentPaintOpPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisPaintOpRegistry *r = KisPaintOpRegistry::instance();
    r->add(new KisSimplePaintOpFactory<KisExperimentPaintOp,
           KisExperimentPaintOpSettings,
           KisExperimentPaintOpSettingsWidget>("experimentbrush",
                                               i18n("Shape"),
                                               KisPaintOpFactory::categoryStable(),
                                               "krita-experiment.png",
                                               QString(), QStringList(), 5));
}

ExperimentPaintOpPlugin::~ExperimentPaintOpPlugin()
{
}

#include "experiment_paintop_plugin.moc"
