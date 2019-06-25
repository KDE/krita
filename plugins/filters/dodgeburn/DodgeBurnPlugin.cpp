/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "DodgeBurnPlugin.h"

#include <kpluginfactory.h>
#include <filter/kis_filter_registry.h>

#include "DodgeBurn.h"

K_PLUGIN_FACTORY_WITH_JSON(DodgeBurnPluginFactory, "kritadodgeburn.json", registerPlugin<DodgeBurnPlugin>();)

DodgeBurnPlugin::DodgeBurnPlugin(QObject *parent, const QVariantList &)
{
    Q_UNUSED(parent);
    KisFilterRegistry::instance()->add(new KisFilterDodgeBurn("dodge", "Dodge", i18n("Dodge")));
    KisFilterRegistry::instance()->add(new KisFilterDodgeBurn("burn", "Burn", i18n("Burn")));
}

DodgeBurnPlugin::~DodgeBurnPlugin()
{
}

#include "DodgeBurnPlugin.moc"
