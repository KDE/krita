/* This file is part of the KDE project

   Copyright (C) 2006, 2010 Thomas Zander <zander@kde.org>

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

#include "KoShapeControllerBase.h"
#include "KoResourceManager.h"
#include "KoShapeRegistry.h"
#include <KGlobal>
#include <KConfig>
#include <KConfigGroup>

class KoShapeControllerBasePrivate
{
public:
    KoShapeControllerBasePrivate()
        : resourceManager(new KoResourceManager())
    {
        KoShapeRegistry *registry = KoShapeRegistry::instance();
        foreach (const QString &id, registry->keys()) {
            KoShapeFactoryBase *shapeFactory = registry->value(id);
            shapeFactory->newDocumentResourceManager(resourceManager);
        }
        // read persistent application wide resources
        KSharedConfigPtr config = KGlobal::config();
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

    ~KoShapeControllerBasePrivate()
    {
        delete resourceManager;
    }

    KoResourceManager *resourceManager;
};

KoShapeControllerBase::KoShapeControllerBase()
    : d(new KoShapeControllerBasePrivate())
{
}

KoShapeControllerBase::~KoShapeControllerBase()
{
    delete d;
}

KoResourceManager *KoShapeControllerBase::resourceManager() const
{
    return d->resourceManager;
}
