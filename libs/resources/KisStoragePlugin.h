/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KISSTORAGEPLUGIN_H
#define KISSTORAGEPLUGIN_H

#include <QScopedPointer>
#include <QString>

#include <KisResourceStorage.h>
#include "kritaresources_export.h"

/**
 * The KisStoragePlugin class is the base class
 * for storage plugins. A storage plugin is used by
 * KisResourceStorage to locate resources and tags in
 * a kind of storage, like a folder, a bundle or an adobe
 * resource library.
 */
class KRITARESOURCES_EXPORT KisStoragePlugin
{
public:
    KisStoragePlugin(const QString &location);
    virtual ~KisStoragePlugin();

    virtual KisResourceStorage::ResourceItem resourceItem(const QString &url) = 0;

    /// Retrieve the given resource. The url is the unique identifier of the resource,
    /// for instance resourcetype plus filename.
    virtual KoResourceSP resource(const QString &url);
    virtual QString resourceMd5(const QString &url);
    virtual bool loadVersionedResource(KoResourceSP resource) = 0;
    virtual bool supportsVersioning() const;
    virtual QSharedPointer<KisResourceStorage::ResourceIterator> resources(const QString &resourceType) = 0;
    virtual QSharedPointer<KisResourceStorage::TagIterator> tags(const QString &resourceType) = 0;

    virtual bool addTag(const QString &resourceType, KisTagSP tag) {Q_UNUSED(resourceType); Q_UNUSED(tag); return false;}
    virtual bool saveAsNewVersion(const QString &resourceType, KoResourceSP resource) {Q_UNUSED(resourceType); Q_UNUSED(resource); return false;}
    virtual bool importResourceFile(const QString &resourceType, const QString &resourceFile) {Q_UNUSED(resourceType); Q_UNUSED(resourceFile); return false;}
    virtual bool addResource(const QString &resourceType, KoResourceSP resource) {Q_UNUSED(resourceType); Q_UNUSED(resource); return false;}
    virtual QImage thumbnail() const { return QImage(); }

    virtual void setMetaData(const QString &key, const QVariant &value) {Q_UNUSED(key); Q_UNUSED(value);}
    virtual QStringList metaDataKeys() const { return QStringList(); }
    virtual QVariant metaData(const QString &key) const { Q_UNUSED(key); return QString(); }

    QDateTime timestamp();

protected:
    friend class TestBundleStorage;
    QString location() const;
private:
    class Private;
    QScopedPointer<Private> d;
};

#endif // KISSTORAGEPLUGIN_H
