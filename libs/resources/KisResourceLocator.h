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

#ifndef KISRESOURCELOCATOR_H
#define KISRESOURCELOCATOR_H

#include <QObject>
#include <QScopedPointer>
#include <QStringList>
#include <QString>

#include "kritaresources_export.h"

#include <KisResourceStorage.h>


/**
 * The KisResourceLocator class locates all resource storages (folders,
 * bundles, various adobe resource libraries) in the resource location.
 *
 * The resource location is always a writable folder.
 *
 * There is one resource locator which is owned by the QApplication
 * object.
 *
 * The resource location is configurable, but there is only one location
 * where Krita will look for resources.
 */
class KRITARESOURCES_EXPORT KisResourceLocator : public QObject
{
    Q_OBJECT
public:

    // The configuration key that holds the resource location
    // for this installation of Krita. The location is
    // QStandardPaths::AppDataLocation by default, but that
    // can be changed.
    static const QString resourceLocationKey;

    static KisResourceLocator *instance();

    ~KisResourceLocator();

    enum class LocatorError {
        Ok,
        LocationReadOnly,
        CannotCreateLocation,
        CannotInitializeDb,
        CannotSynchronizeDb
    };

    /**
     * @brief initialize Setup the resource locator for use.
     *
     * @param installationResourcesLocation the place where the resources
     * that come packaged with Krita reside.
     */
    LocatorError initialize(const QString &installationResourcesLocation);

    /**
     * @brief errorMessages
     * @return
     */
    QStringList errorMessages() const;

    /**
     * @brief resourceLocationBase is the place where all resource storages (folder,
     * bundles etc. are located. This is a writable place.
     * @return the base location for all storages.
     */
    QString resourceLocationBase() const;

    /**
     * @brief purge purges the local resource cache
     */
    void purge();

    /**
     * @brief addStorage Adds a new resource storage to the database. The storage is
     * will be marked as not pre-installed.
     * @param storageLocation a unique name for the given storage
     * @param storage a storage object
     * @return true if the storage has been added succesfully
     */
    bool addStorage(const QString &storageLocation, KisResourceStorageSP storage);

    /**
     * @brief removeStorage removes the temporary storage from the database
     * @param document the unique name of the document
     * @return true is succesful.
     */
    bool removeStorage(const QString &storageLocation);

    /**
     * @brief hasStorage can be used to check whether the given storage already exists
     * @param storageLocation the name of the storage
     * @return true if the storage is known
     */
    bool hasStorage(const QString &storageLocation);

Q_SIGNALS:

    void progressMessage(const QString&);

private:

    friend class KisResourceModel;
    friend class KisStorageModel;
    friend class TestResourceLocator;
    friend class TestResourceModel;
    friend class Resource;
    friend class KisResourceCacheDb;

    /// @return true if the resource is present in the cache, false if it hasn't been loaded
    bool resourceCached(QString storageLocation, const QString &resourceType, const QString &filename) const;

    /**
     * @brief resource finds a physical resource in one of the storages
     * @param storageLocation the storage containing the resource. If empty,
     * this is the folder storage.
     *
     * Note that the resource does not have the version or id field set, so this cannot be used directly,
     * but only through KisResourceModel.
     *
     * @param resourceType the type of the resource
     * @param filename the filename of the resource including extension, but withou
     * any paths
     * @return A resource if found, or 0
     */
    KoResourceSP resource(QString storageLocation, const QString &resourceType, const QString &filename);

    /**
     * @brief resourceForId returns the resource with the given id, or 0 if no such resource exists.
     * The resource object will have its id set but not its version.
     * @param resourceId the id
     */
    KoResourceSP resourceForId(int resourceId);

    /**
     * @brief removeResource
     * @param resourceId
     * @param optional: the storage that contains the given resource
     * @return
     */
    bool removeResource(int resourceId, const QString &storageLocation = QString());

    /**
     * @brief importResourceFromFile
     * @param resourceType
     * @param fileName
     * @param storageLocation: optional, the storage where the resource will be stored. Empty means in the default Folder storage.
     * @return
     */
    bool importResourceFromFile(const QString &resourceType, const QString &fileName, const QString &storageLocation = QString());

    /**
     * @brief addResource adds the given resource to the database and potentially a storage
     * @param resourceType the type of the resource
     * @param resource the actual resource object
     * @param storageLocation the storage where the resource will be saved. By default this is the the default folder storage.
     * @return true if succesfull
     */
    bool addResource(const QString &resourceType, const KoResourceSP resource, const QString &storageLocation = QString());

    /**
     * @brief updateResource
     * @param resourceType
     * @param resource
     * @return
     */
    bool updateResource(const QString &resourceType, const KoResourceSP resource);

    /**
     * @brief metaDataForResource
     * @param id
     * @return
     */
    QMap<QString, QVariant> metaDataForResource(int id) const;

    bool setMetaDataForResource(int id, QMap<QString, QVariant> map) const;

    KisResourceLocator(QObject *parent);
    KisResourceLocator(const KisResourceLocator&);
    KisResourceLocator operator=(const KisResourceLocator&);

    enum class InitalizationStatus {
        Unknown,      // We don't know whether Krita has run on this system for this resource location yet
        Initialized,  // Everything is ready to start synchronizing the database
        FirstRun,     // Krita hasn't run for this resource location yet
        FirstUpdate,  // Krita was installed, but it's a version from before the resource locator existed, only user-defined resources are present
        Updating      // Krita is updating from an older version with resource locator
    };

    LocatorError firstTimeInstallation(InitalizationStatus initalizationStatus, const QString &installationResourcesLocation);

    // First time installation
    bool initializeDb();

    // Synchronize on restarting Krita to see whether the user has added any storages or resources to the resources location
    bool synchronizeDb();

    void findStorages();
    QList<KisResourceStorageSP> storages() const;

    KisResourceStorageSP storageByLocation(const QString &location) const;
    KisResourceStorageSP folderStorage() const;
    KisResourceStorageSP memoryStorage() const;

    struct ResourceStorage {
        QString storageLocation;
        QString resourceType;
        QString resourceFileName;
    };

    ResourceStorage getResourceStorage(int resourceId) const;
    QString makeStorageLocationAbsolute(QString storageLocation) const;
    QString makeStorageLocationRelative(QString location) const;


    class Private;
    QScopedPointer<Private> d;
};

#endif // KISRESOURCELOCATOR_H
