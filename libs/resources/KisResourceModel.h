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

#include <kritaresources_export.h>

#include <KoResource.h>
#include <KisTag.h>

class KRITARESOURCES_EXPORT KisAbstractResourceModel {

public:

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
    virtual QModelIndex indexFromResource(KoResourceSP resource) const = 0;

    /**
     * @brief removeResource
     * @param index
     * @return
     */
    virtual bool removeResource(const QModelIndex &index) = 0;

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
     * @return true if adding the resoruce succeded.
     */
    virtual bool addResource(KoResourceSP resource, const QString &storageId = QString()) = 0;

    /**
     * @brief updateResource
     * @param resource
     * @return
     */
    virtual bool updateResource(KoResourceSP resource) = 0;

    /**
     * @brief removeResource
     * @param resource
     * @return
     */
    virtual bool removeResource(KoResourceSP resource) = 0;

    /**
     * @brief setResourceMetaData
     * @param metadata
     * @return
     */
    virtual bool setResourceMetaData(KoResourceSP resource, QMap<QString, QVariant> metadata) = 0;
};

/**
 * @brief The KisResourceModel class provides access to the cache database
 * for a particular resource type. Instances should be retrieved using
 * KisResourceModelProvider.
 */
class KRITARESOURCES_EXPORT KisResourceModel : public QAbstractTableModel, public KisAbstractResourceModel
{
    Q_OBJECT
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
        Image,
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
        KoResourceRole
    };

private:
    friend class KisResourceModelProvider;
    friend class TestResourceModel;
    KisResourceModel(const QString &resourceType, QObject *parent = 0);

public:

    ~KisResourceModel() override;

// QAbstractItemModel API

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

// Resources API
    /**
     * @brief resourceForIndex returns a properly versioned and id's resource object
     */
    KoResourceSP resourceForIndex(QModelIndex index = QModelIndex()) const override;
    KoResourceSP resourceForId(int id) const;
    KoResourceSP resourceForFilename(QString name) const;
    KoResourceSP resourceForName(QString name) const;


    QModelIndex indexFromResource(KoResourceSP resource) const override;
    bool removeResource(const QModelIndex &index) override;
    bool removeResource(KoResourceSP resource) override;
    bool importResourceFile(const QString &filename) override;
    bool addResource(KoResourceSP resource, const QString &storageId = QString()) override;
    bool updateResource(KoResourceSP resource) override;
    bool setResourceMetaData(KoResourceSP resource, QMap<QString, QVariant> metadata) override;
    QVector<KisTagSP> tagsForResource(int resourceId) const;


Q_SIGNALS:

    // XXX: emit these signals
    void beforeResourcesLayoutReset(QModelIndex activateAfterReformat);
    void afterResourcesLayoutReset();

private:

    bool resetQuery();

    struct Private;
    Private *const d;

};

#endif // KISRESOURCEMODEL_H
