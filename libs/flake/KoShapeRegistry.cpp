/* This file is part of the KDE project
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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
#include "KoShapeRegistry.h"
#include "KoPathShapeFactory.h"
#include "KoShapeLoadingContext.h"

#include <KoPluginLoader.h>
#include <KoXmlReader.h>
#include <KoXmlNS.h>

#include <QString>
#include <QHash>
#include <QMultiMap>

#include <kdebug.h>
#include <kstaticdeleter.h>

class KoShapeRegistry::Private
{
public:

    // Map namespace,tagname to priority:factory
    QHash<QPair<QString, QString>, QMultiMap<int, KoShapeFactory*> > factoryMap;
};

KoShapeRegistry::KoShapeRegistry()
    : d(new Private())
{
}

KoShapeRegistry::~KoShapeRegistry()
{
    delete d;
}

void KoShapeRegistry::init() {
    KoPluginLoader::PluginsConfig config;
    config.whiteList = "FlakePlugins";
    config.blacklist = "FlakePluginsDisabled";
    config.group = "koffice";
    KoPluginLoader::instance()->load( QString::fromLatin1("KOffice/Flake"),
                                      QString::fromLatin1("[X-Flake-Version] == 1"),
                                      config);
    config.whiteList = "ShapePlugins";
    config.blacklist = "ShapePluginsDisabled";
    KoPluginLoader::instance()->load(QString::fromLatin1("KOffice/Shape"),
                                     QString::fromLatin1("[X-Flake-Version] == 1"),
                                     config);

    // Also add our hard-coded basic shape
    add( new KoPathShapeFactory(this, QStringList()) );

    // Now all shape factories are registered with us, determine their
    // assocated odf tagname & priority and prepare ourselves for
    // loading ODF.

    QList<KoShapeFactory*> factories = values();
    for ( int i = 0; i < factories.size(); ++i ) {
        KoShapeFactory * factory = factories[i];
        if ( factory->odfNameSpace().isEmpty() || factory->odfElementNames().isEmpty() )
        {
            kDebug(30006) << "Shape factory " << factory->id() << " does not have OdfNamespace defined, ignoring" << endl;
        }
        else {
            foreach( QString elementName, factory->odfElementNames() ) {

                QPair<QString, QString> p ( factory->odfNameSpace(), elementName );

                QMultiMap<int, KoShapeFactory*> priorityMap = d->factoryMap[p];

                d->factoryMap[p].insert( factory->loadingPriority(), factory );

                kDebug(30006) << "Inserting factory " << factory->id() << " for "
                         << p << " with priority "
                         << factory->loadingPriority() << " into factoryMap making "
                         << d->factoryMap[p].size() << " entries. " << endl;
            }
        }
    }
}

KoShapeRegistry *KoShapeRegistry::s_singleton = 0;
static KStaticDeleter<KoShapeRegistry> staticShapeRegistryDeleter;

KoShapeRegistry* KoShapeRegistry::instance()
{
    if(KoShapeRegistry::s_singleton == 0)
    {
        staticShapeRegistryDeleter.setObject(s_singleton, new KoShapeRegistry());
        KoShapeRegistry::s_singleton->init();
    }
    return KoShapeRegistry::s_singleton;
}

KoShape * KoShapeRegistry::createShapeFromOdf(const KoXmlElement & e, KoShapeLoadingContext & context) const
{
    kDebug(30006) << "Going to check for " << e.namespaceURI() << ":" << e.tagName() << endl;

    // If the element is in a frame, the frame is already added by the
    // application and we only want to create a shape from the
    // embedded element. The very first shape we create is accepted.
    // XXX: we might want to have some code to determine which is the
    // "best" of the creatable shapes.
    if ( e.tagName() == "frame" && e.namespaceURI() == KoXmlNS::draw ) {

        KoXmlElement element;
        forEachElement( element, e ) {
            KoShape * shape = createShapeInternal( element, context );
            if ( shape )
                return shape;
        }
    }
    else {
        return createShapeInternal( e, context );
    }

    return 0;
}

KoShape * KoShapeRegistry::createShapeInternal( const KoXmlElement & element, KoShapeLoadingContext & context ) const
{
    QPair<QString, QString> p = QPair<QString, QString>(element.namespaceURI(), element.tagName());
    kDebug(30006) << p << endl;

    if ( !d->factoryMap.contains( p ) ) return 0;

    QMultiMap<int,KoShapeFactory*> priorityMap = d->factoryMap[p];
    QList<KoShapeFactory*> factories = priorityMap.values();

    // Higher numbers are more specific, map is sorted by keys
    for ( int i = factories.size() - 1; i >= 0; --i ) {

        KoShapeFactory * factory = factories[i];
        if ( factory->supports( element ) ) {

            KoShape * shape = factory->createDefaultShape();

            if( shape->shapeId().isEmpty() )
                shape->setShapeId(factory->id());

            if ( shape->loadOdf( element, context ) )
                return shape;

            // Maybe a shape with a lower priority can load our
            // element, but this attempt has failed.
            delete shape;
        }
    }

    return 0;

}

#include "KoShapeRegistry.moc"
