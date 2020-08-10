/*
 * Copyright (c) 2019 Agata Cacko <cacko.azh@gmail.com>
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
#include "KisTagModelProvider.h"

#include <QMap>
#include <QString>
#include <memory>

Q_GLOBAL_STATIC(KisTagModelProvider, s_instance)

struct KisTagModelProvider::Private {

    std::map<QString, std::unique_ptr<KisTagModel>> tagModelsMap;
    std::unique_ptr<KisTagResourceModel> tagResourceModel;

};

KisTagModelProvider::KisTagModelProvider()
    : d(new Private())
{
    d->tagResourceModel.reset(new KisTagResourceModel());
}


KisTagModelProvider::~KisTagModelProvider()
{
    delete d;
}

KisTagModel *KisTagModelProvider::tagModel(const QString& resourceType)
{

    std::map<QString, std::unique_ptr<KisTagModel> >::const_iterator found = s_instance->d->tagModelsMap.find(resourceType);

    if (found == s_instance->d->tagModelsMap.end())
    {
        std::unique_ptr<KisTagModel> modelStorage(new KisTagModel(resourceType));
        KisTagModel *model = modelStorage.get();
        s_instance->d->tagModelsMap.insert(std::make_pair(resourceType, std::move(modelStorage)));
        return model;
    }
    return found->second.get();
}

KisTagResourceModel *KisTagModelProvider::tagResourceModel()
{
    return s_instance->d->tagResourceModel.get();
}
