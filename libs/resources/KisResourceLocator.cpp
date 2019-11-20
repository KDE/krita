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

#include "KisResourceLocator.h"

#include <QApplication>
#include <QDebug>
#include <QList>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QMessageBox>
#include <QVersionNumber>

#include <QSqlQuery>
#include <QSqlError>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

#include <KritaVersionWrapper.h>
#include <KisMimeDatabase.h>

#include "KoResourcePaths.h"
#include "KisResourceStorage.h"
#include "KisResourceCacheDb.h"
#include "KisResourceLoaderRegistry.h"
#include "KisMemoryStorage.h"
#include "KisResourceModelProvider.h"


const QString KisResourceLocator::resourceLocationKey {"ResourceDirectory"};


class KisResourceLocator::Private {
public:
    QString resourceLocation;
    QMap<QString, KisResourceStorageSP> storages;
    QHash<QPair<QString, QString>, KoResourceSP> resourceCache;
    QStringList errorMessages;
};

KisResourceLocator::KisResourceLocator(QObject *parent)
    : QObject(parent)
    , d(new Private())
{
}

KisResourceLocator *KisResourceLocator::instance()
{
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
    InitalizationStatus initalizationStatus = InitalizationStatus::Unknown;

    KConfigGroup cfg(KSharedConfig::openConfig(), "");
    d->resourceLocation = cfg.readEntry(resourceLocationKey, QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (!d->resourceLocation.endsWith('/')) d->resourceLocation += '/';

    QFileInfo fi(d->resourceLocation);

    if (!fi.exists()) {
        if (!QDir().mkpath(d->resourceLocation)) {
            d->errorMessages << i18n("1. Could not create the resource location at %1.", d->resourceLocation);
            return LocatorError::CannotCreateLocation;
        }
        initalizationStatus = InitalizationStatus::FirstRun;
    }

    if (!fi.isWritable()) {
        d->errorMessages << i18n("2. The resource location at %1 is not writable.", d->resourceLocation);
        return LocatorError::LocationReadOnly;
    }

    // Check whether we're updating from an older version
    if (initalizationStatus != InitalizationStatus::FirstRun) {
        QFile fi(d->resourceLocation + '/' + "KRITA_RESOURCE_VERSION");
        if (!fi.exists()) {
            initalizationStatus = InitalizationStatus::FirstUpdate;
        }
        else {
            fi.open(QFile::ReadOnly);
            QVersionNumber resource_version = QVersionNumber::fromString(QString::fromUtf8(fi.readAll()));
            QVersionNumber krita_version = QVersionNumber::fromString(KritaVersionWrapper::versionString());
            if (krita_version > resource_version) {
                initalizationStatus = InitalizationStatus::Updating;
            }
            else {
                initalizationStatus = InitalizationStatus::Initialized;
            }
        }
    }

    if (initalizationStatus != InitalizationStatus::Initialized) {
        KisResourceLocator::LocatorError res = firstTimeInstallation(initalizationStatus, installationResourcesLocation);
        if (res != LocatorError::Ok) {
            return res;
        }
        initalizationStatus = InitalizationStatus::Initialized;
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
        Q_ASSERT(storage);

        resource = storage->resource(resourceType + "/" + filename);
        Q_ASSERT(resource);

        if (resource) {
            d->resourceCache[key] = resource;
        }
    }
    Q_ASSERT(resource);
    resource->setStorageLocation(storageLocation);
    Q_ASSERT(!resource->storageLocation().isEmpty());
    return resource;
}

KoResourceSP KisResourceLocator::resourceForId(int resourceId)
{
    ResourceStorage rs = getResourceStorage(resourceId);
    KoResourceSP r = resource(rs.storageLocation, rs.resourceType, rs.resourceFileName);
    Q_ASSERT(r);
    r->setResourceId(resourceId);

    return r;
}

bool KisResourceLocator::removeResource(int resourceId)
{
    // First remove the resource from the cache
    ResourceStorage rs = getResourceStorage(resourceId);
    QPair<QString, QString> key = QPair<QString, QString> (rs.storageLocation, rs.resourceType + "/" + rs.resourceFileName);

    d->resourceCache.remove(key);

    return KisResourceCacheDb::removeResource(resourceId);
}

bool KisResourceLocator::importResourceFromFile(const QString &resourceType, const QString &fileName)
{
    KisResourceLoaderBase *loader = KisResourceLoaderRegistry::instance()->loader(resourceType, KisMimeDatabase::mimeTypeForFile(fileName));
    QFile f(fileName);
    if (!f.open(QFile::ReadOnly)) {
        qWarning() << "Could not open" << fileName << "for loading";
        return false;
    }

    KoResourceSP resource = loader->load(QFileInfo(fileName).fileName(), f);
    if (!resource) {
        qWarning() << "Could not import" << fileName << ": resource doesn't load.";
        return false;
    }

    KisResourceStorageSP storage = d->storages[d->resourceLocation];
    Q_ASSERT(storage);
    if (!storage->addResource(resourceType, resource)) {
        qWarning() << "Could not add resource" << resource->filename() << "to the folder storage";
        return false;
    }

    return KisResourceCacheDb::addResource(folderStorage(), QFileInfo(resource->filename()).lastModified(), resource, resourceType);
}

bool KisResourceLocator::addResource(const QString &resourceType, const KoResourceSP resource, bool save)
{
    if (!resource || !resource->valid()) return false;

    if (save) {
        KisResourceStorageSP storage = d->storages[d->resourceLocation];
        Q_ASSERT(storage);

        //If we have gotten this far and the resource still doesn't have a filename to save to, we should generate one.
        if (resource->filename().isEmpty()) {
            resource->setFilename(resource->name().split(" ").join("_") + resource->defaultFileExtension() );
        }
        if (!storage->addResource(resourceType, resource)) {
            qWarning() << "Could not add resource" << resource->filename() << "to the folder storage";
            return false;
        }
    }
    else {
        resource->setFilename("memory/" + resourceType + "/" + resource->name());
        memoryStorage()->addResource(resourceType, resource);
    }

    return KisResourceCacheDb::addResource(save ? folderStorage() : memoryStorage(),
                                           QFileInfo(resource->filename()).lastModified(),
                                           resource,
                                           resourceType,
                                           !save);

}

bool KisResourceLocator::updateResource(const QString &resourceType, const KoResourceSP resource)
{

    QString storageLocation = makeStorageLocationAbsolute(resource->storageLocation());

    Q_ASSERT(d->storages.contains(storageLocation));
    Q_ASSERT(resource->resourceId() > -1);

    KisResourceStorageSP storage = d->storages[storageLocation];

    int version = resource->version();

    if (!storage->addResource(resourceType, resource)) {
        qWarning() << "Failed to save the new version of " << resource->name() << "to storage" << storageLocation;
        return false;
    }

    // It's the storages that keep track of the version
    Q_ASSERT(resource->version() == version + 1);

    if (!KisResourceCacheDb::addResourceVersion(resource->resourceId(), QDateTime::currentDateTime(), storage, resource)) {
        qWarning() << "Failed to add a new version of the resource to the database" << resource->name();
        return false;
    }

    // Update the resource in the cache
    QPair<QString, QString> key = QPair<QString, QString> (storageLocation, resourceType + "/" + QFileInfo(resource->filename()).fileName());
    d->resourceCache[key] = resource;

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

void KisResourceLocator::purge()
{
    d->resourceCache.clear();
}

bool KisResourceLocator::addDocumentStorage(const QString &document, KisResourceStorageSP storage)
{
    Q_ASSERT(!d->storages.contains(document));

    d->storages[document] = storage;

    if (!KisResourceCacheDb::addStorage(storage, false)) {
        d->errorMessages.append(i18n("Could not add %1 to the database", storage->location()));
        return false;
    }

    KisResourceModelProvider::resetAllModels();
    return true;
}

bool KisResourceLocator::removeDocumentStorage(const QString &document)
{
    purge();
    Q_ASSERT(d->storages.contains(document));
    KisResourceStorageSP storage = d->storages.take(document);
    if (!KisResourceCacheDb::deleteStorage(storage)) {
        d->errorMessages.append(i18n("Could not remove storage %1 from the database", storage->location()));
        return false;
    }
    KisResourceModelProvider::resetAllModels();
    return true;
}

bool KisResourceLocator::hasDocumentStorage(const QString &document)
{
    return d->storages.contains(document);
}

KisResourceLocator::LocatorError KisResourceLocator::firstTimeInstallation(InitalizationStatus initalizationStatus, const QString &installationResourcesLocation)
{
    emit progressMessage(i18n("Krita is running for the first time. Intialization will take some time."));
    Q_UNUSED(initalizationStatus);

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
    emit progressMessage(i18n("Initalizing the resources."));
    d->errorMessages.clear();
    findStorages();

    Q_FOREACH(KisResourceStorageSP storage, d->storages) {

        QTime t;
        t.start();

        if (!KisResourceCacheDb::addStorage(storage, (storage->type() == KisResourceStorage::StorageType::Folder ? false : true))) {
            d->errorMessages.append(i18n("Could not add storage %1 to the cache database", storage->location()));
        }

        qDebug() << "Adding storage" << storage->location() << "to the database took" << t.elapsed() << "ms";
    }

    return (d->errorMessages.isEmpty());
}

void KisResourceLocator::findStorages()
{
    d->storages.clear();

    // Add the folder
    KisResourceStorageSP storage = QSharedPointer<KisResourceStorage>::create(d->resourceLocation);
    Q_ASSERT(storage->location() == d->resourceLocation);
    d->storages[d->resourceLocation] = storage;

    // Add the memory storage
    d->storages["memory"] = QSharedPointer<KisResourceStorage>::create("memory");

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

KisResourceStorageSP KisResourceLocator::storageByName(const QString &name) const
{
    if (!d->storages.contains(name)) {
        qWarning() << "No" << name << "storage defined";
        return 0;
    }
    KisResourceStorageSP storage = d->storages[name];
    if (!storage || !storage->valid()) {
        qWarning() << "Could not retrieve the" << name << "storage object or the object is not valid";
        return 0;
    }

    return storage;
}

KisResourceStorageSP KisResourceLocator::folderStorage() const
{
    return storageByName(d->resourceLocation);
}

KisResourceStorageSP KisResourceLocator::memoryStorage() const
{
    return storageByName("memory");
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

    if (storageLocation.isEmpty()) {
        storageLocation = resourceLocationBase();
    }
    else {
        storageLocation = resourceLocationBase() + storageLocation;
    }

    rs.storageLocation = storageLocation;
    rs.resourceType = resourceType;
    rs.resourceFileName = resourceFilename;

    return rs;
}

QString KisResourceLocator::makeStorageLocationAbsolute(QString storageLocation) const
{
    if (storageLocation.isEmpty()) {
        storageLocation = resourceLocationBase();
    }
    if (!storageLocation.startsWith('/') && storageLocation != "memory") {
        if (resourceLocationBase().endsWith('/')) {
            storageLocation = resourceLocationBase() + storageLocation;
        }
        else {
            storageLocation = resourceLocationBase() + '/' + storageLocation;
        }
    }
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
    return d->errorMessages.isEmpty();
}


QString KisResourceLocator::makeStorageLocationRelative(QString location) const
{
    return location.remove(resourceLocationBase());
}
