/*
 * Copyright (C) 2019 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
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

#include <QDebug>
#include <qmath.h>

#include <KisResourceModel.h>

/*
 * Largely based on
 * https://github.com/thirtythreeforty/UtilityProxies/blob/master/KisResourceProxyModel.cpp
 * by George Hilliard.
 */

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

QModelIndex KisResourceProxyModel::index(int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return createIndex(row, column);
}

QModelIndex KisResourceProxyModel::parent(const QModelIndex& child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}


int KisResourceProxyModel::rowCount(const QModelIndex& parent) const
{
    if (!sourceModel())  return 0;

    int row;
    int col;
    proxyRow(sourceModel()->rowCount(parent), row, col);
    return row;
}

int KisResourceProxyModel::columnCount(const QModelIndex& /*parent*/) const
{
    return d->rowStride;
}

QVariant KisResourceProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (!sourceModel()) return QVariant();
    return sourceModel()->headerData(section, orientation, role);
}

void KisResourceProxyModel::setSourceModel(QAbstractItemModel* newSourceModel)
{
    QAbstractItemModel* oldSourceModel = sourceModel();

    if (oldSourceModel == newSourceModel) {
        return;
    }

    beginResetModel();

    if (oldSourceModel) {
        disconnect(oldSourceModel, &QAbstractItemModel::columnsAboutToBeInserted,
                   this,           &KisResourceProxyModel::handle_columnsAboutToBeInserted);
        disconnect(oldSourceModel, &QAbstractItemModel::columnsInserted,
                   this,           &KisResourceProxyModel::handle_columnsInserted);
        disconnect(oldSourceModel, &QAbstractItemModel::columnsAboutToBeMoved,
                   this,           &KisResourceProxyModel::handle_columnsAboutToBeMoved);
        disconnect(oldSourceModel, &QAbstractItemModel::columnsMoved,
                   this,           &KisResourceProxyModel::handle_columnsMoved);
        disconnect(oldSourceModel, &QAbstractItemModel::columnsAboutToBeRemoved,
                   this,           &KisResourceProxyModel::handle_columnsAboutToBeRemoved);
        disconnect(oldSourceModel, &QAbstractItemModel::columnsRemoved,
                   this,           &KisResourceProxyModel::handle_columnsRemoved);

        disconnect(oldSourceModel, &QAbstractItemModel::rowsAboutToBeInserted,
                   this,           &KisResourceProxyModel::handle_rowsAboutToBeInserted);
        disconnect(oldSourceModel, &QAbstractItemModel::rowsInserted,
                   this,           &KisResourceProxyModel::handle_rowsInserted);
        disconnect(oldSourceModel, &QAbstractItemModel::rowsAboutToBeMoved,
                   this,           &KisResourceProxyModel::handle_rowsAboutToBeMoved);
        disconnect(oldSourceModel, &QAbstractItemModel::rowsMoved,
                   this,           &KisResourceProxyModel::handle_rowsMoved);
        disconnect(oldSourceModel, &QAbstractItemModel::rowsAboutToBeRemoved,
                   this,           &KisResourceProxyModel::handle_rowsAboutToBeRemoved);
        disconnect(oldSourceModel, &QAbstractItemModel::rowsRemoved,
                   this,           &KisResourceProxyModel::handle_rowsRemoved);

        disconnect(oldSourceModel, &QAbstractItemModel::dataChanged,
                   this,           &KisResourceProxyModel::handle_dataChanged);
        disconnect(oldSourceModel, &QAbstractItemModel::headerDataChanged,
                   this,           &KisResourceProxyModel::handle_headerDataChanged);
        disconnect(oldSourceModel, &QAbstractItemModel::layoutAboutToBeChanged,
                   this,           &KisResourceProxyModel::handle_layoutAboutToBeChanged);
        disconnect(oldSourceModel, &QAbstractItemModel::layoutChanged,
                   this,           &KisResourceProxyModel::handle_layoutChanged);

        disconnect(oldSourceModel, &QAbstractItemModel::modelAboutToBeReset,
                   this,           &KisResourceProxyModel::handle_modelAboutToBeReset);
        disconnect(oldSourceModel, &QAbstractItemModel::modelReset,
                   this,           &KisResourceProxyModel::handle_modelReset);
    }

    QAbstractProxyModel::setSourceModel(newSourceModel);

    if (newSourceModel) {
        connect(newSourceModel, &QAbstractItemModel::columnsAboutToBeInserted,
                this,           &KisResourceProxyModel::handle_columnsAboutToBeInserted);
        connect(newSourceModel, &QAbstractItemModel::columnsInserted,
                this,           &KisResourceProxyModel::handle_columnsInserted);
        connect(newSourceModel, &QAbstractItemModel::columnsAboutToBeMoved,
                this,           &KisResourceProxyModel::handle_columnsAboutToBeMoved);
        connect(newSourceModel, &QAbstractItemModel::columnsMoved,
                this,           &KisResourceProxyModel::handle_columnsMoved);
        connect(newSourceModel, &QAbstractItemModel::columnsAboutToBeRemoved,
                this,           &KisResourceProxyModel::handle_columnsAboutToBeRemoved);
        connect(newSourceModel, &QAbstractItemModel::columnsRemoved,
                this,           &KisResourceProxyModel::handle_columnsRemoved);

        connect(newSourceModel, &QAbstractItemModel::rowsAboutToBeInserted,
                this,           &KisResourceProxyModel::handle_rowsAboutToBeInserted);
        connect(newSourceModel, &QAbstractItemModel::rowsInserted,
                this,           &KisResourceProxyModel::handle_rowsInserted);
        connect(newSourceModel, &QAbstractItemModel::rowsAboutToBeMoved,
                this,           &KisResourceProxyModel::handle_rowsAboutToBeMoved);
        connect(newSourceModel, &QAbstractItemModel::rowsMoved,
                this,           &KisResourceProxyModel::handle_rowsMoved);
        connect(newSourceModel, &QAbstractItemModel::rowsAboutToBeRemoved,
                this,           &KisResourceProxyModel::handle_rowsAboutToBeRemoved);
        connect(newSourceModel, &QAbstractItemModel::rowsRemoved,
                this,           &KisResourceProxyModel::handle_rowsRemoved);

        connect(newSourceModel, &QAbstractItemModel::dataChanged,
                this,           &KisResourceProxyModel::handle_dataChanged);
        connect(newSourceModel, &QAbstractItemModel::headerDataChanged,
                this,           &KisResourceProxyModel::handle_headerDataChanged);
        connect(newSourceModel, &QAbstractItemModel::layoutAboutToBeChanged,
                this,           &KisResourceProxyModel::handle_layoutAboutToBeChanged);
        connect(newSourceModel, &QAbstractItemModel::layoutChanged,
                this,           &KisResourceProxyModel::handle_layoutChanged);

        connect(newSourceModel, &QAbstractItemModel::modelAboutToBeReset,
                this,           &KisResourceProxyModel::handle_modelAboutToBeReset);
        connect(newSourceModel, &QAbstractItemModel::modelReset,
                this,           &KisResourceProxyModel::handle_modelReset);
    }

    endResetModel();
}

