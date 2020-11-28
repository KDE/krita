/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "KisStorageFilterProxyModel.h"

#include <QDebug>
#include <KisResourceModel.h>
#include <kis_debug.h>
#include <KisResourceSearchBoxFilter.h>
#include <KisResourceLocator.h>

struct KisStorageFilterProxyModel::Private
{
    FilterType filterType {KisStorageFilterProxyModel::ByStorageType};
    QVariant filter;

};

KisStorageFilterProxyModel::KisStorageFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , d(new Private)
{
}

KisStorageFilterProxyModel::~KisStorageFilterProxyModel()
{
    delete d;
}

KisResourceStorageSP KisStorageFilterProxyModel::storageForIndex(QModelIndex index) const
{
    KisStorageModel *source = dynamic_cast<KisStorageModel*>(sourceModel());
    if (source) {
        return source->storageForIndex(mapToSource(index));
    }
    return 0;
}

void KisStorageFilterProxyModel::setFilter(KisStorageFilterProxyModel::FilterType filterType, QVariant filter)
{
    d->filter = filter;
    d->filterType = filterType;
}


bool KisStorageFilterProxyModel::filterAcceptsColumn(int /*source_column*/, const QModelIndex &/*source_parent*/) const
{
    return true;
}

bool KisStorageFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (d->filter.isNull()) return true;

    QModelIndex idx = sourceModel()->index(source_row, KisAbstractResourceModel::Name, source_parent);

    switch (d->filterType) {
    case ByFileName:
    {
        QString filename = d->filter.toString();
        return (sourceModel()->data(idx, Qt::UserRole + KisStorageModel::Location).toString().contains(filename));
    }
    case ByStorageType:
    {
        QString storageType = sourceModel()->data(idx, Qt::UserRole + KisStorageModel::StorageType).toString();
        return (d->filter.toStringList().contains(storageType));
    }
    case ByActive:
    {
        bool active = d->filter.toBool();
        bool isActive = sourceModel()->data(idx, Qt::UserRole + KisStorageModel::Active).toBool();
        return (active == isActive);
    }
    default:
        ;
    }

    return false;
}

bool KisStorageFilterProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    QString nameLeft = sourceModel()->data(source_left, Qt::UserRole + KisAbstractResourceModel::Name).toString();
    QString nameRight = sourceModel()->data(source_right, Qt::UserRole + KisAbstractResourceModel::Name).toString();
    return nameLeft < nameRight;
}

void KisStorageFilterProxyModel::slotModelReset()
{
    invalidateFilter();
}
