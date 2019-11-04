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
#include "KisResourceGridProxyModel.h"

#include <QDebug>
#include <qmath.h>

#include <KisResourceModel.h>

/*
 * Largely based on
 * https://github.com/thirtythreeforty/UtilityProxies
 * by George Hilliard.
 */

struct KisResourceGridProxyModel::Private {
    int rowStride {1};
};

KisResourceGridProxyModel::KisResourceGridProxyModel(QObject *parent)
    : QAbstractProxyModel(parent)
    , d(new Private)
{
}

KisResourceGridProxyModel::~KisResourceGridProxyModel()
{
}

void KisResourceGridProxyModel::setRowStride(int rowStride)
{
    d->rowStride = rowStride;
}

KoResourceSP KisResourceGridProxyModel::resourceForIndex(QModelIndex index) const
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->resourceForIndex(mapToSource(index));
    }
    return 0;
}

QModelIndex KisResourceGridProxyModel::indexFromResource(KoResourceSP resource) const
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return mapFromSource(source->indexFromResource(resource));
    }
    return QModelIndex();
}

bool KisResourceGridProxyModel::removeResource(const QModelIndex &index)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->removeResource(mapToSource(index));
    }
    return false;
}

bool KisResourceGridProxyModel::importResourceFile(const QString &filename)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->importResourceFile(filename);
    }
    return false;
}

bool KisResourceGridProxyModel::addResource(KoResourceSP resource, bool save)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->addResource(resource, save);
    }
    return false;
}

bool KisResourceGridProxyModel::updateResource(KoResourceSP resource)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->updateResource(resource);
    }
    return false;
}

bool KisResourceGridProxyModel::removeResource(KoResourceSP resource)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->removeResource(resource);
    }
    return false;
}

bool KisResourceGridProxyModel::setResourceMetaData(KoResourceSP resource, QMap<QString, QVariant> metadata)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->setResourceMetaData(resource, metadata);
    }
    return false;
}


QModelIndex KisResourceGridProxyModel::index(int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return createIndex(row, column);
}

QModelIndex KisResourceGridProxyModel::parent(const QModelIndex& child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

int KisResourceGridProxyModel::rowCount(const QModelIndex& parent) const
{
    if (!sourceModel())  return 0;

    int row;
    int col;
    proxyRow(sourceModel()->rowCount(parent), row, col);
    if (col>0) {
        row += 1;
    }
    return row;
}

int KisResourceGridProxyModel::columnCount(const QModelIndex& /*parent*/) const
{
    return d->rowStride;
}

QVariant KisResourceGridProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (!sourceModel()) return QVariant();
    return sourceModel()->headerData(section, orientation, role);
}

