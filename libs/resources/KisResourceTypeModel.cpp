/*
 * SPDX-FileCopyrightText: 2018 boud <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

int KisResourceTypeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    if (d->cachedRowCount < 0) {
        QSqlQuery q;
        q.prepare("SELECT count(*)\n"
                  "FROM   resource_types\n");
        q.exec();
        q.first();

        const_cast<KisResourceTypeModel*>(this)->d->cachedRowCount = q.value(0).toInt();
    }
    return d->cachedRowCount;
}

int KisResourceTypeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return 3;
}

QVariant KisResourceTypeModel::data(const QModelIndex &index, int role) const
{
    QVariant v;
    if (!index.isValid()) return v;

    if (index.row() > rowCount()) return v;
    if (index.column() > (int)Name) return v;

    bool pos = d->query.seek(index.row());

    if (pos) {
        QString id = d->query.value("id").toString();
        QString resourceType = d->query.value("name").toString();
        QString name = ResourceName::resourceTypeToName(resourceType);

        switch(role) {
        case Qt::DisplayRole:
        {
            switch(index.column()) {
            case Id:
                return id;
            case ResourceType:
                return resourceType;
            case Name:
            default:
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
