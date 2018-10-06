/*
   Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
   Copyright (C) 2007, 2010 Thomas Zander <zander@kde.org>
   Copyright (c) 2008 Carlos Licea <carlos.licea@kdemail.net>
   Copyright (c) 2011 Jan Hambrecht <jaham@gmx.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */
#include "KoCanvasResourceProvider.h"

#include <QVariant>
#include <FlakeDebug.h>

#include "KoShape.h"
#include "KoShapeStroke.h"
#include "KoResourceManager_p.h"
#include <KoColorSpaceRegistry.h>

class Q_DECL_HIDDEN KoCanvasResourceProvider::Private
{
public:
    KoResourceManager manager;
};

KoCanvasResourceProvider::KoCanvasResourceProvider(QObject *parent)
    : QObject(parent)
    , d(new Private())
{
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
    setForegroundColor(KoColor(Qt::black, cs));
    setBackgroundColor(KoColor(Qt::white, cs));
    setResource(ApplicationSpeciality, NoSpecial);

    connect(&d->manager, &KoResourceManager::resourceChanged,
            this, &KoCanvasResourceProvider::canvasResourceChanged);
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
    setResource(ForegroundColor, color);
}

KoColor KoCanvasResourceProvider::foregroundColor() const
{
    return koColorResource(ForegroundColor);
}

void KoCanvasResourceProvider::setBackgroundColor(const KoColor &color)
{
    setResource(BackgroundColor, color);
}

KoColor KoCanvasResourceProvider::backgroundColor() const
{
    return koColorResource(BackgroundColor);
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