void KisResourceGridProxyModel::setSourceModel(QAbstractItemModel* newSourceModel)
{
    QAbstractItemModel* oldSourceModel = sourceModel();

    if (oldSourceModel == newSourceModel) {
        return;
    }

    beginResetModel();

    if (oldSourceModel) {
        disconnect(oldSourceModel, &QAbstractItemModel::columnsAboutToBeInserted,
                   this,           &KisResourceGridProxyModel::handle_columnsAboutToBeInserted);
        disconnect(oldSourceModel, &QAbstractItemModel::columnsInserted,
                   this,           &KisResourceGridProxyModel::handle_columnsInserted);
        disconnect(oldSourceModel, &QAbstractItemModel::columnsAboutToBeMoved,
                   this,           &KisResourceGridProxyModel::handle_columnsAboutToBeMoved);
        disconnect(oldSourceModel, &QAbstractItemModel::columnsMoved,
                   this,           &KisResourceGridProxyModel::handle_columnsMoved);
        disconnect(oldSourceModel, &QAbstractItemModel::columnsAboutToBeRemoved,
                   this,           &KisResourceGridProxyModel::handle_columnsAboutToBeRemoved);
        disconnect(oldSourceModel, &QAbstractItemModel::columnsRemoved,
                   this,           &KisResourceGridProxyModel::handle_columnsRemoved);

        disconnect(oldSourceModel, &QAbstractItemModel::rowsAboutToBeInserted,
                   this,           &KisResourceGridProxyModel::handle_rowsAboutToBeInserted);
        disconnect(oldSourceModel, &QAbstractItemModel::rowsInserted,
                   this,           &KisResourceGridProxyModel::handle_rowsInserted);
        disconnect(oldSourceModel, &QAbstractItemModel::rowsAboutToBeMoved,
                   this,           &KisResourceGridProxyModel::handle_rowsAboutToBeMoved);
        disconnect(oldSourceModel, &QAbstractItemModel::rowsMoved,
                   this,           &KisResourceGridProxyModel::handle_rowsMoved);
        disconnect(oldSourceModel, &QAbstractItemModel::rowsAboutToBeRemoved,
                   this,           &KisResourceGridProxyModel::handle_rowsAboutToBeRemoved);
        disconnect(oldSourceModel, &QAbstractItemModel::rowsRemoved,
                   this,           &KisResourceGridProxyModel::handle_rowsRemoved);

        disconnect(oldSourceModel, &QAbstractItemModel::dataChanged,
                   this,           &KisResourceGridProxyModel::handle_dataChanged);
        disconnect(oldSourceModel, &QAbstractItemModel::headerDataChanged,
                   this,           &KisResourceGridProxyModel::handle_headerDataChanged);
        disconnect(oldSourceModel, &QAbstractItemModel::layoutAboutToBeChanged,
                   this,           &KisResourceGridProxyModel::handle_layoutAboutToBeChanged);
        disconnect(oldSourceModel, &QAbstractItemModel::layoutChanged,
                   this,           &KisResourceGridProxyModel::handle_layoutChanged);

        disconnect(oldSourceModel, &QAbstractItemModel::modelAboutToBeReset,
                   this,           &KisResourceGridProxyModel::handle_modelAboutToBeReset);
        disconnect(oldSourceModel, &QAbstractItemModel::modelReset,
                   this,           &KisResourceGridProxyModel::handle_modelReset);
    }

    QAbstractProxyModel::setSourceModel(newSourceModel);

    if (newSourceModel) {
        connect(newSourceModel, &QAbstractItemModel::columnsAboutToBeInserted,
                this,           &KisResourceGridProxyModel::handle_columnsAboutToBeInserted);
        connect(newSourceModel, &QAbstractItemModel::columnsInserted,
                this,           &KisResourceGridProxyModel::handle_columnsInserted);
        connect(newSourceModel, &QAbstractItemModel::columnsAboutToBeMoved,
                this,           &KisResourceGridProxyModel::handle_columnsAboutToBeMoved);
        connect(newSourceModel, &QAbstractItemModel::columnsMoved,
                this,           &KisResourceGridProxyModel::handle_columnsMoved);
        connect(newSourceModel, &QAbstractItemModel::columnsAboutToBeRemoved,
                this,           &KisResourceGridProxyModel::handle_columnsAboutToBeRemoved);
        connect(newSourceModel, &QAbstractItemModel::columnsRemoved,
                this,           &KisResourceGridProxyModel::handle_columnsRemoved);

        connect(newSourceModel, &QAbstractItemModel::rowsAboutToBeInserted,
                this,           &KisResourceGridProxyModel::handle_rowsAboutToBeInserted);
        connect(newSourceModel, &QAbstractItemModel::rowsInserted,
                this,           &KisResourceGridProxyModel::handle_rowsInserted);
        connect(newSourceModel, &QAbstractItemModel::rowsAboutToBeMoved,
                this,           &KisResourceGridProxyModel::handle_rowsAboutToBeMoved);
        connect(newSourceModel, &QAbstractItemModel::rowsMoved,
                this,           &KisResourceGridProxyModel::handle_rowsMoved);
        connect(newSourceModel, &QAbstractItemModel::rowsAboutToBeRemoved,
                this,           &KisResourceGridProxyModel::handle_rowsAboutToBeRemoved);
        connect(newSourceModel, &QAbstractItemModel::rowsRemoved,
                this,           &KisResourceGridProxyModel::handle_rowsRemoved);

        connect(newSourceModel, &QAbstractItemModel::dataChanged,
                this,           &KisResourceGridProxyModel::handle_dataChanged);
        connect(newSourceModel, &QAbstractItemModel::headerDataChanged,
                this,           &KisResourceGridProxyModel::handle_headerDataChanged);
        connect(newSourceModel, &QAbstractItemModel::layoutAboutToBeChanged,
                this,           &KisResourceGridProxyModel::handle_layoutAboutToBeChanged);
        connect(newSourceModel, &QAbstractItemModel::layoutChanged,
                this,           &KisResourceGridProxyModel::handle_layoutChanged);

        connect(newSourceModel, &QAbstractItemModel::modelAboutToBeReset,
                this,           &KisResourceGridProxyModel::handle_modelAboutToBeReset);
        connect(newSourceModel, &QAbstractItemModel::modelReset,
                this,           &KisResourceGridProxyModel::handle_modelReset);
    }

    endResetModel();
}

QModelIndex KisResourceGridProxyModel::mapToSource(const QModelIndex& proxyIndex) const
{
    if (!sourceModel()) return QModelIndex();
    return sourceModel()->index(sourceRow(proxyIndex.row(), proxyIndex.column()), KisResourceModel::Image);
}

QModelIndex KisResourceGridProxyModel::mapFromSource(const QModelIndex& sourceIndex) const
{
    if (!sourceModel()) return QModelIndex();
    int row;
    int col;
    proxyRow(sourceIndex.row(), row, col);
    return index(row, col, QModelIndex());
}

