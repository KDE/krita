/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisResourceLocator.h"

#include <QApplication>
#include <QDebug>
#include <QList>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QMessageBox>
#include <QVersionNumber>
#include <QElapsedTimer>
#include <QSqlQuery>
#include <QSqlError>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

#include <KritaVersionWrapper.h>
#include <KisMimeDatabase.h>
#include <kis_assert.h>

#include "KoResourcePaths.h"
#include "KisResourceStorage.h"
#include "KisResourceCacheDb.h"
#include "KisResourceLoaderRegistry.h"
#include "KisMemoryStorage.h"
#include "KisResourceModelProvider.h"
#include <KisGlobalResourcesInterface.h>
#include <KisStorageModel.h>

#include "ResourceDebug.h"

const QString KisResourceLocator::resourceLocationKey {"ResourceDirectory"};


class KisResourceLocator::Private {
public:
    QString resourceLocation;
    QMap<QString, KisResourceStorageSP> storages;
    QHash<QPair<QString, QString>, KoResourceSP> resourceCache;
    QMap<QPair<QString, QString>, QImage> thumbnailCache;
    QStringList errorMessages;
};

KisResourceLocator::KisResourceLocator(QObject *parent)
    : QObject(parent)
    , d(new Private())
{
}

KisResourceLocator *KisResourceLocator::instance()
{
    // Not a regular Q_GLOBAL_STATIC, because we want this deleted as
    // part of the app destructor.
    KisResourceLocator *locator = qApp->findChild<KisResourceLocator *>(QString());
    if (!locator) {
        locator = new KisResourceLocator(qApp);
    }
    return locator;
}

KisResourceLocator::~KisResourceLocator()
{
}

