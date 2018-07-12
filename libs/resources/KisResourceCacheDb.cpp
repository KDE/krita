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
                QSqlQuery q(f.readAll());
                if (q.size() > 0) {
                    q.first();
                    QString schemaVersion = q.value(0).toString();
                    QString kritaVersion = q.value(1).toString();
                    QString creationDate = q.value(2).toString();

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
            QSqlQuery q;
            if (!q.exec(f.readAll())) {
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
            QSqlQuery q;
            if (!q.exec(f.readAll())) {
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
            QSqlQuery q;
            if (!q.exec("DELETE * FROM origin_types;")) {
                qWarning() << "Could not clear table origin_types" << db.lastError();
            }
        }

        QFile f(":/fill_origin_types.sql");
        if (f.open(QFile::ReadOnly)) {
            QString sql = f.readAll();
            Q_FOREACH(const QString &originType, KisResourceCacheDb::storageTypes) {
                QSqlQuery q(sql);
                q.addBindValue(originType);
                if (!q.exec()) {
                    qWarning() << "Could not insert" << originType << db.lastError() << q.executedQuery();
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
            QSqlQuery q;
            if (!q.exec("DELETE * FROM resource_types;")) {
                qWarning() << "Could not cleare table resource_types" << db.lastError();
            }
        }
        QFile f(":/fill_resource_types.sql");
        if (f.open(QFile::ReadOnly)) {
            QString sql = f.readAll();
            Q_FOREACH(const QString &resourceType, KisResourceLoaderRegistry::instance()->resourceTypes()) {
                QSqlQuery q(sql);
                q.addBindValue(resourceType);
                if (!q.exec()) {
                    qWarning() << "Could not insert" << resourceType << db.lastError() << q.executedQuery();
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
            QSqlQuery q;
            q.prepare(sql);
            q.addBindValue(KisResourceCacheDb::databaseVersion);
            q.addBindValue(KritaVersionWrapper::versionString());
            q.addBindValue(QDateTime::currentDateTimeUtc().toString());
            if (!q.exec()) {
                qWarning() << "Could not insert the current version" << db.lastError() << q.executedQuery() << q.boundValues();
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

int KisResourceCacheDb::resourceIdForResource(KoResourceSP resource, const QString &resourceType)
{
    QFile f(":/select_resource_id.sql");
    f.open(QFile::ReadOnly);
    QSqlQuery q;
    if (!q.prepare(f.readAll())) {
        qWarning() << "Could not read and prepare resourceIdForResource" << q.lastError();
        return false;
    }

    q.bindValue(":name", resource->name());
    q.bindValue(":filename", resource->filename());
    q.bindValue(":resource_type", resourceType);

    if (!q.exec()) {
        qWarning() << "Could not query resourceIdForResource" << q.boundValues() << q.lastError();
        return -1;
    }
    if (!q.first()) {
        qWarning() << "Could not find this reosurce" <<  resource->filename() << q.boundValues() << q.lastError();
        return -1;
    }
    return q.value(0).toInt();

}

bool KisResourceCacheDb::resourceNeedsUpdating(int resourceId, QDateTime timestamp)
{
    QSqlQuery q;
    if (!q.prepare("SELECT timestamp\n"
                  "FROM   versioned_resources\n"
                  "WHERE  resourceId = :resource_id\n"
                  "AND    version = (SELECT MAX(version)\n"
                  "                  FROM   versioned_resources\n"
                   "                  WHERE  resource_id = :resource_id);")) {
        qWarning() << "Could not prepare resourceNeedsUpdating statement";
        return false;
    }

    q.bindValue(":resource_id", resourceId);

    if (!q.exec()) {
        qDebug() << "Could not query for the most recent timestamp" << q.boundValues() << q.lastError();
        return false;
    }

    if (!q.first()) {
        qWarning() << "Inconsistent database: could not find a version for resource with Id" << resourceId;
        return false;
    }

    QVariant resourceTimeStamp = q.value(0);

    if (!resourceTimeStamp.isValid()) {
        qWarning() << "Could not retrieve timestamp from versioned_resources" << resourceId;
    }

    return (timestamp > QDateTime::fromString(resourceTimeStamp.toString()));
}

bool KisResourceCacheDb::addResourceVersion(int resourceId, QDateTime timestamp, KisResourceStorageSP storage, KoResourceSP resource)
{
    bool r = false;

    // Create the new version
    {
        QSqlQuery q;
        r = q.prepare("INSERT INTO versioned_resources \n"
                      "(resource_id, storage_id, version, location, datestamp, deleted, checksum)\n"
                      "VALUES\n"
                      "(:resource_id,\n"
                      ",    (SELECT id FROM storages \n"
                      "      WHERE location = :storage_location)\n"
                      ",    (SELECT MAX(version) + 1 FROM versioned_resources\n"
                      "      WHERE  resource_id = :resource_id)\n"
                      ", :location, :datestamp, 0, :checksum\n"
                      ");");

        if (!r) {
            qWarning() << "Could not prepare addResourceVersion statement" << q.lastError();
            return r;
        }

        q.bindValue(":resource_id", resourceId);
        q.bindValue(":storage_location", storage->location());
        q.bindValue(":location", resource->filename());
        q.bindValue(":datestamp", timestamp.toString()); // XXX: make the right format for sqlite to work with
        q.bindValue(":deleted", 0);
        q.bindValue(":checksum", resource->md5());

        r = q.exec();
        if (!r) {
            qWarning() << "Could not execute addResourceVersion statement" << q.boundValues() << q.lastError();
            return r;
        }
    }
    // Update the resource itself
    {
        QSqlQuery q;
        r = q.prepare("UPDATE resources\n"
                      "SET name = :name\n"
                      ", filename = :filename\n"
                      ", tooltip = :tooltip\n"
                      ", thumbnail = :thumbnail)\n"
                      "WHERE resourceId = :resourceId");
        if (!r) {
            qWarning() << "Could not prepare updateResource statement" << q.lastError();
            return r;
        }
        q.bindValue(":name", resource->name());
        q.bindValue(":filename", resource->filename());
        q.bindValue(":tooltip", i18n(resource->name().toUtf8()));

        QByteArray ba;
        QBuffer buf(&ba);
        buf.open(QBuffer::WriteOnly);
        resource->image().save(&buf, "PNG");
        buf.close();
        q.bindValue(":thumbnail", ba);

        q.bindValue(":resource_id", resourceId);

        r = q.exec();
        if (!r) {
            qWarning() << "Could not update resource" << q.boundValues() << q.lastError();
        }
    }

    return r;
}

bool KisResourceCacheDb::hasResource(KisResourceStorageSP storage, KoResourceSP resource, const QString &resourceType)
{
    QFile f(":/select_resource.sql");
    if (f.open(QFile::ReadOnly)) {
        QSqlQuery q;
        if (!q.prepare(f.readAll())) {
            qWarning() << "Could not read and prepare select_resource.sql" << q.lastError();
            return false;
        }
        q.bindValue(":storage", storage->location());
        q.bindValue(":location", resource->filename());
        q.bindValue(":resource_type", resourceType);
        if (!q.exec()) {
            qWarning() << "Could not query resources" << q.boundValues() << q.lastError();
        }
        return q.first();
    }
    qWarning() << "Could not open select_resource.sql";
    return false;
}

bool KisResourceCacheDb::addResource(KisResourceStorageSP storage, QDateTime timestamp, KoResourceSP resource, const QString &resourceType)
{
    bool r = false;

    if (!s_valid) {
        qWarning() << "KisResourceCacheDb::addResource: The database is not valid";
        return false;
    }

    if (!resource || !resource->valid()) {
        qWarning() << "KisResourceCacheDb::addResource: The resource is not valid";
        return false;
    }

    // Check whether it already exists
    if (hasResource(storage, resource, resourceType)) {
        int resourceId = resourceIdForResource(resource, resourceType);
        if (resourceNeedsUpdating(resourceId, timestamp)) {
            r = addResourceVersion(resourceId, timestamp, storage, resource);
        }
    }
    else {
        QSqlQuery q;
        r = q.prepare("INSERT INTO resources "
                      "(resource_type_id, name, filename, tooltip, thumbnail, status)"
                      "VALUES"
                      "((SELECT id FROM resource_types WHERE name = :resource_type), :name, :filename, :tooltip, :thumbnail, :status);");

        if (!r) {
            qWarning() << "Could not prepare addResource statement" << q.lastError();
            return r;
        }

        q.bindValue(":resource_type", resourceType);
        q.bindValue(":name", resource->name());
        q.bindValue(":filename", resource->filename());
        q.bindValue(":tooltip", i18n(resource->name().toUtf8()));

        QByteArray ba;
        QBuffer buf(&ba);
        buf.open(QBuffer::WriteOnly);
        resource->image().save(&buf, "PNG");
        buf.close();
        q.bindValue(":thumbnail", ba);

        q.bindValue(":status", 1);

        r = q.exec();
        if (!r) {
            qWarning() << "Could not execute addResource statement" << q.boundValues() << q.lastError();
            return r;
        }
    }
    // Then add a new version
    int resourceId = resourceIdForResource(resource, resourceType);
    QSqlQuery q;
    r = q.prepare("INSERT INTO versioned_resources "
                  "(resource_id, storage_id, version, location, datestamp, deleted, checksum)"
                  "VALUES"
                  "(:resource_id"
                  ",    (SELECT id FROM storages "
                  "      WHERE location = :storage_location)"
                  ", 1"
                  ", :location"
                  ", :datestamp"
                  ", 0"
                  ", :checksum"
                  ");");

    if (!r) {
        qWarning() << "Could not prepare addResourceVersion statement" << q.lastError();
        return r;
    }

    q.bindValue(":resource_id", resourceId);
    q.bindValue(":storage_location", storage->location());
    q.bindValue(":location", resource->filename());
    q.bindValue(":datestamp", timestamp.toString()); // XXX: make the right format for sqlite to work with
    q.bindValue(":deleted", 0);
    q.bindValue(":checksum", resource->md5());

    r = q.exec();
    if (!r) {
        qWarning() << "Could not execute addResourceVersion statement" << q.boundValues() << q.lastError();
    }

    return r;
}

bool KisResourceCacheDb::addResources(KisResourceStorageSP storage, QString resourceType)
{
    QSharedPointer<KisResourceStorage::ResourceIterator> iter = storage->resources(resourceType);
    while(iter->hasNext()) {
        iter->next();
        KoResourceSP res = iter->resource();
        if (res) {
            if (!addResource(storage, iter->lastModified(), res, iter->type())) {
                qWarning() << "Could not add resource" << res->filename() << "to the database";
            }
        }
    }
    return true;
}

bool KisResourceCacheDb::hasTag(const QString &url, const QString &resourceType)
{
    QFile f(":/select_tag.sql");
    if (f.open(QFile::ReadOnly)) {
        QSqlQuery q;
        if (!q.prepare(f.readAll())) {
            qWarning() << "Could not read and prepare select_tag.sql" << q.lastError();
            return false;
        }
        q.bindValue(":url", url);
        q.bindValue(":resource_type", resourceType);
        if (!q.exec()) {
            qWarning() << "Could not query tags" << q.boundValues() << q.lastError();
        }
        return q.first();
    }
    qWarning() << "Could not open select_tag.sql";
    return false;
}

bool KisResourceCacheDb::addTag(const QString &resourceType, const QString url, const QString name, const QString comment)
{
    if (hasTag(url, resourceType)) {
        return true;
    }

    QSqlQuery q;
    if (!q.prepare("INSERT INTO tags\n"
                   "( url, name, comment, resource_type_id, active)\n"
                   "VALUES\n"
                   "( :url\n"
                   ", :name\n"
                   ", :comment\n"
                   ", (SELECT id\n"
                   "   FROM   resource_types\n"
                   "   WHERE  name = :resource_type)\n"
                   ", 1"
                   ");")) {
        qWarning() << "Could not prepare add tag statement" << q.lastError();
        return false;
    }

    q.bindValue(":url", url);
    q.bindValue(":name", name);
    q.bindValue(":comment", comment);
    q.bindValue(":resource_type", resourceType);

    if (!q.exec()) {
        qWarning() << "Could not insert tag" << q.boundValues() << q.lastError();
    }

    return true;
}

bool KisResourceCacheDb::addTags(KisResourceStorageSP storage, QString resourceType)
{
    QSharedPointer<KisResourceStorage::TagIterator> iter = storage->tags(resourceType);
    while(iter->hasNext()) {
        iter->next();
        if (!addTag(resourceType, iter->url(), iter->name(), iter->comment())) {
            qWarning() << "Could not add tag" << iter->url() << "to the database";
        }
        else {
            qDebug() << "Added tag" << iter->url();
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

