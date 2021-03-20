/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_raindrops_filter_plugin.h"

#include <kpluginfactory.h>

#include <filter/kis_filter_registry.h>


#include "kis_raindrops_filter.h"

K_PLUGIN_FACTORY_WITH_JSON(KisRainDropsFilterPluginFactory, "kritaraindropsfilter.json", registerPlugin<KisRainDropsFilterPlugin>();)

KisRainDropsFilterPlugin::KisRainDropsFilterPlugin(QObject *parent, const QVariantList &) : QObject(parent)
{
    KisFilterRegistry::instance()->add(new KisRainDropsFilter());

}

KisRainDropsFilterPlugin::~KisRainDropsFilterPlugin()
{
}

#include "kis_raindrops_filter_plugin.moc"
