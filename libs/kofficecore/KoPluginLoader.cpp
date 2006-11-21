/* This file is part of the KDE project
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
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

#include <QString>
#include <QStringList>

#include <kdebug.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kstaticdeleter.h>

#include <KoPluginLoader.h>

KoPluginLoader::KoPluginLoader()
{
}

KoPluginLoader::~KoPluginLoader()
{
}

KoPluginLoader *KoPluginLoader::m_singleton = 0;
static KStaticDeleter<KoPluginLoader> staticShapeRegistryDeleter;

KoPluginLoader* KoPluginLoader::instance()
{
    if(KoPluginLoader::m_singleton == 0)
    {
        staticShapeRegistryDeleter.setObject(m_singleton, new KoPluginLoader());
    }
    return KoPluginLoader::m_singleton;
}

void KoPluginLoader::load(const QString & serviceType, const QString & versionString)
{
    // Don't load the same plugins again
    if (m_loadedServiceTypes.contains(serviceType)) {
        return;
    }
    m_loadedServiceTypes << serviceType;

    const KService::List offers = KServiceTypeTrader::self()->query(serviceType,
                                 QString::fromLatin1("(Type == 'Service') and (%1)").arg(versionString));


    foreach(KService::Ptr service, offers) {
        int errCode = 0;
        QObject * plugin = KService::createInstance<QObject>(service, this, QStringList(), &errCode );
        if ( plugin ) {
            delete plugin;
        }
        else {
            kWarning(30008) <<"loading plugin '" << service->name() << "' failed, "<< KLibLoader::errorString( errCode ) << " ("<< errCode << ")\n";
        }
    }
}
#include "KoPluginLoader.moc"