void KisResourceGridProxyModel::handle_columnsAboutToBeInserted(QModelIndex parent, int first, int last)
{
    beginInsertRows(mapFromSource(parent), first, last);
}

void KisResourceGridProxyModel::handle_columnsInserted(QModelIndex parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(first);
    Q_UNUSED(last);
    endInsertRows();
}

void KisResourceGridProxyModel::handle_columnsAboutToBeMoved(QModelIndex sourceParent, int sourceStart, int sourceEnd, QModelIndex destinationParent, int destinationColumn)
{
    beginMoveRows(mapFromSource(sourceParent), sourceStart, sourceEnd, mapFromSource(destinationParent), destinationColumn);
}

void KisResourceGridProxyModel::handle_columnsMoved(QModelIndex sourceParent, int sourceStart, int sourceEnd, QModelIndex destinationParent, int destinationColumn)
{
    Q_UNUSED(sourceParent);
    Q_UNUSED(sourceStart);
    Q_UNUSED(sourceEnd);
    Q_UNUSED(destinationParent);
    Q_UNUSED(destinationColumn);
    endMoveRows();
}

void KisResourceGridProxyModel::handle_columnsAboutToBeRemoved(QModelIndex parent, int first, int last)
{
    beginRemoveRows(mapFromSource(parent), first, last);
}

void KisResourceGridProxyModel::handle_columnsRemoved(QModelIndex parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(first);
    Q_UNUSED(last);
    endRemoveRows();
}

void KisResourceGridProxyModel::handle_rowsAboutToBeInserted(QModelIndex parent, int first, int last)
{
    beginInsertColumns(mapFromSource(parent), first, last);
}

void KisResourceGridProxyModel::handle_rowsInserted(QModelIndex parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(first);
    Q_UNUSED(last);
    endInsertColumns();
}

void KisResourceGridProxyModel::handle_rowsAboutToBeMoved(QModelIndex sourceParent, int sourceStart, int sourceEnd, QModelIndex destinationParent, int destinationColumn)
{
    beginMoveColumns(mapFromSource(sourceParent), sourceStart, sourceEnd, destinationParent, destinationColumn);
}

void KisResourceGridProxyModel::handle_rowsMoved(QModelIndex sourceParent, int sourceStart, int sourceEnd, QModelIndex destinationParent, int destinationColumn)
{
    Q_UNUSED(sourceParent);
    Q_UNUSED(sourceStart);
    Q_UNUSED(sourceEnd);
    Q_UNUSED(destinationParent);
    Q_UNUSED(destinationColumn);
    endMoveColumns();
}

void KisResourceGridProxyModel::handle_rowsAboutToBeRemoved(QModelIndex parent, int first, int last)
{
    beginRemoveColumns(mapFromSource(parent), first, last);
}

void KisResourceGridProxyModel::handle_rowsRemoved(QModelIndex parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(first);
    Q_UNUSED(last);
    endRemoveColumns();
}

void KisResourceGridProxyModel::handle_dataChanged(QModelIndex topLeft, QModelIndex bottomRight, QVector<int> roles)
{
    emit dataChanged(mapFromSource(topLeft), mapFromSource(bottomRight), roles);
}

void KisResourceGridProxyModel::handle_headerDataChanged(Qt::Orientation orientation, int first, int last)
{
    emit headerDataChanged(orientation, first, last);
}

void KisResourceGridProxyModel::handle_layoutAboutToBeChanged(QList<QPersistentModelIndex> parents, QAbstractItemModel::LayoutChangeHint hint)
{
    QList<QPersistentModelIndex> proxyParents;
    proxyParents.reserve(parents.size());
    for(QList<QPersistentModelIndex>::const_iterator it = parents.cbegin(); it != parents.cend(); ++it) {
        proxyParents.append(mapFromSource(*it));
    }
    emit layoutAboutToBeChanged(proxyParents, hint);
}

void KisResourceGridProxyModel::handle_layoutChanged(QList<QPersistentModelIndex> parents, QAbstractItemModel::LayoutChangeHint hint)
{
    QList<QPersistentModelIndex> proxyParents;
    proxyParents.reserve(parents.size());
    for(QList<QPersistentModelIndex>::const_iterator it = parents.cbegin(); it != parents.cend(); ++it) {
        proxyParents.append(mapFromSource(*it));
    }
    emit layoutChanged(proxyParents, hint);
}

void KisResourceGridProxyModel::handle_modelAboutToBeReset()
{
    beginResetModel();
}

void KisResourceGridProxyModel::handle_modelReset()
{
    endResetModel();
}

int KisResourceGridProxyModel::sourceRow(int rowIndex, int column) const
{
    return rowIndex * d->rowStride + column;
}

void KisResourceGridProxyModel::proxyRow(int rowIndex, int &proxyRow, int &proxyColumn) const
{
    proxyRow = rowIndex / d->rowStride;
    proxyColumn = rowIndex % d->rowStride;
}
