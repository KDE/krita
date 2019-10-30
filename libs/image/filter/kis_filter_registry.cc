/*
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "filter/kis_filter_registry.h"

#include <math.h>

#include <QString>
#include <QApplication>

#include <klocalizedstring.h>

#include <KoPluginLoader.h>

#include "kis_debug.h"
#include "kis_types.h"

#include "kis_paint_device.h"
#include "filter/kis_filter.h"
#include "kis_filter_configuration.h"

KisFilterRegistry::KisFilterRegistry(QObject *parent)
    : QObject(parent)
{
}

KisFilterRegistry::~KisFilterRegistry()
{
    dbgRegistry << "deleting KisFilterRegistry";
    Q_FOREACH (KisFilterSP filter, values()) {
        remove(filter->id());
        filter.clear();
    }
}

KisFilterRegistry* KisFilterRegistry::instance()
{
    KisFilterRegistry *reg = qApp->findChild<KisFilterRegistry *>(QString());
    if (!reg) {
        dbgRegistry << "initializing KisFilterRegistry";
        reg = new KisFilterRegistry(qApp);
        KoPluginLoader::instance()->load("Krita/Filter", "Type == 'Service' and ([X-Krita-Version] == 28)");
    }
    return reg;
}

void KisFilterRegistry::add(KisFilterSP item)
{
    add(item->id(), item);
}

void KisFilterRegistry::add(const QString &id, KisFilterSP item)
{
    KoGenericRegistry<KisFilterSP>::add(id, item);
    emit(filterAdded(id));
}

KisFilterConfigurationSP KisFilterRegistry::cloneConfiguration(const KisFilterConfigurationSP kfc)
{
    Q_ASSERT(kfc);
    KisFilterSP filter = value(kfc->name());
    KisFilterConfigurationSP newkfc(filter->factoryConfiguration());
    newkfc->fromXML(kfc->toXML());
    return newkfc;
}

