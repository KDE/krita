/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "DodgeBurnPlugin.h"

#include <kgenericfactory.h>
#include <filter/kis_filter_registry.h>

#include "DodgeBurn.h"

typedef KGenericFactory<DodgeBurnPlugin> DodgeBurnPluginFactory;
K_EXPORT_COMPONENT_FACTORY(kritadodgeburn, DodgeBurnPluginFactory("krita"))

DodgeBurnPlugin::DodgeBurnPlugin(QObject *parent, const QStringList &)
{
    Q_UNUSED(parent);
    KisFilterRegistry::instance()->add(new KisFilterDodgeBurn("dodge", "Dodge", i18n("Dodge")));
    KisFilterRegistry::instance()->add(new KisFilterDodgeBurn("burn", "Burn", i18n("Burn")));
}

DodgeBurnPlugin::~DodgeBurnPlugin()
{
}
