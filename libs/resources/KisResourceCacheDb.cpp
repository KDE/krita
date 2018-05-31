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
#include <QDir>
#include <QDirIterator>

#include <KritaVersionWrapper.h>

#include <kis_debug.h>

const QString dbDriver = "QSQLITE";

const QStringList KisResourceCacheDb::resourceTypes = QStringList() << "BRUSH_TIP"
                                                                    << "GRADIENT"
                                                                    << "PAINTOP_PRESET"
                                                                    << "COLORSET"
                                                                    << "PATTERN"
                                                                    << "SYMBOL_LIBRARY"
                                                                    << "TEMPLATE"
                                                                    << "WORKSPACE"
                                                                    << "SESSION";

const QStringList KisResourceCacheDb::originTypes = QStringList() << "INSTALLATION" // Installed by Krita
                                                                  << "BUNDLE" // Bundle installed by the user
                                                                  << "ADOBE_LIBRARY" // ABR or ASL or similar Adobe resource library
                                                                  << "USER"; // Installed or created by the user

const QString KisResourceCacheDb::dbLocationKey {"ResourceCacheDbDirectory"};
const QString KisResourceCacheDb::ResourceCacheDbFilename {"resourcecache.sqlite"};
const QString KisResourceCacheDb::databaseVersion {"0.0.1"};



class KisResourceCacheDb::Private
{
public:

    bool valid {false};

    QSqlError initDb(const QString &location);
};

KisResourceCacheDb::KisResourceCacheDb()
    : d(new Private())
{
}

KisResourceCacheDb::~KisResourceCacheDb()
{
}

bool KisResourceCacheDb::isValid() const
{
    return d->valid;
}

bool KisResourceCacheDb::initialize(const QString &location) const
{
    QSqlError err = d->initDb(location);
    if (err.isValid()) {
        qWarning() << "Could not initialize the database:" << err;
    }
    d->valid = !err.isValid();

    return d->valid;
}

QSqlError KisResourceCacheDb::Private::initDb(const QString &location)
{
    if (!QSqlDatabase::connectionNames().isEmpty()) {
        infoResources << "Already connected to resource cache database";
        return QSqlError();
    }

    QDir dbLocation(location);
    if (!dbLocation.exists()) {
        dbLocation.mkpath(dbLocation.path());
    }

    QSqlDatabase db = QSqlDatabase::addDatabase(dbDriver);
    db.setDatabaseName(location + "/" + ResourceCacheDbFilename);

    if (!db.open()) {
        infoResources << "Could not connect to resource cache database";
        return db.lastError();
    }

    QStringList tables = QStringList() << "version_information"
                                       << "origin_types"
                                       << "resource_types"
                                       << "stores"
                                       << "tags"
                                       << "resources"
                                       << "translations"
                                       << "versioned_resources"
                                       << "resource_tags";

    QStringList dbTables;
    // Verify whether we should recreate the database
    {
        bool allTablesPresent = true;
        dbTables = db.tables();
        Q_FOREACH(const QString &table, tables) {
            if (!dbTables.contains(table)) {
                allTablesPresent = false;
            }
        }

        bool schemaIsOutDated = false;

        if (dbTables.contains("version_information")) {
            // Verify the version number
            QSqlQuery query;
            query.exec("SELECT database_version, krita_version, creation_date"
                       "FROM version_information"
                       "ORDER BY id DESC LIMIT 1");
            if (query.size() > 0) {
                query.first();
                QString schemaVersion = query.value(0).toString();
                QString kritaVersion = query.value(1).toString();
                QString creationDate = query.value(2).toString();

                infoResources << "Database version" << schemaVersion
                              << "Krita version that created the database" << kritaVersion
                              << "At" << creationDate;

                if (schemaVersion != databaseVersion) {
                    warnResources << "Database schema is outdated, migration is needed";
                    schemaIsOutDated = true;
                }
            }
        }

        if (allTablesPresent && !schemaIsOutDated) {
            infoResources << "All tables are present and up to date";
            return QSqlError();
        }
    }

    Q_FOREACH(const QString &table, tables) {
        QFile f(":/create_" + table + ".sql");
        if (f.open(QFile::ReadOnly)) {
            QString sql(f.readAll());
            QSqlQuery query;
            if (!query.exec(sql)) {
                qWarning() << "Could not create table" << table;
                return db.lastError();
            }
            infoResources << "Created table" << table;
        }
        else {
            return QSqlError("Error executing SQL", QString("Could not find SQL file %1").arg(table), QSqlError::StatementError);
        }
    }

    {
        if (dbTables.contains("origin_types")) {
            QSqlQuery query;
            if (!query.exec("DELETE * FROM origin_types;")) {
                qWarning() << "Could not clear table origin_types" << db.lastError();
            }
        }

        QFile f(":/fill_origin_types.sql");
        if (f.open(QFile::ReadOnly)) {
            QString sql = f.readAll();
            Q_FOREACH(const QString &originType, originTypes) {
                QSqlQuery query;
                query.prepare(sql);
                query.addBindValue(originType);
                if (!query.exec()) {
                    qWarning() << "Could not insert" << originType << db.lastError() << query.executedQuery();
                    return db.lastError();
                }
            }
            infoResources << "Filled lookup table origin_types";
        }
        else {
            return QSqlError("Error executing SQL", QString("Could not find SQL fill_origin_types.sql."), QSqlError::StatementError);
        }
    }

    {
        if (dbTables.contains("resource_types")) {
            QSqlQuery query;
            if (!query.exec("DELETE * FROM resource_types;")) {
                qWarning() << "Could not cleare table resource_types" << db.lastError();
            }
        }
        QFile f(":/fill_resource_types.sql");
        if (f.open(QFile::ReadOnly)) {
            QString sql = f.readAll();
            Q_FOREACH(const QString &resourceType, resourceTypes) {
                QSqlQuery query;
                query.prepare(sql);
                query.addBindValue(resourceType);
                if (!query.exec()) {
                    qWarning() << "Could not insert" << resourceType << db.lastError() << query.executedQuery();
                    return db.lastError();
                }
            }
            infoResources << "Filled lookup table resource_types";
        }
        else {
            return QSqlError("Error executing SQL", QString("Could not find SQL fill_resource_types.sql."), QSqlError::StatementError);
        }
    }

    {
        QFile f(":/fill_version_information.sql");
        if (f.open(QFile::ReadOnly)) {
            QString sql = f.readAll();
            QSqlQuery query;
            query.prepare(sql);
            query.addBindValue(databaseVersion);
            query.addBindValue(KritaVersionWrapper::versionString());
            query.addBindValue(QDateTime::currentDateTimeUtc().toString());
            if (!query.exec()) {
                qWarning() << "Could not insert the current version" << db.lastError() << query.executedQuery();
                return db.lastError();
            }
            infoResources << "Filled version table";
        }
        else {
            return QSqlError("Error executing SQL", QString("Could not find SQL fill_version_information.sql."), QSqlError::StatementError);
        }
    }

    return QSqlError();
}
