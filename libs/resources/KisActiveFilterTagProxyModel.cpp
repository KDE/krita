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
#include "KisActiveFilterTagProxyModel.h"

#include <QDebug>
#include <kis_debug.h>
#include <KisTagModel.h>



struct KisActiveFilterTagProxyModel::Private
{
    QList<KisTagSP> tags;
};




KisActiveFilterTagProxyModel::KisActiveFilterTagProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , d(new Private)
{

}

KisActiveFilterTagProxyModel::~KisActiveFilterTagProxyModel()
{
    delete d;
}

bool KisActiveFilterTagProxyModel::filterAcceptsColumn(int /*source_column*/, const QModelIndex &/*source_parent*/) const
{
    return true;
}

bool KisActiveFilterTagProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    return true;
}

bool KisActiveFilterTagProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    KisTagModel* model = qobject_cast<KisTagModel*>(sourceModel());
    if (model != 0) {
        KisTagSP left = model->tagForIndex(source_left);
        KisTagSP right = model->tagForIndex(source_right);
        return KisTag::compareNamesAndUrls(left, right);
    }
    return source_left.row() < source_right.row();
}
