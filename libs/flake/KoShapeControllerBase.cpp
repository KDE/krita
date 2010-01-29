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
