/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KISBUNDLESTORAGE_H
#define KISBUNDLESTORAGE_H

#include <KisStoragePlugin.h>
#include "kritaresources_export.h"

/**
 * KisBundleStorage is KisStoragePlugin that can load resources 
 * from bundles. It can also manage overridden resources from bundles,
 * which are not stored in the bundles themselves.
 */
class KRITARESOURCES_EXPORT KisBundleStorage : public KisStoragePlugin
{
public:
    KisBundleStorage(const QString &location);
    virtual ~KisBundleStorage();

    KisResourceStorage::ResourceItem resourceItem(const QString &url) override;

    /// Note: this should find resources in a folder that override a resource in the bundle first
    bool loadVersionedResource(KoResourceSP resource) override;
    QString resourceMd5(const QString &url) override;
    QSharedPointer<KisResourceStorage::ResourceIterator> resources(const QString &resourceType) override;
    QSharedPointer<KisResourceStorage::TagIterator> tags(const QString &resourceType) override;
    QImage thumbnail() const override;
    QStringList metaDataKeys() const override;
    QVariant metaData(const QString &key) const override;

    /// Add a tag to this bundle: note, the bundle itself should NOT be rewritten, but we need to
    /// put these tags in a place in the file system
    bool addTag(const QString &resourceType, KisTagSP tag) override {Q_UNUSED(resourceType); Q_UNUSED(tag); return false;}

    /// Add a resource to this bundle: note, the bundle itself should NOT be rewritten, but we need to
    /// put these tags in a place in the file system
    bool saveAsNewVersion(const QString &resourceType, KoResourceSP resource) override;

    bool exportResource(const QString &url, QIODevice *device) override;

private:
    friend class BundleIterator;

private:
    class Private;
    QScopedPointer<Private> d;
};

#endif // KISBUNDLESTORAGE_H
