/*
 * Copyright (C) 2018 Boudewijn Rempt <boud@valdyas.org>
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
#include "KisTagFilterResourceProxyModel.h"

#include <QDebug>
#include <KisResourceModel.h>
#include <kis_debug.h>
#include <KisResourceSearchBoxFilter.h>

struct KisTagFilterResourceProxyModel::Private
{
    Private()
        : filter(new KisResourceSearchBoxFilter())
    {
    }

    QList<KisTagSP> tags;
    KisTagModel *tagModel;
    QScopedPointer<KisResourceSearchBoxFilter> filter;
    bool filterInCurrentTag {false};

};

KisTagFilterResourceProxyModel::KisTagFilterResourceProxyModel(KisTagModel *model, QObject *parent)
    : QSortFilterProxyModel(parent)
    , d(new Private)
{
    d->tagModel = model;
    //connect(model, SIGNAL(modelReset()), this, SLOT(slotModelReset()));
}

KisTagFilterResourceProxyModel::~KisTagFilterResourceProxyModel()
{
    delete d;
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
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return mapFromSource(source->indexForResource(resource));
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

void KisTagFilterResourceProxyModel::setTag(const KisTagSP tag)
{
    d->tags.clear();
    if (!tag.isNull()) {
        d->tags << tag;
    }
    invalidateFilter();
}

void KisTagFilterResourceProxyModel::setSearchBoxText(const QString& seatchBoxText)
{
    d->filter->setFilter(seatchBoxText);
    invalidateFilter();
}

void KisTagFilterResourceProxyModel::setFilterByCurrentTag(const bool filterInCurrentTag)
{
    d->filterInCurrentTag = filterInCurrentTag;
    invalidateFilter();
}

bool KisTagFilterResourceProxyModel::filterAcceptsColumn(int /*source_column*/, const QModelIndex &/*source_parent*/) const
{
    return true;
}

bool KisTagFilterResourceProxyModel::resourceHasCurrentTag(KisTagSP currentTag, QVector<KisTagSP> tagsForResource) const
{

    if (!d->filterInCurrentTag && !d->filter->isEmpty()) {
        // we don't need to check anything else because the user wants to search in all resources
        // but if the filter text is empty, we do need to filter by the current tag
        return true;
    }

    if (currentTag.isNull()) {
        // no tag set; all resources are allowed
        return true;
    } else {
        if (currentTag->id() == KisAllTagsModel::All) {
            // current tag is "All", all resources are allowed
            return true;
        } else if (currentTag->id() == KisAllTagsModel::AllUntagged) {
            // current tag is "All Untagged", all resources without any tags are allowed
            return tagsForResource.size() == 0;
        } else {
             // checking whether the current tag is on the list of tags assigned to the resource
            Q_FOREACH(KisTagSP temp, tagsForResource) {
                if (temp->id() == currentTag->id()) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool KisTagFilterResourceProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (d->tagModel == 0) {
        return true;
    }

    QModelIndex idx = sourceModel()->index(source_row, KisAbstractResourceModel::Name, source_parent);
    int resourceId = sourceModel()->data(idx, Qt::UserRole + KisAbstractResourceModel::Id).toInt();
    QString resourceName = sourceModel()->data(idx, Qt::UserRole + KisAbstractResourceModel::Name).toString();

    QVector<KisTagSP> tagsForResource = d->tagModel->tagsForResource(resourceId);
    KisTagSP tag = d->tags.isEmpty() ? KisTagSP() : d->tags.first();

    bool hasCurrentTag = resourceHasCurrentTag(tag, tagsForResource);
    if (!hasCurrentTag) {
        return false;
    }

    bool currentFilterMatches = d->filter->matchesResource(resourceName);

    return currentFilterMatches;
}

bool KisTagFilterResourceProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    QString nameLeft = sourceModel()->data(source_left, Qt::UserRole + KisAbstractResourceModel::Name).toString();
    QString nameRight = sourceModel()->data(source_right, Qt::UserRole + KisAbstractResourceModel::Name).toString();
    return nameLeft < nameRight;
}

void KisTagFilterResourceProxyModel::slotModelReset()
{
    invalidateFilter();
}
