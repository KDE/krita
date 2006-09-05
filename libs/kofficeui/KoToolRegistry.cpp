/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoToolRegistry.h"
#include "kdebug.h"
#include <kparts/plugin.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kparts/componentfactory.h>
#include <kstaticdeleter.h>
#include <KoPathToolFactory.h>

KoToolRegistry::KoToolRegistry() {
    const KService::List offers = KServiceTypeTrader::self()->query(
        QString::fromLatin1("KOffice/Tool"),
        QString::fromLatin1("(Type == 'Service') and ([X-Flake-Version] == 1)"));
    kDebug(30008) << "KoToolRegistry searching for plugins, " << offers.count() << " found\n";

    foreach(KService::Ptr service, offers) {
        int errCode = 0;
        KoToolFactory* plugin =
            KService::createInstance<KoToolFactory>(
                service, this, QStringList(), &errCode );
        if ( plugin ) {
            kDebug(30008) <<"found plugin '"<< service->name() << "'\n";
            add(plugin);
        }
        else {
            kWarning(30008) <<"loading plugin '" << service->name() <<
                "' failed, "<< KLibLoader::errorString( errCode ) << " ("<< errCode << ")\n";
        }
    }
    // path tool is always there
    add( new KoPathToolFactory(this, QStringList()) );
}

KoToolRegistry::~KoToolRegistry()
{
}

// static
KoToolRegistry *KoToolRegistry::s_instance = 0;
static KStaticDeleter<KoToolRegistry> staticToolRegistryDeleter;

KoToolRegistry* KoToolRegistry::instance()
{
    if(KoToolRegistry::s_instance == 0)
        staticToolRegistryDeleter.setObject(s_instance, new KoToolRegistry());
    return KoToolRegistry::s_instance;
}

#include "KoToolRegistry.moc"
