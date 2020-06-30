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
        /// A larger thumbnail for displaying in a tooltip. 200x200 or so.
        LargeThumbnail,
        /// A dirty resource is one that has been modified locally but not saved
        Dirty,
        /// MetaData is a map of key, value pairs that is associated with this resource
        MetaData,
        /// XXX: what is this used for, again?
        KoResourceRole,
        /// Whether the current resource is active
        ResourceActive,
        /// Whether the current resource's storage isa ctive
        StorageActive
    };

    virtual ~KisAbstractResourceModel(){}

    /**
     * @brief resourceForIndex
     * @param index
     * @return
     */
    virtual KoResourceSP resourceForIndex(QModelIndex index = QModelIndex()) const = 0;

    /**
     * @brief indexFromResource
     * @param resource
     * @return
     */
    virtual QModelIndex indexForResource(KoResourceSP resource) const = 0;

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
    virtual bool importResourceFile(const QString &filename) = 0;

    /**
     * @brief addResource adds the given resource to the database and storage
     * @param resource the resource itself
     * @param storageId the id of the storage (could be "memory" for temporary
     * resources, the document's storage id for document storages or empty to save
     * to the default resources folder
     * @return true if adding the resoruce succeeded.
     */
    virtual bool addResource(KoResourceSP resource, const QString &storageId = QString()) = 0;

    /**
     * @brief updateResource
     * @param resource
     * @return
     */
    virtual bool updateResource(KoResourceSP resource) = 0;

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
     * @brief setResourceInactive sets the specified resource's status to inactive
     * @param resource
     * @return
     */
    virtual bool setResourceInactive(KoResourceSP resource) = 0;

    virtual bool setResourceActive(KoResourceSP resource) = 0;


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
    friend class KisResourceModel;
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
    /**
     * @brief resourceForIndex returns a properly versioned and id's resource object
     */
    KoResourceSP resourceForIndex(QModelIndex index = QModelIndex()) const override;
    QModelIndex indexForResource(KoResourceSP resource) const override;
    bool setResourceInactive(const QModelIndex &index) override;
    bool setResourceInactive(KoResourceSP resource) override;
    bool setResourceActive(KoResourceSP resource) override;
    bool importResourceFile(const QString &filename) override;
    bool addResource(KoResourceSP resource, const QString &storageId = QString()) override;
    bool updateResource(KoResourceSP resource) override;
    bool renameResource(KoResourceSP resource, const QString &name) override;
    bool setResourceMetaData(KoResourceSP resource, QMap<QString, QVariant> metadata) override;

private Q_SLOTS:

    void addStorage(const QString &location);
    void removeStorage(const QString &location);

private:

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
private:
    friend class TestResourceModel;
    friend class KisResourceModelProvider;
    KisResourceModel(const QString &type, QObject *parent = 0);
public:
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

public:

    KoResourceSP resourceForIndex(QModelIndex index = QModelIndex()) const override;
    QModelIndex indexForResource(KoResourceSP resource) const override;
    bool setResourceInactive(const QModelIndex &index) override;
    bool importResourceFile(const QString &filename) override;
    bool addResource(KoResourceSP resource, const QString &storageId = QString()) override;
    bool updateResource(KoResourceSP resource) override;
    bool renameResource(KoResourceSP resource, const QString &name) override;
    bool setResourceInactive(KoResourceSP resource) override;
    bool setResourceActive(KoResourceSP resource) override;
    bool setResourceMetaData(KoResourceSP resource, QMap<QString, QVariant> metadata) override;

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

protected:

    bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

private:

    struct Private;
    Private *const d;

    Q_DISABLE_COPY(KisResourceModel)

};



#endif // KISRESOURCEMODEL_H
