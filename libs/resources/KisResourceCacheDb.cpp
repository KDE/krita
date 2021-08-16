/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "KisResourceCacheDb.h"

#include <QtSql>
#include <QStandardPaths>
#include <QDir>
#include <QDirIterator>
#include <QStringList>
#include <QElapsedTimer>
#include <QDataStream>
#include <QByteArray>
#include <QMessageBox>

#include <KritaVersionWrapper.h>

#include <klocalizedstring.h>
#include <kbackup.h>

#include <kis_debug.h>
#include <KisUsageLogger.h>

#include "KisResourceLocator.h"
#include "KisResourceLoaderRegistry.h"

#include "ResourceDebug.h"
#include <kis_assert.h>

const QString dbDriver = "QSQLITE";

const QString KisResourceCacheDb::dbLocationKey { "ResourceCacheDbDirectory" };
const QString KisResourceCacheDb::resourceCacheDbFilename { "resourcecache.sqlite" };
const QString KisResourceCacheDb::databaseVersion { "0.0.12" };
QStringList KisResourceCacheDb::storageTypes { QStringList() };
QStringList KisResourceCacheDb::disabledBundles { QStringList() << "Krita_3_Default_Resources.bundle" };

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

QSqlError createDatabase(const QString &location)
{
    // NOTE: if the id's of Unknown and Memory in the database
    //       will change, and that will break the queries that
    //       remove Unknown and Memory storages on start-up.
    KisResourceCacheDb::storageTypes << KisResourceStorage::storageTypeToUntranslatedString(KisResourceStorage::StorageType(1))
                                     << KisResourceStorage::storageTypeToUntranslatedString(KisResourceStorage::StorageType(2))
                                     << KisResourceStorage::storageTypeToUntranslatedString(KisResourceStorage::StorageType(3))
                                     << KisResourceStorage::storageTypeToUntranslatedString(KisResourceStorage::StorageType(4))
                                     << KisResourceStorage::storageTypeToUntranslatedString(KisResourceStorage::StorageType(5))
                                     << KisResourceStorage::storageTypeToUntranslatedString(KisResourceStorage::StorageType(6))
                                     ;

    if (!QSqlDatabase::connectionNames().isEmpty()) {
        return QSqlError();
    }

    QDir dbLocation(location);
    if (!dbLocation.exists()) {
        dbLocation.mkpath(dbLocation.path());
    }

    QSqlDatabase db = QSqlDatabase::addDatabase(dbDriver);
    db.setDatabaseName(location + "/" + KisResourceCacheDb::resourceCacheDbFilename);

    if (!db.open()) {
        qWarning() << "Could not connect to resource cache database";
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
                                       << "metadata"
                                       << "tags_storages";

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
        QString schemaVersion = "0.0.0";
        QString kritaVersion = "Unknown";
        int creationDate = 0;

        if (dbTables.contains("version_information")) {
            // Verify the version number

            QSqlQuery q("SELECT database_version\n"
                        ",      krita_version\n"
                        ",      creation_date\n"
                        "FROM version_information\n"
                        "ORDER BY id\n"
                        "DESC\n"
                        "LIMIT 1;\n");

            if (!q.exec()) {
                qWarning() << "Could not retrieve version information from the database." << q.lastError();
                abort();
            }
            q.first();
            schemaVersion = q.value(0).toString();
            kritaVersion = q.value(1).toString();
            creationDate = q.value(2).toInt();

            QVersionNumber schemaVersionNumber = QVersionNumber::fromString(schemaVersion);
            QVersionNumber currentSchemaVersionNumber = QVersionNumber::fromString(KisResourceCacheDb::databaseVersion);

            if (QVersionNumber::compare(schemaVersionNumber, currentSchemaVersionNumber) != 0) {
                // XXX: Implement migration
                schemaIsOutDated = true;
                QMessageBox::critical(0, i18nc("@title:window", "Krita"), i18n("The resource database scheme has changed. Krita will backup your database and create a new database."));
                KisResourceLocator::instance()->saveTags();
                db.close();
                KBackup::numberedBackupFile(location + "/" + KisResourceCacheDb::resourceCacheDbFilename);
                QFile::remove(location + "/" + KisResourceCacheDb::resourceCacheDbFilename);
                db.open();
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
                qWarning() << "Could not create table" << table << q.lastError();
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
    QSqlError err = createDatabase(location);

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

    // Delete all storages that are no longer known to the resource locator (including the memory storages)
    deleteTemporaryResources();

    return s_valid;
}

int KisResourceCacheDb::resourceIdForResource(const QString &/*resourceName*/, const QString &resourceFileName, const QString &resourceType, const QString &storageLocation)
{
    //qDebug() << "resourceIdForResource" << resourceName << resourceFileName << resourceType << storageLocation;

    QSqlQuery q;

    if (!q.prepare("SELECT resources.id\n"
                   "FROM   resources\n"
                   ",      resource_types\n"
                   ",      storages\n"
                   "WHERE  resources.resource_type_id = resource_types.id\n"
                   "AND    storages.id = resources.storage_id\n"
                   "AND    storages.location = :storage_location\n"
                   "AND    resource_types.name = :resource_type\n"
                   "AND    resources.filename = :filename\n")) {
        qWarning() << "Could not read and prepare resourceIdForResource" << q.lastError();
        return -1;
    }

    q.bindValue(":filename", resourceFileName);
    q.bindValue(":resource_type", resourceType);
    q.bindValue(":storage_location", storageLocation);

    if (!q.exec()) {
        qWarning() << "Could not query resourceIdForResource" << q.boundValues() << q.lastError();
        return -1;
    }

    if (q.first()) {
        return q.value(0).toInt();
    }

    // couldn't be found in the `resources` table, but can still be in versioned_resources

    if (!q.prepare("SELECT versioned_resources.resource_id\n"
                   "FROM   resources\n"
                   ",      resource_types\n"
                   ",      versioned_resources\n"
                   ",      storages\n"
                   "WHERE  resources.resource_type_id = resource_types.id\n"    // join resources and resource_types by resource id
                   "AND    versioned_resources.resource_id = resources.id\n"    // join versioned_resources and resources by resource id
                   "AND    storages.id = versioned_resources.storage_id\n"      // join storages and versioned_resources by storage id
                   "AND    storages.location = :storage_location\n"             // storage location must be the same as asked for
                   "AND    resource_types.name = :resource_type\n"              // resource type must be the same as asked for
                   "AND    versioned_resources.filename = :filename\n")) {      // filename must be the same as asked for
        qWarning() << "Could not read and prepare resourceIdForResource (in versioned resources)" << q.lastError();
        return -1;
    }

    q.bindValue(":filename", resourceFileName);
    q.bindValue(":resource_type", resourceType);
    q.bindValue(":storage_location", storageLocation);

    if (!q.exec()) {
        qWarning() << "Could not query resourceIdForResource (in versioned resources)" << q.boundValues() << q.lastError();
        return -1;
    }

    if (q.first()) {
        return q.value(0).toInt();
    }

    // commenting out, because otherwise it spams the console on every new resource in the local resources folder
    // qWarning() << "Could not find resource" << resourceName << resourceFileName << resourceType << storageLocation;
    return -1;

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


    r = addResourceVersionImpl(resourceId, timestamp, storage, resource);

    if (!r) return r;

    r = makeResourceTheCurrentVersion(resourceId, resource);

    return r;
}

bool KisResourceCacheDb::addResourceVersionImpl(int resourceId, QDateTime timestamp, KisResourceStorageSP storage, KoResourceSP resource)
{
    bool r = false;

    // Create the new version. The resource is expected to have an updated version number, or
    // this will fail on the unique index on resource_id, storage_id and version.
    //
    // This function **only** adds to the versioned_resources table.
    // The resources table should be updated by the caller manually using
    // updateResourceTableForResourceIfNeeded()

    Q_ASSERT(resource->version() >= 0);

    QSqlQuery q;
    r = q.prepare("INSERT INTO versioned_resources \n"
                  "(resource_id, storage_id, version, filename, timestamp, md5sum)\n"
                  "VALUES\n"
                  "( :resource_id\n"
                  ", (SELECT id \n"
                  "   FROM   storages \n"
                  "   WHERE  location = :storage_location)\n"
                  ", :version\n"
                  ", :filename\n"
                  ", :timestamp\n"
                  ", :md5sum\n"
                  ");");

    if (!r) {
        qWarning() << "Could not prepare addResourceVersion statement" << q.lastError();
        return r;
    }

    q.bindValue(":resource_id", resourceId);
    q.bindValue(":storage_location", KisResourceLocator::instance()->makeStorageLocationRelative(storage->location()));
    q.bindValue(":version", resource->version());
    q.bindValue(":filename", resource->filename());
    q.bindValue(":timestamp", timestamp.toSecsSinceEpoch());
    KIS_SAFE_ASSERT_RECOVER_NOOP(!resource->md5Sum().isEmpty());
    q.bindValue(":md5sum", resource->md5Sum());
    r = q.exec();
    if (!r) {

        qWarning() << "Could not execute addResourceVersionImpl statement" << q.lastError() << resourceId << storage->name() << storage->location() << resource->name() << resource->filename() << "version" << resource->version();
        return r;
    }

    return r;
}

bool KisResourceCacheDb::removeResourceVersionImpl(int resourceId, int version, KisResourceStorageSP storage)
{
    bool r = false;

    // Remove a version of the resource. This function **only** removes data from
    // the versioned_resources table. The resources table should be updated by
    // the caller manually using updateResourceTableForResourceIfNeeded()

    QSqlQuery q;
    r = q.prepare("DELETE FROM versioned_resources \n"
                  "WHERE resource_id = :resource_id\n"
                  "AND version = :version\n"
                  "AND storage_id = (SELECT id \n"
                  "                  FROM   storages \n"
                  "                  WHERE  location = :storage_location);");

    if (!r) {
        qWarning() << "Could not prepare removeResourceVersionImpl statement" << q.lastError();
        return r;
    }

    q.bindValue(":resource_id", resourceId);
    q.bindValue(":storage_location", KisResourceLocator::instance()->makeStorageLocationRelative(storage->location()));
    q.bindValue(":version", version);
    r = q.exec();
    if (!r) {

        qWarning() << "Could not execute removeResourceVersionImpl statement" << q.lastError() << resourceId << storage->name() << storage->location() << "version" << version;
        return r;
    }

    return r;
}

bool KisResourceCacheDb::updateResourceTableForResourceIfNeeded(int resourceId, const QString &resourceType, KisResourceStorageSP storage)
{
    bool r = false;

    int maxVersion = -1;
    {
        QSqlQuery q;
        r = q.prepare("SELECT MAX(version)\n"
                      "FROM   versioned_resources\n"
                      "WHERE  resource_id = :resource_id;");
        if (!r) {
            qWarning() << "Could not prepare findMaxVersion statement" << q.lastError();
            return r;
        }

        q.bindValue(":resource_id", resourceId);

        r = q.exec();
        if (!r) {
            qWarning() << "Could not execute findMaxVersion query" << q.boundValues() << q.lastError();
            return r;
        }

        r = q.first();
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(r, false);

        maxVersion = q.value(0).toInt();
    }

    QString maxVersionFilename;
    {
        QSqlQuery q;
        r = q.prepare("SELECT filename\n"
                      "FROM   versioned_resources\n"
                      "WHERE  resource_id = :resource_id\n"
                      "AND    version = :version;");
        if (!r) {
            qWarning() << "Could not prepare findMaxVersionFilename statement" << q.lastError();
            return r;
        }

        q.bindValue(":resource_id", resourceId);
        q.bindValue(":version", maxVersion);

        r = q.exec();
        if (!r) {
            qWarning() << "Could not execute findMaxVersionFilename query" << q.boundValues() << q.lastError();
            return r;
        }

        if (!q.first()) {
            return removeResourceCompletely(resourceId);
        } else {
            maxVersionFilename = q.value(0).toString();
        }
    }

    int currentVersion = -1;
    {
        QSqlQuery q;
        r = q.prepare("SELECT MAX(versioned_resources.version)\n"
                      "FROM   versioned_resources\n"
                      "WHERE  versioned_resources.id = :resource_id;");
        if (!r) {
            qWarning() << "Could not prepare findMaxVersion statement" << q.lastError();
            return r;
        }

        q.bindValue(":resource_id", resourceId);

        r = q.exec();
        if (!r) {
            qWarning() << "Could not execute findMaxVersion query" << q.boundValues() << q.lastError();
            return r;
        }

        r = q.first();
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(r, false);

        currentVersion = q.value(0).toInt();
    }

    if (currentVersion != maxVersion) {
        const QString url = resourceType + "/" + maxVersionFilename;
        KoResourceSP resource = storage->resource(url);
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(resource, false);
        resource->setVersion(maxVersion);
        resource->setMD5Sum(storage->resourceMd5(url));
        r = makeResourceTheCurrentVersion(resourceId, resource);
    }

    return r;
}

bool KisResourceCacheDb::makeResourceTheCurrentVersion(int resourceId, KoResourceSP resource)
{
    bool r = false;

    QSqlQuery q;
    r = q.prepare("UPDATE resources\n"
                  "SET name    = :name\n"
                  ", filename  = :filename\n"
                  ", tooltip   = :tooltip\n"
                  ", thumbnail = :thumbnail\n"
                  ", status    = 1\n"
                  "WHERE id    = :id");
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
    resource->thumbnail().save(&buf, "PNG");
    buf.close();
    q.bindValue(":thumbnail", ba);
    q.bindValue(":id", resourceId);

    r = q.exec();
    if (!r) {
        qWarning() << "Could not update resource" << q.boundValues() << q.lastError();
    }

    return r;
}

bool KisResourceCacheDb::removeResourceCompletely(int resourceId)
{
    bool r = false;

    {
        QSqlQuery q;
        r = q.prepare("DELETE FROM versioned_resources \n"
                      "WHERE resource_id = :resource_id;");

        if (!r) {
            qWarning() << "Could not prepare removeResourceCompletely1 statement" << q.lastError();
            return r;
        }

        q.bindValue(":resource_id", resourceId);
        r = q.exec();
        if (!r) {
            qWarning() << "Could not execute removeResourceCompletely1 statement" << q.lastError() << resourceId;
            return r;
        }
    }

    {
        QSqlQuery q;
        r = q.prepare("DELETE FROM resources \n"
                      "WHERE id = :resource_id;");

        if (!r) {
            qWarning() << "Could not prepare removeResourceCompletely2 statement" << q.lastError();
            return r;
        }

        q.bindValue(":resource_id", resourceId);
        r = q.exec();
        if (!r) {
            qWarning() << "Could not execute removeResourceCompletely2 statement" << q.lastError() << resourceId;
            return r;
        }
    }

    {
        QSqlQuery q;
        r = q.prepare("DELETE FROM resource_tags \n"
                      "WHERE resource_id = :resource_id;");

        if (!r) {
            qWarning() << "Could not prepare removeResourceCompletely3 statement" << q.lastError();
            return r;
        }

        q.bindValue(":resource_id", resourceId);
        r = q.exec();
        if (!r) {
            qWarning() << "Could not execute removeResourceCompletely3 statement" << q.lastError() << resourceId;
            return r;
        }
    }

    return r;
}

bool KisResourceCacheDb::getResourceIdFromVersionedFilename(QString filename, QString resourceType, int &outResourceId)
{
    QSqlQuery q;

    bool r = q.prepare("SELECT resource_id FROM versioned_resources\n"
                       ", resources\n"
                       ", resource_types\n"
                       "WHERE versioned_resources.filename = :filename\n" // bind to filename
                       "AND resources.id = versioned_resources.resource_id\n" // join resources + versioned_resources
                       "AND resource_types.id = resources.resource_type_id\n"  // join resources_types + resources
                       "AND resource_types.name = :resourceType;");

    if (!r) {
        qWarning() << "Could not prepare getResourceIdFromVersionedFilename statement" << q.lastError() << q.executedQuery();
        return r;
    }

    q.bindValue(":filename", filename);
    q.bindValue(":resourceType", resourceType);

    r = q.exec();
    if (!r) {
        qWarning() << "Could not execute getResourceIdFromVersionedFilename statement" << q.lastError() << filename << resourceType;
        return r;
    }

    r = q.first();
    if (r) {
        outResourceId = q.value("resource_id").toInt();
    }

    return r;
}

bool KisResourceCacheDb::getAllVersionsLocations(int resourceId, QStringList &outVersionsLocationsList)
{
    QSqlQuery q;
    bool r = q.prepare("SELECT filename FROM versioned_resources \n"
                  "WHERE resource_id = :resource_id;");

    if (!r) {
        qWarning() << "Could not prepare getAllVersionsLocations statement" << q.lastError();
        return r;
    }

    q.bindValue(":resource_id", resourceId);
    r = q.exec();
    if (!r) {
        qWarning() << "Could not execute getAllVersionsLocations statement" << q.lastError() << resourceId;
        return r;
    }

    outVersionsLocationsList = QStringList();
    while (q.next()) {
        outVersionsLocationsList << q.value("filename").toString();
    }

    return r;

}

bool KisResourceCacheDb::addResource(KisResourceStorageSP storage, QDateTime timestamp, KoResourceSP resource, const QString &resourceType)
{
    bool r = false;

    if (!s_valid) {
        qWarning() << "KisResourceCacheDb::addResource: The database is not valid";
        return false;
    }

    if (!resource || !resource->valid()) {
        qWarning() << "KisResourceCacheDb::addResource: The resource is not valid:" << resource->filename();
        // We don't care about invalid resources and will just ignore them.
        return true;
    }
    bool temporary = (storage->type() == KisResourceStorage::StorageType::Memory);

    // Check whether it already exists
    int resourceId = resourceIdForResource(resource->name(), resource->filename(), resourceType, KisResourceLocator::instance()->makeStorageLocationRelative(storage->location()));
    if (resourceId > -1) {
        return true;
    }

    // Check whether another resource with this MD5 already exists: then we set the resource status to hidden
    int active = 1;
    {
        QSqlQuery q;
        if (!q.prepare("SELECT count(*)\n"
                       "FROM   versioned_resources\n"
                       ",      resources\n"
                       "WHERE  versioned_resources.md5sum = :md5sum\n"
                       "AND    resources.id = versioned_resources.resource_id\n"
                       "AND    resources.status = 1")) {
            qWarning() << "Could not prepare select count from versioned_resources query" << q.lastError();
            return false;
        }
        q.bindValue(":md5sum", resource->md5Sum());
        if (!q.exec()) {
            qWarning() << "Could not execute select from resource_types query" << q.lastError();
            return false;
        }
        q.first();
        int rowCount = q.value(0).toInt();

        if (rowCount > 0) {
            active = 0;
        }

    }

    QSqlQuery q;
    r = q.prepare("INSERT INTO resources \n"
                  "(storage_id, resource_type_id, name, filename, tooltip, thumbnail, status, temporary, md5sum) \n"
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
                  ", :temporary\n"
                  ", :md5sum)");

    if (!r) {
        qWarning() << "Could not prepare addResource statement" << q.lastError();
        return r;
    }

    q.bindValue(":storage_location", KisResourceLocator::instance()->makeStorageLocationRelative(storage->location()));
    q.bindValue(":resource_type", resourceType);
    q.bindValue(":name", resource->name());
    q.bindValue(":filename", resource->filename());

    QString translationContext;
    if (storage->type() == KisResourceStorage::StorageType::Bundle) {
        translationContext = "./krita/data/bundles/" + KisResourceLocator::instance()->makeStorageLocationRelative(storage->location())
                + ":" + resourceType + "/" + resource->filename();
    } else if (storage->location() == "memory") {
        translationContext = "memory/" + resourceType + "/" + resource->filename();
    }
    else if (resource->filename().endsWith(".myb")) {
        translationContext = "./plugins/paintops/mypaint/brushes/" + resource->filename();
    } else {
        translationContext = "./krita/data/" + resourceType + "/" + resource->filename();
    }

    {
        QByteArray ctx = translationContext.toUtf8();
        QString translatedName = i18nc(ctx, resource->name().toUtf8());
        if (translatedName == resource->name()) {
            // Try using the file name without the file extension, and replaces '_' with spaces.
            QString altName = QFileInfo(resource->filename()).completeBaseName().replace('_', ' ');
            QString altTranslatedName = i18nc(ctx, altName.toUtf8());
            if (altName != altTranslatedName) {
                translatedName = altTranslatedName;
            }
        }
        q.bindValue(":tooltip", translatedName);
    }

    QByteArray ba;
    QBuffer buf(&ba);
    buf.open(QBuffer::WriteOnly);
    resource->image().save(&buf, "PNG");
    buf.close();
    q.bindValue(":thumbnail", ba);

    q.bindValue(":status", active);
    q.bindValue(":temporary", (temporary ? 1 : 0));
    q.bindValue(":md5sum", resource->md5Sum());

    r = q.exec();
    if (!r) {
        qWarning() << "Could not execute addResource statement" << q.lastError() << q.boundValues();
        return r;
    }
    resourceId = resourceIdForResource(resource->name(), resource->filename(), resourceType, KisResourceLocator::instance()->makeStorageLocationRelative(storage->location()));

    if (resourceId < 0) {

        qWarning() << "Adding to database failed, resource id after adding is " << resourceId << "! (Probable reason: the resource has the same filename, storage, resource type as an existing resource). Resource is: "
                   << resource->name()
                   << resource->filename()
                   << resourceType
                   << KisResourceLocator::instance()->makeStorageLocationRelative(storage->location());
        return false;
    }

    resource->setResourceId(resourceId);


    if (!addResourceVersionImpl(resourceId, timestamp, storage, resource)) {
        qWarning() << "Could not add resource version" << resource;
        return false;
    }

    return addMetaDataForId(resource->metadata(), resource->resourceId(), "resources");

}

bool KisResourceCacheDb::addResources(KisResourceStorageSP storage, QString resourceType)
{
    QSqlDatabase::database().transaction();
    QSharedPointer<KisResourceStorage::ResourceIterator> iter = storage->resources(resourceType);
    while (iter->hasNext()) {
        iter->next();

        QSharedPointer<KisResourceStorage::ResourceIterator> verIt =
            iter->versions();

        int resourceId = -1;

        while (verIt->hasNext()) {
            verIt->next();

            KoResourceSP resource = verIt->resource();
            if (resource && resource->valid()) {
                resource->setVersion(verIt->guessedVersion());
                resource->setMD5Sum(storage->resourceMd5(verIt->url()));

                if (resourceId < 0) {
                    if (addResource(storage, iter->lastModified(), resource, iter->type())) {
                        resourceId = resource->resourceId();
                    } else {
                        qWarning() << "Could not add resource" << resource->filename() << "to the database";
                    }
                } else {
                    if (!addResourceVersion(resourceId, iter->lastModified(), storage, resource)) {
                        qWarning() << "Could not add resource version" << resource->filename() << "to the database";
                    }
                }
            }
        }
    }
    QSqlDatabase::database().commit();
    return true;
}

bool KisResourceCacheDb::setResourceActive(int resourceId, bool active)
{
    if (resourceId < 0) {
        qWarning() << "Invalid resource id; cannot remove resource";
        return false;
    }
    QSqlQuery q;
    bool r = q.prepare("UPDATE resources\n"
                       "SET    status = :status\n"
                       "WHERE  id = :resource_id");
    if (!r) {
        qWarning() << "Could not prepare removeResource query" << q.lastError();
    }
    q.bindValue(":status", active);
    q.bindValue(":resource_id", resourceId);
    if (!q.exec()) {
        qWarning() << "Could not update resource" << resourceId << "to  inactive" << q.lastError();
        return false;
    }

    return true;
}

bool KisResourceCacheDb::tagResource(KisResourceStorageSP storage, const QString &resourceName, const QString &resourceFileName, KisTagSP tag, const QString &resourceType)
{
    // Get resource id
    int resourceId = resourceIdForResource(resourceName, resourceFileName, resourceType, KisResourceLocator::instance()->makeStorageLocationRelative(storage->location()));

    if (resourceId < 0) {
        qWarning() << "Could not find resource to tag" << KisResourceLocator::instance()->makeStorageLocationRelative(storage->location())  << resourceName << resourceFileName << resourceType;
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

bool KisResourceCacheDb::linkTagToStorage(const QString &url, const QString &resourceType, const QString &storageLocation)
{
    QSqlQuery q;
    if (!q.prepare("INSERT INTO tags_storages\n"
                   "(tag_id, storage_id)\n"
                   "VALUES\n"
                   "(\n"
                   " ( SELECT id\n"
                   "   FROM  tags\n"
                   "   WHERE url = :url\n"
                   "   AND   resource_type_id = (SELECT id \n"
                   "                              FROM   resource_types\n"
                   "                              WHERE  name = :resource_type)"
                   " )\n"
                   ",( SELECT id\n"
                   "   FROM   storages\n"
                   "   WHERE  location = :storage_location\n"
                   " )\n"
                   ");")) {
        qWarning() << "Could not prepare add tag/storage statement" << q.lastError();
        return false;
    }

    q.bindValue(":url", url);
    q.bindValue(":resource_type", resourceType);
    q.bindValue(":storage_location", KisResourceLocator::instance()->makeStorageLocationRelative(storageLocation));

    if (!q.exec()) {
        qWarning() << "Could not insert tag/storage link" << q.boundValues() << q.lastError();
        return false;
    }
    return true;
}


bool KisResourceCacheDb::addTag(const QString &resourceType, const QString storageLocation, const QString url, const QString name, const QString comment, const QString &filename)
{

    if (hasTag(url, resourceType)) {
        // Check whether this storage is already registered for this tag
        QSqlQuery q;
        if (!q.prepare("SELECT storages.location\n"
                       "FROM   tags_storages\n"
                       ",      tags\n"
                       ",      storages\n"
                       "WHERE  tags.id = tags_storages.tag_id\n"
                       "AND    storages.id = tags_storages.storage_id\n"
                       "AND    tags.resource_type_id = (SELECT id\n"
                       "                                FROM   resource_types\n"
                       "                                WHERE  name = :resource_type)\n"
                       "AND    tags.url = :url"))
        {
            qWarning() << "Could not prepare select tags from tags_storages query" << q.lastError();
        }

        q.bindValue(":url", url);
        q.bindValue(":resource_type", resourceType);

        if (!q.exec()) {
            qWarning() << "Could not execute tags_storages query" << q.boundValues() << q.lastError();
        }

        // If this tag is not yet linked to the storage, link it
        if (!q.first()) {
            return linkTagToStorage(url, resourceType, storageLocation);
        }

        return true;
    }

    // Insert the tag
    {
        QSqlQuery q;
        if (!q.prepare("INSERT INTO tags\n"
                       "(url, name, comment, resource_type_id, active, filename)\n"
                       "VALUES\n"
                       "( :url\n"
                       ", :name\n"
                       ", :comment\n"
                       ", (SELECT id\n"
                       "   FROM   resource_types\n"
                       "   WHERE  name = :resource_type)\n"
                       ", 1\n"
                       ", :filename"
                       ");")) {
            qWarning() << "Could not prepare insert tag statement" << q.lastError();
            return false;
        }



        q.bindValue(":url", url);
        q.bindValue(":name", name);
        q.bindValue(":comment", comment);
        q.bindValue(":resource_type", resourceType);
        q.bindValue(":filename", filename);


        if (!q.exec()) {
            qWarning() << "Could not insert tag" << q.boundValues() << q.lastError();
        }
    }

    linkTagToStorage(url, resourceType, storageLocation);

    return true;
}

bool KisResourceCacheDb::addTags(KisResourceStorageSP storage, QString resourceType)
{
    QSqlDatabase::database().transaction();
    QSharedPointer<KisResourceStorage::TagIterator> iter = storage->tags(resourceType);
    while(iter->hasNext()) {
        iter->next();
        if (!addTag(resourceType, storage->location(), iter->url(), iter->name(), iter->comment(), iter->filename())) {
            qWarning() << "Could not add tag" << iter->url() << "to the database";
        }
        if (!iter->tag()->defaultResources().isEmpty()) {
            Q_FOREACH(const QString &resourceFileName, iter->tag()->defaultResources()) {
                QString resourceName = resourceFileName;

                if (resourceName.contains("_default")) {
                    resourceName = resourceName.remove("_default");
                }

                //qDebug() << "Tagging" << storage << QFileInfo(resourceName).baseName() << resourceFileName <<  resourceType << "with tag" << iter->url();

                if (!tagResource(storage, QFileInfo(resourceName).baseName(), resourceFileName, iter->tag(), resourceType)) {
                    qWarning() << "Could not tag resource" << QFileInfo(resourceName).baseName() << "from" << storage->name() << "filename" << resourceName << "with tag" << iter->url();
                }
            }
        }
    }
    QSqlDatabase::database().commit();
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
        q.bindValue(":location", KisResourceLocator::instance()->makeStorageLocationRelative(storage->location()));
        r = q.exec();
        if (!r) {
            qWarning() << "Could not select from storages";
            return r;
        }
        if (q.first()) {
            debugResource << "Storage already exists" << storage;
            return true;
        }
    }

    // Insert the storage;
    {
        QSqlQuery q;

        r = q.prepare("INSERT INTO storages\n "
                      "(storage_type_id, location, timestamp, pre_installed, active, thumbnail)\n"
                      "VALUES\n"
                      "(:storage_type_id, :location, :timestamp, :pre_installed, :active, :thumbnail);");

        if (!r) {
            qWarning() << "Could not prepare query" << q.lastError();
            return r;
        }

        q.bindValue(":storage_type_id", static_cast<int>(storage->type()));
        q.bindValue(":location", KisResourceLocator::instance()->makeStorageLocationRelative(storage->location()));
        q.bindValue(":timestamp", storage->timestamp().toSecsSinceEpoch());
        q.bindValue(":pre_installed", preinstalled ? 1 : 0);
        q.bindValue(":active", !disabledBundles.contains(storage->name()));
        QByteArray ba;
        QBuffer buf(&ba);
        buf.open(QBuffer::WriteOnly);
        storage->thumbnail().save(&buf, "PNG");
        buf.close();
        q.bindValue(":thumbnail", ba);

        r = q.exec();

        if (!r) qWarning() << "Could not execute query" << q.lastError();

    }

    // Insert the metadata
    {
        QStringList keys = storage->metaDataKeys();
        if (keys.size() > 0) {

            QSqlQuery q;
            if (!q.prepare("SELECT MAX(id)\n"
                           "FROM   storages\n")) {
                qWarning() << "Could not create select storages query for metadata" << q.lastError();
            }

            if (!q.exec()) {
                qWarning() << "Could not execute select storages query for metadata" << q.lastError();
            }

            q.first();
            int id = q.value(0).toInt();

            QMap<QString, QVariant> metadata;

            Q_FOREACH(const QString &key, storage->metaDataKeys()) {
                metadata[key] = storage->metaData(key);
            }

            addMetaDataForId(metadata, id, "storages");
        }
    }

    Q_FOREACH(const QString &resourceType, KisResourceLoaderRegistry::instance()->resourceTypes()) {
        if (!KisResourceCacheDb::addResources(storage, resourceType)) {
            qWarning() << "Failed to add all resources for storage" << storage;
            r = false;
        }
        if (!KisResourceCacheDb::addTags(storage, resourceType)) {
            qWarning() << "Failed to add all tags for storage" << storage;
        }
    }

    return r;
}

bool KisResourceCacheDb::deleteStorage(QString location)
{
    // location is already relative
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
        q.bindValue(":location", location);
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
        q.bindValue(":location", location);
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
        q.bindValue(":location", location);
        if (!q.exec()) {
            qWarning() << "Could not execute delete storages query" << q.lastError();
            return false;
        }
    }
    return true;
}

bool KisResourceCacheDb::deleteStorage(KisResourceStorageSP storage)
{
    return deleteStorage(KisResourceLocator::instance()->makeStorageLocationRelative(storage->location()));
}

namespace {
struct ResourceVersion : public boost::less_than_comparable<ResourceVersion>
{
    int resourceId = -1;
    int version = -1;
    QDateTime timestamp;
    QString url;

    bool operator<(const ResourceVersion &rhs) const {
        return resourceId < rhs.resourceId ||
                (resourceId == rhs.resourceId && version < rhs.version);
    }

    struct CompareByResourceId {
        bool operator() (const ResourceVersion &lhs, const ResourceVersion &rhs) const {
            return lhs.resourceId < rhs.resourceId;
        }
    };


};

QDebug operator<<(QDebug dbg, const ResourceVersion &ver)
{
    dbg.nospace() << "ResourceVersion("
                  << ver.resourceId << ", "
                  << ver.version << ", "
                  << ver.timestamp << ", "
                  << ver.url << ")";

    return dbg.space();
}
}

bool KisResourceCacheDb::synchronizeStorage(KisResourceStorageSP storage)
{
    QElapsedTimer t;
    t.start();

    QSqlDatabase::database().transaction();

    if (!s_valid) {
        qWarning() << "KisResourceCacheDb::addResource: The database is not valid";
        return false;
    }

    bool success = true;

    // Find the storage in the database
    QSqlQuery q;
    if (!q.prepare("SELECT id\n"
                   ",      timestamp\n"
                   ",      pre_installed\n"
                   "FROM   storages\n"
                   "WHERE  location = :location\n")) {
        qWarning() << "Could not prepare storage timestamp statement" << q.lastError();
    }

    q.bindValue(":location", KisResourceLocator::instance()->makeStorageLocationRelative(storage->location()));
    if (!q.exec()) {
        qWarning() << "Could not execute storage timestamp statement" << q.boundValues() << q.lastError();
    }

    if (!q.first()) {
        // This is a new storage, the user must have dropped it in the path before restarting Krita, so add it.
        debugResource << "Adding storage to the database:" << storage;
        if (!addStorage(storage, false)) {
            qWarning() << "Could not add new storage" << storage->name() << "to the database";
            success = false;
        }
        return true;
    }

    storage->setStorageId(q.value("id").toInt());

    /// We compare resource versions one-by-one because the storage may have multiple
    /// versions of them

    Q_FOREACH(const QString &resourceType, KisResourceLoaderRegistry::instance()->resourceTypes()) {

        /// Firstly, fetch information about the existing resources
        /// in the storage

        QVector<ResourceVersion> resourcesInStorage;

        /// A fake resourceId to group resources which are not yet present
        /// in the database. This value is always negative, therefore it
        /// cannot overlap with normal ids.

        int nextInexistentResourceId = std::numeric_limits<int>::min();

        QSharedPointer<KisResourceStorage::ResourceIterator> iter = storage->resources(resourceType);
        while (iter->hasNext()) {
            iter->next();

            const int firstResourceVersionPosition = resourcesInStorage.size();

            int detectedResourceId = nextInexistentResourceId;
            QSharedPointer<KisResourceStorage::ResourceIterator> verIt =
                    iter->versions();

            while (verIt->hasNext()) {
                verIt->next();

                // verIt->url() contains paths like "brushes/ink.png" or "brushes/subfolder/splash.png".
                // we need to cut off the first part and get "ink.png" in the first case,
                // but "subfolder/splash.png" in the second case in order for subfolders to work
                // so it cannot just use QFileInfo(verIt->url()).fileName() here.
                QString path = QDir::fromNativeSeparators(verIt->url()); // make sure it uses Unix separators
                int folderEndIdx = path.indexOf("/");
                QString properFilenameWithSubfolders = path.right(path.length() - folderEndIdx - 1);
                int id = resourceIdForResource("", properFilenameWithSubfolders,
                                               verIt->type(),
                                               KisResourceLocator::instance()->makeStorageLocationRelative(storage->location()));

                ResourceVersion item;
                item.url = verIt->url();
                item.version = verIt->guessedVersion();

                // we use lower precision than the normal QDateTime
                item.timestamp = QDateTime::fromSecsSinceEpoch(verIt->lastModified().toSecsSinceEpoch());

                item.resourceId = id;

                if (detectedResourceId < 0 && id >= 0) {
                    detectedResourceId = id;
                }

                resourcesInStorage.append(item);
            }

            /// Assign the detected resource id to all the versions of
            /// this resource (if they are not present in the database).
            /// If no id has been detected, then a fake one will be assigned.

            for (int i = firstResourceVersionPosition; i < resourcesInStorage.size(); i++) {
                if (resourcesInStorage[i].resourceId < 0) {
                    resourcesInStorage[i].resourceId = detectedResourceId;
                }
            }

            nextInexistentResourceId++;
        }


        /// Secondly, fetch the resources present in the database

        QVector<ResourceVersion> resourcesInDatabase;

        QSqlQuery q;
        q.setForwardOnly(true);
        if (!q.prepare("SELECT versioned_resources.resource_id, versioned_resources.filename, versioned_resources.version, versioned_resources.timestamp\n"
                       "FROM   versioned_resources\n"
                       ",      resource_types\n"
                       ",      resources\n"
                       "WHERE  resources.resource_type_id = resource_types.id\n"
                       "AND    resources.id = versioned_resources.resource_id\n"
                       "AND    resource_types.name = :resource_type\n"
                       "AND    versioned_resources.storage_id == :storage_id")) {
            qWarning() << "Could not prepare resource by type query" << q.lastError();
            success = false;
            continue;
        }

        q.bindValue(":resource_type", resourceType);
        q.bindValue(":storage_id", int(storage->storageId()));

        if (!q.exec()) {
            qWarning() << "Could not exec resource by type query" << q.boundValues() << q.lastError();
            success = false;
            continue;
        }

        while (q.next()) {
            ResourceVersion item;
            item.url = resourceType + "/" + q.value(1).toString();
            item.version = q.value(2).toInt();
            item.timestamp = QDateTime::fromSecsSinceEpoch(q.value(3).toInt());
            item.resourceId = q.value(0).toInt();

            resourcesInDatabase.append(item);
        }

        QSet<int> resourceIdForUpdate;

        std::sort(resourcesInStorage.begin(), resourcesInStorage.end());
        std::sort(resourcesInDatabase.begin(), resourcesInDatabase.end());

        auto itA = resourcesInStorage.begin();
        auto endA = resourcesInStorage.end();

        auto itB = resourcesInDatabase.begin();
        auto endB = resourcesInDatabase.end();

        /// The head of itA array contains some resources with fake
        /// (negative) resourceId. These resources are obviously new
        /// resources and should be added to the cache database.

        while (itA != endA) {
            if (itA->resourceId >= 0) break;

            KoResourceSP res = storage->resource(itA->url);

            if (!res) {
                KisUsageLogger::log("Could not load resource" + itA->url);
                ++itA;
                continue;
            }

            res->setVersion(itA->version);
            res->setMD5Sum(storage->resourceMd5(itA->url));
            if (!res->valid()) {
                KisUsageLogger::log("Could not retrieve md5 for resource" + itA->url);
                ++itA;
                continue;
            }

            const bool retval = addResource(storage, itA->timestamp, res, resourceType);
            if (!retval) {
                KisUsageLogger::log("Could not add resource" + itA->url);
                ++itA;
                continue;
            }

            const int resourceId = res->resourceId();
            KIS_SAFE_ASSERT_RECOVER(resourceId >= 0) {
                KisUsageLogger::log("Could not get id for resource" + itA->url);
                ++itA;
                continue;
            }

            auto nextResource = std::upper_bound(itA, endA, *itA, ResourceVersion::CompareByResourceId());
            for (auto it = std::next(itA); it != nextResource; ++it) {
                KoResourceSP res = storage->resource(it->url);
                res->setVersion(it->version);
                res->setMD5Sum(storage->resourceMd5(it->url));
                if (!res->valid()) {
                    continue;
                }

                const bool retval = addResourceVersion(resourceId, it->timestamp, storage, res);
                KIS_SAFE_ASSERT_RECOVER(retval) {
                    KisUsageLogger::log("Could not add version for resource" + itA->url);
                    continue;
                }
            }

            itA = nextResource;
        }

        /// Now both arrays are sorted in resourceId/version/timestamp
        /// order. It lets us easily find the resources that are unique
        /// to the storage or database. If *itA < *itB, then the resource
        /// is present in the storage only and should be added to the
        /// database. If *itA > *itB, then the resource is present in
        /// the database only and should be removed (because it has been
        /// removed from the storage);

        while (itA != endA || itB != endB) {
            if ((itA != endA && itB != endB && *itA < *itB) ||
                    itB == endB) {

                // add a version to the database

                KoResourceSP res = storage->resource(itA->url);
                if (res) {
                    res->setVersion(itA->version);
                    res->setMD5Sum(storage->resourceMd5(itA->url));

                    const bool result = addResourceVersionImpl(itA->resourceId, itA->timestamp, storage, res);
                    KIS_SAFE_ASSERT_RECOVER_NOOP(result);

                    resourceIdForUpdate.insert(itA->resourceId);
                }
                ++itA;

            } else if ((itA != endA && itB != endB && *itA > *itB) ||
                       itA == endA) {

                // remove a version from the database
                const bool result = removeResourceVersionImpl(itB->resourceId, itB->version, storage);
                KIS_SAFE_ASSERT_RECOVER_NOOP(result);
                resourceIdForUpdate.insert(itB->resourceId);
                ++itB;

            } else {
                // resources are equal, just skip them
                ++itA;
                ++itB;
            }
        }


        /// In the main loop we modified the versioned_resource table only,
        /// now we should update the head resources table with the latest
        /// version of the resource (and upload the thumbnail as well)

        for (auto it = resourceIdForUpdate.begin(); it != resourceIdForUpdate.end(); ++it) {
            updateResourceTableForResourceIfNeeded(*it, resourceType, storage);
        }
    }

    QSqlDatabase::database().commit();
    debugResource << "Synchronizing the storages took" << t.elapsed() << "milliseconds for" << storage->location();

    return success;
}

void KisResourceCacheDb::deleteTemporaryResources()
{
    QSqlDatabase::database().transaction();

    QSqlQuery q;

    if (!q.prepare("DELETE FROM versioned_resources\n"
                   "WHERE  storage_id in (SELECT id\n"
                   "                      FROM   storages\n"
                   "                      WHERE  storage_type_id == :storage_type)"))
    {
        qWarning() << "Could not prepare delete versioned resources from Unknown or Memory storages query." << q.lastError();
    }

    q.bindValue(":storage_type", (int)KisResourceStorage::StorageType::Memory);

    if (!q.exec()) {
        qWarning() << "Could not execute delete versioned resources from Unknown or Memory storages query." << q.lastError();
    }

    if (!q.prepare("DELETE FROM resources\n"
                   "WHERE  storage_id in (SELECT id\n"
                   "                      FROM   storages\n"
                   "                      WHERE  storage_type_id  == :storage_type)"))
    {
        qWarning() << "Could not prepare delete resources from Unknown or Memory storages query." << q.lastError();
    }

    q.bindValue(":storage_type", (int)KisResourceStorage::StorageType::Memory);

    if (!q.exec()) {
        qWarning() << "Could not execute delete resources from Unknown or Memory storages query." << q.lastError();
    }


    if (!q.prepare("DELETE FROM versioned_resources\n"
                   "WHERE resource_id IN (SELECT id FROM resources\n"
                   "                      WHERE  temporary = 1)")) {
        qWarning() << "Could not prepare delete temporary versioned resources query." << q.lastError();
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

    if (!q.prepare("DELETE FROM storages\n"
                   "WHERE  storage_type_id  == :storage_type\n"))
    {
        qWarning() << "Could not prepare delete Unknown or Memory storages query." << q.lastError();
    }

    q.bindValue(":storage_type", (int)KisResourceStorage::StorageType::Memory);

    if (!q.exec()) {
        qWarning() << "Could not execute delete Unknown or Memory storages query." << q.lastError();
    }

    QSqlDatabase::database().commit();
}

bool KisResourceCacheDb::registerResourceType(const QString &resourceType)
{
    // Check whether the type already exists
    {
        QSqlQuery q;
        if (!q.prepare("SELECT count(*)\n"
                       "FROM   resource_types\n"
                       "WHERE  name = :resource_type\n")) {
            qWarning() << "Could not prepare select from resource_types query" << q.lastError();
            return false;
        }
        q.bindValue(":resource_type", resourceType);
        if (!q.exec()) {
            qWarning() << "Could not execute select from resource_types query" << q.lastError();
            return false;
        }
        q.first();
        int rowCount = q.value(0).toInt();
        if (rowCount > 0) {
            return true;
        }
    }
    // if not, add it
    QFile f(":/fill_resource_types.sql");
    if (f.open(QFile::ReadOnly)) {
        QString sql = f.readAll();
        QSqlQuery q(sql);
        q.addBindValue(resourceType);
        if (!q.exec()) {
            qWarning() << "Could not insert" << resourceType << q.lastError();
            return false;
        }
        return true;
    }
    qWarning() << "Could not open fill_resource_types.sql";
    return false;
}

QMap<QString, QVariant> KisResourceCacheDb::metaDataForId(int id, const QString &tableName)
{
    QMap<QString, QVariant> map;

    QSqlQuery q;
    q.setForwardOnly(true);
    if (!q.prepare("SELECT key\n"
                   ",      value\n"
                   "FROM   metadata\n"
                   "WHERE  foreign_id = :id\n"
                   "AND    table_name = :table")) {
        qWarning() << "Could not prepare metadata query" << q.lastError();
        return map;
    }

    q.bindValue(":id", id);
    q.bindValue(":table", tableName);

    if (!q.exec()) {
        qWarning() << "Could not execute metadata query" << q.lastError();
        return map;
    }

    while (q.next()) {
        QString key = q.value(0).toString();
        QByteArray ba = q.value(1).toByteArray();
        if (!ba.isEmpty()) {
            QDataStream ds(QByteArray::fromBase64(ba));
            QVariant value;
            ds >> value;
            map[key] = value;
        }
    }

    return map;
}

bool KisResourceCacheDb::updateMetaDataForId(const QMap<QString, QVariant> map, int id, const QString &tableName)
{
    QSqlDatabase::database().transaction();

    {
        QSqlQuery q;
        if (!q.prepare("DELETE FROM metadata\n"
                       "WHERE  foreign_id = :id\n"
                       "AND    table_name = :table\n")) {
            qWarning() << "Could not prepare delete metadata query" << q.lastError();
            return false;
        }

        q.bindValue(":id", id);
        q.bindValue(":table", tableName);

        if (!q.exec()) {
            QSqlDatabase::database().rollback();
            qWarning() << "Could not execute delete metadata query" << q.lastError();
            return false;

        }
    }

    if (addMetaDataForId(map, id, tableName)) {
        QSqlDatabase::database().commit();
    }
    else {
        QSqlDatabase::database().rollback();
    }
    return true;
}

bool KisResourceCacheDb::addMetaDataForId(const QMap<QString, QVariant> map, int id, const QString &tableName)
{

    QSqlQuery q;
    if (!q.prepare("INSERT INTO metadata\n"
                   "(foreign_id, table_name, key, value)\n"
                   "VALUES\n"
                   "(:id, :table, :key, :value)")) {
        QSqlDatabase::database().rollback();
        qWarning() << "Could not create insert metadata query" << q.lastError();
        return false;
    }

    QMap<QString, QVariant>::const_iterator iter = map.cbegin();
    while (iter != map.cend()) {
        q.bindValue(":id", id);
        q.bindValue(":table", tableName);
        q.bindValue(":key", iter.key());

        QVariant v = iter.value();
        if (!v.isNull() && v.isValid()) {
            QByteArray ba;
            QDataStream ds(&ba, QIODevice::WriteOnly);
            ds << v;
            ba = ba.toBase64();
            q.bindValue(":value", QString::fromLatin1(ba));

            if (!q.exec()) {
                qWarning() << "Could not insert metadata" << q.lastError();
                return false;
            }
        }
        ++iter;
    }
    return true;
}
