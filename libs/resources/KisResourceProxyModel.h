/*
 * Copyright (C) 2019 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KISRESOURCEPROXYMODEL_H
#define KISRESOURCEPROXYMODEL_H

#include <QAbstractProxyModel>
#include <QObject>
#include <QScopedPointer>

#include "kritaresources_export.h"

class KisResourceModel;

/**
 * @brief The KisResourceProxyModel class can be used in the grid
 * based resource widgets.
 */
class KRITARESOURCES_EXPORT KisResourceProxyModel : public QAbstractProxyModel
{
    Q_OBJECT
public:
    KisResourceProxyModel(QObject *parent = 0);
    ~KisResourceProxyModel() override;
    // QAbstractItemModel interface

    /// Set the number of items in a row
    void setRowStride(int rowStride);

public:
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    // QAbstractProxyModel interface
public:
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;

private:
    struct Private;
    const QScopedPointer<Private> d;

};

#endif // KISRESOURCEPROXYMODEL_H
