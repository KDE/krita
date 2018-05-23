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
#include "KisResourceCacheDb.h"

#include <QtSql>
#include <QStandardPaths>

#include <kconfiggroup.h>
#include <ksharedconfig.h>

const QString dbDriver = "QSQLITE";


class KisResourceCacheDb::Private
{
public:

    QSqlError initDb();
};

KisResourceCacheDb::KisResourceCacheDb(QObject *parent)
    : QObject(parent)
    , d(new Private())
{
    QSqlError err = d->initDb();
}

KisResourceCacheDb::~KisResourceCacheDb()
{
}

QSqlError KisResourceCacheDb::Private::initDb()
{
    QSqlDatabase db = QSqlDatabase::addDatabase(dbDriver);

    const KConfigGroup group(KSharedConfig::openConfig(), "ResourceManagement");
    QString dbFile(group.readEntry<QString>("cachedb", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/resourcecache"));
    db.setDatabaseName(dbFile);
    if (!db.open()) {
        return db.lastError();
    }

    QStringList tables = QStringList() << "origin_types"
                                       << "resource_types"
                                       << "stores"
                                       << "tags"
                                       << "resources"
                                       << "translations"
                                       << "versioned_resources"
                                       << "resource_tags";


    QSqlQuery query;

    Q_FOREACH(const QString &table, tables) {
        QFile f(":/create_" + table + ".sql");
        Q_ASSERT(f.exists());
        if (f.open(QFile::ReadOnly)) {
            QString sql(f.readAll());
            if (!query.exec(sql)) {
                return db.lastError();
            }
        }
    }

    return QSqlError();
}
