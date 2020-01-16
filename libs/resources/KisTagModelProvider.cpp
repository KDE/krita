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

#include <KisTagsResourcesModelProvider.h>

Q_GLOBAL_STATIC(KisTagModelProvider, s_instance)


struct KisTagModelProvider::Private {

    std::map<QString, std::unique_ptr<KisTagModel>> tagModelsMap;

};



KisTagModelProvider::KisTagModelProvider()
    : d(new Private())
{
}


KisTagModelProvider::~KisTagModelProvider()
{
    delete d;
}

KisTagModel* KisTagModelProvider::tagModel(const QString& resourceType)
{

    std::map<QString, std::unique_ptr<KisTagModel> >::const_iterator found = s_instance->d->tagModelsMap.find(resourceType);

    if (found == s_instance->d->tagModelsMap.end())
    {
        KisTagModel* model = new KisTagModel(resourceType);
        s_instance->d->tagModelsMap.insert(std::make_pair(resourceType, model));
        return model;
    }
    return found->second.get();
}


void KisTagModelProvider::resetModels()
{
    typedef std::map<QString, std::unique_ptr<KisTagModel>>::iterator mapIterator;

    mapIterator begin = s_instance->d->tagModelsMap.begin();
    mapIterator end = s_instance->d->tagModelsMap.end();

    for (mapIterator iter = begin; iter!=end; iter++) {
        begin->second->prepareQuery();
    }
}

void KisTagModelProvider::resetModel(const QString& resourceType)
{
    std::map<QString, std::unique_ptr<KisTagModel> >::const_iterator found
            = s_instance->d->tagModelsMap.find(resourceType);

    if (found != s_instance->d->tagModelsMap.end())
    {
        found->second->prepareQuery();
    }
}
