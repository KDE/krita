/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "KisTagFilterResourceProxyModel.h"

#include <QDebug>

#include <KisResourceModelProvider.h>
#include <KisResourceModel.h>
#include <KisTagResourceModel.h>
#include <KisTagModel.h>

#include <kis_debug.h>
#include <KisResourceSearchBoxFilter.h>

struct KisTagFilterResourceProxyModel::Private
{
    Private()
        : filter(new KisResourceSearchBoxFilter())
    {
    }

    QString resourceType;

    KisResourceModel *resourceModel {0}; // This is the source model if we are _not_ filtering by tag
    KisTagResourceModel *tagResourceModel {0}; // This is the source model if we are filtering by tag

    QScopedPointer<KisResourceSearchBoxFilter> filter;
    bool filterInCurrentTag {false};

    QMap<QString, QVariant> metaDataMapFilter;
    KisTagSP currentTagFilter;
    KoResourceSP currentResourceFilter;

};

KisTagFilterResourceProxyModel::KisTagFilterResourceProxyModel(const QString &resourceType, QObject *parent)
    : QSortFilterProxyModel(parent)
    , d(new Private)
{
    d->resourceType = resourceType;
    d->resourceModel = new KisResourceModel(resourceType);
    d->tagResourceModel = new KisTagResourceModel(resourceType);

    setSourceModel(d->resourceModel);
}

KisTagFilterResourceProxyModel::~KisTagFilterResourceProxyModel()
{
    delete d->resourceModel;
    delete d->tagResourceModel;
    delete d;
}

void KisTagFilterResourceProxyModel::setResourceModel(KisResourceModel *resourceModel)
{
    d->resourceModel = resourceModel;
}

KoResourceSP KisTagFilterResourceProxyModel::resourceForIndex(QModelIndex index) const
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->resourceForIndex(mapToSource(index));
    }
    return 0;
}

QModelIndex KisTagFilterResourceProxyModel::indexForResource(KoResourceSP resource) const
{
    if (!resource || !resource->valid() || resource->resourceId() < 0) return QModelIndex();

    for (int i = 0; i < rowCount(); ++i)  {
        QModelIndex idx = index(i, 0);
        Q_ASSERT(idx.isValid());
        if (idx.data() == resource->resourceId()) {
            return idx;
        }
    }
    return QModelIndex();
}

bool KisTagFilterResourceProxyModel::setResourceInactive(const QModelIndex &index)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->setResourceInactive(mapToSource(index));
    }
    return false;
}

bool KisTagFilterResourceProxyModel::importResourceFile(const QString &filename)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->importResourceFile(filename);
    }
    return false;
}

bool KisTagFilterResourceProxyModel::addResource(KoResourceSP resource, const QString &storageId)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->addResource(resource, storageId);
    }
    return false;
}

bool KisTagFilterResourceProxyModel::updateResource(KoResourceSP resource)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->updateResource(resource);
    }
    return false;
}

bool KisTagFilterResourceProxyModel::reloadResource(KoResourceSP resource)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->reloadResource(resource);
    }
    return false;
}

bool KisTagFilterResourceProxyModel::renameResource(KoResourceSP resource, const QString &name)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->renameResource(resource, name);
    }
    return false;
}

bool KisTagFilterResourceProxyModel::setResourceMetaData(KoResourceSP resource, QMap<QString, QVariant> metadata)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->setResourceMetaData(resource, metadata);
    }
    return false;
}

void KisTagFilterResourceProxyModel::setMetaDataFilter(QMap<QString, QVariant> metaDataMap)
{
    d->metaDataMapFilter = metaDataMap;
    invalidateFilter();
}

void KisTagFilterResourceProxyModel::setTagFilter(const KisTagSP tag)
{
    d->currentTagFilter = tag;
    updateTagFilter();
}

