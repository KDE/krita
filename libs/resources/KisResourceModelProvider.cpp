/*
 *  Copyright (c) 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "KisResourceModelProvider.h"

#include "KisResourceModel.h"
#include "KisTagModel.h"
#include "KisTagResourceModel.h"

#include "KoResource.h"

#include <memory>

#include <QGlobalStatic>

Q_GLOBAL_STATIC(KisResourceModelProvider, s_instance)

struct KisResourceModelProvider::Private
{
    QMap<QString, KisAllResourcesModel*> resourceModels;
    QMap<QString, KisAllTagsModel*> tagModels;
    QMap<QString, KisAllTagResourceModel*> tagResourceModels;
};

KisResourceModelProvider::KisResourceModelProvider()
    : d(new Private())
{
}

KisResourceModelProvider::~KisResourceModelProvider()
{
    qDeleteAll(d->resourceModels);
    qDeleteAll(d->tagModels);
    qDeleteAll(d->tagResourceModels);
    delete d;
}

KisAllResourcesModel *KisResourceModelProvider::resourceModel(const QString &resourceType)
{
    if (!s_instance->d->resourceModels.contains(resourceType)) {
       s_instance->d->resourceModels[resourceType] = new KisAllResourcesModel(resourceType);
    }
    return s_instance->d->resourceModels[resourceType];
}

KisAllTagsModel *KisResourceModelProvider::tagModel(const QString &resourceType)
{
    if (!s_instance->d->tagModels.contains(resourceType)) {
       s_instance->d->tagModels[resourceType] = new KisAllTagsModel(resourceType);
    }
    return s_instance->d->tagModels[resourceType];
}


KisAllTagResourceModel *KisResourceModelProvider::tagResourceModel(const QString &resourceType)
{
    if (!s_instance->d->tagResourceModels.contains(resourceType)) {
       s_instance->d->tagResourceModels[resourceType] = new KisAllTagResourceModel(resourceType);
    }
    return s_instance->d->tagResourceModels[resourceType];
}
