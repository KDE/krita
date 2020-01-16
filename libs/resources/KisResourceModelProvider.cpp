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
#include "KoResource.h"

#include <memory>

#include <KisTagsResourcesModelProvider.h>

#include <QGlobalStatic>

Q_GLOBAL_STATIC(KisResourceModelProvider, s_instance)

struct KisResourceModelProvider::Private
{
    QMap<QString, KisResourceModel*> resourceModels;
};

KisResourceModelProvider::KisResourceModelProvider()
    : d(new Private())
{
}

KisResourceModelProvider::~KisResourceModelProvider()
{
    qDeleteAll(d->resourceModels);
    delete d;
}

KisResourceModel *KisResourceModelProvider::resourceModel(const QString &resourceType)
{
    if (!s_instance->d->resourceModels.contains(resourceType)) {
       s_instance->d->resourceModels[resourceType] = new KisResourceModel(resourceType);
    }
    return s_instance->d->resourceModels[resourceType];
}

void KisResourceModelProvider::resetAllModels()
{
    Q_FOREACH(KisResourceModel *model, s_instance->d->resourceModels.values()) {
        model->resetQuery();
    }
}

void KisResourceModelProvider::resetModel(const QString& resourceType)
{
    QMap<QString, KisResourceModel*>::iterator found
            = s_instance->d->resourceModels.find(resourceType);

    if (found != s_instance->d->resourceModels.end())
    {
        found.value()->resetQuery();
    }
}
