/*
   Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
   Copyright (C) 2007, 2010 Thomas Zander <zander@kde.org>
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
#include "KoResourceManager.h"

#include <QVariant>
#include <KUndoStack>
#include <KDebug>

#include "KoShape.h"
#include "KoLineBorder.h"

class KoResourceManager::Private
{
public:
    QHash<int, QVariant> resources;
};

KoResourceManager::KoResourceManager(QObject *parent)
        : QObject(parent),
        d(new Private())
{
    setGrabSensitivity(3);
}

KoResourceManager::~KoResourceManager()
{
    delete d;
}

void KoResourceManager::setResource(int key, const QVariant &value)
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

QVariant KoResourceManager::resource(int key) const
{
    if (!d->resources.contains(key)) {
        QVariant empty;
        return empty;
    } else
        return d->resources.value(key);
}

void KoResourceManager::setResource(int key, const KoColor &color)
{
    QVariant v;
    v.setValue(color);
    setResource(key, v);
}

void KoResourceManager::setResource(int key, KoShape *shape)
{
    QVariant v;
    v.setValue(shape);
    setResource(key, v);
}

void KoResourceManager::setResource(int key, const KoUnit &unit)
{
    QVariant v;
    v.setValue(unit);
    setResource(key, v);
}

KoColor KoResourceManager::koColorResource(int key) const
{
    if (! d->resources.contains(key)) {
        KoColor empty;
        return empty;
    }
    return resource(key).value<KoColor>();
}

void KoResourceManager::setForegroundColor(const KoColor &color)
{
    setResource(KoCanvasResource::ForegroundColor, color);
}

KoColor KoResourceManager::foregroundColor() const
{
    return koColorResource(KoCanvasResource::ForegroundColor);
}

void KoResourceManager::setBackgroundColor(const KoColor &color)
{
    setResource(KoCanvasResource::BackgroundColor, color);
}

KoColor KoResourceManager::backgroundColor() const
{
    return koColorResource(KoCanvasResource::BackgroundColor);
}

KoShape *KoResourceManager::koShapeResource(int key) const
{
    if (! d->resources.contains(key))
        return 0;

    return resource(key).value<KoShape *>();
}

void KoResourceManager::setHandleRadius(int handleRadius)
{
    // do not allow arbitrary small handles
    if (handleRadius < 3)
        handleRadius = 3;
    setResource(KoCanvasResource::HandleRadius, QVariant(handleRadius));
}

int KoResourceManager::handleRadius() const
{
    if (d->resources.contains(KoCanvasResource::HandleRadius))
        return d->resources.value(KoCanvasResource::HandleRadius).toInt();
    return 3; // default value.
}

KoUnit KoResourceManager::unitResource(int key) const
{
    return resource(key).value<KoUnit>();
}

void KoResourceManager::setGrabSensitivity(int grabSensitivity)
{
    // do not allow arbitrary small handles
    if (grabSensitivity < 1)
        grabSensitivity = 1;
    setResource(KoCanvasResource::GrabSensitivity, QVariant(grabSensitivity));
}

int KoResourceManager::grabSensitivity() const
{
    return resource(KoCanvasResource::GrabSensitivity).toInt();
}

void KoResourceManager::setActiveBorder(const KoLineBorder &border)
{
    QVariant v;
    v.setValue(border);
    setResource(KoCanvasResource::ActiveBorder, v);
}

KoLineBorder KoResourceManager::activeBorder() const
{
    if (! d->resources.contains(KoCanvasResource::ActiveBorder)) {
        KoLineBorder empty;
        return empty;
    }
    return resource(KoCanvasResource::ActiveBorder).value<KoLineBorder>();
}

bool KoResourceManager::boolResource(int key) const
{
    if (! d->resources.contains(key))
        return false;
    return d->resources[key].toBool();
}

int KoResourceManager::intResource(int key) const
{
    if (! d->resources.contains(key))
        return 0;
    return d->resources[key].toInt();
}

QString KoResourceManager::stringResource(int key) const
{
    if (! d->resources.contains(key)) {
        QString empty;
        return empty;
    }
    return qvariant_cast<QString>(resource(key));
}

QSizeF KoResourceManager::sizeResource(int key) const
{
    if (! d->resources.contains(key)) {
        QSizeF empty;
        return empty;
    }
    return qvariant_cast<QSizeF>(resource(key));
}

bool KoResourceManager::hasResource(int key) const
{
    return d->resources.contains(key);
}

void KoResourceManager::clearResource(int key)
{
    if (! d->resources.contains(key))
        return;
    d->resources.remove(key);
    QVariant empty;
    emit resourceChanged(key, empty);
}

KUndoStack *KoResourceManager::undoStack() const
{
    if (!hasResource(KoDocumentResource::UndoStack))
        return 0;
    return static_cast<KUndoStack*>(resource(KoDocumentResource::UndoStack).value<void*>());
}

void KoResourceManager::setUndoStack(KUndoStack *undoStack)
{
    QVariant variant;
    variant.setValue<void*>(undoStack);
    setResource(KoDocumentResource::UndoStack, variant);
}

KoImageCollection *KoResourceManager::imageCollection() const
{
    if (!hasResource(KoDocumentResource::ImageCollection))
        return 0;
    return static_cast<KoImageCollection*>(resource(KoDocumentResource::ImageCollection).value<void*>());
}

void KoResourceManager::setImageCollection(KoImageCollection *ic)
{
    QVariant variant;
    variant.setValue<void*>(ic);
    setResource(KoDocumentResource::ImageCollection, variant);
}

KoOdfDocument *KoResourceManager::odfDocument() const
{
    if (!hasResource(KoDocumentResource::OdfDocument))
        return 0;
    return static_cast<KoOdfDocument*>(resource(KoDocumentResource::OdfDocument).value<void*>());
}

void KoResourceManager::setOdfDocument(KoOdfDocument *currentDocument)
{
    QVariant variant;
    variant.setValue<void*>(currentDocument);
    setResource(KoDocumentResource::OdfDocument, variant);
}

#include <KoResourceManager.moc>
