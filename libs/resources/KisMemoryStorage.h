/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KISMEMORYSTORAGE_H
#define KISMEMORYSTORAGE_H

#include <KisStoragePlugin.h>

#include <kritaresources_export.h>

/**
 * @brief The KisMemoryStorage class stores the temporary resources
 * that are not saved to disk or bundle. It is also used to stores
 * transient per-document resources, such as the document-local palette
 * list.
 */
class KRITARESOURCES_EXPORT KisMemoryStorage : public KisStoragePlugin
{
public:
    KisMemoryStorage(const QString &location = QString("memory"));
    virtual ~KisMemoryStorage();

    /// Copying the memory storage clones all contained resources and tags
    KisMemoryStorage(const KisMemoryStorage &rhs);

    /// This clones all contained resources and tags from rhs
    KisMemoryStorage &operator=(const KisMemoryStorage &rhs);

    bool addTag(const QString &resourceType, KisTagSP tag) override;
    bool saveAsNewVersion(const QString &resourceType, KoResourceSP resource) override;

    KisResourceStorage::ResourceItem resourceItem(const QString &url) override;
    bool loadVersionedResource(KoResourceSP resource) override;
    bool importResourceFile(const QString &resourceType, const QString &resourceFile) override;
    bool addResource(const QString &resourceType,  KoResourceSP resource) override;

    QString resourceMd5(const QString &url) override;
    QSharedPointer<KisResourceStorage::ResourceIterator> resources(const QString &resourceType) override;
    QSharedPointer<KisResourceStorage::TagIterator> tags(const QString &resourceType) override;

    void setMetaData(const QString &key, const QVariant &value) override;
    QStringList metaDataKeys() const override;
    QVariant metaData(const QString &key) const override;

private:
    class Private;
    QScopedPointer<Private> d;

};


#endif // KISMEMORYSTORAGE_H
