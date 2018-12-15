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

#include "kritaresources_export.h"

class KRITARESOURCES_EXPORT KisTagFilterResourceProxyModel : public QSortFilterProxyModel
{
public:
    KisTagFilterResourceProxyModel(QObject *parent = 0);
    ~KisTagFilterResourceProxyModel() override;

protected:

    /**
     * @brief setTag
     * @param tag
     */
    void setTag(const QString& tag);

    bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
private:
    struct Private;
    Private *const d;

    Q_DISABLE_COPY(KisTagFilterResourceProxyModel)

};

#endif // KISTAGFILTERRESOURCEPROXYMODEL_H
