/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KISRESOURCEMODEL_H
#define KISRESOURCEMODEL_H

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>

#include <kritaresources_export.h>

#include <KoResource.h>
#include <KisTag.h>


/**
 * KisAbstractResourceModel defines the interface for accessing resources
 * that is used in KisResourceModel and the various filter/proxy models
 */
class KRITARESOURCES_EXPORT KisAbstractResourceModel {

public:

    /**
     * @brief The Columns enum indexes the columns in the model. To get
     * the thumbnail for a particular resource, create the index with
     * QModelIndex(row, Thumbnail).
     */
    enum Columns {
        Id = 0,
        StorageId,
        Name,
        Filename,
        Tooltip,
        Thumbnail,
        Status,
        Location,
        ResourceType,
        Tags,
        MD5,
        /// A larger thumbnail for displaying in a tooltip. 200x200 or so.
        LargeThumbnail,
        /// A dirty resource is one that has been modified locally but not saved
        Dirty,
        /// MetaData is a map of key, value pairs that is associated with this resource
        MetaData,
        /// Whether the current resource is active
        ResourceActive,
        /// Whether the current resource's storage is active
        StorageActive,
    };

    virtual ~KisAbstractResourceModel(){}

    /**
     * @brief resourceForIndex returns a properly versioned and id'ed resource object
     */
    virtual KoResourceSP resourceForIndex(QModelIndex index = QModelIndex()) const = 0;

    /**
     * @brief indexFromResource
     * @param resource
     * @return
     */
    virtual QModelIndex indexForResource(KoResourceSP resource) const = 0;

    /**
     * @brief indexFromResource
     * @param resourceId resource id for which we want to get an index
     * @return
     */
    virtual QModelIndex indexForResourceId(int resourceId) const = 0;

    /**
     * @brief setResourceInactive deactivates the specified resource
     * @param index
     * @return
     */
    virtual bool setResourceInactive(const QModelIndex &index) = 0;

    /**
     * @brief importResourceFile
     * @param filename
     * @return
     */
    virtual KoResourceSP importResourceFile(const QString &filename, const QString &storageId = QString()) = 0;

    /**
     * @brief addResource adds the given resource to the database and storage
     * @param resource the resource itself
     * @param storageId the id of the storage (could be "memory" for temporary
     * resources, the document's storage id for document storages or empty to save
     * to the default resources folder
     * @return true if adding the resource succeeded.
     */
    virtual bool addResource(KoResourceSP resource, const QString &storageId = QString()) = 0;

    /**
     * @brief updateResource
     * @param resource
     * @return
     */
    virtual bool updateResource(KoResourceSP resource) = 0;

    /**
     * @brief reloadResource
     * @param resource
     * @return
     */
    virtual bool reloadResource(KoResourceSP resource) = 0;

    /**
     * @brief renameResource name the given resource. The resource will have its
     * name field reset, will be saved to the storage and there will be a new
     * version created in the database.
     * @param resource The resource to rename
     * @param name The new name
     * @return true if the operation succeeded.
     */
    virtual bool renameResource(KoResourceSP resource, const QString &name) = 0;

    /**
     * @brief setResourceMetaData
     * @param metadata
     * @return
     */
    virtual bool setResourceMetaData(KoResourceSP resource, QMap<QString, QVariant> metadata) = 0;


};

/**
 * @brief The KisAllresourcesModel class provides access to the cache database
 * for a particular resource type. Instances should be retrieved using
 * KisResourceModelProvider. All resources are part of this model, active and
 * inactive, from all storages, active and inactive.
 */
class KRITARESOURCES_EXPORT KisAllResourcesModel : public QAbstractTableModel, public KisAbstractResourceModel
{
    Q_OBJECT

private:
    friend class KisResourceModelProvider;
    friend class KisResourceModel;
    friend class KisResourceQueryMapper;
    KisAllResourcesModel(const QString &resourceType, QObject *parent = 0);

public:

    ~KisAllResourcesModel() override;

// QAbstractItemModel API

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

// Resources API

