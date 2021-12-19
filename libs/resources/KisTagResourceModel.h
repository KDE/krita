/*
 * SPDX-FileCopyrightText: 2020 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KISTAGRESOURCEMODEL_H
#define KISTAGRESOURCEMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>

#include <KisTag.h>
#include <KoResource.h>
#include <KisResourceModel.h>

#include "kritaresources_export.h"

class KRITARESOURCES_EXPORT KisAbstractTagResourceModel
{
public:
    virtual ~KisAbstractTagResourceModel() {}

    virtual bool tagResource(const KisTagSP tag, const int resourceId) = 0;
    virtual bool untagResource(const KisTagSP tag, const int resourceId) = 0;

    /**
     * @brief isResourceTagged
     * @param tag the tag to check
     * @param resourceId the id of the resource to check
     * @return  -1 if the resource was never tagged before, 0 if the resource
     * was tagged, but then untagged, 1 if the resource is already tagged
     */
    virtual int isResourceTagged(const KisTagSP tag, const int resourceId) = 0;
};

class KRITARESOURCES_EXPORT KisAllTagResourceModel
        : public QAbstractTableModel
        , public KisAbstractTagResourceModel
{
    Q_OBJECT
private:

    friend class KisResourceModelProvider;
    friend class TestTagResourceModel;
    friend class KisTagResourceModel;

    KisAllTagResourceModel(const QString &resourceType, QObject *parent = 0);

public:
    ~KisAllTagResourceModel() override;

public:

    enum Columns {
        TagId = KisAbstractResourceModel::StorageActive + 1,
        ResourceId,
        Tag,
        Resource,
        ResourceActive,
        TagActive,
        ResourceStorageActive,
        ResourceName,
        TagName
    };

    // QAbstractItemModel API

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /// Note: only role is significant, column is not.
    QVariant data(const QModelIndex &index, int role) const override;

    // Abstract Tag API
    bool tagResource(const KisTagSP tag, const int resourceId) override;
    bool untagResource(const KisTagSP tag, const int resourceId) override;
    int isResourceTagged(const KisTagSP tag, const int resourceId) override;

private Q_SLOTS:
    void addStorage(const QString &location);
    void removeStorage(const QString &location);

    void slotResourceActiveStateChanged(const QString &resourceType, int resourceId);

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
class KRITARESOURCES_EXPORT KisTagResourceModel : public QSortFilterProxyModel
    , public KisAbstractTagResourceModel
    , public KisAbstractResourceModel
    , public KisAbstractResourceFilterInterface
{
    Q_OBJECT

public:

    KisTagResourceModel(const QString &resourceType, QObject *parent = 0);
    ~KisTagResourceModel() override;

public:

    enum TagFilter {
        ShowInactiveTags = 0,
        ShowActiveTags,
        ShowAllTags
    };

    void setTagFilter(TagFilter filter);

    void setResourceFilter(ResourceFilter filter) override;
    void setStorageFilter(StorageFilter filter) override;

    void setTagsFilter(const QVector<int> tagIds);
    void setResourcesFilter(const QVector<int> resourceIds);

    void setTagsFilter(const QVector<KisTagSP> tags);
    void setResourcesFilter(const QVector<KoResourceSP> resources);

    // KisAbstractTagResourceModel API

    bool tagResource(const KisTagSP tag, const int resourceId) override;
    bool untagResource(const KisTagSP tag, const int resourceId) override;
    int isResourceTagged(const KisTagSP tag, const int resourceId) override;

    // KisAbstractResourceModel interface

    KoResourceSP resourceForIndex(QModelIndex index) const override;
    QModelIndex indexForResource(KoResourceSP resource) const override;
    QModelIndex indexForResourceId(int resourceId) const override;
    bool setResourceActive(const QModelIndex &index, bool value) override;
    KoResourceSP importResourceFile(const QString &filename, const bool allowOverwrite, const QString &storageId = QString()) override;
    KoResourceSP importResource(const QString &filename, QIODevice *device, const bool allowOverwrite, const QString &storageId = QString()) override;
    bool importWillOverwriteResource(const QString &fileName, const QString &storageLocation) const override;
    bool exportResource(KoResourceSP resource, QIODevice *device) override;
    bool addResource(KoResourceSP resource, const QString &storageId) override;
    bool updateResource(KoResourceSP resource) override;
    bool reloadResource(KoResourceSP resource) override;
    bool renameResource(KoResourceSP resource, const QString &name) override;
    bool setResourceMetaData(KoResourceSP resource, QMap<QString, QVariant> metadata) override;


protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

protected Q_SLOTS:
    void storageChanged(const QString &location);

private:
    struct Private;
    Private* const d;
};

#endif // KISTAGRESOURCEMODEL_H
