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

#include <kglobal.h>
#include <kaction.h>
#include <klocale.h>
#include <kparts/plugin.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kparts/componentfactory.h>

#include <KoPluginLoader.h>

#include "kis_debug.h"
#include "kis_types.h"

#include "kis_paint_device.h"
#include "filter/kis_filter.h"

KisFilterRegistry::KisFilterRegistry()
{
}

KisFilterRegistry::~KisFilterRegistry()
{
    dbgRegistry << "deleting KisFilterRegistry";
}

KisFilterRegistry* KisFilterRegistry::instance()
{
    K_GLOBAL_STATIC(KisFilterRegistry, s_instance);
    if (!s_instance.exists()) {
        KoPluginLoader::instance()->load("Krita/Filter", "Type == 'Service' and ([X-Krita-Version] == 3)");
    }
    return s_instance;
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

#include "kis_filter_registry.moc"
