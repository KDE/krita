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
    kDebug(30008) << "KoToolRegistry searching for plugins, " << offers.count() << " found\n";

    foreach(KService::Ptr service, offers) {
        int errCode = 0;
        KoToolFactory* plugin =
            KParts::ComponentFactory::createInstanceFromService<KoToolFactory>(
                    service, this, QStringList(), &errCode );
        if ( plugin ) {
            kDebug(30008) <<"found plugin '"<< service->property("Name").toString() << "'\n";
            add(plugin);
        }
        else {
            QString err;
            switch (errCode) {
                case KParts::ComponentFactory::ErrNoServiceFound:
                    err = "No Service Found"; break;
                case KParts::ComponentFactory::ErrServiceProvidesNoLibrary:
                    err = "Service provides no library"; break;
                case KParts::ComponentFactory::ErrNoLibrary:
                    err = KLibLoader::self()->lastErrorMessage(); break;
                case KParts::ComponentFactory::ErrNoFactory:
                    err = "No factory found"; break;
                case KParts::ComponentFactory::ErrNoComponent:
                    err = "No Component"; break;
                default:
                    err = "Unknown error";
            }
            kWarning(30008) <<"loading plugin '" << service->property("Name").toString() <<
                "' failed, "<< err << "("<< errCode << ")\n";
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
