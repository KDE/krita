/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include "kdebug.h"
#include <kaction.h>
#include <kparts/plugin.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kparts/componentfactory.h>

#include "kis_debug_areas.h"
#include "kis_tool_registry.h"

KisToolRegistry *KisToolRegistry::m_singleton = 0;

KisToolRegistry::KisToolRegistry()
{
    // Load all modules: color models, paintops, filters
    KService::List offers = KServiceTypeTrader::self()->query(QString::fromLatin1("Krita/Tool"),
                                                         QString::fromLatin1("(Type == 'Service') and "
                                                                             "([X-Krita-Version] == 3)"));

    KService::List::ConstIterator iter;

    for(iter = offers.begin(); iter != offers.end(); ++iter)
    {
        KService::Ptr service = *iter;
        int errCode = 0;
        KParts::Plugin* plugin =
             KService::createInstance<KParts::Plugin> ( service, this, QStringList(), &errCode);
        if ( plugin )
            kDebug(DBG_AREA_PLUGINS) << "found plugin " << service->property("Name").toString() << "\n";
        else {
            kDebug(41006) << "found plugin " << service->property("Name").toString() << ", " << errCode << "\n";
            if( errCode == KLibLoader::ErrNoLibrary)
            {
                kWarning(41006) << " Error loading plugin was : ErrNoLibrary " << KLibLoader::self()->lastErrorMessage() << endl;
            }
        }

    }

}

KisToolRegistry::~KisToolRegistry()
{
}

KisToolRegistry* KisToolRegistry::instance()
{
     if(KisToolRegistry::m_singleton == 0)
     {
         KisToolRegistry::m_singleton = new KisToolRegistry();
     }
    return KisToolRegistry::m_singleton;
}


#include "kis_tool_registry.moc"
