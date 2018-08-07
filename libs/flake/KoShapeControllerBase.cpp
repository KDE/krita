/* This file is part of the KDE project

   Copyright (C) 2006, 2010 Thomas Zander <zander@kde.org>
   Copyright (C) 2011 Jan Hambrecht <jaham@gmx.net>

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
 * Boston, MA 02110-1301, USA.
*/

#include <QTransform>
#include <QPointer>

#include "KoShapeControllerBase.h"
#include "KoDocumentResourceManager.h"
#include "KoShapeRegistry.h"
#include "KoShapeFactoryBase.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

class KoshapeControllerBasePrivate
{
public:
    KoshapeControllerBasePrivate()
        : resourceManager(new KoDocumentResourceManager())
    {
        KoShapeRegistry *registry = KoShapeRegistry::instance();
        foreach (const QString &id, registry->keys()) {
            KoShapeFactoryBase *shapeFactory = registry->value(id);
            shapeFactory->newDocumentResourceManager(resourceManager);
        }
        // read persistent application wide resources
        KSharedConfigPtr config =  KSharedConfig::openConfig();
        KConfigGroup miscGroup = config->group("Misc");
        const uint grabSensitivity = miscGroup.readEntry("GrabSensitivity", 10);
        resourceManager->setGrabSensitivity(grabSensitivity);
        const uint handleRadius = miscGroup.readEntry("HandleRadius", 5);
        resourceManager->setHandleRadius(handleRadius);
    }

    ~KoshapeControllerBasePrivate()
    {
        delete resourceManager;
    }

    QPointer<KoDocumentResourceManager> resourceManager;
};

KoShapeControllerBase::KoShapeControllerBase()
    : d(new KoshapeControllerBasePrivate())
{
}

KoShapeControllerBase::~KoShapeControllerBase()
{
    delete d;
}

void KoShapeControllerBase::addShape(KoShape *shape)
{
    addShapes({shape});
}

void KoShapeControllerBase::shapesRemoved(const QList<KoShape*> & /*shapes*/, KUndo2Command * /*command*/)
{
}

KoDocumentResourceManager *KoShapeControllerBase::resourceManager() const
{
    return d->resourceManager;
}

QRectF KoShapeControllerBase::documentRect() const
{
    const qreal pxToPt = 72.0 / pixelsPerInch();

    QTransform t = QTransform::fromScale(pxToPt, pxToPt);
    return t.mapRect(documentRectInPixels());
}
