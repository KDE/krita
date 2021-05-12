/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KISFOLDERSTORAGE_H
#define KISFOLDERSTORAGE_H

#include <KisStoragePlugin.h>

#include <kritaresources_export.h>

/**
 * KisFolderStorage is a KisStoragePlugin which handles resources 
 * stored in the user's resource folder. On initial startup, every
 * resource that comes as a folder resource is copied to the user's
 * resource folder. This is also the default location where the
 * resources the user creates are stored. 
 */
class KRITARESOURCES_EXPORT KisFolderStorage : public KisStoragePlugin
{
public:
    KisFolderStorage(const QString &location);
    virtual ~KisFolderStorage();

    /// Adds or updates this tag to the storage
    bool addTag(const QString &resourceType, KisTagSP tag) override;

    /// Adds or updates this resource to the storage
    bool saveAsNewVersion(const QString &resourceType, KoResourceSP resource) override;

    KisResourceStorage::ResourceItem resourceItem(const QString &url) override;
    bool loadVersionedResource(KoResourceSP resource) override;
    QSharedPointer<KisResourceStorage::ResourceIterator> resources(const QString &resourceType) override;
    QSharedPointer<KisResourceStorage::TagIterator> tags(const QString &resourceType) override;
    bool importResourceFile(const QString &resourceType, const QString &resourceFile) override;
    bool addResource(const QString  &resourceType, KoResourceSP resource) override;

    QStringList metaDataKeys() const override;
    QVariant metaData(const QString &key) const override;

    QByteArray resourceMd5(const QString &url) override;
private:
    friend class FolderIterator;

};

#endif // KISFOLDERSTORAGE_H
