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

#include "KoShapeBasedDocumentBase.h"
#include "KoDocumentResourceManager.h"
#include "KoShapeRegistry.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

class KoShapeBasedDocumentBasePrivate
{
public:
    KoShapeBasedDocumentBasePrivate()
        : resourceManager(new KoDocumentResourceManager())
    {
        KoShapeRegistry *registry = KoShapeRegistry::instance();
        foreach (const QString &id, registry->keys()) {
            KoShapeFactoryBase *shapeFactory = registry->value(id);
            shapeFactory->newDocumentResourceManager(resourceManager);
        }
        // read persistent application wide resources
        KSharedConfigPtr config =  KSharedConfig::openConfig();
        if (config->hasGroup("Misc")) {
            KConfigGroup miscGroup = config->group("Misc");
            const qreal pasteOffset = miscGroup.readEntry("CopyOffset", 10.0);
            resourceManager->setPasteOffset(pasteOffset);
            const bool pasteAtCursor = miscGroup.readEntry("PasteAtCursor", true);
            resourceManager->enablePasteAtCursor(pasteAtCursor);
            const uint grabSensitivity = miscGroup.readEntry("GrabSensitivity", 3);
            resourceManager->setGrabSensitivity(grabSensitivity);
            const uint handleRadius = miscGroup.readEntry("HandleRadius", 3);
            resourceManager->setHandleRadius(handleRadius);
        }
    }

    ~KoShapeBasedDocumentBasePrivate()
    {
        delete resourceManager;
    }

    KoDocumentResourceManager *resourceManager;
};

KoShapeBasedDocumentBase::KoShapeBasedDocumentBase()
    : d(new KoShapeBasedDocumentBasePrivate())
{
}

KoShapeBasedDocumentBase::~KoShapeBasedDocumentBase()
{
    delete d;
}

void KoShapeBasedDocumentBase::shapesRemoved(const QList<KoShape*> & /*shapes*/, KUndo2Command * /*command*/)
{
}

KoDocumentResourceManager *KoShapeBasedDocumentBase::resourceManager() const
{
    return d->resourceManager;
}
