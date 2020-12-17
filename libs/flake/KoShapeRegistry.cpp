/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Boudewijn Rempt (boud@valdyas.org)
 * SPDX-FileCopyrightText: 2006-2007, 2010 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2006, 2008-2010 Thorsten Zachmann <zachmann@kde.org>
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2010 Inge Wallin <inge@lysator.liu.se>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

// Own
#include "KoShapeRegistry.h"

#include "KoSvgTextShape.h"
#include "KoPathShapeFactory.h"
#include "KoShapeLoadingContext.h"
#include "KoShapeSavingContext.h"
#include "KoShapeGroup.h"
#include "KoShapeLayer.h"
#include "SvgShapeFactory.h"

#include <KoPluginLoader.h>
#include <KoXmlReader.h>
#include <KoXmlNS.h>

#include <QString>
#include <QHash>
#include <QMultiMap>
#include <QPainter>
#include <QGlobalStatic>

#include <FlakeDebug.h>

Q_GLOBAL_STATIC(KoShapeRegistry, s_instance)

class Q_DECL_HIDDEN KoShapeRegistry::Private
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
    qDeleteAll(doubleEntries());
    qDeleteAll(values());
    delete d;
}

void KoShapeRegistry::Private::init(KoShapeRegistry *q)
{
    KoPluginLoader::PluginsConfig config;
    config.whiteList = "FlakePlugins";
    config.blacklist = "FlakePluginsDisabled";
    config.group = "krita";
    KoPluginLoader::instance()->load(QString::fromLatin1("Krita/Flake"),
                                     QString::fromLatin1("[X-Flake-PluginVersion] == 28"),
                                     config);
    config.whiteList = "ShapePlugins";
    config.blacklist = "ShapePluginsDisabled";
    KoPluginLoader::instance()->load(QString::fromLatin1("Krita/Shape"),
                                     QString::fromLatin1("[X-Flake-PluginVersion] == 28"),
                                     config);

    // Also add our hard-coded basic shapes
    q->add(new KoSvgTextShapeFactory());
    q->add(new KoPathShapeFactory(QStringList()));
    // As long as there is no shape dealing with embedded svg images
    // we add the svg shape factory here by default
    q->add(new SvgShapeFactory);

    // Now all shape factories are registered with us, determine their
    // associated odf tagname & priority and prepare ourselves for
    // loading ODF.

    QList<KoShapeFactoryBase*> factories = q->values();
    for (int i = 0; i < factories.size(); ++i) {
        insertFactory(factories[i]);
    }
}

KoShapeRegistry* KoShapeRegistry::instance()
{
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
        debugFlake << "Shape factory" << factory->id() << " does not have OdfNamespace defined, ignoring";
    }
    else {
        int priority = factory->loadingPriority();
        for (QList<QPair<QString, QStringList> >::const_iterator it(odfElements.begin()); it != odfElements.end(); ++it) {
            foreach (const QString &elementName, (*it).second) {
                QPair<QString, QString> p((*it).first, elementName);

                QMultiMap<int, KoShapeFactoryBase*> & priorityMap = factoryMap[p];

                priorityMap.insert(priority, factory);

                debugFlake << "Inserting factory" << factory->id() << " for"
                    << p << " with priority "
                    << priority << " into factoryMap making "
                    << priorityMap.size() << " entries. ";
            }
        }
    }
}

#include <svg/SvgShapeFactory.h>
#include "kis_debug.h"
#include <QMimeDatabase>
#include <KoUnit.h>
#include <KoDocumentResourceManager.h>
#include <KoShapeController.h>
#include <KoShapeGroupCommand.h>


KoShape * KoShapeRegistry::createShapeFromXML(const KoXmlElement & e, KoShapeLoadingContext & context) const
{
    Q_UNUSED(e);
    Q_UNUSED(context);
    return 0;
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
    debugFlake << "Supported factories for=" << p;
    foreach (KoShapeFactoryBase *f, factories)
        debugFlake << f->id() << f->name();
#endif

    // Loop through all shape factories. If any of them supports this
    // element, then we let the factory create a shape from it. This
    // may fail because the element itself is too generic to draw any
    // real conclusions from it - we actually have to try to load it.
    // An example of this is the draw:image element which have
    // potentially hundreds of different image formats to support,
    // including vector formats.
    //
    // If it succeeds, then we use this shape, if it fails, then just
    // try the next.
    //
    // Higher numbers are more specific, map is sorted by keys.
    for (int i = factories.size() - 1; i >= 0; --i) {
        KoShapeFactoryBase * factory = factories[i];
        if (factory->supports(element, context)) {
            KoShape *shape = factory->createShapeFromXML(fullElement, context);
            if (shape) {
                debugFlake << "Shape found for factory " << factory->id() << factory->name();
                // we return the top-level most shape as that's the one that we'll have to
                // add to the KoShapeManager for painting later (and also to avoid memory leaks)
                // but don't go past a KoShapeLayer as KoShape adds those from the context
                // during loading and those are already added.
                while (shape->parent() && dynamic_cast<KoShapeLayer*>(shape->parent()) == 0)
                    shape = shape->parent();

                return shape;
            }
            // Maybe a shape with a lower priority can load our
            // element, but this attempt has failed.
        }
        else {
            debugFlake << "No support for" << p << "by" << factory->id();
        }
    }

    return 0;
}

QList<KoShapeFactoryBase*> KoShapeRegistry::factoriesForElement(const QString &nameSpace, const QString &elementName)
{
    // Pair of namespace, tagname
    QPair<QString, QString> p = QPair<QString, QString>(nameSpace, elementName);

    QMultiMap<int, KoShapeFactoryBase*> priorityMap = d->factoryMap.value(p);
    QList<KoShapeFactoryBase*> shapeFactories;
    // sort list by priority
    Q_FOREACH (KoShapeFactoryBase *f, priorityMap.values()) {
        shapeFactories.prepend(f);
    }

    return shapeFactories;
}
