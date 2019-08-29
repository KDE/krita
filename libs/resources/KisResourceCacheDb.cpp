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
#include <QStringList>

#include <KritaVersionWrapper.h>
#include <klocalizedstring.h>
#include <kis_debug.h>
#include <KisUsageLogger.h>

#include "KisResourceLocator.h"
#include "KisResourceLoaderRegistry.h"

const QString dbDriver = "QSQLITE";

const QString KisResourceCacheDb::dbLocationKey {"ResourceCacheDbDirectory"};
const QString KisResourceCacheDb::resourceCacheDbFilename {"resourcecache.sqlite"};
const QString KisResourceCacheDb::databaseVersion {"0.0.2"};
QStringList KisResourceCacheDb::storageTypes { QStringList() };

bool KisResourceCacheDb::s_valid {false};
QString KisResourceCacheDb::s_lastError {QString()};

bool KisResourceCacheDb::isValid()
{
    return s_valid;
}

QString KisResourceCacheDb::lastError()
{
    return s_lastError;
}

QSqlError initDb(const QString &location)
{
    KisResourceCacheDb::storageTypes << i18n("Unknown")
                                     << i18n("Folder")
                                     << i18n("Bundle")
                                     << i18n("Abobe Brush Library")
                                     << i18n("Adobe Style Library")
                                     << i18n("Memory");

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

    //qDebug() << "QuerySize supported" << db.driver()->hasFeature(QSqlDriver::QuerySize);

    if (!db.open()) {
        infoResources << "Could not connect to resource cache database";
        return db.lastError();
    }

    QStringList tables = QStringList() << "version_information"
                                       << "storage_types"
                                       << "resource_types"
                                       << "storages"
                                       << "tags"
                                       << "resources"
                                       << "versioned_resources"
                                       << "resource_tags"
                                       << "resource_metadata";

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
        QString schemaVersion = "Unknown";
        QString kritaVersion = "Unknown";
        int creationDate = 0;


        if (dbTables.contains("version_information")) {
            // Verify the version number
            QFile f(":/get_version_information.sql");
            if (f.open(QFile::ReadOnly)) {
                QSqlQuery q(f.readAll());
                if (q.size() > 0) {
                    q.first();
                    schemaVersion = q.value(0).toString();
                    kritaVersion = q.value(1).toString();
                    creationDate = q.value(2).toInt();

                    if (schemaVersion != KisResourceCacheDb::databaseVersion) {
                        // XXX: Implement migration
                        schemaIsOutDated = true;
                        qFatal("Database schema is outdated, migration is needed. Database migration has NOT been implemented yet.");
                    }
                }
            }
            else {
                return QSqlError("Error executing SQL", "Could not open get_version_information.sql", QSqlError::StatementError);
            }
        }

        if (allTablesPresent && !schemaIsOutDated) {
            KisUsageLogger::log(QString("Database is up to date. Version: %1, created by Krita %2, at %3")
                                .arg(schemaVersion)
                                .arg(kritaVersion)
                                .arg(QDateTime::fromSecsSinceEpoch(creationDate).toString()));
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
    QStringList indexes = QStringList() << "storages" << "versioned_resources";

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
        if (dbTables.contains("storage_types")) {
            QSqlQuery q;
            if (!q.exec("DELETE * FROM storage_types;")) {
                qWarning() << "Could not clear table storage_types" << db.lastError();
            }
        }

        QFile f(":/fill_storage_types.sql");
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
            infoResources << "Filled lookup table storage_types";
        }
        else {
            return QSqlError("Error executing SQL", QString("Could not find SQL fill_storage_types.sql."), QSqlError::StatementError);
        }
    }

    {
        if (dbTables.contains("resource_types")) {
            QSqlQuery q;
            if (!q.exec("DELETE * FROM resource_types;")) {
                qWarning() << "Could not clear table resource_types" << db.lastError();
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
            q.addBindValue(QDateTime::currentDateTimeUtc().toSecsSinceEpoch());
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

    s_valid = !err.isValid();
    switch (err.type()) {
    case QSqlError::NoError:
        s_lastError = QString();
        break;
    case QSqlError::ConnectionError:
        s_lastError = QString("Could not initialize the resource cache database. Connection error: %1").arg(err.text());
        break;
    case QSqlError::StatementError:
        s_lastError = QString("Could not initialize the resource cache database. Statement error: %1").arg(err.text());
        break;
    case QSqlError::TransactionError:
        s_lastError = QString("Could not initialize the resource cache database. Transaction error: %1").arg(err.text());
        break;
    case QSqlError::UnknownError:
        s_lastError = QString("Could not initialize the resource cache database. Unknown error: %1").arg(err.text());
        break;
    }

    deleteTemporaryResources();

    return s_valid;
}

int KisResourceCacheDb::resourceIdForResource(const QString &resourceName, const QString &resourceType, const QString &storageLocation)
{
    QFile f(":/select_resource_id.sql");
    f.open(QFile::ReadOnly);
    QSqlQuery q;
    if (!q.prepare(f.readAll())) {
        qWarning() << "Could not read and prepare resourceIdForResource" << q.lastError();
        return -1;
    }

    q.bindValue(":name", resourceName);
    q.bindValue(":resource_type", resourceType);
    q.bindValue(":storage_location", storageLocation);
    if (!q.exec()) {
        qWarning() << "Could not query resourceIdForResource" << q.boundValues() << q.lastError();
        return -1;
    }
    if (!q.first()) {
        return -1;
    }
    return q.value(0).toInt();

}

bool KisResourceCacheDb::resourceNeedsUpdating(int resourceId, QDateTime timestamp)
{
    QSqlQuery q;
    if (!q.prepare("SELECT timestamp\n"
                   "FROM   versioned_resources\n"
                   "WHERE  resource_id = :resource_id\n"
                   "AND    version = (SELECT MAX(version)\n"
                   "                  FROM   versioned_resources\n"
                   "                  WHERE  resource_id = :resource_id);")) {
        qWarning() << "Could not prepare resourceNeedsUpdating statement" << q.lastError();
        return false;
    }

    q.bindValue(":resource_id", resourceId);

    if (!q.exec()) {
        qWarning() << "Could not query for the most recent timestamp" << q.boundValues() << q.lastError();
        return false;
    }

    if (!q.first()) {
        qWarning() << "Inconsistent database: could not find a version for resource with Id" << resourceId;
        return false;
    }

    QVariant resourceTimeStamp = q.value(0);

    if (!resourceTimeStamp.isValid()) {
        qWarning() << "Could not retrieve timestamp from versioned_resources" << resourceId;
        return false;
    }
    return (timestamp.toSecsSinceEpoch() > resourceTimeStamp.toInt());
}

bool KisResourceCacheDb::addResourceVersion(int resourceId, QDateTime timestamp, KisResourceStorageSP storage, KoResourceSP resource)
{
    bool r = false;

    // Create the new version
    {
        QSqlQuery q;
        r = q.prepare("INSERT INTO versioned_resources \n"
                      "(resource_id, storage_id, version, location, timestamp)\n"
                      "VALUES\n"
                      "( :resource_id\n"
                      ", (SELECT id FROM storages \n"
                      "      WHERE location = :storage_location)\n"
                      ", (SELECT MAX(version) + 1 FROM versioned_resources\n"
                      "      WHERE  resource_id = :resource_id)\n"
                      ", :location, :timestamp\n"
                      ");");

        if (!r) {
            qWarning() << "Could not prepare addResourceVersion statement" << q.lastError();
            return r;
        }

        q.bindValue(":resource_id", resourceId);
        q.bindValue(":storage_location", makeRelative(storage->location()));
        q.bindValue(":location", makeRelative(resource->filename()));
        q.bindValue(":timestamp", timestamp.toSecsSinceEpoch());

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
                      "SET name    = :name\n"
                      ", filename  = :filename\n"
                      ", tooltip   = :tooltip\n"
                      ", thumbnail = :thumbnail\n"
                      "WHERE id    = :id");
        if (!r) {
            qWarning() << "Could not prepare updateResource statement" << q.lastError();
            return r;
        }
        q.bindValue(":name", resource->name());
        q.bindValue(":filename", makeRelative(resource->filename()));
        q.bindValue(":tooltip", i18n(resource->name().toUtf8()));

        QByteArray ba;
        QBuffer buf(&ba);
        buf.open(QBuffer::WriteOnly);
        resource->image().save(&buf, "PNG");
        buf.close();
        q.bindValue(":thumbnail", ba);

        q.bindValue(":id", resourceId);

        r = q.exec();
        if (!r) {
            qWarning() << "Could not update resource" << q.boundValues() << q.lastError();
        }
    }

    return r;
}

bool KisResourceCacheDb::addResource(KisResourceStorageSP storage, QDateTime timestamp, KoResourceSP resource, const QString &resourceType, bool temporary)
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
    int resourceId = resourceIdForResource(resource->name(), resourceType, makeRelative(storage->location()));
    if (resourceId > -1) {
        if (resourceNeedsUpdating(resourceId, timestamp)) {
            r = addResourceVersion(resourceId, timestamp, storage, resource);
        }
    }
    else {
        QSqlQuery q;
        r = q.prepare("INSERT INTO resources \n"
                      "(storage_id, resource_type_id, name, filename, tooltip, thumbnail, status, temporary) \n"
                      "VALUES \n"
                      "((SELECT id "
                      "  FROM storages "
                      "  WHERE location = :storage_location)\n"
                      ", (SELECT id\n"
                      "   FROM resource_types\n"
                      "   WHERE name = :resource_type)\n"
                      ", :name\n"
                      ", :filename\n"
                      ", :tooltip\n"
                      ", :thumbnail\n"
                      ", :status\n"
                      ", :temporary);");

        if (!r) {
            qWarning() << "Could not prepare addResource statement" << q.lastError();
            return r;
        }

        q.bindValue(":storage_location", makeRelative(storage->location()));
        q.bindValue(":resource_type", resourceType);
        q.bindValue(":name", resource->name());
        q.bindValue(":filename", makeRelative(resource->filename()));
        q.bindValue(":tooltip", i18n(resource->name().toUtf8()));

        QByteArray ba;
        QBuffer buf(&ba);
        buf.open(QBuffer::WriteOnly);
        resource->image().save(&buf, "PNG");
        buf.close();
        q.bindValue(":thumbnail", ba);

        q.bindValue(":status", 1);
        q.bindValue(":temporary", (temporary ? 1 : 0));

        r = q.exec();
        if (!r) {
            qWarning() << "Could not execute addResource statement" << q.boundValues() << q.lastError();
            return r;
        }

        resourceId = resourceIdForResource(resource->name(), resourceType, makeRelative(storage->location()));
    }
    // Then add a new version
    QSqlQuery q;
    r = q.prepare("INSERT INTO versioned_resources "
                  "(resource_id, storage_id, version, location, timestamp) "
                  "VALUES "
                  "(:resource_id "
                  ",    (SELECT id FROM storages "
                  "      WHERE location = :storage_location) "
                  ", 1 "
                  ", :location "
                  ", :timestamp "
                  ");");

    if (!r) {
        qWarning() << "Could not prepare addResourceVersion statement" << q.lastError();
        return r;
    }

    q.bindValue(":resource_id", resourceId);
    q.bindValue(":storage_location", makeRelative(storage->location()));
    q.bindValue(":location", makeRelative(resource->filename()));
    q.bindValue(":timestamp", timestamp.toSecsSinceEpoch());

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
        KoResourceSP resource = iter->resource();
        if (resource) {
            if (!addResource(storage, iter->lastModified(), resource, iter->type())) {
                qWarning() << "Could not add resource" << makeRelative(resource->filename()) << "to the database";
            }
        }
    }
    return true;
}

