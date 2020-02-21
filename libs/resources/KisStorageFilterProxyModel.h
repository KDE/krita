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
        ByFileName = 0,
        ByStorageType
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
