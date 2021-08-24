/*
 * SPDX-FileCopyrightText: 2020 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KISACTIVEFILTERMODEL_H
#define KISACTIVEFILTERMODEL_H

#include <QSortFilterProxyModel>
#include <KoResource.h>
#include <KisResourceModel.h>

#include "kritaresources_export.h"

/**
 * @brief The KisActiveResourcesModel class
 */
class KRITARESOURCES_EXPORT KisResourceModel : public QSortFilterProxyModel, public KisAbstractResourceModel
{
    Q_OBJECT
public:
    KisResourceModel(int column, QObject *parent);
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
    QModelIndex indexFromResource(KoResourceSP resource) const override;
    bool setResourceInactive(const QModelIndex &index) override;
    KoResourceSP importResourceFile(const QString &filename, const bool allowOverwrite) override;
    bool addResource(KoResourceSP resource, const QString &storageId = QString()) override;
    bool updateResource(KoResourceSP resource) override;
    bool renameResource(KoResourceSP resource, const QString &name) override;
    bool setResourceInactive(KoResourceSP resource) override;
    bool setResourceMetaData(KoResourceSP resource, QMap<QString, QVariant> metadata) override;

protected:

    bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

private:

    struct Private;
    Private *const d;

    Q_DISABLE_COPY(KisResourcesActiveFilterModel)

};

#endif // KISACTIVEFILTERMODEL_H