bool KisResourceCacheDb::removeResource(int resourceId)
{
    if (resourceId < 0) {
        qWarning() << "Invalid resource id; cannot remove resource";
        return false;
    }
    QSqlQuery q;
    bool r = q.prepare("UPDATE resources\n"
                       "SET    status = 0\n"
                       "WHERE  id = :resource_id");
    if (!r) {
        qWarning() << "Could not prepare removeResource query" << q.lastError();
    }
    q.bindValue(":resource_id", resourceId);
    if (!q.exec()) {
        qWarning() << "Could not update resource" << resourceId << "to  inactive" << q.lastError();
        return false;
    }

    return true;
}

bool KisResourceCacheDb::tagResource(KisResourceStorageSP storage, const QString resourceName, KisTagSP tag, const QString &resourceType)
{
    // Get resource id
    int resourceId = resourceIdForResource(resourceName, resourceType, makeRelative(storage->location()));

    if (resourceId < 0) {
        qWarning() << "Could not find resource to tag" << makeRelative(storage->location())  << resourceName << resourceType;
        return false;
    }

    // Get tag id
    int tagId {-1};
    {
        QFile f(":/select_tag.sql");
        if (f.open(QFile::ReadOnly)) {
            QSqlQuery q;
            if (!q.prepare(f.readAll())) {
                qWarning() << "Could not read and prepare select_tag.sql" << q.lastError();
                return false;
            }
            q.bindValue(":url", tag->url());
            q.bindValue(":resource_type", resourceType);

            if (!q.exec()) {
                qWarning() << "Could not query tags" << q.boundValues() << q.lastError();
                return false;
            }

            if (!q.first()) {
                qWarning() << "Could not find tag" << q.boundValues() << q.lastError();
                return false;
            }

            tagId = q.value(0).toInt();
        }
    }

    QSqlQuery q;
    if (!q.prepare("INSERT INTO resource_tags\n"
                   "(resource_id, tag_id)\n"
                   "VALUES\n"
                   "(:resource_id, :tag_id);")) {
        qWarning() << "Could not prepare tagResource statement" << q.lastError();
        return false;
    }
    q.bindValue(":resource_id", resourceId);
    q.bindValue(":tag_id", tagId);
    if (!q.exec()) {
        qWarning() << "Could not execute tagResource stagement" << q.boundValues() << q.lastError();
        return false;
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
        if (!iter->tag()->defaultResources().isEmpty()) {
            Q_FOREACH(const QString &resourceName, iter->tag()->defaultResources()) {
                if (!tagResource(storage, resourceName, iter->tag(), resourceType)) {
                    qWarning() << "Could not tag resource" << resourceName << "with tag" << iter->url();
                }
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
        q.bindValue(":location", makeRelative(storage->location()));
        r = q.exec();
        if (!r) {
            qWarning() << "Could not select from storages";
            return r;
        }
        if (q.first()) {
            return true;
        }
    }

    {
        QSqlQuery q;

        r = q.prepare("INSERT INTO storages\n "
                      "(storage_type_id, location, timestamp, pre_installed, active)\n"
                      "VALUES\n"
                      "(:storage_type_id, :location, :timestamp, :pre_installed, :active);");

        if (!r) {
            qWarning() << "Could not prepare query" << q.lastError();
            return r;
        }

        q.bindValue(":storage_type_id", static_cast<int>(storage->type()));
        q.bindValue(":location", makeRelative(storage->location()));
        q.bindValue(":timestamp", storage->timestamp().toSecsSinceEpoch());
        q.bindValue(":pre_installed", preinstalled ? 1 : 0);
        q.bindValue(":active", 1);

        r = q.exec();

        if (!r) qWarning() << "Could not execute query" << q.lastError();

    }

    return r;
}

bool KisResourceCacheDb::deleteStorage(KisResourceStorageSP storage)
{
    {
        QSqlQuery q;
        if (!q.prepare("DELETE FROM resources\n"
                       "WHERE       id IN (SELECT versioned_resources.resource_id\n"
                       "                   FROM   versioned_resources\n"
                       "                   WHERE  versioned_resources.storage_id = (SELECT storages.id\n"
                       "                                                            FROM   storages\n"
                       "                                                            WHERE storages.location = :location)\n"
                       "                   );")) {
            qWarning() << "Could not prepare delete resources query in deleteStorage" << q.lastError();
            return false;
        }
        q.bindValue(":location", makeRelative(storage->location()));
        if (!q.exec()) {
            qWarning() << "Could not execute delete resources query in deleteStorage" << q.lastError();
            return false;
        }
    }

    {
        QSqlQuery q;
        if (!q.prepare("DELETE FROM versioned_resources\n"
                       "WHERE storage_id = (SELECT storages.id\n"
                       "                    FROM   storages\n"
                       "                    WHERE  storages.location = :location);")) {
            qWarning() << "Could not prepare delete versioned_resources query" << q.lastError();
            return false;
        }
        q.bindValue(":location", makeRelative(storage->location()));
        if (!q.exec()) {
            qWarning() << "Could not execute delete versioned_resources query" << q.lastError();
            return false;
        }
    }

    {
        QSqlQuery q;
        if (!q.prepare("DELETE FROM storages\n"
                       "WHERE location = :location;")) {
            qWarning() << "Could not prepare delete storages query" << q.lastError();
            return false;
        }
        q.bindValue(":location", makeRelative(storage->location()));
        if (!q.exec()) {
            qWarning() << "Could not execute delete storages query" << q.lastError();
            return false;
        }
    }
    return true;
}

bool KisResourceCacheDb::synchronizeStorage(KisResourceStorageSP storage)
{
    if (!s_valid) {
        qWarning() << "KisResourceCacheDb::addResource: The database is not valid";
        return false;
    }

    // Find the storage in the database
    QSqlQuery q;
    if (!q.prepare("SELECT id\n"
                   ",      timestamp\n"
                   ",      pre_installed\n"
                   "FROM   storages\n"
                   "WHERE  location = :location\n")) {
        qWarning() << "Could not prepare storage timestamp statement" << q.lastError();
    }

    q.bindValue(":location", makeRelative(storage->location()));
    if (!q.exec()) {
        qWarning() << "Could not execute storage timestamp statement" << q.boundValues() << q.lastError();
    }

    if (!q.first()) {
        // This is a new storage, the user must have dropped it in the path before restarting Krita, so add it.
        addStorage(storage, false);
    }

    // Only check the time stamp for container storages, not the contents
    if (storage->type() != KisResourceStorage::StorageType::Folder) {

//        qDebug() << storage->location() << "is not a folder, going to check timestamps. Database:"
//                 << q.value(1).toInt() << ", File:" << storage->timestamp().toSecsSinceEpoch();

        if (!q.value(0).isValid()) {
            qWarning() << "Could not retrieve timestamp for storage" << makeRelative(storage->location());
        }
        if (storage->timestamp().toSecsSinceEpoch() > q.value(1).toInt()) {
//            qDebug() << "Deleting" << storage->location();
            if (!deleteStorage(storage)) {
                qWarning() << "Could not delete storage" << makeRelative(storage->location());
            }
//            qDebug() << "Inserting" << storage->location();
            if (!addStorage(storage, q.value(2).toBool())) {
                qWarning() << "Could not add storage" << makeRelative(storage->location());
            }
        }

    }
    else {

        // Check whether everything in the storage is in the database
        QMap<QString, QStringList> typeResourceMap;
        Q_FOREACH(const QString &resourceType, KisResourceLoaderRegistry::instance()->resourceTypes()) {
            typeResourceMap.insert(resourceType, QStringList());
            QSharedPointer<KisResourceStorage::ResourceIterator> iter = storage->resources(resourceType);
            while(iter->hasNext()) {
                iter->next();
                KoResourceSP resource = iter->resource();
                typeResourceMap[resourceType] << iter->url();
                if (resource) {
                    if (!addResource(storage, iter->lastModified(), resource, iter->type())) {
                        qWarning() << "Could not add resource" << makeRelative(resource->filename()) << "to the database";
                    }
                }
            }
        }
        // Remove everything from the database which is no longer in the storage
        QList<int> resourceIdList;
        Q_FOREACH(const QString &resourceType, KisResourceLoaderRegistry::instance()->resourceTypes()) {
            QSqlQuery q;
            if (!q.prepare("SELECT resources.id, resources.filename\n"
                           "FROM   resources\n"
                           ",      resource_types\n"
                           "WHERE  resources.resource_type_id = resource_types.id\n"
                           "AND    resource_types.name == :resource_type;")) {
                qWarning() << "Could not prepare resource by type query" << q.lastError();
                continue;
            }
            q.bindValue(":resource_type", resourceType);
            if (!q.exec()) {
                qWarning() << "Could not exec resource by type query" << q.boundValues() << q.lastError();
                continue;
            }
            while (q.nextResult()) {
                if (!typeResourceMap[resourceType].contains(q.value(1).toString())) {
                    resourceIdList << q.value(0).toInt();
                }
            }
        }
        QSqlQuery deleteResources;
        if (!deleteResources.prepare("DELETE FROM resources WHERE id = :id")) {
            qWarning() << "Could not prepare delete Resources query";
        }
        QSqlQuery deleteResourceVersions;
        if (!deleteResourceVersions.prepare("DELETE FROM versioned_resources WHERE resource_id = :id")) {
            qWarning() << "Could not prepare delete Resources query";
        }

        Q_FOREACH(int id, resourceIdList) {
            deleteResourceVersions.bindValue(":id", id);
            if (!deleteResourceVersions.exec()) {
                qWarning() << "Could not delete resource version" << deleteResourceVersions.boundValues() << deleteResourceVersions.lastError();
            }

            deleteResources.bindValue(":id", id);
            if (!deleteResources.exec()) {
                qWarning() << "Could not delete resource" << deleteResources.boundValues() << deleteResources.lastError();
            }
        }
    }
    return true;
}

QString KisResourceCacheDb::makeRelative(QString location)
{
    location = location.remove(KisResourceLocator::instance()->resourceLocationBase());
    if (location.startsWith('/')) {
        location = location.remove(0, 1);
    }
    return location;
}

QString KisResourceCacheDb::makeAbsolute(const QString &location)
{
    return KisResourceLocator::instance()->resourceLocationBase() + '/' + location;
}

void KisResourceCacheDb::deleteTemporaryResources()
{
    QSqlQuery q;
    if (!q.prepare("DELETE FROM versioned_resources\n"
                   "WHERE resource_id IN (SELECT id FROM resources\n"
                   "                      WHERE  temporary = 1)")) {
        qWarning() << "Could not prepare delete temporary versioned resources query." << q.lastError();
        return;
    }

    if (!q.exec()) {
        qWarning() << "Could not execute delete temporary versioned resources query." << q.lastError();
    }

    if (!q.prepare("DELETE FROM resources\n"
                   "WHERE  temporary = 1")) {
        qWarning() << "Could not prepare delete temporary resources query." << q.lastError();
        return;
    }

    if (!q.exec()) {
        qWarning() << "Could not execute delete temporary resources query." << q.lastError();
    }
}

