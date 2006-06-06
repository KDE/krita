/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2006 Thomas Zander <zander@kde.org>
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

#include "KoToolRegistry.h"
#include "kdebug.h"
#include <kparts/plugin.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kparts/componentfactory.h>

KoToolRegistry::KoToolRegistry() {
    KService::List offers = KServiceTypeTrader::self()->query(QString::fromLatin1("KOffice/Tool"),
            QString::fromLatin1("(Type == 'Service') and ([X-Flake-Version] == 1)"));

    foreach(KService::Ptr service, offers) {
        int errCode = 0;
        KoToolFactory* plugin =
            KParts::ComponentFactory::createInstanceFromService<KoToolFactory>(
                    service, this, QStringList(), &errCode );
        if ( plugin ) {
            kDebug(30008) << "found plugin " << service->property("Name").toString() << endl;
            add(plugin);
        }
        else {
            kDebug(30008) << "      plugin " << service->property("Name").toString() << ", " <<
                errCode << endl;
            if( errCode == KParts::ComponentFactory::ErrNoLibrary)
                kWarning(30008) << " Error loading plugin was : ErrNoLibrary " <<
                    KLibLoader::self()->lastErrorMessage() << endl;
        }
    }
}

KoToolRegistry::~KoToolRegistry()
{
}

// static
KoToolRegistry *KoToolRegistry::s_instance = 0;
KoToolRegistry* KoToolRegistry::instance()
{
     if(KoToolRegistry::s_instance == 0)
         KoToolRegistry::s_instance = new KoToolRegistry();
    return KoToolRegistry::s_instance;
}

#include "KoToolRegistry.moc"