void KisTagFilterResourceProxyModel::updateTagFilter()
{
    const bool ignoreTagFiltering =
        !d->filterInCurrentTag && !d->filter->isEmpty();

    QAbstractItemModel *desiredModel = 0;

    if (d->currentResourceFilter) {
        QVector<KisTagSP> filter;
        if (d->currentTagFilter &&
            !ignoreTagFiltering &&
            d->currentTagFilter->url() != "All" &&
            d->currentTagFilter->url() != "All Untagged") {

            filter << d->currentTagFilter;
        } else {
            // combination with for untagged resources in not implemented
            // in KisTagResourceModel
            KIS_SAFE_ASSERT_RECOVER_NOOP(!d->currentTagFilter ||
                                         d->currentTagFilter->url() != "All Untagged");
        }
        d->tagResourceModel->setTagsFilter(filter);
        d->tagResourceModel->setResourcesFilter({d->currentResourceFilter});
        desiredModel = d->tagResourceModel;
    } else {
        d->tagResourceModel->setResourcesFilter(QVector<KoResourceSP>());

        if (ignoreTagFiltering ||
                !d->currentTagFilter ||
                d->currentTagFilter->url() == "All") {

            d->tagResourceModel->setTagsFilter(QVector<KisTagSP>());
            desiredModel = d->resourceModel;
            d->resourceModel->showOnlyUntaggedResources(false);
        }
        else {
            if (d->currentTagFilter->url() == "All Untagged") {
                desiredModel = d->resourceModel;
                d->resourceModel->showOnlyUntaggedResources(true);
            }
            else {
                desiredModel = d->tagResourceModel;
                d->tagResourceModel->setTagsFilter(QVector<KisTagSP>() << d->currentTagFilter);
            }
        }
    }

    // TODO: when model changes the current selection in the
    //       view disappears. We should try to keep it somehow.
    setSourceModel(desiredModel);

    invalidateFilter();
}

void KisTagFilterResourceProxyModel::setResourceFilter(const KoResourceSP resource)
{
    d->currentResourceFilter = resource;
    updateTagFilter();
}

void KisTagFilterResourceProxyModel::setSearchText(const QString& searchText)
{
    d->filter->setFilter(searchText);
    updateTagFilter();
}

void KisTagFilterResourceProxyModel::setFilterInCurrentTag(bool filterInCurrentTag)
{
    d->filterInCurrentTag = filterInCurrentTag;
    updateTagFilter();
}

bool KisTagFilterResourceProxyModel::tagResource(KisTagSP tag, KoResourceSP resource)
{
    return d->tagResourceModel->tagResource(tag, resource);
}

bool KisTagFilterResourceProxyModel::untagResource(const KisTagSP tag, const KoResourceSP resource)
{
    return d->tagResourceModel->untagResource(tag, resource);
}

bool KisTagFilterResourceProxyModel::filterAcceptsColumn(int /*source_column*/, const QModelIndex &/*source_parent*/) const
{
    return true;
}

bool KisTagFilterResourceProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    // if both filters are empty, just accept everything
    if (d->filter->isEmpty() && d->metaDataMapFilter.isEmpty()) {
        return true;
    }

    // If there's a tag set to filter on, we use the tagResourceModel, so that already filters for the tag
    // Here, we only have to filter by the search string.
    QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);

    if (!idx.isValid()) {
        return false;
    }

    bool metaDataMatches = true;
    QMap<QString, QVariant> resourceMetaData = sourceModel()->data(idx, Qt::UserRole + KisAbstractResourceModel::MetaData).toMap();
    Q_FOREACH(const QString &key, d->metaDataMapFilter.keys()) {
        if (resourceMetaData.contains(key)) {
            metaDataMatches = (resourceMetaData[key] == d->metaDataMapFilter[key]);
            if (!metaDataMatches) {
                return false;
            }
        }
    }

    QString resourceName = sourceModel()->data(idx, Qt::UserRole + KisAbstractResourceModel::Name).toString();
    bool resourceNameMatches = d->filter->matchesResource(resourceName);


    return (resourceNameMatches && metaDataMatches);
}

bool KisTagFilterResourceProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    QString nameLeft = sourceModel()->data(source_left, Qt::UserRole + KisAbstractResourceModel::Name).toString();
    QString nameRight = sourceModel()->data(source_right, Qt::UserRole + KisAbstractResourceModel::Name).toString();
    return nameLeft < nameRight;
}

