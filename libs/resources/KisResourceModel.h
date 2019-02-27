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

/**
 * @brief The KisResourceModel class provides access to the cache database
 * for a particular resource type. Instances should be retrieved using
 * KisResourceModelProvider.
 */
class KRITARESOURCES_EXPORT KisResourceModel : public QAbstractTableModel
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
        LargeThumbnail
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

    // Resources API
    KoResourceSP resourceForIndex(QModelIndex index = QModelIndex()) const;
    QModelIndex indexFromResource(KoResourceSP resource) const;
    bool importResourceFile(const QString &filename);
    bool removeResource(const QModelIndex &index);

Q_SIGNALS:

    void beforeResourcesLayoutReset(QModelIndex);
    void afterResourcesLayoutReset();

private:

    bool prepareQuery();
    QStringList tagsForResource(int resourceId) const;

    struct Private;
    Private *const d;

};

#endif // KISRESOURCEMODEL_H
