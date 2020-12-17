/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_small_tiles_filter_plugin.h"

#include <kpluginfactory.h>

#include "kis_small_tiles_filter.h"
#include "kis_global.h"
#include "filter/kis_filter_registry.h"

K_PLUGIN_FACTORY_WITH_JSON(KisSmallTilesFilterPluginFactory, "kritasmalltilesfilter.json", registerPlugin<KisSmallTilesFilterPlugin>();)

KisSmallTilesFilterPlugin::KisSmallTilesFilterPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisFilterRegistry::instance()->add(new KisSmallTilesFilter());
}

KisSmallTilesFilterPlugin::~KisSmallTilesFilterPlugin()
{
}

#include "kis_small_tiles_filter_plugin.moc"
