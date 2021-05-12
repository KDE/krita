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

#include "kis_debug.h"
#include <QMimeDatabase>
#include <KoUnit.h>
#include <KoDocumentResourceManager.h>
#include <KoShapeController.h>
#include <KoShapeGroupCommand.h>


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