QModelIndex KisResourceProxyModel::mapToSource(const QModelIndex& proxyIndex) const
{
    if (!sourceModel()) return QModelIndex();
    return sourceModel()->index(sourceRow(proxyIndex.row(), proxyIndex.column()), KisResourceModel::Thumbnail);
}

QModelIndex KisResourceProxyModel::mapFromSource(const QModelIndex& sourceIndex) const
{
    if (!sourceModel()) return QModelIndex();
    int row;
    int col;
    proxyRow(sourceIndex.row(), row, col);
    return index(row, col, QModelIndex());
}

void KisResourceProxyModel::handle_columnsAboutToBeInserted(QModelIndex parent, int first, int last)
{
    beginInsertRows(mapFromSource(parent), first, last);
}

void KisResourceProxyModel::handle_columnsInserted(QModelIndex parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(first);
    Q_UNUSED(last);
    endInsertRows();
}

void KisResourceProxyModel::handle_columnsAboutToBeMoved(QModelIndex sourceParent, int sourceStart, int sourceEnd, QModelIndex destinationParent, int destinationColumn)
{
    beginMoveRows(mapFromSource(sourceParent), sourceStart, sourceEnd, mapFromSource(destinationParent), destinationColumn);
}

void KisResourceProxyModel::handle_columnsMoved(QModelIndex sourceParent, int sourceStart, int sourceEnd, QModelIndex destinationParent, int destinationColumn)
{
    Q_UNUSED(sourceParent);
    Q_UNUSED(sourceStart);
    Q_UNUSED(sourceEnd);
    Q_UNUSED(destinationParent);
    Q_UNUSED(destinationColumn);
    endMoveRows();
}

