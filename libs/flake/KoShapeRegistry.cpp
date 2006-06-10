/*
 * KoShapeRegistry.h -- Part of KOffice
 *
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
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
#include <QString>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kparts/plugin.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kparts/componentfactory.h>

#include <KoGenericRegistry.h>

#include <KoShapeRegistry.h>
#include <KoRectangleShapeFactory.h>
#include <KoPathShapeFactory.h>

KoShapeRegistry *KoShapeRegistry::m_singleton = 0;

KoShapeRegistry::KoShapeRegistry()
{
    KService::List  offers = KServiceTypeTrader::self()->query(QString::fromLatin1("KOffice/Shape"),
            QString::fromLatin1("(Type == 'Service') and "
                "([X-KOffice-Version] == 1)"));

    KService::List::ConstIterator iter;

    for(iter = offers.begin(); iter != offers.end(); ++iter)
    {
        KService::Ptr service = *iter;
        int errCode = 0;

        // Create a plugin: the plugin will register the necessary
        // factory classes with us.
        KParts::Plugin* plugin =
            KParts::ComponentFactory::createInstanceFromService<KParts::Plugin> ( service, this, QStringList(), &errCode);
        if ( plugin )
            kDebug() << "found plugin " << service->property("Name").toString() << "\n";
        else {
            kDebug() << "found plugin " << service->property("Name").toString() << ", " << errCode << "\n";
            if( errCode == KParts::ComponentFactory::ErrNoLibrary)
            {
                kWarning(41006) << " Error loading plugin was : ErrNoLibrary " << KLibLoader::self()->lastErrorMessage() << endl;
            }
        }

    }

    // Also add our hard-coded dumb test shapes
    add( new KoRectangleShapeFactory() );
    add( new KoPathShapeFactory() );
}


KoShapeRegistry::~KoShapeRegistry()
{
}

KoShapeRegistry* KoShapeRegistry::instance()
{
    if(KoShapeRegistry::m_singleton == 0)
    {
        KoShapeRegistry::m_singleton = new KoShapeRegistry();
    }
    return KoShapeRegistry::m_singleton;
}

#include "KoShapeRegistry.moc"
