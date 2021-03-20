/*
   SPDX-FileCopyrightText: 2006 Boudewijn Rempt (boud@valdyas.org)
   SPDX-FileCopyrightText: 2007, 2010 Thomas Zander <zander@kde.org>
   SPDX-FileCopyrightText: 2008 Carlos Licea <carlos.licea@kdemail.net>
   SPDX-FileCopyrightText: 2011 Jan Hambrecht <jaham@gmx.net>

   SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "KoCanvasResourceProvider.h"

#include <QVariant>
#include <FlakeDebug.h>

#include "KoShape.h"
#include "KoShapeStroke.h"
#include "KoResourceManager_p.h"
#include <KoColorSpaceRegistry.h>

#include <KoCanvasResourcesInterface.h>

struct Q_DECL_HIDDEN CanvasResourceProviderInterfaceWrapper : public KoCanvasResourcesInterface
{
    CanvasResourceProviderInterfaceWrapper(KoCanvasResourceProvider *provider)
        : m_provider(provider)
    {
    }

    QVariant resource(int key) const override {
        return m_provider->resource(key);
    }

private:
    KoCanvasResourceProvider *m_provider = 0;
};


class Q_DECL_HIDDEN KoCanvasResourceProvider::Private
{
public:
    Private(KoCanvasResourceProvider *q)
        : interfaceWrapper(new CanvasResourceProviderInterfaceWrapper(q))
    {
    }

    KoResourceManager manager;
    QSharedPointer<CanvasResourceProviderInterfaceWrapper> interfaceWrapper;
};

KoCanvasResourceProvider::KoCanvasResourceProvider(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
    setForegroundColor(KoColor(Qt::black, cs));
    setBackgroundColor(KoColor(Qt::white, cs));

    connect(&d->manager, &KoResourceManager::resourceChanged,
            this, &KoCanvasResourceProvider::canvasResourceChanged);
    connect(&d->manager, &KoResourceManager::resourceChangeAttempted,
            this, &KoCanvasResourceProvider::canvasResourceChangeAttempted);
}

KoCanvasResourceProvider::~KoCanvasResourceProvider()
{
    delete d;
}

void KoCanvasResourceProvider::setResource(int key, const QVariant &value)
{
    d->manager.setResource(key, value);
}

QVariant KoCanvasResourceProvider::resource(int key) const
{
    return d->manager.resource(key);
}

void KoCanvasResourceProvider::setResource(int key, const KoColor &color)
{
    QVariant v;
    v.setValue(color);
    setResource(key, v);
}

void KoCanvasResourceProvider::setResource(int key, KoShape *shape)
{
    QVariant v;
    v.setValue(shape);
    setResource(key, v);
}

void KoCanvasResourceProvider::setResource(int key, const KoUnit &unit)
{
    QVariant v;
    v.setValue(unit);
    setResource(key, v);
}

KoColor KoCanvasResourceProvider::koColorResource(int key) const
{
    return d->manager.koColorResource(key);
}

void KoCanvasResourceProvider::setForegroundColor(const KoColor &color)
{
    setResource(KoCanvasResource::ForegroundColor, color);
}

KoColor KoCanvasResourceProvider::foregroundColor() const
{
    return koColorResource(KoCanvasResource::ForegroundColor);
}

void KoCanvasResourceProvider::setBackgroundColor(const KoColor &color)
{
    setResource(KoCanvasResource::BackgroundColor, color);
}

KoColor KoCanvasResourceProvider::backgroundColor() const
{
    return koColorResource(KoCanvasResource::BackgroundColor);
}

KoShape *KoCanvasResourceProvider::koShapeResource(int key) const
{
    return d->manager.koShapeResource(key);
}

KoUnit KoCanvasResourceProvider::unitResource(int key) const
{
    return resource(key).value<KoUnit>();
}

bool KoCanvasResourceProvider::boolResource(int key) const
{
    return d->manager.boolResource(key);
}

int KoCanvasResourceProvider::intResource(int key) const
{
    return d->manager.intResource(key);
}

QString KoCanvasResourceProvider::stringResource(int key) const
{
    return d->manager.stringResource(key);
}

QSizeF KoCanvasResourceProvider::sizeResource(int key) const
{
    return d->manager.sizeResource(key);
}

bool KoCanvasResourceProvider::hasResource(int key) const
{
    return d->manager.hasResource(key);
}

void KoCanvasResourceProvider::clearResource(int key)
{
    d->manager.clearResource(key);
}

void KoCanvasResourceProvider::addDerivedResourceConverter(KoDerivedResourceConverterSP converter)
{
    d->manager.addDerivedResourceConverter(converter);
}

bool KoCanvasResourceProvider::hasDerivedResourceConverter(int key)
{
    return d->manager.hasDerivedResourceConverter(key);
}

void KoCanvasResourceProvider::removeDerivedResourceConverter(int key)
{
    d->manager.removeDerivedResourceConverter(key);
}

void KoCanvasResourceProvider::addResourceUpdateMediator(KoResourceUpdateMediatorSP mediator)
{
    d->manager.addResourceUpdateMediator(mediator);
}

bool KoCanvasResourceProvider::hasResourceUpdateMediator(int key)
{
    return d->manager.hasResourceUpdateMediator(key);
}

void KoCanvasResourceProvider::removeResourceUpdateMediator(int key)
{
    d->manager.removeResourceUpdateMediator(key);
}

void KoCanvasResourceProvider::addActiveCanvasResourceDependency(KoActiveCanvasResourceDependencySP dep)
{
    d->manager.addActiveCanvasResourceDependency(dep);
}

bool KoCanvasResourceProvider::hasActiveCanvasResourceDependency(int sourceKey, int targetKey) const
{
    return d->manager.hasActiveCanvasResourceDependency(sourceKey, targetKey);
}

void KoCanvasResourceProvider::removeActiveCanvasResourceDependency(int sourceKey, int targetKey)
{
    d->manager.removeActiveCanvasResourceDependency(sourceKey, targetKey);
}

KoCanvasResourcesInterfaceSP KoCanvasResourceProvider::canvasResourcesInterface() const
{
    return d->interfaceWrapper;
}
