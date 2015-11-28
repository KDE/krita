/* This file is part of the KDE project
 * Copyright (C) 2013 Thorsten Zachmann <zachmann@kde.org>
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

#include "StylesSortFilterProxyModel.h"

#include <QDebug>

StylesSortFilterProxyModel::StylesSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool StylesSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    QVariant leftData = sourceModel()->data(left, Qt::DisplayRole);
    QVariant rightData = sourceModel()->data(right, Qt::DisplayRole);

    QString leftName = leftData.toString();
    QString rightName = rightData.toString();
    return QString::localeAwareCompare(leftName, rightName) < 0;
}
