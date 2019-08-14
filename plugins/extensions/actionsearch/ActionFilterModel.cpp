/*
 *  Copyright (c) 2019 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "ActionFilterModel.h"

#include <QDebug>
#include <QPushButton>
#include <QFrame>
#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QAction>
#include <QVariant>

#include <kis_global.h>

#include "ActionModel.h"

ActionFilterModel::ActionFilterModel(QObject *parent)
{

}

void ActionFilterModel::setFilterText(const QString &filter)
{
    m_filter = filter;
}

bool ActionFilterModel::filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const
{
    return true;
}

bool ActionFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    if (!index.isValid()) return false;
    return true;

    QStringList tags = index.data(Qt::UserRole + 1).toStringList();
    bool hit = false;
    Q_FOREACH(const QString &tag, tags) {
        if (true || tag.contains(m_filter)) {
            hit = true;
            break;
        }
    }
    return hit;
}

bool ActionFilterModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    QVariant leftData = sourceModel()->data(source_left, Qt::DisplayRole);
    QVariant rightData = sourceModel()->data(source_right, Qt::DisplayRole);

    QString leftName = leftData.toString();
    QString rightName = rightData.toString();
    return QString::localeAwareCompare(leftName, rightName) < 0;
}
