/*
 *  SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisResourceModelProvider.h"

#include "KisResourceModel.h"
#include "KisTagModel.h"
#include "KisTagResourceModel.h"
#include "KisResourceMetaDataModel.h"

#include "KoResource.h"

#include <memory>
#include <optional>

#include <QGlobalStatic>

Q_GLOBAL_STATIC(KisResourceModelProvider, s_instance)

struct KisResourceModelProvider::Private
{
    QMap<QString, KisAllResourcesModel*> resourceModels;
    QMap<QString, KisAllTagsModel*> tagModels;
    QMap<QString, KisAllTagResourceModel*> tagResourceModels;
    std::optional<KisResourceMetaDataModel> metaDataModel;
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

void KisResourceModelProvider::testingResetAllModels()
{
    for (auto it = s_instance->d->tagModels.begin(); it != s_instance->d->tagModels.end(); ++it) {
        it.value()->resetQuery();
    }
    for (auto it = s_instance->d->resourceModels.begin(); it != s_instance->d->resourceModels.end(); ++it) {
        it.value()->resetQuery();
    }
    for (auto it = s_instance->d->tagResourceModels.begin(); it != s_instance->d->tagResourceModels.end(); ++it) {
        it.value()->resetQuery();
    }

    /// NOTE: we just remove the entire metadata model when we want to reset it,
    /// please refactor it when the metadata model becomes a QObject and will get
    /// any kind of connection to outer world.
    s_instance->d->metaDataModel = std::nullopt;
}

void KisResourceModelProvider::testingCloseAllQueries()
{
    for (auto it = s_instance->d->tagModels.begin(); it != s_instance->d->tagModels.end(); ++it) {
        it.value()->closeQuery();
    }
    for (auto it = s_instance->d->resourceModels.begin(); it != s_instance->d->resourceModels.end(); ++it) {
        it.value()->closeQuery();
    }
    for (auto it = s_instance->d->tagResourceModels.begin(); it != s_instance->d->tagResourceModels.end(); ++it) {
        it.value()->closeQuery();
    }

    /// NOTE: we just remove the entire metadata model when we want to reset it,
    /// please refactor it when the metadata model becomes a QObject and will get
    /// any kind of connection to outer world.
    s_instance->d->metaDataModel = std::nullopt;
}

KisResourceMetaDataModel* KisResourceModelProvider::resourceMetadataModel()
{
    if (!s_instance->d->metaDataModel) {
        s_instance->d->metaDataModel.emplace("resources");
    }
    return &s_instance->d->metaDataModel.value();
}