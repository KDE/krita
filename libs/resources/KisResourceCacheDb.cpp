/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "KisResourceCacheDb.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QSqlDatabase>

#include <QBuffer>
#include <QVersionNumber>
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
#include <KisBackup.h>

#include <kis_debug.h>
#include <KisUsageLogger.h>

#include <KisSqlQueryLoader.h>
#include <KisDatabaseTransactionLock.h>
#include "KisResourceLocator.h"
#include "KisResourceLoaderRegistry.h"

#include "ResourceDebug.h"
#include <kis_assert.h>

#include <KisCppQuirks.h>

const QString dbDriver = "QSQLITE";
const QString METADATA_RESOURCES = "resources";
const QString METADATA_STORAGES = "storages";

const QString KisResourceCacheDb::resourceCacheDbFilename { "resourcecache.sqlite" };
const QString KisResourceCacheDb::databaseVersion { "0.0.18" };
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

// use in WHERE QSqlQuery clauses
// because if the string is null, the query will also have null there
// and every comparison with null is false, so the query won't find anything
// (especially important for storage location where empty string is common)
QString changeToEmptyIfNull(QString s)
{
    return s.isNull() ? QString("") : s;
}

bool updateSchemaVersion()
{
    QFile f(":/fill_version_information.sql");
    if (f.open(QFile::ReadOnly)) {
        QString sql = f.readAll();
        QSqlQuery q;
        if (!q.prepare(sql)) {
            warnDbMigration << "Could not prepare the schema information query" << q.lastError() << q.boundValues();
            return false;
        }
        q.addBindValue(KisResourceCacheDb::databaseVersion);
        q.addBindValue(KritaVersionWrapper::versionString());
        q.addBindValue(QDateTime::currentDateTimeUtc().toSecsSinceEpoch());
        if (!q.exec()) {
            warnDbMigration << "Could not insert the current version" << q.lastError() << q.boundValues();
            return false;
        }

        infoDbMigration << "Filled version table";
    }
    return true;
}

QSqlError runUpdateScriptFile(const QString &path, const QString &message)
{
    try {

        KisSqlQueryLoader loader(path);
        loader.exec();

    } catch (const KisSqlQueryLoader::FileException &e) {
        warnDbMigration.noquote() << "ERROR: Could not execute DB update step:" << message;
        warnDbMigration.noquote() << "       error" << e.message;
        warnDbMigration.noquote() << "       file:" << e.filePath;
        warnDbMigration.noquote() << "       file-error:" << e.fileErrorString;
        return
            QSqlError("Error executing SQL",
                QString("Could not find SQL file %1").arg(e.filePath),
                QSqlError::StatementError);
    } catch (const KisSqlQueryLoader::SQLException &e) {
        warnDbMigration.noquote() << "ERROR: Could not execute DB update step:" << message;
        warnDbMigration.noquote() << "       error" << e.message;
        warnDbMigration.noquote() << "       file:" << e.filePath;
        warnDbMigration.noquote() << "       statement:" << e.statementIndex;
        warnDbMigration.noquote() << "       sql-error:" << e.sqlError.text();
        return e.sqlError;
    }

    infoDbMigration << "Completed DB update step:" << message;
    return QSqlError();
}

QSqlError runUpdateScript(const QString &script, const QString &message)
{
    try {

        KisSqlQueryLoader loader("", script);
        loader.exec();

    } catch (const KisSqlQueryLoader::SQLException &e) {
        warnDbMigration.noquote() << "ERROR: Could execute DB update step:" << message;
        warnDbMigration.noquote() << "       error" << e.message;
        warnDbMigration.noquote() << "       sql-error:" << e.sqlError.text();
        return e.sqlError;
    }

    infoDbMigration << "Completed DB update step:" << message;
    return QSqlError();
}

