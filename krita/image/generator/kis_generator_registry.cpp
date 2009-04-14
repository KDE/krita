/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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

#include "generator/kis_generator_registry.h"

#include <math.h>

#include <QString>

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
#include "generator/kis_generator.h"

KisGeneratorRegistry *KisGeneratorRegistry::m_singleton = 0;

KisGeneratorRegistry::KisGeneratorRegistry()
{
    Q_ASSERT(KisGeneratorRegistry::m_singleton == 0);
    KisGeneratorRegistry::m_singleton = this;
}

KisGeneratorRegistry::~KisGeneratorRegistry()
{
}

KisGeneratorRegistry* KisGeneratorRegistry::instance()
{
    if (KisGeneratorRegistry::m_singleton == 0) {
        KisGeneratorRegistry::m_singleton = new KisGeneratorRegistry();
        Q_CHECK_PTR( KisGeneratorRegistry::m_singleton );
        KoPluginLoader::instance()->load( "Krita/Generator", "Type == 'Service' and ([X-Krita-Version] == 3)" );

    }
    return KisGeneratorRegistry::m_singleton;
}

void KisGeneratorRegistry::add(KisGeneratorSP item)
{
    dbgPlugins << "adding " << item->name();
    add(item->id(), item);
}

void KisGeneratorRegistry::add(const QString &id, KisGeneratorSP item)
{
    dbgPlugins << "adding " << item->name() << " with id " << id;
    KoGenericRegistry<KisGeneratorSP>::add(id, item);
    emit(generatorAdded(id));
}

#include "kis_generator_registry.moc"
