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
#include "KisTagFilterResourceProxyModel.h"

#include <QDebug>
#include <KisResourceModel.h>

struct KisTagFilterResourceProxyModel::Private
{
    QStringList tags;
};

KisTagFilterResourceProxyModel::KisTagFilterResourceProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , d(new Private)
{
}

KisTagFilterResourceProxyModel::~KisTagFilterResourceProxyModel()
{
    delete d;
}

KoResourceSP KisTagFilterResourceProxyModel::resourceForIndex(QModelIndex index) const
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->resourceForIndex(mapToSource(index));
    }
    return 0;
}

QModelIndex KisTagFilterResourceProxyModel::indexFromResource(KoResourceSP resource) const
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return mapFromSource(source->indexFromResource(resource));
    }
    return QModelIndex();
}

bool KisTagFilterResourceProxyModel::removeResource(const QModelIndex &index)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->removeResource(mapToSource(index));
    }
    return false;
}

bool KisTagFilterResourceProxyModel::importResourceFile(const QString &filename)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->importResourceFile(filename);
    }
    return false;
}

bool KisTagFilterResourceProxyModel::addResource(KoResourceSP resource, bool save)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->addResource(resource, save);
    }
    return false;
}

bool KisTagFilterResourceProxyModel::updateResource(KoResourceSP resource)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->updateResource(resource);
    }
    return false;
}

bool KisTagFilterResourceProxyModel::removeResource(KoResourceSP resource)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->removeResource(resource);
    }
    return false;
}

void KisTagFilterResourceProxyModel::setTag(const QString& tag)
{
    d->tags = tag.split(QRegExp("[,]\\s*"), QString::SkipEmptyParts);
}

bool KisTagFilterResourceProxyModel::filterAcceptsColumn(int /*source_column*/, const QModelIndex &/*source_parent*/) const
{
    return true;
}

bool KisTagFilterResourceProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (d->tags.isEmpty()) return true;

    QModelIndex idx = sourceModel()->index(source_row, KisResourceModel::Name, source_parent);

    QStringList tags = sourceModel()->data(idx, Qt::UserRole + KisResourceModel::Tags).toStringList();
    QSet<QString> tagResult = tags.toSet().subtract(tags.toSet());
    if (!tagResult.isEmpty()) {
        return true;
    }

    QString name = sourceModel()->data(idx, Qt::UserRole + KisResourceModel::Name).toString();
    Q_FOREACH(const QString &tag, d->tags) {
        if (name.startsWith(tag)) {
            return true;
        }
    }

    return false;
}

bool KisTagFilterResourceProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    QString nameLeft = sourceModel()->data(source_left, Qt::UserRole + KisResourceModel::Name).toString();
    QString nameRight = sourceModel()->data(source_right, Qt::UserRole + KisResourceModel::Name).toString();
    return nameLeft < nameRight;
}