QSqlError createDatabase(const QString &location)
{
    // NOTE: if the id's of Unknown and Memory in the database
    //       will change, and that will break the queries that
    //       remove Unknown and Memory storages on start-up.
    KisResourceCacheDb::storageTypes = QStringList {
        KisResourceStorage::storageTypeToUntranslatedString(KisResourceStorage::StorageType(1)),
        KisResourceStorage::storageTypeToUntranslatedString(KisResourceStorage::StorageType(2)),
        KisResourceStorage::storageTypeToUntranslatedString(KisResourceStorage::StorageType(3)),
        KisResourceStorage::storageTypeToUntranslatedString(KisResourceStorage::StorageType(4)),
        KisResourceStorage::storageTypeToUntranslatedString(KisResourceStorage::StorageType(5)),
        KisResourceStorage::storageTypeToUntranslatedString(KisResourceStorage::StorageType(6)),
        KisResourceStorage::storageTypeToUntranslatedString(KisResourceStorage::StorageType(7))};

    QDir dbLocation(location);
    if (!dbLocation.exists()) {
        dbLocation.mkpath(dbLocation.path());
    }

    std::optional<QSqlDatabase> existingDatabase =
        QSqlDatabase::database(QSqlDatabase::defaultConnection, false);

    const bool databaseConnectionExists = !QSqlDatabase::connectionNames().isEmpty()
        && existingDatabase->isValid() && existingDatabase->isOpen();

    if (databaseConnectionExists && existingDatabase->tables().contains("version_information")) {
        return QSqlError();
    }

    existingDatabase = std::nullopt;

    QSqlDatabase db;

    if (!databaseConnectionExists) {
        db = QSqlDatabase::addDatabase(dbDriver);
        db.setDatabaseName(location + "/" + KisResourceCacheDb::resourceCacheDbFilename);

        if (!db.open()) {
            warnDbMigration << "Could not connect to resource cache database";
            return db.lastError();
        }
    } else {
        db = QSqlDatabase::database();
    }

    // will be filled correctly later
    QVersionNumber oldSchemaVersionNumber;
    QVersionNumber newSchemaVersionNumber = QVersionNumber::fromString(KisResourceCacheDb::databaseVersion);


    QStringList tables = QStringList() << "version_information"
                                       << "storage_types"
                                       << "resource_types"
                                       << "storages"
                                       << "tags"
                                       << "resources"
                                       << "versioned_resources"
                                       << "resource_tags"
                                       << "metadata"
                                       << "tags_storages"
                                       << "tag_translations";

    QStringList dbTables;
    // Verify whether we should recreate the database
    {
        bool allTablesPresent = true;
        dbTables = db.tables();
        Q_FOREACH(const QString &table, tables) {
            if (!dbTables.contains(table)) {
                allTablesPresent = false;
                break;
            }
        }

        bool schemaIsOutDated = false;
        QString schemaVersion = "0.0.0";
        QString kritaVersion = "Unknown";
        int creationDate = 0;

        if (dbTables.contains("version_information")) {
            // Verify the version number

            {
                QSqlQuery q(
                    "SELECT database_version\n"
                    ",      krita_version\n"
                    ",      creation_date\n"
                    "FROM version_information\n"
                    "ORDER BY id\n"
                    "DESC\n"
                    "LIMIT 1;\n");

                if (!q.exec()) {
                    warnDbMigration << "Could not retrieve version information from the database." << q.lastError();
                    abort();
                }
                q.first();
                schemaVersion = q.value(0).toString();
                kritaVersion = q.value(1).toString();
                creationDate = q.value(2).toInt();
            }

            oldSchemaVersionNumber = QVersionNumber::fromString(schemaVersion);
            newSchemaVersionNumber = QVersionNumber::fromString(KisResourceCacheDb::databaseVersion);

            if (QVersionNumber::compare(oldSchemaVersionNumber, newSchemaVersionNumber) != 0) {

                infoDbMigration << "Old schema:" << schemaVersion << "New schema:" << newSchemaVersionNumber;

                schemaIsOutDated = true;
                KisBackup::numberedBackupFile(location + "/" + KisResourceCacheDb::resourceCacheDbFilename);

                if (newSchemaVersionNumber == QVersionNumber::fromString("0.0.18")
                        && QVersionNumber::compare(oldSchemaVersionNumber, QVersionNumber::fromString("0.0.14")) >= 0
                        && QVersionNumber::compare(oldSchemaVersionNumber, QVersionNumber::fromString("0.0.18")) < 0) {

                    bool from14to15 = oldSchemaVersionNumber == QVersionNumber::fromString("0.0.14");

                    bool from15to16 = oldSchemaVersionNumber == QVersionNumber::fromString("0.0.14")
                            || oldSchemaVersionNumber == QVersionNumber::fromString("0.0.15");

                    bool from16to17 = oldSchemaVersionNumber == QVersionNumber::fromString("0.0.14")
                            || oldSchemaVersionNumber == QVersionNumber::fromString("0.0.15")
                            || oldSchemaVersionNumber == QVersionNumber::fromString("0.0.16");

                    bool from17to18 = oldSchemaVersionNumber == QVersionNumber::fromString("0.0.14")
                            || oldSchemaVersionNumber == QVersionNumber::fromString("0.0.15")
                            || oldSchemaVersionNumber == QVersionNumber::fromString("0.0.16")
                            || oldSchemaVersionNumber == QVersionNumber::fromString("0.0.17");

                    KisDatabaseTransactionLock transactionLock(QSqlDatabase::database());

                    bool success = true;
                    if (from14to15) {
                        QSqlError error = runUpdateScript(
                            "ALTER TABLE  resource_tags\n"
                            "ADD   COLUMN active INTEGER NOT NULL DEFAULT 1", 
                            "Update resource tags table (add \'active\' column)");
                        if (error.type() != QSqlError::NoError) {
                            success = false;
                        }
                    }
                    if (success && from15to16) {
                        infoDbMigration << "Going to update indices";

                        QStringList indexes = QStringList() << "tags" << "resources" << "tag_translations" << "resource_tags";

                        Q_FOREACH(const QString &index, indexes) {
                            QSqlError error = runUpdateScriptFile(":/create_index_" + index + ".sql",
                                                                  QString("Create index for %1").arg(index));
                            if (error.type() != QSqlError::NoError) {
                                success = false;
                            }
                        }
                    }

                    if (success && from16to17) {
                        QSqlError error = runUpdateScriptFile(":/create_index_resources_signature.sql",
                                                              "Create index for resources_signature");
                        if (error.type() != QSqlError::NoError) {
                            success = false;
                        }
                    }

                    if (success && from17to18) {
                        {
                            QSqlError error = runUpdateScriptFile(":/0_0_18_0001_cleanup_metadata_table.sql",
                                                                  "Cleanup and deduplicate metadata table");
                            if (error.type() != QSqlError::NoError) {
                                success = false;
                            }
                        }
                        if (success) {
                            QSqlError error = runUpdateScriptFile(":/0_0_18_0002_update_metadata_table_constraints.sql",
                                                                  "Update metadata table constraints");
                            if (error.type() != QSqlError::NoError) {
                                success = false;
                            }
                        }
                        if (success) {
                            QSqlError error = runUpdateScriptFile(":/create_index_metadata_key.sql",
                                                                  "Create index for metadata_key");
                            if (error.type() != QSqlError::NoError) {
                                success = false;
                            }
                        }
                    }

                    if (success) {
                        if (!updateSchemaVersion()) {
                            success = false;
                        }

                        transactionLock.commit();

                        if (success) {
                            QSqlError error = runUpdateScript("VACUUM",
                                                              "Vacuum database after updating schema");
                            if (error.type() != QSqlError::NoError) {
                                success = false;
                            }
                        }
                    } else {
                        transactionLock.rollback();
                    }

                    schemaIsOutDated = !success;

                }

                if (schemaIsOutDated) {
                    QMessageBox::critical(0, i18nc("@title:window", "Krita"), i18n("The resource database scheme has changed. Krita will backup your database and create a new database."));
                    if (QVersionNumber::compare(oldSchemaVersionNumber, QVersionNumber::fromString("0.0.14")) > 0) {
                        KisResourceLocator::instance()->saveTags();
                    }
                    db.close();
                    QFile::remove(location + "/" + KisResourceCacheDb::resourceCacheDbFilename);
                    db.open();
                }
            }

        }

        if (allTablesPresent && !schemaIsOutDated) {
            KisUsageLogger::log(QString("Database is up to date. Version: %1, created by Krita %2, at %3")
                                .arg(schemaVersion)
                                .arg(kritaVersion)
                                .arg(QDateTime::fromSecsSinceEpoch(creationDate).toString()));

            /// initialization is completed, transaction is over,
            /// now enable the foreign_keys constraint if necessary
            KisResourceCacheDb::synchronizeForeignKeysState();

            return QSqlError();
        }
    }

    KisUsageLogger::log(QString("Creating database from scratch (%1, %2).")
                        .arg(oldSchemaVersionNumber.toString().isEmpty() ? QString("database didn't exist") : ("old schema version: " + oldSchemaVersionNumber.toString()))
                        .arg("new schema version: " + newSchemaVersionNumber.toString()));

    KisDatabaseTransactionLock transactionLock(QSqlDatabase::database());

    // Create tables
    Q_FOREACH(const QString &table, tables) {
        QSqlError error =
            runUpdateScriptFile(":/create_" + table + ".sql", QString("Create table %1").arg(table));
        if (error.type() != QSqlError::NoError) {
            return error;
        }
    }

    {
        // metadata table constraints were updated in version 0.0.18
        QSqlError error = runUpdateScriptFile(":/0_0_18_0002_update_metadata_table_constraints.sql",
                                              "Update metadata table constraints");

        if (error.type() != QSqlError::NoError) {
            return error;
        }
    }

    // Create indexes
    QStringList indexes;

    // these indexes came in version 0.0.16
    indexes << "storages" << "versioned_resources" << "tags" << "resources" << "tag_translations" << "resource_tags";

    // this index came in version 0.0.17
    indexes << "resources_signature";

    // this index came in version 0.0.18
    indexes << "metadata_key";

    Q_FOREACH(const QString &index, indexes) {
        QSqlError error = runUpdateScriptFile(":/create_index_" + index + ".sql",
                                              QString("Create index for %1").arg(index));
        if (error.type() != QSqlError::NoError) {
            return error;
        }
    }

    // Fill lookup tables
    {
        QFile f(":/fill_storage_types.sql");
        if (f.open(QFile::ReadOnly)) {
            QString sql = f.readAll();
            Q_FOREACH(const QString &originType, KisResourceCacheDb::storageTypes) {
                const QString updateStep = QString("Register storage type: %1").arg(originType);
                QSqlQuery q(sql);
                q.addBindValue(originType);
                if (!q.exec()) {
                    warnDbMigration << "Could execute DB update step:" << updateStep << q.lastError();
                    warnDbMigration << "    faulty statement:" << sql;
                    return db.lastError();
                }
                infoDbMigration << "Completed DB update step:" << updateStep;
            }
        }
        else {
            return QSqlError("Error executing SQL", QString("Could not find SQL fill_storage_types.sql."), QSqlError::StatementError);
        }
    }

    {
        QFile f(":/fill_resource_types.sql");
        if (f.open(QFile::ReadOnly)) {
            QString sql = f.readAll();
            Q_FOREACH(const QString &resourceType, KisResourceLoaderRegistry::instance()->resourceTypes()) {
                const QString updateStep = QString("Register resource type: %1").arg(resourceType);
                QSqlQuery q(sql);
                q.addBindValue(resourceType);
                if (!q.exec()) {
                    warnDbMigration << "Could execute DB update step:" << updateStep << q.lastError();
                    warnDbMigration << "    faulty statement:" << sql;
                    return db.lastError();
                }
                infoDbMigration << "Completed DB update step:" << updateStep;
            }
        }
        else {
            return QSqlError("Error executing SQL", QString("Could not find SQL fill_resource_types.sql."), QSqlError::StatementError);
        }
    }

    if (!updateSchemaVersion()) {
       return QSqlError("Error executing SQL", QString("Could not update schema version."), QSqlError::StatementError);
    }

    transactionLock.commit();

    /// initialization is completed, transaction is over,
    /// now enable the foreign_keys constraint if necessary
    KisResourceCacheDb::synchronizeForeignKeysState();

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

QVector<int> KisResourceCacheDb::resourcesForStorage(const QString &resourceType, const QString &storageLocation)
{
    QVector<int> result;

    QSqlQuery q;

    if (!q.prepare("SELECT resources.id\n"
                   "FROM   resources\n"
                   ",      resource_types\n"
                   ",      storages\n"
                   "WHERE  resources.resource_type_id = resource_types.id\n"
                   "AND    storages.id = resources.storage_id\n"
                   "AND    storages.location = :storage_location\n"
                   "AND    resource_types.name = :resource_type\n")) {

        qWarning() << "Could not read and prepare resourcesForStorage" << q.lastError();
        return result;
    }

    q.bindValue(":resource_type", resourceType);
    q.bindValue(":storage_location", changeToEmptyIfNull(storageLocation));

    if (!q.exec()) {
        qWarning() << "Could not query resourceIdForResource" << q.boundValues() << q.lastError();
        return result;
    }

    while (q.next()) {
        result << q.value(0).toInt();
    }

    return result;
}

int KisResourceCacheDb::resourceIdForResource(const QString &resourceFileName, const QString &resourceType, const QString &storageLocation)
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
    q.bindValue(":storage_location", changeToEmptyIfNull(storageLocation));

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
    q.bindValue(":storage_location", changeToEmptyIfNull(storageLocation));

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
    q.bindValue(":storage_location", changeToEmptyIfNull(KisResourceLocator::instance()->makeStorageLocationRelative(storage->location())));
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
    q.bindValue(":storage_location", changeToEmptyIfNull(KisResourceLocator::instance()->makeStorageLocationRelative(storage->location())));
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

    QString currentFilename;
    {
        QSqlQuery q;
        r = q.prepare("SELECT filename\n"
                      "FROM   resources\n"
                      "WHERE  id = :resource_id;");
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

        currentFilename = q.value(0).toString();
    }

    if (currentFilename != maxVersionFilename) {
        const QString url = resourceType + "/" + maxVersionFilename;
        KoResourceSP resource = storage->resource(url);
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(resource, false);
        resource->setVersion(maxVersion);
        resource->setMD5Sum(storage->resourceMd5(url));
        resource->setStorageLocation(storage->location());
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
                  ", md5sum    = :md5sum\n"
                  "WHERE id    = :id");
    if (!r) {
        qWarning() << "Could not prepare updateResource statement" << q.lastError();
        return r;
    }

    q.bindValue(":name", resource->name());
    q.bindValue(":filename", resource->filename());
    q.bindValue(":tooltip", i18n(resource->name().toUtf8()));
    q.bindValue(":md5sum", resource->md5Sum());

    QBuffer buf;
    buf.open(QBuffer::WriteOnly);
    resource->thumbnail().save(&buf, "PNG");
    buf.close();
    q.bindValue(":thumbnail", buf.data());
    q.bindValue(":id", resourceId);

    r = q.exec();
    if (!r) {
        qWarning() << "Could not update resource" << q.boundValues() << q.lastError();
    }

    if (!resource->metadata().isEmpty()) {
        return updateMetaDataForId(resource->metadata(), resourceId, METADATA_RESOURCES);
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

    {
        QSqlQuery q;
        r = q.prepare("DELETE FROM metadata \n"
                      "WHERE foreign_id = :resource_id\n"
                      "AND    table_name = :table;");

        if (!r) {
            qWarning() << "Could not prepare removeResourceCompletely4 statement" << q.lastError();
            return r;
        }

        q.bindValue(":resource_id", resourceId);
        q.bindValue(":table", METADATA_RESOURCES);
        r = q.exec();
        if (!r) {
            qWarning() << "Could not execute removeResourceCompletely4 statement" << q.lastError() << resourceId;
            return r;
        }
    }

    return r;
}

bool KisResourceCacheDb::getResourceIdFromFilename(QString filename, QString resourceType, QString storageLocation, int &outResourceId)
{
    QSqlQuery q;

    bool r = q.prepare("SELECT resources.id FROM resources\n"
                       ", resource_types\n"
                       ", storages\n"
                       "WHERE resources.filename = :filename\n" // bind to filename
                       "AND resource_types.id = resources.resource_type_id\n"  // join resources_types + resources
                       "AND resource_types.name = :resourceType\n" // bind to resource type
                       "AND resources.storage_id = storages.id\n" // join resources + storages
                       "AND storages.location = :storageLocation"); // bind to storage location

    if (!r) {
        qWarning() << "Could not prepare getResourceIdFromFilename statement" << q.lastError() << q.executedQuery();
        return r;
    }

    q.bindValue(":filename", filename);
    q.bindValue(":resourceType", resourceType);
    q.bindValue(":storageLocation",  changeToEmptyIfNull(storageLocation));

    r = q.exec();
    if (!r) {
        qWarning() << "Could not execute getResourceIdFromFilename statement" << q.lastError() << filename << resourceType;
        return r;
    }

    r = q.first();
    if (r) {
        outResourceId = q.value("resources.id").toInt();
    }

    return r;
}

bool KisResourceCacheDb::getResourceIdFromVersionedFilename(QString filename, QString resourceType, QString storageLocation, int &outResourceId)
{
    QSqlQuery q;

    bool r = q.prepare("SELECT resource_id FROM versioned_resources\n"
                       ", resources\n"
                       ", resource_types\n"
                       ", storages\n"
                       "WHERE versioned_resources.filename = :filename\n" // bind to filename
                       "AND resources.id = versioned_resources.resource_id\n" // join resources + versioned_resources
                       "AND resource_types.id = resources.resource_type_id\n"  // join resources_types + resources
                       "AND resource_types.name = :resourceType\n" // bind to resource type
                       "AND resources.storage_id = storages.id\n" // join resources + storages
                       "AND storages.location = :storageLocation"); // bind to storage location

    if (!r) {
        qWarning() << "Could not prepare getResourceIdFromVersionedFilename statement" << q.lastError() << q.executedQuery();
        return r;
    }


    q.bindValue(":filename", filename);
    q.bindValue(":resourceType", resourceType);
    q.bindValue(":storageLocation",  changeToEmptyIfNull(storageLocation));

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
    int resourceId = resourceIdForResource(resource->filename(), resourceType, KisResourceLocator::instance()->makeStorageLocationRelative(storage->location()));
    if (resourceId > -1) {
        return true;
    }

    QSqlQuery q;
    r = q.prepare("INSERT INTO resources \n"
                  "(storage_id, resource_type_id, name, filename, tooltip, thumbnail, status, temporary, md5sum) \n"
                  "VALUES \n"
                  "((SELECT  id "
                  "  FROM    storages "
                  "  WHERE   location = :storage_location)\n"
                  ", (SELECT id\n"
                  "   FROM   resource_types\n"
                  "   WHERE  name = :resource_type)\n"
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

    q.bindValue(":resource_type", resourceType);
    q.bindValue(":storage_location", changeToEmptyIfNull(KisResourceLocator::instance()->makeStorageLocationRelative(storage->location())));
    q.bindValue(":name", resource->name());
    q.bindValue(":filename", resource->filename());

    QString translationContext;
    if (storage->type() == KisResourceStorage::StorageType::Bundle) {
        translationContext = "./krita/data/bundles/" + KisResourceLocator::instance()->makeStorageLocationRelative(storage->location())
                + ":" + resourceType + "/" + resource->filename();
    } else if (storage->location() == "memory") {
        translationContext = "memory/" + resourceType + "/" + resource->filename();
    }
    else if (resource->filename().endsWith(".myb", Qt::CaseInsensitive)) {
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

    QBuffer buf;
    buf.open(QBuffer::WriteOnly);
    resource->image().save(&buf, "PNG");
    buf.close();
    q.bindValue(":thumbnail", buf.data());

    q.bindValue(":status", resource->active());
    q.bindValue(":temporary", (temporary ? 1 : 0));
    q.bindValue(":md5sum", resource->md5Sum());

    r = q.exec();
    if (!r) {
        qWarning() << "Could not execute addResource statement" << q.lastError() << q.boundValues();
        return r;
    }
    resourceId = resourceIdForResource(resource->filename(), resourceType, KisResourceLocator::instance()->makeStorageLocationRelative(storage->location()));

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

    if (!resource->metadata().isEmpty()) {
        return updateMetaDataForId(resource->metadata(), resource->resourceId(), METADATA_RESOURCES);
    }

    return true;


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

bool KisResourceCacheDb::tagResource(const QString &resourceFileName, KisTagSP tag, const QString &resourceType)
{
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


    // Get resource id
    QSqlQuery q;
    bool r = q.prepare("SELECT resources.id\n"
                       "FROM   resources\n"
                       ",      resource_types\n"
                       "WHERE  resources.resource_type_id = resource_types.id\n"
                       "AND    resource_types.name = :resource_type\n"
                       "AND    resources.filename = :resource_filename\n");
    if (!r) {
        qWarning() << "Could not prepare tagResource query" << q.lastError();
        return false;
    }

    q.bindValue(":resource_type", resourceType);
    q.bindValue(":resource_filename", resourceFileName);

    if (!q.exec()) {
        qWarning() << "Could not execute tagResource statement" << q.boundValues() << q.lastError();
        return false;
    }


    while (q.next()) {

        int resourceId = q.value(0).toInt();

        if (resourceId < 0) {
            qWarning() << "Could not find resource to tag" << resourceFileName << resourceType;
            continue;
        }

        {
            QSqlQuery q;
            if (!q.prepare("SELECT COUNT(*)\n"
                           "FROM   resource_tags\n"
                           "WHERE  resource_id = :resource_id\n"
                           "AND    tag_id = :tag_id")) {
                qWarning() << "Could not prepare tagResource query 2" << q.lastError();
                continue;
            }
            q.bindValue(":resource_id", resourceId);
            q.bindValue(":tag_id", tagId);

            if (!q.exec()) {
                qWarning() << "Could not execute tagResource query 2" << q.lastError() << q.boundValues();
                continue;
            }

            q.first();
            int count = q.value(0).toInt();
            if (count > 0) {
                continue;
            }
        }

        {
            QSqlQuery q;
            if (!q.prepare("INSERT INTO resource_tags\n"
                           "(resource_id, tag_id)\n"
                           "VALUES\n"
                           "(:resource_id, :tag_id);")) {
                qWarning() << "Could not prepare tagResource insert statement" << q.lastError();
                continue;
            }

            q.bindValue(":resource_id", resourceId);
            q.bindValue(":tag_id", tagId);

            if (!q.exec()) {
                qWarning() << "Could not execute tagResource stagement" << q.boundValues() << q.lastError();
                continue;
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
    q.bindValue(":storage_location", changeToEmptyIfNull(KisResourceLocator::instance()->makeStorageLocationRelative(storageLocation)));

    if (!q.exec()) {
        qWarning() << "Could not insert tag/storage link" << q.boundValues() << q.lastError();
        return false;
    }
    return true;
}


bool KisResourceCacheDb::addTag(const QString &resourceType, const QString storageLocation, KisTagSP tag)
{

    if (hasTag(tag->url(), resourceType)) {
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

        q.bindValue(":url", tag->url());
        q.bindValue(":resource_type", resourceType);

        if (!q.exec()) {
            qWarning() << "Could not execute tags_storages query" << q.boundValues() << q.lastError();
        }

        // If this tag is not yet linked to the storage, link it
        if (!q.first()) {
            return linkTagToStorage(tag->url(), resourceType, storageLocation);
        }

        return true;
    }

    int tagId;

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
                       ", :filename\n"
                       ");")) {
            qWarning() << "Could not prepare insert tag statement" << q.lastError();
            return false;
        }

        q.bindValue(":url", tag->url());
        q.bindValue(":name", tag->name(false));
        q.bindValue(":comment", tag->comment(false));
        q.bindValue(":resource_type", resourceType);
        q.bindValue(":filename", tag->filename());

        if (!q.exec()) {
            qWarning() << "Could not insert tag" << q.boundValues() << q.lastError();
        }

        tagId = q.lastInsertId().toInt();
    }

    {
        Q_FOREACH(const QString language, tag->names().keys()) {

            QString name = tag->names()[language];
            QString comment = name;
            if (tag->comments().contains(language)) {
                comment = tag->comments()[language];
            }

            QSqlQuery q;
            if (!q.prepare("INSERT INTO tag_translations\n"
                           "( tag_id\n"
                           ", language\n"
                           ", name\n"
                           ", comment\n"
                           ")\n"
                           "VALUES\n"
                           "( :id\n"
                           ", :language\n"
                           ", :name\n"
                           ", :comment\n"
                           ");")) {
                qWarning() << "Could not prepare insert tag_translation query" << q.lastError();
            }

            q.bindValue(":id", tagId);
            q.bindValue(":language", language);
            q.bindValue(":name", name);
            q.bindValue(":comment", comment);

            if (!q.exec()) {
                qWarning() << "Could not execute insert tag_translation query" << q.lastError() << q.boundValues();
            }
        }

    }


    linkTagToStorage(tag->url(), resourceType, storageLocation);

    return true;
}

bool KisResourceCacheDb::addTags(KisResourceStorageSP storage, QString resourceType)
{
    QSqlDatabase::database().transaction();
    QSharedPointer<KisResourceStorage::TagIterator> iter = storage->tags(resourceType);
    while(iter->hasNext()) {
        iter->next();
        KisTagSP tag = iter->tag();
        if (tag && tag->valid()) {
            if (!addTag(resourceType, storage->location(), tag)) {
                qWarning() << "Could not add tag" << tag << "to the database";
                continue;
            }
            if (!tag->defaultResources().isEmpty()) {
                Q_FOREACH(const QString &resourceFileName, tag->defaultResources()) {
                    if (!tagResource(resourceFileName, tag, resourceType)) {
                        qWarning() << "Could not tag resource" << QFileInfo(resourceFileName).baseName() << "from" << storage->name() << "filename" << resourceFileName << "with tag" << iter->tag();
                    }
                }
            }
        }
    }
    QSqlDatabase::database().commit();
    return true;
}

bool KisResourceCacheDb::registerStorageType(const KisResourceStorage::StorageType storageType)
{
    // Check whether the type already exists
    const QString name = KisResourceStorage::storageTypeToUntranslatedString(storageType);

    {
        QSqlQuery q;
        if (!q.prepare("SELECT count(*)\n"
                       "FROM   storage_types\n"
                       "WHERE  name = :storage_type\n")) {
            qWarning() << "Could not prepare select from storage_types query" << q.lastError();
            return false;
        }
        q.bindValue(":storage_type", name);
        if (!q.exec()) {
            qWarning() << "Could not execute select from storage_types query" << q.lastError();
            return false;
        }
        q.first();
        int rowCount = q.value(0).toInt();
        if (rowCount > 0) {
            return true;
        }
    }
    // if not, add it
    QFile f(":/fill_storage_types.sql");
    if (f.open(QFile::ReadOnly)) {
        QString sql = f.readAll();
        QSqlQuery q(sql);
        q.addBindValue(name);
        if (!q.exec()) {
            qWarning() << "Could not insert" << name << q.lastError();
            return false;
        }
        return true;
    }
    qWarning() << "Could not open fill_storage_types.sql";
    return false;
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
        q.bindValue(":location", changeToEmptyIfNull(KisResourceLocator::instance()->makeStorageLocationRelative(storage->location())));
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

        const QString sanitizedStorageLocation =
            changeToEmptyIfNull(KisResourceLocator::instance()->makeStorageLocationRelative(storage->location()));

        q.bindValue(":storage_type_id", static_cast<int>(storage->type()));
        q.bindValue(":location", sanitizedStorageLocation);
        q.bindValue(":timestamp", storage->timestamp().toSecsSinceEpoch());
        q.bindValue(":pre_installed", preinstalled ? 1 : 0);
        q.bindValue(":active", !disabledBundles.contains(storage->name()));

        QBuffer buf;
        buf.open(QBuffer::WriteOnly);
        storage->thumbnail().save(&buf, "PNG");
        buf.close();
        q.bindValue(":thumbnail", buf.data());

        r = q.exec();

        if (!r) qWarning() << "Could not execute query" << q.lastError();

        if (!q.prepare("SELECT id\n"
                       "FROM   storages\n"
                       "WHERE  location = :location\n")) {
            qWarning() << "Could not prepare storage id statement" << q.lastError();
        }

        q.bindValue(":location", sanitizedStorageLocation);
        if (!q.exec()) {
            qWarning() << "Could not execute storage id statement" << q.boundValues() << q.lastError();
        }

        if (!q.first()) {
            qWarning() << "Could not find id for the newly added storage" << q.lastError();
        } else {
            storage->setStorageId(q.value("id").toInt());
        }
    }

    // Insert the metadata
    {
        QStringList keys = storage->metaDataKeys();
        if (keys.size() > 0 && storage->storageId() >= 0) {

            QMap<QString, QVariant> metadata;

            Q_FOREACH(const QString &key, storage->metaDataKeys()) {
                metadata[key] = storage->metaData(key);
            }

            updateMetaDataForId(metadata, storage->storageId(), METADATA_STORAGES);
        }
    }

    Q_FOREACH(const QString &resourceType, KisResourceLoaderRegistry::instance()->resourceTypes()) {
        if (!KisResourceCacheDb::addResources(storage, resourceType)) {
            qWarning() << "Failed to add all resources for storage" << storage;
            r = false;
        }
    }

    return r;
}

bool KisResourceCacheDb::addStorageTags(KisResourceStorageSP storage)
{

    bool r = true;
    Q_FOREACH(const QString &resourceType, KisResourceLoaderRegistry::instance()->resourceTypes()) {
        if (!KisResourceCacheDb::addTags(storage, resourceType)) {
            qWarning() << "Failed to add all tags for storage" << storage;
            r = false;
        }
    }
    return r;
}

bool KisResourceCacheDb::deleteStorage(QString location)
{
    // location is already relative

    try {
        KisDatabaseTransactionLock transactionLock(QSqlDatabase::database());

        {
            KisSqlQueryLoader loader(":/sql/delete_versioned_resources_for_storage_indirect.sql",
                                     KisSqlQueryLoader::single_statement_mode);
            loader.query().bindValue(":location", changeToEmptyIfNull(location));
            loader.exec();
        }

        {
            KisSqlQueryLoader loader(":/sql/delete_resource_tags_for_storage_indirect.sql",
                                     KisSqlQueryLoader::single_statement_mode);
            loader.query().bindValue(":location", changeToEmptyIfNull(location));
            loader.exec();
        }

        {
            KisSqlQueryLoader loader(":/sql/delete_versioned_resources_for_storage_direct.sql",
                                     KisSqlQueryLoader::single_statement_mode);
            loader.query().bindValue(":location", changeToEmptyIfNull(location));
            loader.exec();
            if (loader.query().numRowsAffected() > 0) {
                qWarning() << "WARNING: deleteStorage: versioned_resurces table contained resource versions not being "
                              "present in the main table. Deleted: "
                           << loader.query().numRowsAffected();
            }
        }

        {
            KisSqlQueryLoader loader(":/sql/delete_resource_metadata_for_storage.sql",
                                     KisSqlQueryLoader::single_statement_mode);
            loader.query().bindValue(":location", changeToEmptyIfNull(location));
            loader.query().bindValue(":table", METADATA_RESOURCES);
            loader.exec();
        }

        {
            KisSqlQueryLoader loader("inline://delete_current_resources_for_storage",
                                     "DELETE FROM resources\n"
                                     "WHERE storage_id = (SELECT storages.id\n"
                                     "                    FROM   storages\n"
                                     "                    WHERE storages.location = :location)\n",
                                     KisSqlQueryLoader::single_statement_mode);
            loader.query().bindValue(":location", changeToEmptyIfNull(location));
            loader.exec();
        }

        {
            KisSqlQueryLoader loader("inline://delete_tags_for_storage",
                                     "DELETE FROM tags \n"
                                     "WHERE id IN (SELECT tags_storages.tag_id \n "
                                     "             FROM tags_storages \n"
                                     "             WHERE tags_storages.storage_id = \n"
                                     "                   (SELECT storages.id\n"
                                     "                    FROM   storages\n"
                                     "                    WHERE  storages.location = :location)\n"
                                     "            )",
                                     KisSqlQueryLoader::single_statement_mode);
            loader.query().bindValue(":location", changeToEmptyIfNull(location));
            loader.exec();
        }

        {
            KisSqlQueryLoader loader("inline://delete_tags_storage_links_for_storage",
                                     "DELETE FROM tags_storages \n"
                                     "WHERE tags_storages.storage_id = \n"
                                     "      (SELECT storages.id\n"
                                     "       FROM   storages\n"
                                     "       WHERE  storages.location = :location)",
                                     KisSqlQueryLoader::single_statement_mode);
            loader.query().bindValue(":location", changeToEmptyIfNull(location));
            loader.exec();
        }

        {
            KisSqlQueryLoader loader("inline://delete_starage_metadata_for_storage",
                                     "DELETE FROM metadata\n"
                                     "WHERE foreign_id = (SELECT storages.id\n"
                                     "                    FROM   storages\n"
                                     "                    WHERE  storages.location = :location)"
                                     "AND table_name = :table;",
                                     KisSqlQueryLoader::single_statement_mode);
            loader.query().bindValue(":location", changeToEmptyIfNull(location));
            loader.query().bindValue(":table", METADATA_STORAGES);
            loader.exec();
        }

        {
            KisSqlQueryLoader loader("inline://delete_storage",
                                     "DELETE FROM storages\n"
                                     "WHERE location = :location;",
                                     KisSqlQueryLoader::single_statement_mode);
            loader.query().bindValue(":location", changeToEmptyIfNull(location));
            loader.exec();
        }

        transactionLock.commit();

    } catch (const KisSqlQueryLoader::FileException &e) {
        qWarning().noquote() << "ERROR: deleteStorage:" << e.message;
        qWarning().noquote() << "       file:" << e.filePath;
        qWarning().noquote() << "       error:" << e.fileErrorString;
        return false;
    } catch (const KisSqlQueryLoader::SQLException &e) {
        qWarning().noquote() << "ERROR: deleteStorage:" << e.message;
        qWarning().noquote() << "       file:" << e.filePath;
        qWarning().noquote() << "       statement:" << e.statementIndex;
        qWarning().noquote() << "       error:" << e.sqlError.text();
        return false;
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

[[maybe_unused]]
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

    q.bindValue(":location", changeToEmptyIfNull(KisResourceLocator::instance()->makeStorageLocationRelative(storage->location())));
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
        return success;
    }

    storage->setStorageId(q.value("id").toInt());

    /// Start the transaction that will add all the resources
    QSqlDatabase::database().transaction();

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
                int id = resourceIdForResource(properFilenameWithSubfolders,
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
                KisUsageLogger::log("Could not load resource " + itA->url);
                ++itA;
                continue;
            }

            res->setVersion(itA->version);
            res->setMD5Sum(storage->resourceMd5(itA->url));
            if (!res->valid()) {
                KisUsageLogger::log("Could not retrieve md5 for resource " + itA->url);
                ++itA;
                continue;
            }

            const bool retval = addResource(storage, itA->timestamp, res, resourceType);
            if (!retval) {
                KisUsageLogger::log("Could not add resource " + itA->url);
                ++itA;
                continue;
            }

            const int resourceId = res->resourceId();
            KIS_SAFE_ASSERT_RECOVER(resourceId >= 0) {
                KisUsageLogger::log("Could not get id for resource " + itA->url);
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
                    KisUsageLogger::log("Could not add version for resource " + itA->url);
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
    try {
        KisDatabaseTransactionLock transactionLock(QSqlDatabase::database());

        /**
         * Remove all temporary resources
         */
        {
            KisSqlQueryLoader loader(
                "inline://delete_metadata_for_resources_in_memory_storages",
                "DELETE FROM metadata\n"
                "WHERE foreign_id IN (SELECT id\n"
                "                     FROM resources\n"
                "                     WHERE storage_id in (SELECT id\n"
                "                                          FROM storages\n"
                "                                          WHERE  storage_type_id == :storage_type))\n"
                "AND   table_name = :table",
                KisSqlQueryLoader::single_statement_mode);
            loader.query().bindValue(":table", METADATA_RESOURCES);
            loader.query().bindValue(":storage_type", (int)KisResourceStorage::StorageType::Memory);
            loader.exec();
        }

        {
            KisSqlQueryLoader loader("inline://delete_metadata_for_temporary_resources",
                                     "DELETE FROM metadata\n"
                                     "WHERE foreign_id IN (SELECT id\n"
                                     "                     FROM   resources\n"
                                     "                     WHERE temporary = 1)\n"
                                     "AND   table_name = :table",
                                     KisSqlQueryLoader::single_statement_mode);
            loader.query().bindValue(":table", METADATA_RESOURCES);
            loader.exec();
        }

        {
            KisSqlQueryLoader loader("inline://delete_versions_of_resources_in_temporary_storages",
                                     "DELETE FROM versioned_resources\n"
                                     "WHERE  storage_id in (SELECT id\n"
                                     "                      FROM   storages\n"
                                     "                      WHERE  storage_type_id == :storage_type)",
                                     KisSqlQueryLoader::single_statement_mode);
            loader.query().bindValue(":storage_type", (int)KisResourceStorage::StorageType::Memory);
            loader.exec();
        }

        {
            KisSqlQueryLoader loader("inline://delete_versions_of_temporary_resources",
                                     "DELETE FROM versioned_resources\n"
                                     "WHERE resource_id IN (SELECT id FROM resources\n"
                                     "                      WHERE  temporary = 1)",
                                     KisSqlQueryLoader::single_statement_mode);
            loader.exec();
        }

        {
            KisSqlQueryLoader loader("inline://delete_current_resources_in_temporary_storages",
                                     "DELETE FROM resources\n"
                                     "WHERE  storage_id in (SELECT id\n"
                                     "                      FROM   storages\n"
                                     "                      WHERE  storage_type_id  == :storage_type)",
                                     KisSqlQueryLoader::single_statement_mode);
            loader.query().bindValue(":storage_type", (int)KisResourceStorage::StorageType::Memory);
            loader.exec();
        }

        {
            KisSqlQueryLoader loader("inline://delete_current_temporary_resources",
                                     "DELETE FROM resources\n"
                                     "WHERE  temporary = 1",
                                     KisSqlQueryLoader::single_statement_mode);
            loader.exec();
        }

        /**
         * Remove all temporary storages
         */

        {
            KisSqlQueryLoader loader("inline://delete_metadata_for_temporary_storages",
                                     "DELETE FROM metadata\n"
                                     "WHERE foreign_id IN (SELECT id\n"
                                     "                     FROM   storages\n"
                                     "                     WHERE  storage_type_id  == :storage_type)\n"
                                     "AND   table_name = :table;",
                                     KisSqlQueryLoader::single_statement_mode);
            loader.query().bindValue(":storage_type", (int)KisResourceStorage::StorageType::Memory);
            loader.query().bindValue(":table", METADATA_STORAGES);
            loader.exec();
        }

        {
            KisSqlQueryLoader loader("inline://delete_temporary_storages",
                                     "DELETE FROM storages\n"
                                     "WHERE  storage_type_id  == :storage_type\n",
                                     KisSqlQueryLoader::single_statement_mode);
            loader.query().bindValue(":storage_type", (int)KisResourceStorage::StorageType::Memory);
            loader.exec();
        }

        transactionLock.commit();
    } catch (const KisSqlQueryLoader::SQLException &e) {
        qWarning().noquote() << "ERROR: failed to execute query:" << e.message;
        qWarning().noquote() << "       file:" << e.filePath;
        qWarning().noquote() << "       statement:" << e.statementIndex;
        qWarning().noquote() << "       error:" << e.sqlError.text();
    }
}

void KisResourceCacheDb::performHouseKeepingOnExit()
{
    QSqlQuery q;

    if (!q.prepare("PRAGMA optimize;")) {
        qWarning() << "Could not prepare query" << q.lastQuery() << q.lastError();
    }

    if (!q.exec()) {
        qWarning() << "Could not execute query" << q.lastQuery() << q.lastError();
    }
}

void KisResourceCacheDb::setForeignKeysStateImpl(bool isEnabled)
{
    KisSqlQueryLoader loader("inline://set_foreign_keys_state",
                             QString("PRAGMA foreign_keys = %1").arg(isEnabled ? "ON" : "OFF"));
    loader.exec();
}

bool KisResourceCacheDb::getForeignKeysStateImpl()
{
    KisSqlQueryLoader loader("inline://get_foreign_keys_state",
                             "PRAGMA foreign_keys");

    loader.exec();

    if (loader.query().first()) {
        return loader.query().value(0).toInt();
    }

    return false;
}

void KisResourceCacheDb::synchronizeForeignKeysState()
{
#ifdef KRITA_STABLE
    bool useForeignKeys = false;
    KisUsageLogger::log("INFO: detected stable build of Krita, foreign_keys constraint will be disabled");
#else
    bool useForeignKeys = true;
    KisUsageLogger::log("INFO: detected unstable build of Krita, foreign_keys constraint will be enabled");
#endif

    if (qEnvironmentVariableIsSet("KRITA_OVERRIDE_USE_FOREIGN_KEYS")) {
        useForeignKeys = qEnvironmentVariableIntValue("KRITA_OVERRIDE_USE_FOREIGN_KEYS") > 0;
        KisUsageLogger::log("INFO: foreign_keys constraint was overridden by KRITA_OVERRIDE_USE_FOREIGN_KEYS: " + QString::number(useForeignKeys));
    }

    try {
        const bool oldForeignKeysState = KisResourceCacheDb::getForeignKeysStateImpl();

        if (oldForeignKeysState != useForeignKeys) {
            KisUsageLogger::log(
                "INFO: switch foreign_keys state: " +
                QString::number(oldForeignKeysState) +
                " -> " +
                QString::number(useForeignKeys));

            KisResourceCacheDb::setForeignKeysStateImpl(useForeignKeys);
        }

    } catch (const KisSqlQueryLoader::SQLException &e) {
        qWarning().noquote() << "ERROR: failed to execute query:" << e.message;
        qWarning().noquote() << "       file:" << e.filePath;
        qWarning().noquote() << "       statement:" << e.statementIndex;
        qWarning().noquote() << "       error:" << e.sqlError.text();
    }

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
            ds.setVersion(QDataStream::Qt_5_15); // so Qt6 can read metatypes written by Qt5
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
            QSqlDatabase::database().rollback();
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
            ds.setVersion(QDataStream::Qt_5_15); // so Qt6 can write metatypes readable by Qt5
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

bool KisResourceCacheDb::removeOrphanedMetaData()
{
    auto deleteMetadataForType = [] (const QString &tableName) {
        KisSqlQueryLoader loader("inline://delete_orphaned_records (" + tableName + ")",
                                 QString("DELETE FROM metadata\n"
                                         "WHERE  foreign_id NOT IN (SELECT id FROM %1)\n"
                                         "AND    table_name = \"%1\"\n")
                                         .arg(tableName));
        loader.exec();

        if (loader.query().numRowsAffected() > 0) {
            qWarning().noquote().nospace() << "WARNING: orphaned metadata records were found for " << tableName << "!";
            qWarning().noquote().nospace() << "         Num records removed: " << loader.query().numRowsAffected();
        }
    };

    try {
        KisDatabaseTransactionLock transactionLock(QSqlDatabase::database());

        deleteMetadataForType(METADATA_RESOURCES);
        deleteMetadataForType(METADATA_STORAGES);

        transactionLock.commit();

    } catch (const KisSqlQueryLoader::SQLException &e) {
        qWarning().noquote() << "ERROR: failed to execute query:" << e.message;
        qWarning().noquote() << "       file:" << e.filePath;
        qWarning().noquote() << "       statement:" << e.statementIndex;
        qWarning().noquote() << "       error:" << e.sqlError.text();

        return false;
    }

    return true;
}
