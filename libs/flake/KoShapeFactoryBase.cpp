/* This file is part of the KDE project
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 C. Boemann <cbo@boemann.dk>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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
#include <QMutexLocker>
#include <QMutex>

#include <kservice.h>
#include <kservicetypetrader.h>

#include <KoDocumentResourceManager.h>
#include "KoShapeFactoryBase.h"
#include "KoDeferredShapeFactoryBase.h"
#include "KoShape.h"
#include "KoShapeLoadingContext.h"
#include <KoOdfLoadingContext.h>
#include <KoProperties.h>

#include <kdebug.h>

class KoShapeFactoryBase::Private
{
public:
    Private(const QString &_id, const QString &_name, const QString &_deferredPluginName)
        : deferredFactory(0),
          deferredPluginName(_deferredPluginName),
          id(_id),
          name(_name),
          loadingPriority(0),
          hidden(false)
    {
    }

    ~Private() {
        foreach(const KoShapeTemplate & t, templates)
            delete t.properties;
        templates.clear();
    }

    KoDeferredShapeFactoryBase *deferredFactory;
    QMutex pluginLoadingMutex;
    QString deferredPluginName;
    QList<KoShapeTemplate> templates;
    QList<KoShapeConfigFactoryBase*> configPanels;
    const QString id;
    const QString name;
    QString family;
    QString tooltip;
    QString iconName;
    int loadingPriority;
    QList<QPair<QString, QStringList> > xmlElements; // xml name space -> xml element names
    bool hidden;
    QList<KoDocumentResourceManager *> resourceManagers;
};


KoShapeFactoryBase::KoShapeFactoryBase(const QString &id, const QString &name, const QString &deferredPluginName)
    : d(new Private(id, name, deferredPluginName))
{
}

KoShapeFactoryBase::~KoShapeFactoryBase()
{
    delete d;
}

QString KoShapeFactoryBase::toolTip() const
{
    return d->tooltip;
}

QString KoShapeFactoryBase::iconName() const
{
    return d->iconName;
}

QString KoShapeFactoryBase::name() const
{
    return d->name;
}

QString KoShapeFactoryBase::family() const
{
    return d->family;
}

int KoShapeFactoryBase::loadingPriority() const
{
    return d->loadingPriority;
}

QList<QPair<QString, QStringList> > KoShapeFactoryBase::odfElements() const
{
    return d->xmlElements;
}

void KoShapeFactoryBase::addTemplate(const KoShapeTemplate &params)
{
    KoShapeTemplate tmplate = params;
    tmplate.id = d->id;
    d->templates.append(tmplate);
}

void KoShapeFactoryBase::setToolTip(const QString & tooltip)
{
    d->tooltip = tooltip;
}

void KoShapeFactoryBase::setIconName(const char *iconName)
{
    d->iconName = QLatin1String(iconName);
}

void KoShapeFactoryBase::setFamily(const QString & family)
{
    d->family = family;
}

QString KoShapeFactoryBase::id() const
{
    return d->id;
}

void KoShapeFactoryBase::setOptionPanels(const QList<KoShapeConfigFactoryBase*> &panelFactories)
{
    d->configPanels = panelFactories;
}

QList<KoShapeConfigFactoryBase*> KoShapeFactoryBase::panelFactories()
{
    return d->configPanels;
}

QList<KoShapeTemplate> KoShapeFactoryBase::templates() const
{
    return d->templates;
}

void KoShapeFactoryBase::setLoadingPriority(int priority)
{
    d->loadingPriority = priority;
}

void KoShapeFactoryBase::setXmlElementNames(const QString & nameSpace, const QStringList & names)
{
    d->xmlElements.clear();
    d->xmlElements.append(QPair<QString, QStringList>(nameSpace, names));
}

void KoShapeFactoryBase::setXmlElements(const QList<QPair<QString, QStringList> > &elementNamesList)
{
    d->xmlElements = elementNamesList;
}

bool KoShapeFactoryBase::hidden() const
{
    return d->hidden;
}

void KoShapeFactoryBase::setHidden(bool hidden)
{
    d->hidden = hidden;
}

void KoShapeFactoryBase::newDocumentResourceManager(KoDocumentResourceManager *manager) const
{
    d->resourceManagers.append(manager);
    connect(manager, SIGNAL(destroyed(QObject *)), this, SLOT(pruneDocumentResourceManager(QObject*)));
}

QList<KoDocumentResourceManager *> KoShapeFactoryBase::documentResourceManagers() const
{
    return d->resourceManagers;
}

KoShape *KoShapeFactoryBase::createDefaultShape(KoDocumentResourceManager *documentResources) const
{
    if (!d->deferredPluginName.isEmpty()) {
        const_cast<KoShapeFactoryBase*>(this)->getDeferredPlugin();
        Q_ASSERT(d->deferredFactory);
        if (d->deferredFactory) {
            return d->deferredFactory->createDefaultShape(documentResources);
        }
    }
    return 0;
}

KoShape *KoShapeFactoryBase::createShape(const KoProperties* properties,
                                         KoDocumentResourceManager *documentResources) const
{
    if (!d->deferredPluginName.isEmpty()) {
        const_cast<KoShapeFactoryBase*>(this)->getDeferredPlugin();
        Q_ASSERT(d->deferredFactory);
        if (d->deferredFactory) {
            return d->deferredFactory->createShape(properties, documentResources);
        }
    }
    return createDefaultShape(documentResources);
}

KoShape *KoShapeFactoryBase::createShapeFromOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    KoShape *shape = createDefaultShape(context.documentResourceManager());
    if (!shape)
        return 0;

    if (shape->shapeId().isEmpty())
        shape->setShapeId(id());

    context.odfLoadingContext().styleStack().save();
    bool loaded = shape->loadOdf(element, context);
    context.odfLoadingContext().styleStack().restore();

    if (!loaded) {
        delete shape;
        return 0;
    }

    return shape;
}

void KoShapeFactoryBase::getDeferredPlugin()
{
    QMutexLocker(&d->pluginLoadingMutex);
    if (d->deferredFactory) return;

    const QString serviceType = "Calligra/Deferred";
    const KService::List offers = KServiceTypeTrader::self()->query(serviceType, QString());
    Q_ASSERT(offers.size() > 0);

    foreach(KSharedPtr<KService> service, offers) {
        KoDeferredShapeFactoryBase *plugin = service->createInstance<KoDeferredShapeFactoryBase>(this);
        if (plugin && plugin->deferredPluginName() == d->deferredPluginName) {
            d->deferredFactory = plugin;
        }
    }

}

void KoShapeFactoryBase::pruneDocumentResourceManager(QObject *obj)
{
    KoDocumentResourceManager *r = qobject_cast<KoDocumentResourceManager*>(obj);
    d->resourceManagers.removeAll(r);
}
