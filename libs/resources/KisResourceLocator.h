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
     * @brief addDocumentStorage Adds a temporary resource storage to the database
     * @param document a unique name for the given storage
     * @param storage a storage that contains the resources stored in the document
     * @return true if the storage has been added succesfully
     */
    bool addDocumentStorage(const QString &document, KisResourceStorageSP storage);

    /**
     * @brief removeDocumentStorage removes the temporary storage from the database
     * @param document the unique name of the document
     * @return true is succesful.
     */
    bool removeDocumentStorage(const QString &document);

    /**
     * @brief hasDocumentStorage can be used to check whether the given document storage already exists
     * @param document the name of the storage
     * @return true if the document is known
     */
    bool hasDocumentStorage(const QString &document);

Q_SIGNALS:

    void progressMessage(const QString&);

private:

    friend class KisResourceModel;
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
     * @return
     */
    bool removeResource(int resourceId);

    /**
     * @brief importResourceFromFile
     * @param resourceType
     * @param fileName
     * @return
     */
    bool importResourceFromFile(const QString &resourceType, const QString &fileName);

    /**
     * @brief addResource
     * @param resourceType
     * @param resource
     * @param save
     * @return
     */
    bool addResource(const QString &resourceType, const KoResourceSP resource, bool save = true);

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

    KisResourceStorageSP storageByName(const QString &name) const;
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
