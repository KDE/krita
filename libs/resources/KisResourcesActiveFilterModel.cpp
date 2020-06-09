/*
 * Copyright (C) 2020 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "KisResourcesActiveFilterModel.h"

struct KisResourcesActiveFilterModel::Private
{
    int column {-1};
    ResourceFilter resourceFilter {ShowActiveResources};
    StorageFilter storageFilter {ShowActiveStorages};
};

KisResourcesActiveFilterModel::KisResourcesActiveFilterModel(int column, QObject *parent)
    : QSortFilterProxyModel(parent)
    , d(new Private)
{
    d->column = column;
}

KisResourcesActiveFilterModel::~KisResourcesActiveFilterModel()
{
    delete d;
}

void KisResourcesActiveFilterModel::setResourceFilter(ResourceFilter filter)
{
    d->resourceFilter = filter;
}

void KisResourcesActiveFilterModel::setStorageFilter(StorageFilter filter)
{
    d->storageFilter = filter;
}

KoResourceSP KisResourcesActiveFilterModel::resourceForIndex(QModelIndex index) const
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->resourceForIndex(mapToSource(index));
    }
    return 0;
}

QModelIndex KisResourcesActiveFilterModel::indexFromResource(KoResourceSP resource) const
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return mapFromSource(source->indexFromResource(resource));
    }
    return QModelIndex();
}

bool KisResourcesActiveFilterModel::removeResource(const QModelIndex &index)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->removeResource(mapToSource(index));
    }
    return false;
}

bool KisResourcesActiveFilterModel::importResourceFile(const QString &filename)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->importResourceFile(filename);
    }
    return false;
}

bool KisResourcesActiveFilterModel::addResource(KoResourceSP resource, const QString &storageId)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->addResource(resource, storageId);
    }
    return false;
}

bool KisResourcesActiveFilterModel::updateResource(KoResourceSP resource)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->updateResource(resource);
    }
    return false;
}

bool KisResourcesActiveFilterModel::renameResource(KoResourceSP resource, const QString &name)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->renameResource(resource, name);
    }
    return false;
}

bool KisResourcesActiveFilterModel::removeResource(KoResourceSP resource)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->removeResource(resource);
    }
    return false;
}

bool KisResourcesActiveFilterModel::setResourceMetaData(KoResourceSP resource, QMap<QString, QVariant> metadata)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->setResourceMetaData(resource, metadata);
    }
    return false;
}


bool KisResourcesActiveFilterModel::filterAcceptsColumn(int /*source_column*/, const QModelIndex &/*source_parent*/) const
{
    return true;
}

bool KisResourcesActiveFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
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

bool KisResourcesActiveFilterModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    QString nameLeft = sourceModel()->data(source_left, Qt::UserRole + KisResourceModel::Name).toString();
    QString nameRight = sourceModel()->data(source_right, Qt::UserRole + KisResourceModel::Name).toString();
    return nameLeft < nameRight;
}