void KisResourceProxyModel::handle_columnsAboutToBeRemoved(QModelIndex parent, int first, int last)
{
    beginRemoveRows(mapFromSource(parent), first, last);
}

void KisResourceProxyModel::handle_columnsRemoved(QModelIndex parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(first);
    Q_UNUSED(last);
    endRemoveRows();
}

void KisResourceProxyModel::handle_rowsAboutToBeInserted(QModelIndex parent, int first, int last)
{
    beginInsertColumns(mapFromSource(parent), first, last);
}

void KisResourceProxyModel::handle_rowsInserted(QModelIndex parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(first);
    Q_UNUSED(last);
    endInsertColumns();
}

void KisResourceProxyModel::handle_rowsAboutToBeMoved(QModelIndex sourceParent, int sourceStart, int sourceEnd, QModelIndex destinationParent, int destinationColumn)
{
    beginMoveColumns(mapFromSource(sourceParent), sourceStart, sourceEnd, destinationParent, destinationColumn);
}

void KisResourceProxyModel::handle_rowsMoved(QModelIndex sourceParent, int sourceStart, int sourceEnd, QModelIndex destinationParent, int destinationColumn)
{
    Q_UNUSED(sourceParent);
    Q_UNUSED(sourceStart);
    Q_UNUSED(sourceEnd);
    Q_UNUSED(destinationParent);
    Q_UNUSED(destinationColumn);
    endMoveColumns();
}

void KisResourceProxyModel::handle_rowsAboutToBeRemoved(QModelIndex parent, int first, int last)
{
    beginRemoveColumns(mapFromSource(parent), first, last);
}

void KisResourceProxyModel::handle_rowsRemoved(QModelIndex parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(first);
    Q_UNUSED(last);
    endRemoveColumns();
}

void KisResourceProxyModel::handle_dataChanged(QModelIndex topLeft, QModelIndex bottomRight, QVector<int> roles)
{
    emit dataChanged(mapFromSource(topLeft), mapFromSource(bottomRight), roles);
}

void KisResourceProxyModel::handle_headerDataChanged(Qt::Orientation orientation, int first, int last)
{
    emit headerDataChanged(orientation, first, last);
}

void KisResourceProxyModel::handle_layoutAboutToBeChanged(QList<QPersistentModelIndex> parents, QAbstractItemModel::LayoutChangeHint hint)
{
    QList<QPersistentModelIndex> proxyParents;
    proxyParents.reserve(parents.size());
    for(QList<QPersistentModelIndex>::const_iterator it = parents.cbegin(); it != parents.cend(); ++it) {
        proxyParents.append(mapFromSource(*it));
    }
    emit layoutAboutToBeChanged(proxyParents, hint);
}

void KisResourceProxyModel::handle_layoutChanged(QList<QPersistentModelIndex> parents, QAbstractItemModel::LayoutChangeHint hint)
{
    QList<QPersistentModelIndex> proxyParents;
    proxyParents.reserve(parents.size());
    for(QList<QPersistentModelIndex>::const_iterator it = parents.cbegin(); it != parents.cend(); ++it) {
        proxyParents.append(mapFromSource(*it));
    }
    emit layoutChanged(proxyParents, hint);
}

void KisResourceProxyModel::handle_modelAboutToBeReset()
{
    beginResetModel();
}

void KisResourceProxyModel::handle_modelReset()
{
    endResetModel();
}

int KisResourceProxyModel::sourceRow(int rowIndex, int column) const
{
    return rowIndex * d->rowStride + column;
}

void KisResourceProxyModel::proxyRow(int rowIndex, int &proxyRow, int &proxyColumn) const
{
    proxyRow = rowIndex / d->rowStride;
    proxyColumn = rowIndex % d->rowStride;
}
