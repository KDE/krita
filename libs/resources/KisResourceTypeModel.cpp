/*
 * Copyright (c) 2018 boud <boud@valdyas.org>
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
#include "KisResourceTypeModel.h"
#include <QtSql>

#include "KisResourceLoader.h"
#include "KisResourceLoaderRegistry.h"

KisResourceTypeModel::KisResourceTypeModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int KisResourceTypeModel::rowCount(const QModelIndex &/*parent*/) const
{
    QSqlQuery q;
    q.prepare("SELECT count(*)\n"
              "FROM   resource_types\n");
    q.exec();
    q.first();
    return q.value(0).toInt();
}

int KisResourceTypeModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 2;
}

QVariant KisResourceTypeModel::data(const QModelIndex &index, int role) const
{
    QSqlQuery q;
    q.prepare("SELECT id\n"
              ",      name\n"
              "FROM   resource_types\n");
    q.exec();
    bool pos = q.seek(index.row());

    if (pos) {
        QString id = q.value("id").toString();
        QString resourceType = q.value("name").toString();
        QString name = resourceType;
        QVector<KisResourceLoaderBase *> loaders = KisResourceLoaderRegistry::instance()->resourceTypeLoaders(resourceType);
        Q_ASSERT(loaders.size() > 0);
        name = loaders.first()->name();

        switch(role) {
        case Qt::DisplayRole:
        {
            switch(index.column()) {
            case Id:
                return id;
            case ResourceType:
                return resourceType;
            case Name:
                return name;
            }
        }
        case Qt::UserRole + Id:
            return id;
        case Qt::UserRole + ResourceType:
            return resourceType;
        case Qt::UserRole + Name:
            return name;
        default:
            ;
        }
    }
    return QVariant();
}
