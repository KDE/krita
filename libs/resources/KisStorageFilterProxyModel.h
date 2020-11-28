/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KISSTORAGEFILTERPROXYMODEL_H
#define KISSTORAGEFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QObject>
#include <KisResourceStorage.h>
#include <KisStorageModel.h>
#include "kritaresources_export.h"

/**
 * KisStorageFilterProxyModel provides a filtered view on the available storages.
 * It can be used to find the storages that have resource with a particular file
 * name, or storages of particular types.
 * 
 * Filtering by file name takes a string, filtering by storage type a list
 * of untranslated strings (there is a method in KisResourceStorage for retrieving
 * those strings from the ResourceType).
 */
class KRITARESOURCES_EXPORT KisStorageFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    KisStorageFilterProxyModel(QObject *parent = 0);
    ~KisStorageFilterProxyModel() override;

    enum FilterType {
        ByFileName = 0, ///< Pass a string: all storages whose name contains the
                        /// string will be returned.
        ByStorageType,  ///< Pass a string list of storage types
        ByActive        ///< Pass a boolean, false to filter out active bundles,
                        ///  true to filter out inactive bundles
    };

    KisResourceStorageSP storageForIndex(QModelIndex index = QModelIndex()) const;

    void setFilter(FilterType filterType, QVariant filter);

protected:

    bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

private Q_SLOTS:
    void slotModelReset();


private:
    struct Private;
    Private *const d;

    Q_DISABLE_COPY(KisStorageFilterProxyModel)
};

#endif
