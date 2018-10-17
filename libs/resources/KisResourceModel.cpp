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

#include "KisResourceModel.h"

#include <QBuffer>
#include <QImage>
#include <QtSql>

struct KisResourceModel::Private {
    QSqlQuery query;
    QString resourceType;
    int columnCount {7};
    int cachedRowCount {-1};
};


KisResourceModel::KisResourceModel(const QString &resourceType, QObject *parent)
    : QAbstractTableModel(parent)
    , d(new Private)
{
    d->resourceType = resourceType;
    bool r = d->query.prepare("SELECT resources.id\n"
                             ",      resources.storage_id\n"
                             ",      resources.name\n"
                             ",      resources.filename\n"
                             ",      resources.tooltip\n"
                             ",      resources.thumbnail\n"
                             ",      resources.status\n"
                             "FROM   resources\n"
                             ",      resource_types\n"
                             "WHERE  resources.resource_type_id = resource_types.id\n"
                             "AND    resource_types.name = :resource_type\n"
                             "AND    resources.status = 1");
    if (!r) {
        qWarning() << "Could not prepare KisResourceModel query" << d->query.lastError();
    }
    d->query.bindValue(":resource_type", resourceType);
    r = d->query.exec();
    if (!r) {
        qWarning() << "Could not select" << resourceType << "resources" << d->query.lastError();
    }
    Q_ASSERT(d->query.isSelect());
}

KisResourceModel::~KisResourceModel()
{
    delete d;
}

int KisResourceModel::columnCount(const QModelIndex &/*parent*/) const
{
    return d->columnCount;
}

QVariant KisResourceModel::data(const QModelIndex &index, int role) const
{

    QVariant v;
    if (!index.isValid()) return v;

    if (index.row() > rowCount()) return v;
    if (index.column() > d->columnCount) return v;

    bool pos = const_cast<KisResourceModel*>(this)->d->query.seek(index.row());

    if (pos) {
        switch(role) {
        case Qt::DisplayRole:
        {
            switch(index.column()) {
            case 0:
                return d->query.value("id");
            case 1:
                return d->query.value("storage_id");
            case 2:
                return d->query.value("name");
            case 3:
                return d->query.value("filename");
            case 4:
                return d->query.value("tooltip");
            case 5:
                ;
            case 6:
                return d->query.value("status");
            default:
                ;
            };
        }
        case Qt::DecorationRole:
        {
            if (index.column() == 5) {
                QByteArray ba = d->query.value("thumbnail").toByteArray();
                QBuffer buf(&ba);
                buf.open(QBuffer::ReadOnly);
                QImage img;
                img.load(&buf, "PNG");
                return QVariant::fromValue<QImage>(img);
            }
            return QVariant();
        }
        case Qt::ToolTipRole:
            /* Falls through. */
        case Qt::StatusTipRole:
            /* Falls through. */
        case Qt::WhatsThisRole:
            return d->query.value("tooltip");
        default:
            ;
        }
    }

    return v;
}

int KisResourceModel::rowCount(const QModelIndex &) const
{
    if (d->cachedRowCount < 0) {
        QSqlQuery q;
        q.prepare("SELECT count(*)\n"
                  "FROM   resources\n"
                  ",      resource_types\n"
                  "WHERE  resources.resource_type_id = resource_types.id\n"
                  "AND    resource_types.name = :resource_type");
        q.bindValue(":resource_type", d->resourceType);
        q.exec();
        q.first();

        const_cast<KisResourceModel*>(this)->d->cachedRowCount = q.value(0).toInt();
    }

    return d->cachedRowCount;
}
