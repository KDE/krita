/*
 * SPDX-FileCopyrightText: 2020 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "KisResourcesActiveFilterModel.h"

struct KisResourceModel::Private
{
    int column {-1};
    ResourceFilter resourceFilter {ShowActiveResources};
    StorageFilter storageFilter {ShowActiveStorages};
};

KisResourceModel::KisResourceModel(int column, QObject *parent)
    : QSortFilterProxyModel(parent)
    , d(new Private)
{
    d->column = column;
}

KisResourceModel::~KisResourceModel()
{
    delete d;
}

void KisResourceModel::setResourceFilter(ResourceFilter filter)
{
    d->resourceFilter = filter;
}

void KisResourceModel::setStorageFilter(StorageFilter filter)
{
    d->storageFilter = filter;
}

KoResourceSP KisResourceModel::resourceForIndex(QModelIndex index) const
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->resourceForIndex(mapToSource(index));
    }
    return 0;
}

QModelIndex KisResourceModel::indexFromResource(KoResourceSP resource) const
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return mapFromSource(source->indexFromResource(resource));
    }
    return QModelIndex();
}

bool KisResourceModel::setResourceInactive(const QModelIndex &index)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->setResourceInactive(mapToSource(index));
    }
    return false;
}

bool KisResourceModel::importResourceFile(const QString &filename)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->importResourceFile(filename);
    }
    return false;
}

bool KisResourceModel::addResource(KoResourceSP resource, const QString &storageId)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->addResource(resource, storageId);
    }
    return false;
}

bool KisResourceModel::updateResource(KoResourceSP resource)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->updateResource(resource);
    }
    return false;
}

bool KisResourceModel::renameResource(KoResourceSP resource, const QString &name)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->renameResource(resource, name);
    }
    return false;
}

bool KisResourceModel::setResourceInactive(KoResourceSP resource)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->setResourceInactive(resource);
    }
    return false;
}

bool KisResourceModel::setResourceMetaData(KoResourceSP resource, QMap<QString, QVariant> metadata)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->setResourceMetaData(resource, metadata);
    }
    return false;
}


bool KisResourceModel::filterAcceptsColumn(int /*source_column*/, const QModelIndex &/*source_parent*/) const
{
    return true;
}

bool KisResourceModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (d->resourceFilter == ShowAllResources && d->storageFilter == ShowAllStorages) {
        return true;
    }

    QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
    ResourceFilter resourceActive = (ResourceFilter)sourceModel()->data(idx, Qt::UserRole + KisResourceModel::ResourceActive).toInt();
    StorageFilter storageActive =  (StorageFilter)sourceModel()->data(idx, Qt::UserRole + KisResourceModel::StorageActive).toInt();

    if (d->resourceFilter == ShowAllResources) {
        return (storageActive == d->storageFilter);
    }

    if (d->storageFilter == ShowAllStorages) {
        return (resourceActive == d->resourceFilter);
    }

    return ((storageActive == d->storageFilter) && (resourceActive == d->resourceFilter));
}

bool KisResourceModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    QString nameLeft = sourceModel()->data(source_left, Qt::UserRole + KisResourceModel::Name).toString();
    QString nameRight = sourceModel()->data(source_right, Qt::UserRole + KisResourceModel::Name).toString();
    return nameLeft < nameRight;
}


