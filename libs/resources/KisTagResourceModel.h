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
#ifndef KISTAGRESOURCEMODEL_H
#define KISTAGRESOURCEMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>

#include <KisTag.h>
#include <KoResource.h>

#include "kritaresources_export.h"

class KRITARESOURCES_EXPORT KisAbstractTagResourceModel
{
public:
    virtual ~KisAbstractTagResourceModel() {}

    virtual bool tagResource(const KisTagSP tag, const KoResourceSP resource) = 0;
    virtual bool untagResource(const KisTagSP tag, const KoResourceSP resource) = 0;
};

class KRITARESOURCES_EXPORT KisAllTagResourceModel : public QAbstractTableModel, public KisAbstractTagResourceModel
{
    Q_OBJECT
public:
    KisAllTagResourceModel(QObject *parent = 0);
    ~KisAllTagResourceModel() override;

public:

    enum Columns {
        TagId = 0,
        ResourceId,
        Tag,
        Resource,
        ResourceActive,
        TagActive,
        ResourceStorageActive
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    bool tagResource(const KisTagSP tag, const KoResourceSP resource) override;

    bool untagResource(const KisTagSP tag, const KoResourceSP resource) override;

private:

    bool resetQuery();

    struct Private;
    Private* const d;
};

/**
 * @brief The KisTagResourceModel class makes it possible to retrieve the resources for certain
 * tags or the tags for certain resources. If the filter for tags or resources is empty, all
 * tags or resources that match for the active/inactive/all filters will match.
 */
class KRITARESOURCES_EXPORT KisTagResourceModel : public QSortFilterProxyModel, KisAbstractTagResourceModel
{
    Q_OBJECT
public:
    KisTagResourceModel(QObject *parent = 0);
    ~KisTagResourceModel() override;

public:

    enum TagFilter {
        ShowInactiveTags = 0,
        ShowActiveTags,
        ShowAllTags
    };

    void setTagFilter(TagFilter filter);

    enum ResourceFilter {
        ShowInactiveResources = 0,
        ShowActiveResources,
        ShowAllResources
    };
    void setResourceFilter(ResourceFilter filter);

    enum StorageFilter {
        ShowInactiveStorages = 0,
        ShowActiveStorages,
        ShowAllStorages
    };

    void setStorageFilter(StorageFilter filter);

    bool tagResource(const KisTagSP tag, const KoResourceSP resource) override;
    bool untagResource(const KisTagSP tag, const KoResourceSP resource) override;

    void setTagsFilter(const QVector<int> tagIds);
    void setResourcesFilter(const QVector<int> resourceIds);

    void setTagsFilter(const QVector<KisTagSP> tags);
    void setResourcesFilter(const QVector<KoResourceSP> resources);

protected:

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    struct Private;
    Private* const d;
};

#endif // KISTAGRESOURCEMODEL_H
