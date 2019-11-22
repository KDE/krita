/*
 * Copyright (C) 2019 Agata Cacko <cacko.azh@gmail.com>
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

#ifndef KIS_ACTIVE_FILTER_TAG_PROXY_MODEL_H
#define KIS_ACTIVE_FILTER_TAG_PROXY_MODEL_H


#include <QSortFilterProxyModel>
#include <QObject>
#include <KoResource.h>
#include <KisTag.h>

#include "kritaresources_export.h"

class KRITARESOURCES_EXPORT KisActiveFilterTagProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    KisActiveFilterTagProxyModel(QObject *parent);
    ~KisActiveFilterTagProxyModel() override;

protected:

    bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
private:
    struct Private;
    Private *const d;

    Q_DISABLE_COPY(KisActiveFilterTagProxyModel)
};

#endif // KIS_ACTIVE_FILTER_TAG_PROXY_MODEL_H
