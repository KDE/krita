/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "DodgeBurnPlugin.h"

#include <kpluginfactory.h>
#include <filter/kis_filter_registry.h>

#include "DodgeBurn.h"

K_PLUGIN_FACTORY_WITH_JSON(DodgeBurnPluginFactory, "kritadodgeburn.json", registerPlugin<DodgeBurnPlugin>();)

DodgeBurnPlugin::DodgeBurnPlugin(QObject *parent, const QVariantList &)
{
    Q_UNUSED(parent);
    KisFilterRegistry::instance()->add(new KisFilterDodgeBurn("dodge", "Dodge", i18n("Dodge...")));
    KisFilterRegistry::instance()->add(new KisFilterDodgeBurn("burn", "Burn", i18n("Burn...")));
}

DodgeBurnPlugin::~DodgeBurnPlugin()
{
}

#include "DodgeBurnPlugin.moc"
