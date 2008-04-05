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

#include <QString>

#include <kaction.h>
#include <kis_debug.h>
#include <klocale.h>
#include <kparts/plugin.h>
#include <kservice.h>
#include <kservicetypetrader.h>

#include <kparts/componentfactory.h>
#include <math.h>

#include "kis_debug.h"
#include "kis_types.h"

#include "kis_paint_device.h"
#include "generator/kis_generator.h"

KisGeneratorRegistry *KisGeneratorRegistry::m_singleton = 0;

KisGeneratorRegistry::KisGeneratorRegistry()
{
    Q_ASSERT(KisGeneratorRegistry::m_singleton == 0);
    KisGeneratorRegistry::m_singleton = this;

    KService::List  offers = KServiceTypeTrader::self()->query(QString::fromLatin1("Krita/Generator"),
                                                         QString::fromLatin1("(Type == 'Service') and "
                                                                             "([X-Krita-Version] == 4)"));

    KService::List::ConstIterator iter;
    dbgPlugins << "generators found: " << offers.count();
    for(iter = offers.begin(); iter != offers.end(); ++iter)
    {
        KService::Ptr service = *iter;
        int errCode = 0;
        KParts::Plugin* plugin =
             KService::createInstance<KParts::Plugin> ( service, this, QStringList(), &errCode);
        if ( !plugin ) {
            dbgPlugins <<"found plugin" << service->property("Name").toString() <<"," << errCode <<"";
            if( errCode == KLibLoader::ErrNoLibrary)
            {
                kWarning(41006) <<" Error loading plugin was : ErrNoLibrary" << KLibLoader::self()->lastErrorMessage();
            }
        }

    }
}

KisGeneratorRegistry::~KisGeneratorRegistry()
{
}

KisGeneratorRegistry* KisGeneratorRegistry::instance()
{
    if(KisGeneratorRegistry::m_singleton == 0)
    {
        KisGeneratorRegistry::m_singleton = new KisGeneratorRegistry();
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
