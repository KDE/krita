/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Frederic Coiffier <fcoiffie@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <kpluginfactory.h>

#include <filter/kis_filter_registry.h>

#include "KisLevelFilter.h"
#include "KisLevelFilterPlugin.h"

K_PLUGIN_FACTORY_WITH_JSON(LevelFilterFactory, "kritalevelfilter.json", registerPlugin<KisLevelFilterPlugin>();)

KisLevelFilterPlugin::KisLevelFilterPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisFilterRegistry::instance()->add(new KisLevelFilter());
}

KisLevelFilterPlugin::~KisLevelFilterPlugin()
{}

#include "KisLevelFilterPlugin.moc"
