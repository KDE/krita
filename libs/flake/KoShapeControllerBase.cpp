/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2006, 2010 Thomas Zander <zander@kde.org>
   SPDX-FileCopyrightText: 2011 Jan Hambrecht <jaham@gmx.net>

   SPDX-License-Identifier: LGPL-2.0-or-later
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
#include <kundo2command.h>

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

KoShapeContainer* KoShapeControllerBase::createParentForShapes(const QList<KoShape*> shapes, KUndo2Command *parentCommand)
{
    Q_UNUSED(parentCommand);
    Q_UNUSED(shapes);

    return 0;
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
