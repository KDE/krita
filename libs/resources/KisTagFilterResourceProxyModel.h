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

#ifndef KISTAGFILTERRESOURCEPROXYMODEL_H
#define KISTAGFILTERRESOURCEPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QObject>

#include "KoResource.h"
#include "KisResourceModel.h"
#include "KisTag.h"
#include "KisTagModel.h"
#include "KisTagResourceModel.h"

#include "kritaresources_export.h"

/**
 * @brief The KisTagFilterResourceProxyModel class filters the resources by tag or resource name
 */
class KRITARESOURCES_EXPORT KisTagFilterResourceProxyModel
        : public QSortFilterProxyModel
        , public KisAbstractResourceModel
{
    Q_OBJECT
public:

    KisTagFilterResourceProxyModel(const QString &resourceType, QObject *parent = 0);
    ~KisTagFilterResourceProxyModel() override;

    // To be used if we need an extra proxy model, like for
    void setResourceModel(KisResourceModel *resourceModel);

// KisAbstractResourceModel interface

    KoResourceSP resourceForIndex(QModelIndex index = QModelIndex()) const override;
    QModelIndex indexForResource(KoResourceSP resource) const override;
    bool setResourceInactive(const QModelIndex &index) override;
    bool importResourceFile(const QString &filename) override;
    bool addResource(KoResourceSP resource, const QString &storageId = QString()) override;
    bool updateResource(KoResourceSP resource) override;
    bool renameResource(KoResourceSP resource, const QString &name) override;
    bool setResourceMetaData(KoResourceSP resource, QMap<QString, QVariant> metadata) override;

    /**
     * @brief setTag sets the tag to filter with
     * @param tag a valid tag with a valid id, or 0 to clear the filter
     */
    void setTag(const KisTagSP tag);
    /**
     * @brief setResource sets the resource to filter with
     * @param resource a valid resource with a valid id, or 0 to clear the filter
     */
    void setResource(const KoResourceSP resource);
    void setSearchText(const QString& seatchText);
    void setFilterInCurrentTag(bool filterInCurrentTag);

protected:

    bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

private:
    struct Private;
    Private *const d;

    Q_DISABLE_COPY(KisTagFilterResourceProxyModel)
};

#endif // KISTAGFILTERRESOURCEPROXYMODEL_H
