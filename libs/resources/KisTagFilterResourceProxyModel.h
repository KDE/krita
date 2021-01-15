/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
    bool reloadResource(KoResourceSP resource) override;
    bool renameResource(KoResourceSP resource, const QString &name) override;
    bool setResourceMetaData(KoResourceSP resource, QMap<QString, QVariant> metadata) override;


    /**
     * @brief setMetaDataFilter provides a set of metadata to filter on, for instance
     * by paintop id category.
     * @param metaDataMap
     */
    void setMetaDataFilter(QMap<QString, QVariant> metaDataMap);

    /**
     * @brief setTagFilter sets the tag to filter with
     * @param tag a valid tag with a valid id, or 0 to clear the filter
     */
    void setTagFilter(const KisTagSP tag);

    /**
     * @brief setResourceFilter sets the resource to filter with
     * @param resource a valid resource with a valid id, or 0 to clear the filter
     */
    void setResourceFilter(const KoResourceSP resource);

    void setSearchText(const QString& seatchText);

    void setFilterInCurrentTag(bool filterInCurrentTag);

    bool tagResource(KisTagSP tag, KoResourceSP resource);
    bool untagResource(const KisTagSP tag, const KoResourceSP resource);

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