    KoResourceSP resourceForIndex(QModelIndex index = QModelIndex()) const override;
    QModelIndex indexForResource(KoResourceSP resource) const override;
    QModelIndex indexForResourceId(int resourceId) const override;
    bool setResourceInactive(const QModelIndex &index) override;
    KoResourceSP importResourceFile(const QString &filename, const QString &storageId = QString()) override;
    bool addResource(KoResourceSP resource, const QString &storageId = QString()) override;
    bool updateResource(KoResourceSP resource) override;
    bool reloadResource(KoResourceSP resource) override;
    bool renameResource(KoResourceSP resource, const QString &name) override;
    bool setResourceMetaData(KoResourceSP resource, QMap<QString, QVariant> metadata) override;

private Q_SLOTS:

    void addStorage(const QString &location);
    void removeStorage(const QString &location);

public:

    KoResourceSP resourceForId(int id) const;

    /**
     * resourceForFilename returns the first resource with the given filename that
     * is active and is in an active store. Note that the filename does not include
     * a path to the storage, and if there are resources with the same filename
     * in several active storages, only one resource is returned.
     *
     * @return a resource if one is found, or 0 if none are found
     */
    KoResourceSP resourceForFilename(QString fileName) const;

    /**
     * resourceForName returns the first resource with the given name that
     * is active and is in an active store. Note that if there are resources
     * with the same name in several active storages, only one resource
     * is returned.
     *
     * @return a resource if one is found, or 0 if none are found
     */
    KoResourceSP resourceForName(QString name) const;
    KoResourceSP resourceForMD5(const QByteArray md5sum) const;
    QVector<KisTagSP> tagsForResource(int resourceId) const;

private:

    bool resetQuery();

    struct Private;
    Private *const d;

};

/**
 * @brief The KisResourceModel class provides the main access to resources. It is possible
 * to filter the resources returned by the active status flag of the resources and the
 * storages
 */
class KRITARESOURCES_EXPORT KisResourceModel : public QSortFilterProxyModel, public KisAbstractResourceModel
{
    Q_OBJECT

public:

    KisResourceModel(const QString &type, QObject *parent = 0);
    ~KisResourceModel() override;

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

    void showOnlyUntaggedResources(bool showOnlyUntagged);

public:

    KoResourceSP resourceForIndex(QModelIndex index = QModelIndex()) const override;
    QModelIndex indexForResource(KoResourceSP resource) const override;
    QModelIndex indexForResourceId(int resourceId) const override;
    bool setResourceInactive(const QModelIndex &index) override;
    KoResourceSP importResourceFile(const QString &filename, const QString &storageId = QString()) override;
    bool addResource(KoResourceSP resource, const QString &storageId = QString()) override;
    bool updateResource(KoResourceSP resource) override;
    bool reloadResource(KoResourceSP resource) override;
    bool renameResource(KoResourceSP resource, const QString &name) override;
    bool setResourceMetaData(KoResourceSP resource, QMap<QString, QVariant> metadata) override;

public:

    KoResourceSP resourceForId(int id) const;

    /**
     * resourceForFilename returns the resources with the given filename that
     * fit the current filters. Note that the filename does not include
     * a path to the storage.
     *
     * @return a resource if one is found, or 0 if none are found
     */
    QVector<KoResourceSP> resourcesForFilename(QString fileName) const;

    /**
     * resourceForName returns the resources with the given name that
     * fit the current filters.
     *
     * @return a resource if one is found, or 0 if none are found
     */
    QVector<KoResourceSP> resourcesForName(QString name) const;
    QVector<KoResourceSP> resourcesForMD5(const QByteArray md5sum) const;
    QVector<KisTagSP> tagsForResource(int resourceId) const;

protected:

    bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

private:
    QVector<KoResourceSP> filterByColumn(const QString filter, KisAllResourcesModel::Columns column) const;
    bool filterResource(const QModelIndex &idx) const;

    struct Private;
    Private *const d;

    Q_DISABLE_COPY(KisResourceModel)

};



#endif // KISRESOURCEMODEL_H
