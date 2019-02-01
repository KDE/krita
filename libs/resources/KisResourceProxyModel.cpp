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
#include "KisResourceProxyModel.h"

#include <KisResourceModel.h>

struct KisResourceProxyModel::Private {
    int rowStride {1};
};

KisResourceProxyModel::KisResourceProxyModel(QObject *parent)
    : QAbstractProxyModel(parent)
    , d(new Private)
{
}

KisResourceProxyModel::~KisResourceProxyModel()
{
}

void KisResourceProxyModel::setRowStride(int rowStride)
{
    d->rowStride = rowStride;
}

QModelIndex KisResourceProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    return sourceModel()->index(row * d->rowStride + column, 0, parent);
}

QModelIndex KisResourceProxyModel::parent(const QModelIndex &child) const
{
    return QModelIndex();
}

int KisResourceProxyModel::rowCount(const QModelIndex &parent) const
{
    return sourceModel()->rowCount(parent) / d->rowStride;
}


int KisResourceProxyModel::columnCount(const QModelIndex &parent) const
{
    return d->rowStride;
}

QModelIndex KisResourceProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
    return sourceModel()->index(proxyIndex.row() * d->rowStride + proxyIndex.column(), 0, QModelIndex());
}

QModelIndex KisResourceProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    return QModelIndex();
}
