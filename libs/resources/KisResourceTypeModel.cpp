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

struct KisResourceTypeModel::Private {
    int cachedRowCount {-1};
    QSqlQuery query;
};


KisResourceTypeModel::KisResourceTypeModel(QObject *parent)
    : QAbstractTableModel(parent)
    , d(new Private)
{
    prepareQuery();
}

KisResourceTypeModel::~KisResourceTypeModel()
{
    delete d;
}

int KisResourceTypeModel::rowCount(const QModelIndex &/*parent*/) const
{
    if (d->cachedRowCount < 0) {
        QSqlQuery q;
        q.prepare("SELECT count(*)\n"
                  "FROM   resource_types\n");
        q.exec();
        q.first();

        const_cast<KisResourceTypeModel*>(this)->d->cachedRowCount = q.value(0).toInt();

        qDebug() << "KisResourceTypeModel::rowCount()" << d->cachedRowCount;
    }
    return d->cachedRowCount;
}

int KisResourceTypeModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 3;
}

QVariant KisResourceTypeModel::data(const QModelIndex &index, int role) const
{

    QVariant v;
    if (!index.isValid()) return v;

    if (index.row() > rowCount()) return v;
    if (index.column() > (int)Name) return v;

    bool pos = d->query.seek(index.row());

    qDebug() << "KisResourceTypeModel::data" << pos << index.row() << index.column() << role << ":" << Name;

    if (pos) {
        QString id = d->query.value("id").toString();
        QString resourceType = d->query.value("name").toString();
        QString name = resourceType;
        QVector<KisResourceLoaderBase *> loaders = KisResourceLoaderRegistry::instance()->resourceTypeLoaders(resourceType);
        Q_ASSERT(loaders.size() > 0);
        name = loaders.first()->name();

        qDebug() << id << resourceType << name;

        switch(role) {
        case Qt::DisplayRole:
        {
            switch(index.column()) {
            case Id:
                qDebug() << "Id" << id;
                return id;
            case ResourceType:
                qDebug() << "resourcetype" << resourceType;
                return resourceType;
            case Name:
                qDebug() << "Name" << name;
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
    return v;
}

bool KisResourceTypeModel::prepareQuery()
{
    beginResetModel();
    bool r = d->query.prepare("SELECT id\n"
                       ",      name\n"
                       "FROM   resource_types\n");
    if (!r) {
        qWarning() << "Could not prepare KisResourceTypeModel query" << d->query.lastError();
    }
    r = d->query.exec();
    if (!r) {
        qWarning() << "Could not execute KisResourceTypeModel query" << d->query.lastError();
    }
    d->cachedRowCount = -1;
    endResetModel();
    return r;
}
