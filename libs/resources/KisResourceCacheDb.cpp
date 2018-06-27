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
#include <klocalizedstring.h>
#include <kis_debug.h>

#include "KisResourceLocator.h"
#include "KisResourceLoaderRegistry.h"

const QString dbDriver = "QSQLITE";

const QStringList KisResourceCacheDb::storageTypes = QStringList() << "UNKNOWN"
                                                                   << "FOLDER"
                                                                   << "BUNDLE"
                                                                   << "ADOBE_BRUSH_LIBRARY"
                                                                   << "ADOBE_STYLE_LIBRARY"; // Installed or created by the user

const QString KisResourceCacheDb::dbLocationKey {"ResourceCacheDbDirectory"};
const QString KisResourceCacheDb::resourceCacheDbFilename {"resourcecache.sqlite"};
const QString KisResourceCacheDb::databaseVersion {"0.0.1"};

bool KisResourceCacheDb::s_valid {false};

bool KisResourceCacheDb::isValid()
{
    return s_valid;
}

QSqlError initDb(const QString &location)
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
    db.setDatabaseName(location + "/" + KisResourceCacheDb::resourceCacheDbFilename);

    if (!db.open()) {
        infoResources << "Could not connect to resource cache database";
        return db.lastError();
    }

    QStringList tables = QStringList() << "version_information"
                                       << "origin_types"
                                       << "resource_types"
                                       << "storages"
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
            QFile f(":/get_version_information.sql");
            if (f.open(QFile::ReadOnly)) {
                QSqlQuery query(f.readAll());
                if (query.size() > 0) {
                    query.first();
                    QString schemaVersion = query.value(0).toString();
                    QString kritaVersion = query.value(1).toString();
                    QString creationDate = query.value(2).toString();

                    infoResources << "Database version" << schemaVersion
                                  << "Krita version that created the database" << kritaVersion
                                  << "At" << creationDate;

                    if (schemaVersion != KisResourceCacheDb::databaseVersion) {
                        // XXX: Implement migration
                        warnResources << "Database schema is outdated, migration is needed";
                        schemaIsOutDated = true;
                    }
                }
            }
            else {
                return QSqlError("Error executing SQL", "Could not open get_version_information.sql", QSqlError::StatementError);
            }
        }

        if (allTablesPresent && !schemaIsOutDated) {
            infoResources << "All tables are present and up to date";
            return QSqlError();
        }
    }

    // Create tables
    Q_FOREACH(const QString &table, tables) {
        QFile f(":/create_" + table + ".sql");
        if (f.open(QFile::ReadOnly)) {
            QSqlQuery query;
            if (!query.exec(f.readAll())) {
                qWarning() << "Could not create table" << table;
                return db.lastError();
            }
            infoResources << "Created table" << table;
        }
        else {
            return QSqlError("Error executing SQL", QString("Could not find SQL file %1").arg(table), QSqlError::StatementError);
        }
    }

    // Create indexes
    QStringList indexes = QStringList() << "storages";

    Q_FOREACH(const QString &index, indexes) {
        QFile f(":/create_index_" + index + ".sql");
        if (f.open(QFile::ReadOnly)) {
            QSqlQuery query;
            if (!query.exec(f.readAll())) {
                qWarning() << "Could not create index" << index;
                return db.lastError();
            }
            infoResources << "Created table" << index;
        }
        else {
            return QSqlError("Error executing SQL", QString("Could not find SQL file %1").arg(index), QSqlError::StatementError);
        }
    }

    // Fill lookup tables
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
            Q_FOREACH(const QString &originType, KisResourceCacheDb::storageTypes) {
                QSqlQuery query(sql);
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
            Q_FOREACH(const QString &resourceType, KisResourceLoaderRegistry::instance()->resourceTypes()) {
                QSqlQuery query(sql);
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
            query.addBindValue(KisResourceCacheDb::databaseVersion);
            query.addBindValue(KritaVersionWrapper::versionString());
            query.addBindValue(QDateTime::currentDateTimeUtc().toString());
            if (!query.exec()) {
                qWarning() << "Could not insert the current version" << db.lastError() << query.executedQuery() << query.boundValues();
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

bool KisResourceCacheDb::initialize(const QString &location)
{
    QSqlError err = initDb(location);
    if (err.isValid()) {
        qWarning() << "Could not initialize the database:" << err;
    }
    s_valid = !err.isValid();

    return s_valid;
}

bool KisResourceCacheDb::addResource(KisResourceStorageSP storage, KoResourceSP resource, const QString &resourceType)
{
    bool r = true;

    if (!s_valid) {
        qWarning() << "KisResourceCacheDb::addResource: The database is not valid";
        return false;
    }

    if (!resource || !resource->valid()) {
        qWarning() << "KisResourceCacheDb::addResource: The resource is not valid";
        return false;
    }

    /*    // Check whether it already exists
    if (hasResource(storage, resource)) {
        r = addResourceVersion(storage, resource);
    }
    else*/ {
        QSqlQuery q;
        r = q.prepare("INSERT INTO resources "
                      "(resource_type_id, name, tooltip, thumbnail, status)"
                      "VALUES"
                      "((select id from resource_types where name = :resource_type), :name, :tooltip, :thumbnail, :status);");

        if (!r) {
            qWarning() << "Could not prepare addResource statement" << q.lastError();
            return r;
        }

        q.bindValue(":resource_type", resourceType);
        q.bindValue(":name", resource->name());
        q.bindValue(":tooltip", i18n(resource->name().toUtf8()));
        q.bindValue(":thumbnail", resource->image());
        q.bindValue(":status", 1);

        r = q.exec();
        if (!r) {
            qWarning() << "Could not execute addResource statement" << q.boundValues() << q.lastError();
            return r;
        }
    }
    // Then add a new version

    qDebug() << "Adding" << resource->filename();

    // Insert it

    return r;
}

bool KisResourceCacheDb::addResources(KisResourceStorageSP storage, QString folder)
{
    QSharedPointer<KisResourceStorage::ResourceIterator> iter = storage->resources(folder);
    while(iter->hasNext()) {
        iter->next();
        KoResourceSP res = iter->resource();
        if (res) {
            if (!addResource(storage, res, iter->type())) {
                qWarning() << "Could not add resource" << res->filename() << "to the database";
            }
        }
    }
    return true;
}

bool KisResourceCacheDb::addStorage(KisResourceStorageSP storage, bool preinstalled)
{
    bool r = true;

    if (!s_valid) {
        qWarning() << "The database is not valid";
        return false;
    }

    {
        QSqlQuery q;
        r = q.prepare("SELECT * FROM storages WHERE location = :location");
        q.bindValue(":location", storage->location());
        r = q.exec();
        if (!r) {
            qWarning() << "Could not select from storages";
            return r;
        }
        if (q.first()) {
            //qDebug() << "This storage already exists";
            return true;
        }
    }

    {
        QSqlQuery q;

        r = q.prepare("INSERT INTO storages "
                      "(origin_type_id, location, datestamp, pre_installed, active)"
                      "VALUES"
                      "(:origin_type_id, :location, :datestamp, :pre_installed, :active);");

        if (!r) {
            qWarning() << "Could not prepare query" << q.lastError();
            return r;
        }

        q.bindValue(":origin_type_id", static_cast<int>(storage->type()));
        q.bindValue(":location", storage->location());
        q.bindValue(":datestamp", storage->timestamp().toMSecsSinceEpoch());
        q.bindValue(":pre_installed", preinstalled);
        q.bindValue(":active", preinstalled ? 1 : 0);

        r = q.exec();

        if (!r) qWarning() << "Could not execute query" << q.lastError();
    }
    return r;
}

bool KisResourceCacheDb::synchronize(KisResourceStorageSP storage)
{
    // Find the storage in the database
    qDebug() << storage->location() << storage->timestamp();
    return true;
}

