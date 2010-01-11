/*
   Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
   Copyright (C) 2007 Thomas Zander <zander@kde.org>
   Copyright (c) 2008 Carlos Licea <carlos.licea@kdemail.net>

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

#include "KoShape.h"
#include "KoLineBorder.h"

class KoCanvasResourceProvider::Private
{
public:
    QHash<int, QVariant> resources;
};

KoCanvasResourceProvider::KoCanvasResourceProvider(QObject *parent)
        : QObject(parent),
        d(new Private())
{
    setGrabSensitivity(3);
}

KoCanvasResourceProvider::~KoCanvasResourceProvider()
{
    delete d;
}

void KoCanvasResourceProvider::setResource(int key, const QVariant &value)
{
    if (d->resources.contains(key)) {
        if (d->resources.value(key) == value)
            return;
        d->resources[key] = value;
    } else {
        d->resources.insert(key, value);
    }
    emit resourceChanged(key, value);
}

QVariant KoCanvasResourceProvider::resource(int key)
{
    if (!d->resources.contains(key)) {
        QVariant empty;
        return empty;
    } else
        return d->resources.value(key);
}

void KoCanvasResourceProvider::setResource(int key, const KoColor &color)
{
    QVariant v;
    v.setValue(color);
    setResource(key, v);
}

void KoCanvasResourceProvider::setResource(int key, const KoID &id)
{
    QVariant v;
    v.setValue(id);
    setResource(key, v);
}

void KoCanvasResourceProvider::setResource(int key, KoShape *shape)
{
    QVariant v;
    v.setValue(shape);
    setResource(key, v);
}

KoColor KoCanvasResourceProvider::koColorResource(int key)
{
    if (! d->resources.contains(key)) {
        KoColor empty;
        return empty;
    }
    return resource(key).value<KoColor>();
}


void KoCanvasResourceProvider::setForegroundColor(const KoColor &color)
{
    setResource(KoCanvasResource::ForegroundColor, color);
}

KoColor KoCanvasResourceProvider::foregroundColor()
{
    return koColorResource(KoCanvasResource::ForegroundColor);
}


void KoCanvasResourceProvider::setBackgroundColor(const KoColor &color)
{
    setResource(KoCanvasResource::BackgroundColor, color);
}

KoColor KoCanvasResourceProvider::backgroundColor()
{
    return koColorResource(KoCanvasResource::BackgroundColor);
}

KoID KoCanvasResourceProvider::koIDResource(int key)
{
    return resource(key).value<KoID>();
}

KoShape * KoCanvasResourceProvider::koShapeResource(int key)
{
    if (! d->resources.contains(key))
        return 0;

    return resource(key).value<KoShape *>();
}

void KoCanvasResourceProvider::setHandleRadius(int handleRadius)
{
    // do not allow arbitrary small handles
    if (handleRadius < 3)
        handleRadius = 3;
    setResource(KoCanvasResource::HandleRadius, QVariant(handleRadius));
}

int KoCanvasResourceProvider::handleRadius()
{
    if (d->resources.contains(KoCanvasResource::HandleRadius))
        return d->resources.value(KoCanvasResource::HandleRadius).toInt();
    return 3; // default value.
}

void KoCanvasResourceProvider::setGrabSensitivity(int grabSensitivity)
{
    // do not allow arbitrary small handles
    if (grabSensitivity < 1)
        grabSensitivity = 1;
    setResource(KoCanvasResource::GrabSensitivity, QVariant(grabSensitivity));
}

int KoCanvasResourceProvider::grabSensitivity()
{
    return resource(KoCanvasResource::GrabSensitivity).toInt();
}

void KoCanvasResourceProvider::setActiveBorder( const KoLineBorder &border )
{
    QVariant v;
    v.setValue(border);
    setResource(KoCanvasResource::ActiveBorder, v);
}

KoLineBorder KoCanvasResourceProvider::activeBorder()
{
    if (! d->resources.contains(KoCanvasResource::ActiveBorder)) {
        KoLineBorder empty;
        return empty;
    }
    return resource(KoCanvasResource::ActiveBorder).value<KoLineBorder>();
}

void KoCanvasResourceProvider::setUnitChanged()
{
    // do not use setResource with a static value
    // because it exits if the value does not change
    // so we just emit that the resource has changed
    // the current unit can then pulled from the canvas
    emit resourceChanged(KoCanvasResource::Unit, QVariant());
}

bool KoCanvasResourceProvider::boolResource(int key) const
{
    if (! d->resources.contains(key))
        return false;
    return d->resources[key].toBool();
}

int KoCanvasResourceProvider::intResource(int key) const
{
    if (! d->resources.contains(key))
        return 0;
    return d->resources[key].toInt();
}

qreal KoCanvasResourceProvider::doubleResource(int key) const
{
    if (! d->resources.contains(key))
        return 0.;
    return d->resources[key].toDouble();
}

QString KoCanvasResourceProvider::stringResource(int key)
{
    if (! d->resources.contains(key)) {
        QString empty;
        return empty;
    }
    return qvariant_cast<QString>(resource(key));
}

QSizeF KoCanvasResourceProvider::sizeResource(int key)
{
    if (! d->resources.contains(key)) {
        QSizeF empty;
        return empty;
    }
    return qvariant_cast<QSizeF>(resource(key));
}

bool KoCanvasResourceProvider::hasResource(int key)
{
    return d->resources.contains(key);
}

void KoCanvasResourceProvider::clearResource(int key)
{
    if (! d->resources.contains(key))
        return;
    d->resources.remove(key);
    QVariant empty;
    emit resourceChanged(key, empty);
}

#include <KoCanvasResourceProvider.moc>
