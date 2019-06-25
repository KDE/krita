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
#include "KoDocumentResourceManager.h"

#include <QVariant>
#include <kundo2stack.h>
#include <FlakeDebug.h>

#include "KoShape.h"
#include "KoShapeController.h"
#include "KoResourceManager_p.h"

class Q_DECL_HIDDEN KoDocumentResourceManager::Private
{
public:
    KoResourceManager manager;
};

KoDocumentResourceManager::KoDocumentResourceManager(QObject *parent)
        : QObject(parent),
        d(new Private())
{
    connect(&d->manager, &KoResourceManager::resourceChanged,
            this, &KoDocumentResourceManager::resourceChanged);
}

KoDocumentResourceManager::~KoDocumentResourceManager()
{
    delete d;
}

void KoDocumentResourceManager::setResource(int key, const QVariant &value)
{
    d->manager.setResource(key, value);
}

QVariant KoDocumentResourceManager::resource(int key) const
{
    return d->manager.resource(key);
}

void KoDocumentResourceManager::setResource(int key, const KoColor &color)
{
    QVariant v;
    v.setValue(color);
    setResource(key, v);
}

void KoDocumentResourceManager::setResource(int key, KoShape *shape)
{
    QVariant v;
    v.setValue(shape);
    setResource(key, v);
}

void KoDocumentResourceManager::setResource(int key, const KoUnit &unit)
{
    QVariant v;
    v.setValue(unit);
    setResource(key, v);
}

KoColor KoDocumentResourceManager::koColorResource(int key) const
{
    return d->manager.koColorResource(key);
}


bool KoDocumentResourceManager::boolResource(int key) const
{
    return d->manager.boolResource(key);
}

int KoDocumentResourceManager::intResource(int key) const
{
    return d->manager.intResource(key);
}

QString KoDocumentResourceManager::stringResource(int key) const
{
    return d->manager.stringResource(key);
}

QSizeF KoDocumentResourceManager::sizeResource(int key) const
{
    return d->manager.sizeResource(key);
}

bool KoDocumentResourceManager::hasResource(int key) const
{
    return d->manager.hasResource(key);
}

void KoDocumentResourceManager::clearResource(int key)
{
    d->manager.clearResource(key);
}

KUndo2Stack *KoDocumentResourceManager::undoStack() const
{
    if (!hasResource(UndoStack))
        return 0;
    return static_cast<KUndo2Stack*>(resource(UndoStack).value<void*>());
}

void KoDocumentResourceManager::setHandleRadius(int handleRadius)
{
    // do not allow arbitrary small handles
    if (handleRadius < 5)
        handleRadius = 5;
    setResource(HandleRadius, QVariant(handleRadius));
}

int KoDocumentResourceManager::handleRadius() const
{
    if (hasResource(HandleRadius))
        return intResource(HandleRadius);
    return 5; // default value (and is used just about everywhere)
}
void KoDocumentResourceManager::setGrabSensitivity(int grabSensitivity)
{
    // do not allow arbitrary small grab sensitivity
    if (grabSensitivity < 5)
        grabSensitivity = 5;
    setResource(GrabSensitivity, QVariant(grabSensitivity));
}

int KoDocumentResourceManager::grabSensitivity() const
{
    if (hasResource(GrabSensitivity))
        return intResource(GrabSensitivity);
    return 5; // default value (and is used just about everywhere)
}

void KoDocumentResourceManager::setUndoStack(KUndo2Stack *undoStack)
{
    QVariant variant;
    variant.setValue<void*>(undoStack);
    setResource(UndoStack, variant);
}

KoImageCollection *KoDocumentResourceManager::imageCollection() const
{
    if (!hasResource(ImageCollection))
        return 0;
    return static_cast<KoImageCollection*>(resource(ImageCollection).value<void*>());
}

void KoDocumentResourceManager::setImageCollection(KoImageCollection *ic)
{
    QVariant variant;
    variant.setValue<void*>(ic);
    setResource(ImageCollection, variant);
}

KoDocumentBase *KoDocumentResourceManager::odfDocument() const
{
    if (!hasResource(OdfDocument))
        return 0;
    return static_cast<KoDocumentBase*>(resource(OdfDocument).value<void*>());
}

void KoDocumentResourceManager::setOdfDocument(KoDocumentBase *currentDocument)
{
    QVariant variant;
    variant.setValue<void*>(currentDocument);
    setResource(OdfDocument, variant);
}

qreal KoDocumentResourceManager::documentResolution() const
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(hasResource(DocumentResolution), 72.0);
    return resource(DocumentResolution).toReal();
}

QRectF KoDocumentResourceManager::documentRectInPixels() const
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(hasResource(DocumentRectInPixels), QRectF(0,0, 777, 666));
    return resource(DocumentRectInPixels).toRectF();
}

KoShapeController *KoDocumentResourceManager::globalShapeController() const
{
    if (!hasResource(GlobalShapeController))
        return 0;

    return resource(GlobalShapeController).value<KoShapeController *>();
}

void KoDocumentResourceManager::setGlobalShapeController(KoShapeController *shapeController)
{
    QVariant variant;
    variant.setValue<KoShapeController *>(shapeController);
    setResource(GlobalShapeController, variant);
}
