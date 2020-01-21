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
#include "KisTagsResourcesModelProvider.h"

#include <QMap>
#include <QString>
#include <memory>

#include <KisTagsResourcesModel.h>

Q_GLOBAL_STATIC(KisTagsResourcesModelProvider, s_instance)


struct KisTagsResourcesModelProvider::Private {

    std::map<QString, std::unique_ptr<KisTagsResourcesModel>> tagsResourcesModelsMap;

};



KisTagsResourcesModelProvider::KisTagsResourcesModelProvider()
    : d(new Private())
{
}

KisTagsResourcesModelProvider::~KisTagsResourcesModelProvider()
{
    delete d;
}

KisTagsResourcesModel* KisTagsResourcesModelProvider::getModel(const QString& resourceType)
{

    std::map<QString, std::unique_ptr<KisTagsResourcesModel> >::const_iterator found
            = s_instance->d->tagsResourcesModelsMap.find(resourceType);

    if (found == s_instance->d->tagsResourcesModelsMap.end())
    {
        KisTagsResourcesModel* model = new KisTagsResourcesModel(resourceType);
        s_instance->d->tagsResourcesModelsMap.insert(std::make_pair(resourceType, model));
        return model;
    }
    return found->second.get();
}

void KisTagsResourcesModelProvider::resetModels()
{
    typedef std::map<QString, std::unique_ptr<KisTagsResourcesModel>>::iterator mapIterator;

    mapIterator begin = s_instance->d->tagsResourcesModelsMap.begin();
    mapIterator end = s_instance->d->tagsResourcesModelsMap.end();

    for (mapIterator iter = begin; iter!=end; iter++) {
        begin->second->resetQuery();
    }
}

void KisTagsResourcesModelProvider::resetModel(const QString& resourceType)
{
    std::map<QString, std::unique_ptr<KisTagsResourcesModel> >::const_iterator found
            = s_instance->d->tagsResourcesModelsMap.find(resourceType);

    if (found != s_instance->d->tagsResourcesModelsMap.end())
    {
        found->second->resetQuery();
    }
}
