/* This file is part of the KDE project
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
 * Copyright (C) 2006-2007, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006,2008-2010 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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
#include "KoConnectionShapeFactory.h"
#include "KoShapeLoadingContext.h"
#include "KoTextOnShapeContainer.h"
#include "KoShapeSavingContext.h"
#include "KoShapeGroup.h"

#include <KoPluginLoader.h>
#include <KoXmlReader.h>
#include <KoXmlNS.h>
#include <KoOdfLoadingContext.h>
#include <KoStyleStack.h>

#include <QString>
#include <QHash>
#include <QMultiMap>
#include <QPainter>

#include <KDebug>
#include <KGlobal>

class KoShapeRegistry::Private
{
public:
    void insertFactory(KoShapeFactoryBase *factory);
    void init(KoShapeRegistry *q);

    KoShape *createShapeInternal(const KoXmlElement &fullElement, KoShapeLoadingContext &context, const KoXmlElement &element) const;


    // Map namespace,tagname to priority:factory
    QHash<QPair<QString, QString>, QMultiMap<int, KoShapeFactoryBase*> > factoryMap;
};

KoShapeRegistry::KoShapeRegistry()
        : d(new Private())
{
}

KoShapeRegistry::~KoShapeRegistry()
{
    delete d;
}

void KoShapeRegistry::Private::init(KoShapeRegistry *q)
{
    KoPluginLoader::PluginsConfig config;
    config.whiteList = "FlakePlugins";
    config.blacklist = "FlakePluginsDisabled";
    config.group = "koffice";
    KoPluginLoader::instance()->load(QString::fromLatin1("KOffice/Flake"),
                                     QString::fromLatin1("[X-Flake-MinVersion] <= 0"),
                                     config);
    config.whiteList = "ShapePlugins";
    config.blacklist = "ShapePluginsDisabled";
    KoPluginLoader::instance()->load(QString::fromLatin1("KOffice/Shape"),
                                     QString::fromLatin1("[X-Flake-MinVersion] <= 0"),
                                     config);

    // Also add our hard-coded basic shape
    q->add(new KoPathShapeFactory(q, QStringList()));
    q->add(new KoConnectionShapeFactory(q));

    // Now all shape factories are registered with us, determine their
    // assocated odf tagname & priority and prepare ourselves for
    // loading ODF.

    QList<KoShapeFactoryBase*> factories = q->values();
    for (int i = 0; i < factories.size(); ++i) {
        insertFactory(factories[i]);
    }
}

KoShapeRegistry* KoShapeRegistry::instance()
{
    K_GLOBAL_STATIC(KoShapeRegistry, s_instance)
    if (!s_instance.exists()) {
        s_instance->d->init(s_instance);
    }
    return s_instance;
}

void KoShapeRegistry::addFactory(KoShapeFactoryBase * factory)
{
    add(factory);
    d->insertFactory(factory);
}

void KoShapeRegistry::Private::insertFactory(KoShapeFactoryBase *factory)
{
    const QList<QPair<QString, QStringList> > odfElements(factory->odfElements());

    if (odfElements.isEmpty()) {
        kDebug(30006) << "Shape factory" << factory->id() << " does not have OdfNamespace defined, ignoring";
    }
    else {
        int priority = factory->loadingPriority();
        for (QList<QPair<QString, QStringList> >::const_iterator it(odfElements.begin()); it != odfElements.end(); ++it) {
            foreach (const QString &elementName, (*it).second) {
                QPair<QString, QString> p((*it).first, elementName);

                QMultiMap<int, KoShapeFactoryBase*> & priorityMap = factoryMap[p];

                priorityMap.insert(priority, factory);

                kDebug(30006) << "Inserting factory" << factory->id() << " for"
                    << p << " with priority "
                    << priority << " into factoryMap making "
                    << priorityMap.size() << " entries. ";
            }
        }
    }
}

KoShape * KoShapeRegistry::createShapeFromOdf(const KoXmlElement & e, KoShapeLoadingContext & context) const
{
    kDebug(30006) << "Going to check for" << e.namespaceURI() << ":" << e.tagName();

    KoShape * shape = 0;
    // If the element is in a frame, the frame is already added by the
    // application and we only want to create a shape from the
    // embedded element. The very first shape we create is accepted.
    //
    // FIXME: we might want to have some code to determine which is
    //        the "best" of the creatable shapes.
    if (e.tagName() == "frame" && e.namespaceURI() == KoXmlNS::draw) {
        KoXmlElement element;
        forEachElement(element, e) {
            // Check for draw:object
            if (element.tagName() == "object" && element.namespaceURI() == KoXmlNS::draw && element.hasChildNodes()) {
                // Loop through the elements and find the first one
                // that is handled by any shape.
                KoXmlNode n = element.firstChild();
                for (; !n.isNull(); n = n.nextSibling()) {
                    if (n.isElement()) {
                        shape = d->createShapeInternal(e, context, n.toElement());
                        break;
                    }
                }
            }
            else {
                // If not draw:object, e.g draw:image or draw:plugin
                shape = d->createShapeInternal(e, context, element);
            }

            // If we found a shape that can handle the element in question, then break.
            if (shape) {
                break;
            }
        }
    }

    // Hardwire the group shape into the loading as it should not appear
    // in the shape selector
    else if (e.localName() == "g" && e.namespaceURI() == KoXmlNS::draw) {
        KoShapeGroup * group = new KoShapeGroup();

        context.odfLoadingContext().styleStack().save();
        bool loaded = group->loadOdf(e, context);
        context.odfLoadingContext().styleStack().restore();

        if (loaded) {
            shape = group;
        }
        else {
            delete group;
        }
    } else {
        shape = d->createShapeInternal(e, context, e);
    }

    if (shape) {
        context.shapeLoaded(shape);
    }

    return shape;
}

KoShape *KoShapeRegistry::Private::createShapeInternal(const KoXmlElement &fullElement,
                                                       KoShapeLoadingContext &context,
                                                       const KoXmlElement &element) const
{
    // Pair of namespace, tagname
    QPair<QString, QString> p = QPair<QString, QString>(element.namespaceURI(), element.tagName());

    // Remove duplicate lookup.
    if (!factoryMap.contains(p))
        return 0; 

    QMultiMap<int, KoShapeFactoryBase*> priorityMap = factoryMap.value(p);
    QList<KoShapeFactoryBase*> factories = priorityMap.values();

#ifndef NDEBUG
    kDebug(30006) << "Supported factories for=" << p;
    foreach (KoShapeFactoryBase *f, factories)
        kDebug(30006) << f->id() << f->name();
#endif

    // Loop through all shape factories. If any of them supports this
    // element, then we let the factory create a shape from it. This
    // may fail because the element itself is too generic to draw any
    // real conclusions from it - we actually have to try to load it.
    // An example of this is the draw:image element which have
    // potentially hundreds of different image formats to support,
    // including vector formats.
    //
    // If it succeeds, then we use this shape, if it does fail, then
    // just try the next.
    //
    // Higher numbers are more specific, map is sorted by keys.
    for (int i = factories.size() - 1; i >= 0; --i) {
        KoShapeFactoryBase * factory = factories[i];
        if (factory->supports(element)) {
            KoShape *shape = factory->createDefaultShape(context.documentResourceManager());

            if (shape->shapeId().isEmpty())
                shape->setShapeId(factory->id());

            context.odfLoadingContext().styleStack().save();
            bool loaded = shape->loadOdf(fullElement, context);
            context.odfLoadingContext().styleStack().restore();

            if (loaded) {
                KoXmlElement text = KoXml::namedItemNS(fullElement, KoXmlNS::text, "p");
                if (!text.isNull()) {
                    KoTextOnShapeContainer *tos = new KoTextOnShapeContainer(shape,
                            context.documentResourceManager());
                    if (tos->loadOdf(fullElement, context)) {
                        return tos;
                    }
                    delete tos;
                }
                return shape;
            }

            // Maybe a shape with a lower priority can load our
            // element, but this attempt has failed.
            delete shape;
        }
    }

    return 0;
}

#include <KoShapeRegistry.moc>
