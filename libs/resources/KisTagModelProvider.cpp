/*
 * SPDX-FileCopyrightText: 2019 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
}


KisTagModelProvider::~KisTagModelProvider()
{
    delete d;
}

KisTagModel *KisTagModelProvider::tagModel(const QString& resourceType)
{
    std::map<QString, std::unique_ptr<KisTagModel> >::const_iterator found = s_instance->d->tagModelsMap.find(resourceType);

    if (found == s_instance->d->tagModelsMap.end()) {
        std::unique_ptr<KisTagModel> modelStorage(new KisTagModel(resourceType));
        KisTagModel *model = modelStorage.get();
        s_instance->d->tagModelsMap.insert(std::make_pair(resourceType, std::move(modelStorage)));
        return model;
    }
    return found->second.get();
}

KisTagResourceModel *KisTagModelProvider::tagResourceModel(const QString &resourceType)
{
    return s_instance->d->tagResourceModel.get();
}
