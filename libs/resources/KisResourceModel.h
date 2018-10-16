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
#include <QtSql>

#include <kritaresources_export.h>

/**
 * @brief The KisResourceModel class provides access to the cache database
 * for a particular resource type.
 */
class KRITARESOURCES_EXPORT  KisResourceModel : public QAbstractTableModel
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
        Thumbnail,
        Status
    };

    KisResourceModel(const QString &resourceType, QObject *parent = 0);

// QAbstractItemModel API

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

// Resources API

private:
    QSqlQuery m_query;
    QString m_resourceType;
    int m_columnCount {7};
    int m_cachedRowCount {-1};
};

#endif // KISRESOURCEMODEL_H