KisResourceLocator::LocatorError KisResourceLocator::initialize(const QString &installationResourcesLocation)
{
    InitializationStatus initializationStatus = InitializationStatus::Unknown;

    KConfigGroup cfg(KSharedConfig::openConfig(), "");
    d->resourceLocation = cfg.readEntry(resourceLocationKey, QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (d->resourceLocation == "") {
        d->resourceLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    }
    if (!d->resourceLocation.endsWith('/')) d->resourceLocation += '/';

    QFileInfo fi(d->resourceLocation);

    if (!fi.exists()) {
        if (!QDir().mkpath(d->resourceLocation)) {
            d->errorMessages << i18n("1. Could not create the resource location at %1.", d->resourceLocation);
            return LocatorError::CannotCreateLocation;
        }
        initializationStatus = InitializationStatus::FirstRun;
    }

    if (!fi.isWritable()) {
        d->errorMessages << i18n("2. The resource location at %1 is not writable.", d->resourceLocation);
        return LocatorError::LocationReadOnly;
    }

    // Check whether we're updating from an older version
    if (initializationStatus != InitializationStatus::FirstRun) {
        QFile fi(d->resourceLocation + '/' + "KRITA_RESOURCE_VERSION");
        if (!fi.exists()) {
            initializationStatus = InitializationStatus::FirstUpdate;
        }
        else {
            fi.open(QFile::ReadOnly);
            QVersionNumber resource_version = QVersionNumber::fromString(QString::fromUtf8(fi.readAll()));
            QVersionNumber krita_version = QVersionNumber::fromString(KritaVersionWrapper::versionString());
            if (krita_version > resource_version) {
                initializationStatus = InitializationStatus::Updating;
            }
            else {
                initializationStatus = InitializationStatus::Initialized;
            }
        }
    }

    if (initializationStatus != InitializationStatus::Initialized) {
        KisResourceLocator::LocatorError res = firstTimeInstallation(initializationStatus, installationResourcesLocation);
        if (res != LocatorError::Ok) {
            return res;
        }
        initializationStatus = InitializationStatus::Initialized;
    }
    else {
        if (!synchronizeDb()) {
            return LocatorError::CannotSynchronizeDb;
        }
    }
    return LocatorError::Ok;
}

QStringList KisResourceLocator::errorMessages() const
{
    return d->errorMessages;
}

QString KisResourceLocator::resourceLocationBase() const
{
    return d->resourceLocation;
}

bool KisResourceLocator::resourceCached(QString storageLocation, const QString &resourceType, const QString &filename) const
{
    storageLocation = makeStorageLocationAbsolute(storageLocation);
    QPair<QString, QString> key = QPair<QString, QString> (storageLocation, resourceType + "/" + filename);

    return d->resourceCache.contains(key);
}

void KisResourceLocator::cacheThumbnail(QString storageLocation, const QString &resourceType, const QString &filename,
                                        const QImage &img) {
    storageLocation = makeStorageLocationAbsolute(storageLocation);
    QPair<QString, QString> key = QPair<QString, QString> (storageLocation, resourceType + "/" + filename);

    d->thumbnailCache[key] = img;
}

QImage KisResourceLocator::thumbnailCached(QString storageLocation, const QString &resourceType, const QString &filename)
{
    storageLocation = makeStorageLocationAbsolute(storageLocation);
    QPair<QString, QString> key = QPair<QString, QString> (storageLocation, resourceType + "/" + filename);
    if (d->thumbnailCache.contains(key)) {
        return d->thumbnailCache[key];
    }
    return QImage();
}

void KisResourceLocator::loadRequiredResources(KoResourceSP resource)
{
    QList<KoResourceSP> requiredResources = resource->requiredResources(KisGlobalResourcesInterface::instance());
    Q_FOREACH (KoResourceSP res, requiredResources) {
        if (res->resourceId() < 0) {
            // we put all the embedded resources into the global shared "memory" storage
            this->addResource(res->resourceType().first, res, "memory");
        }
    }
}

KoResourceSP KisResourceLocator::resource(QString storageLocation, const QString &resourceType, const QString &filename)
{

    storageLocation = makeStorageLocationAbsolute(storageLocation);

    QPair<QString, QString> key = QPair<QString, QString> (storageLocation, resourceType + "/" + filename);

    KoResourceSP resource;
    if (d->resourceCache.contains(key)) {
        resource = d->resourceCache[key];
    }
    else {
        KisResourceStorageSP storage = d->storages[storageLocation];
        if (!storage) {
            qWarning() << "Could not find storage" << storageLocation;
            return 0;
        }

        resource = storage->resource(resourceType + "/" + filename);

        if (resource) {
            d->resourceCache[key] = resource;
            // load all the embedded resources into temporary "memory" storage
            loadRequiredResources(resource);
        }
    }

    if (!resource) {
        qWarning() << "KoResourceSP KisResourceLocator::resource" << storageLocation << resourceType << filename << "was not found";
        return 0;
    }

    resource->setStorageLocation(storageLocation);
    Q_ASSERT(!resource->storageLocation().isEmpty());

    if (resource->resourceId() < 0 || resource->version() < 0) {
        QSqlQuery q;
        if (!q.prepare("SELECT resources.id\n"
                       ",      versioned_resources.version as version\n"
                       ",      versioned_resources.md5sum as md5sum\n"
                       ",      resources.name\n"
                       "FROM   resources\n"
                       ",      storages\n"
                       ",      resource_types\n"
                       ",      versioned_resources\n"
                       "WHERE  storages.id = resources.storage_id\n"
                       "AND    storages.location = :storage_location\n"
                       "AND    resource_types.id = resources.resource_type_id\n"
                       "AND    resource_types.name = :resource_type\n"
                       "AND    resources.filename  = :filename\n"
                       "AND    versioned_resources.resource_id = resources.id\n"
                       "AND    versioned_resources.version = (SELECT MAX(version) FROM versioned_resources WHERE versioned_resources.resource_id = resources.id)")) {
            qWarning() << "Could not prepare id/version query" << q.lastError();

        }

        q.bindValue(":storage_location", makeStorageLocationRelative(storageLocation));
        q.bindValue(":resource_type", resourceType);
        q.bindValue(":filename", filename);

        if (!q.exec()) {
            qWarning() << "Could not execute id/version query" << q.lastError() << q.boundValues();
        }

        if (!q.first()) {
            qWarning() << "Could not find the resource in the database" << storageLocation << resourceType << filename;
        }

        resource->setResourceId(q.value(0).toInt());
        Q_ASSERT(resource->resourceId() >= 0);

        resource->setVersion(q.value(1).toInt());
        Q_ASSERT(resource->version() >= 0);

        resource->setMD5(QByteArray::fromHex(q.value(2).toByteArray()));
        Q_ASSERT(!resource->md5().isEmpty());

        // To override resources that use the filename for the name, which is versioned, and we don't want the version number in the name
        resource->setName(q.value(3).toString());;
    }

    if (!resource) {
        qWarning() << "Could not find resource" << resourceType + "/" + filename;
        return 0;
    }

    return resource;
}

KoResourceSP KisResourceLocator::resourceForId(int resourceId)
{
    ResourceStorage rs = getResourceStorage(resourceId);
    KoResourceSP r = resource(rs.storageLocation, rs.resourceType, rs.resourceFileName);
    return r;
}

bool KisResourceLocator::setResourceActive(int resourceId, bool active)
{
    // First remove the resource from the cache
    ResourceStorage rs = getResourceStorage(resourceId);
    QPair<QString, QString> key = QPair<QString, QString> (rs.storageLocation, rs.resourceType + "/" + rs.resourceFileName);

    d->resourceCache.remove(key);
    if (!active && d->thumbnailCache.contains(key)) {
        d->thumbnailCache.remove(key);
    }

    return KisResourceCacheDb::setResourceActive(resourceId, active);
}

KoResourceSP KisResourceLocator::importResourceFromFile(const QString &resourceType, const QString &fileName, const QString &storageLocation)
{
    KisResourceStorageSP storage = d->storages[makeStorageLocationAbsolute(storageLocation)];

    KisResourceLoaderBase *loader = KisResourceLoaderRegistry::instance()->loader(resourceType, KisMimeDatabase::mimeTypeForFile(fileName));
    QFile f(fileName);
    if (!f.open(QFile::ReadOnly)) {
        qWarning() << "Could not open" << fileName << "for loading";
        return nullptr;
    }

    KoResourceSP resource = loader->load(QFileInfo(fileName).fileName(), f, KisGlobalResourcesInterface::instance());

    if (!resource || !resource->valid()) {
        qWarning() << "Could not import" << fileName << ": resource doesn't load.";
        return nullptr;
    }

    const QByteArray md5 = resource->md5();

    const KoResourceSP existingResource = storage->resource(resourceType + "/" + resource->filename());

    if (existingResource) {
        if (existingResource->md5() == md5) {
            return existingResource;
        } else {
            qWarning() << "A resource with the same filename but a different MD5 already exists in the storage" << resourceType << fileName << storageLocation;
            return nullptr;
        }
    }

    if (storage->importResourceFile(resourceType, fileName)) {

        resource = storage->resource(resourceType + "/" + resource->filename());

        if (!resource) {
            qWarning() << "Could not retrieve imported resource from the storage" << resourceType << fileName << storageLocation;
            return nullptr;
        }

        Q_ASSERT(resource->md5() == md5);
        resource->setVersion(0);

        // Insert into the database
        const bool result = KisResourceCacheDb::addResource(storage,
                                                storage->timeStampForResource(resourceType, resource->filename()),
                                                resource,
                                                resourceType);
        if (result) {
            return resource;
        }

        // Add to the cache
        d->resourceCache[QPair<QString, QString>(storageLocation, resourceType + "/" + resource->filename())] = resource;

        return resource;
    }
    return nullptr;
}

bool KisResourceLocator::addResource(const QString &resourceType, const KoResourceSP resource, const QString &storageLocation)
{
    if (!resource || !resource->valid()) return false;

    KisResourceStorageSP storage = d->storages[makeStorageLocationAbsolute(storageLocation)];
    Q_ASSERT(storage);

    //If we have gotten this far and the resource still doesn't have a filename to save to, we should generate one.
    if (resource->filename().isEmpty()) {
        resource->setFilename(resource->name().split(" ").join("_") + resource->defaultFileExtension());
    }

    if (resource->version() != 0) { // Can happen with cloned resources
        resource->setVersion(0);
    }

    // Save the resource to the storage storage
    if (!storage->addResource(resource)) {
        qWarning() << "Could not add resource" << resource->filename() << "to the storage" << storageLocation;
        return false;
    }

    resource->setStorageLocation(storageLocation);
    resource->setMD5(storage->resourceMd5(resourceType + "/" + resource->filename()));
    resource->setDirty(false);

    d->resourceCache[QPair<QString, QString>(storageLocation, resourceType + "/" + resource->filename())] = resource;

    // And the database
    const bool result = KisResourceCacheDb::addResource(storage,
                                           storage->timeStampForResource(resourceType, resource->filename()),
                                           resource,
                                           resourceType);
    return result;
}

bool KisResourceLocator::updateResource(const QString &resourceType, const KoResourceSP resource)
{
    QString storageLocation = makeStorageLocationAbsolute(resource->storageLocation());

    Q_ASSERT(d->storages.contains(storageLocation));
    Q_ASSERT(resource->resourceId() > -1);

    KisResourceStorageSP storage = d->storages[storageLocation];

    if (!storage->supportsVersioning()) return false;

    // remove older version
    d->thumbnailCache.remove(QPair<QString, QString> (storageLocation, resourceType + "/" + resource->filename()));

    resource->updateThumbnail();
    resource->setVersion(resource->version() + 1);

    if (!storage->saveAsNewVersion(resource)) {
        qWarning() << "Failed to save the new version of " << resource->name() << "to storage" << storageLocation;
        return false;
    }

    resource->setMD5(storage->resourceMd5(resourceType + "/" + resource->filename()));
    resource->setDirty(false);

    // The version needs already to have been incremented
    if (!KisResourceCacheDb::addResourceVersion(resource->resourceId(), QDateTime::currentDateTime(), storage, resource)) {
        qWarning() << "Failed to add a new version of the resource to the database" << resource->name();
        return false;
    }

    // Update the resource in the cache
    QPair<QString, QString> key = QPair<QString, QString> (storageLocation, resourceType + "/" + resource->filename());
    d->resourceCache[key] = resource;
    d->thumbnailCache[key] = resource->thumbnail();

    return true;
}

bool KisResourceLocator::reloadResource(const QString &resourceType, const KoResourceSP resource)
{
    // This resource isn't in the database yet, so we cannot reload it
    if (resource->resourceId() < 0) return false;

    QString storageLocation = makeStorageLocationAbsolute(resource->storageLocation());
    Q_ASSERT(d->storages.contains(storageLocation));

    KisResourceStorageSP storage = d->storages[storageLocation];

    if (!storage->loadVersionedResource(resource)) {
        qWarning() << "Failed to reload the resource" << resource->name() << "from storage" << storageLocation;
        return false;
    }

    resource->setMD5(storage->resourceMd5(resourceType + "/" + resource->filename()));
    resource->setDirty(false);

    // We haven't changed the version of the resource, so the cache must be still valid
    QPair<QString, QString> key = QPair<QString, QString> (storageLocation, resourceType + "/" + resource->filename());
    Q_ASSERT(d->resourceCache[key] == resource);

    return true;
}

QMap<QString, QVariant> KisResourceLocator::metaDataForResource(int id) const
{
    return KisResourceCacheDb::metaDataForId(id, "resources");
}

bool KisResourceLocator::setMetaDataForResource(int id, QMap<QString, QVariant> map) const
{
    return KisResourceCacheDb::updateMetaDataForId(map, id, "resources");
}

QMap<QString, QVariant> KisResourceLocator::metaDataForStorage(const QString &storageLocation) const
{
    QMap<QString, QVariant> metadata;
    if (!d->storages.contains(makeStorageLocationAbsolute(storageLocation))) {
        qWarning() << storageLocation << "not in" << d->storages.keys();
        return metadata;
    }

    KisResourceStorageSP st = d->storages[makeStorageLocationAbsolute(storageLocation)];

    if (d->storages[makeStorageLocationAbsolute(storageLocation)].isNull()) {
        return metadata;
    }

    Q_FOREACH(const QString key, st->metaDataKeys()) {
        metadata[key] = st->metaData(key);
    }
    return metadata;
}

void KisResourceLocator::setMetaDataForStorage(const QString &storageLocation, QMap<QString, QVariant> map) const
{
    Q_ASSERT(d->storages.contains(storageLocation));
    Q_FOREACH(const QString &key, map.keys()) {
        d->storages[storageLocation]->setMetaData(key, map[key]);
    }
}

void KisResourceLocator::purge(const QString &storageLocation)
{
    Q_FOREACH(const auto key, d->resourceCache.keys()) {
        if (key.first == storageLocation) {
            d->resourceCache.take(key);
            d->thumbnailCache.take(key);
        }
    }
}

bool KisResourceLocator::addStorage(const QString &storageLocation, KisResourceStorageSP storage)
{
    if (d->storages.contains(storageLocation)) {
        if (!removeStorage(storageLocation)) {
            qWarning() << "could not remove" << storageLocation;
            return false;
        }
    }

    d->storages[storageLocation] = storage;
    if (!KisResourceCacheDb::addStorage(storage, false)) {
        d->errorMessages.append(i18n("Could not add %1 to the database", storage->location()));
        qWarning() << d->errorMessages;
        return false;
    }
    emit storageAdded(makeStorageLocationRelative(storage->location()));
    return true;
}

bool KisResourceLocator::removeStorage(const QString &storageLocation)
{
    // Cloned documents have a document storage, but that isn't in the locator.
    if (!d->storages.contains(storageLocation)) {
        qDebug() << storageLocation << "does not exist";
        return true;
    }

    purge(storageLocation);

    KisResourceStorageSP storage = d->storages.take(storageLocation);

    if (!KisResourceCacheDb::deleteStorage(storage)) {
        d->errorMessages.append(i18n("Could not remove storage %1 from the database", storage->location()));
        qWarning() << d->errorMessages;
        return false;
    }
    emit storageRemoved(storage->location());

    return true;
}

bool KisResourceLocator::hasStorage(const QString &document)
{
    return d->storages.contains(document);
}

KisResourceLocator::LocatorError KisResourceLocator::firstTimeInstallation(InitializationStatus initializationStatus, const QString &installationResourcesLocation)
{
    emit progressMessage(i18n("Krita is running for the first time. Initialization will take some time."));
    Q_UNUSED(initializationStatus);

    Q_FOREACH(const QString &folder, KisResourceLoaderRegistry::instance()->resourceTypes()) {
        QDir dir(d->resourceLocation + '/' + folder + '/');
        if (!dir.exists()) {
            if (!QDir().mkpath(d->resourceLocation + '/' + folder + '/')) {
                d->errorMessages << i18n("3. Could not create the resource location at %1.", dir.path());
                return LocatorError::CannotCreateLocation;
            }
        }
    }

    Q_FOREACH(const QString &folder, KisResourceLoaderRegistry::instance()->resourceTypes()) {
        QDir dir(installationResourcesLocation + '/' + folder + '/');
        if (dir.exists()) {
            Q_FOREACH(const QString &entry, dir.entryList(QDir::Files | QDir::Readable)) {
                QFile f(dir.canonicalPath() + '/'+ entry);
                if (!QFileInfo(d->resourceLocation + '/' + folder + '/' + entry).exists()) {
                    if (!f.copy(d->resourceLocation + '/' + folder + '/' + entry)) {
                        d->errorMessages << i18n("Could not copy resource %1 to %2", f.fileName(), d->resourceLocation + '/' + folder + '/' + entry);
                    }
                }
            }
        }
    }

    // And add bundles and adobe libraries
    QStringList filters = QStringList() << "*.bundle" << "*.abr" << "*.asl";
    QDirIterator iter(installationResourcesLocation, filters, QDir::Files, QDirIterator::Subdirectories);
    while (iter.hasNext()) {
        iter.next();
        emit progressMessage(i18n("Installing the resources from bundle %1.", iter.filePath()));
        QFile f(iter.filePath());
        Q_ASSERT(f.exists());
        if (!f.copy(d->resourceLocation + '/' + iter.fileName())) {
            d->errorMessages << i18n("Could not copy resource %1 to %2", f.fileName(), d->resourceLocation);
        }
    }

    QFile f(d->resourceLocation + '/' + "KRITA_RESOURCE_VERSION");
    f.open(QFile::WriteOnly);
    f.write(KritaVersionWrapper::versionString().toUtf8());
    f.close();

    if (!initializeDb()) {
        return LocatorError::CannotInitializeDb;
    }

    return LocatorError::Ok;
}

bool KisResourceLocator::initializeDb()
{
    emit progressMessage(i18n("Initializing the resources."));
    d->errorMessages.clear();
    findStorages();

    Q_FOREACH(auto loader, KisResourceLoaderRegistry::instance()->values()) {
        KisResourceCacheDb::registerResourceType(loader->resourceType());
    }

    Q_FOREACH(KisResourceStorageSP storage, d->storages) {

        QElapsedTimer t;
        t.start();

        if (!KisResourceCacheDb::addStorage(storage, (storage->type() == KisResourceStorage::StorageType::Folder ? false : true))) {
            d->errorMessages.append(i18n("Could not add storage %1 to the cache database", storage->location()));
        }

        debugResource << "Adding storage" << storage->location() << "to the database took" << t.elapsed() << "ms";
    }

    return (d->errorMessages.isEmpty());
}

void KisResourceLocator::findStorages()
{
    d->storages.clear();
    d->resourceCache.clear();

    // Add the folder
    KisResourceStorageSP storage = QSharedPointer<KisResourceStorage>::create(d->resourceLocation);
    Q_ASSERT(storage->location() == d->resourceLocation);
    d->storages[d->resourceLocation] = storage;

    // Add the memory storage
    d->storages["memory"] = QSharedPointer<KisResourceStorage>::create("memory");
    d->storages["memory"]->setMetaData(KisResourceStorage::s_meta_name, i18n("Temporary Resources"));

    // And add bundles and adobe libraries
    QStringList filters = QStringList() << "*.bundle" << "*.abr" << "*.asl";
    QDirIterator iter(d->resourceLocation, filters, QDir::Files, QDirIterator::Subdirectories);
    while (iter.hasNext()) {
        iter.next();
        KisResourceStorageSP storage = QSharedPointer<KisResourceStorage>::create(iter.filePath());
        d->storages[storage->location()] = storage;
    }
}

QList<KisResourceStorageSP> KisResourceLocator::storages() const
{
    return d->storages.values();
}

KisResourceStorageSP KisResourceLocator::storageByLocation(const QString &location) const
{
    if (!d->storages.contains(location)) {
        qWarning() << "No" << location << "storage defined:" << d->storages.keys();
        return 0;
    }
    KisResourceStorageSP storage = d->storages[location];
    if (!storage || !storage->valid()) {
        qWarning() << "Could not retrieve the" << location << "storage object or the object is not valid";
        return 0;
    }

    return storage;
}

KisResourceStorageSP KisResourceLocator::folderStorage() const
{
    return storageByLocation(d->resourceLocation);
}

KisResourceStorageSP KisResourceLocator::memoryStorage() const
{
    return storageByLocation("memory");
}

KisResourceLocator::ResourceStorage KisResourceLocator::getResourceStorage(int resourceId) const
{
    ResourceStorage rs;

    QSqlQuery q;
    bool r = q.prepare("SELECT storages.location\n"
                       ",      resource_types.name as resource_type\n"
                       ",      resources.filename\n"
                       "FROM   resources\n"
                       ",      storages\n"
                       ",      resource_types\n"
                       "WHERE  resources.id = :resource_id\n"
                       "AND    resources.storage_id = storages.id\n"
                       "AND    resource_types.id = resources.resource_type_id");
    if (!r) {
        qWarning() << "KisResourceLocator::removeResource: could not prepare query." << q.lastError();
        return rs;
    }


    q.bindValue(":resource_id", resourceId);

    r = q.exec();
    if (!r) {
        qWarning() << "KisResourceLocator::removeResource: could not execute query." << q.lastError();
        return rs;
    }

    q.first();

    QString storageLocation = q.value("location").toString();
    QString resourceType= q.value("resource_type").toString();
    QString resourceFilename = q.value("filename").toString();

    rs.storageLocation = makeStorageLocationAbsolute(storageLocation);
    rs.resourceType = resourceType;
    rs.resourceFileName = resourceFilename;

    return rs;
}

QString KisResourceLocator::makeStorageLocationAbsolute(QString storageLocation) const
{
//    debugResource << "makeStorageLocationAbsolute" << storageLocation;

    if (storageLocation.isEmpty()) {
        return resourceLocationBase();
    }

    if (QFileInfo(storageLocation).isRelative() && (storageLocation.endsWith("bundle")
                                             || storageLocation.endsWith("asl")
                                             || storageLocation.endsWith("abr"))) {
        if (resourceLocationBase().endsWith('/') || resourceLocationBase().endsWith("\\")) {
            storageLocation = resourceLocationBase() + storageLocation;
        }
        else {
            storageLocation = resourceLocationBase() + '/' + storageLocation;
        }
    }

//    debugResource  << "\t" << storageLocation;
    return storageLocation;
}

bool KisResourceLocator::synchronizeDb()
{
    d->errorMessages.clear();
    findStorages();
    Q_FOREACH(const KisResourceStorageSP storage, d->storages) {
        if (!KisResourceCacheDb::synchronizeStorage(storage)) {
            d->errorMessages.append(i18n("Could not synchronize %1 with the database", storage->location()));
        }
    }
    // now remove the storages that no longer exists
    KisStorageModel model;
    for (int i = 0; i < model.rowCount(); i++) {
        QModelIndex idx = model.index(i, 0);
        QString location = model.data(idx, Qt::UserRole + KisStorageModel::Location).toString();
        if (!d->storages.contains(this->makeStorageLocationAbsolute(location))) {
            if (!KisResourceCacheDb::deleteStorage(location)) {
                d->errorMessages.append(i18n("Could not remove storage %1 from the database", this->makeStorageLocationAbsolute(location)));
                qWarning() << d->errorMessages;
                return false;
            }
            emit storageRemoved(this->makeStorageLocationAbsolute(location));
        }
    }


    d->resourceCache.clear();
    return d->errorMessages.isEmpty();
}


QString KisResourceLocator::makeStorageLocationRelative(QString location) const
{
//    debugResource << "makeStorageLocationRelative" << location << "locationbase" << resourceLocationBase();
    return location.remove(resourceLocationBase());
}
