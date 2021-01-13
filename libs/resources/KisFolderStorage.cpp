/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisFolderStorage.h"

#include <QDirIterator>
#include <KisMimeDatabase.h>
#include <kis_debug.h>
#include <KisTag.h>
#include <KisResourceLoaderRegistry.h>
#include <kbackup.h>
#include <KisGlobalResourcesInterface.h>

class FolderTagIterator : public KisResourceStorage::TagIterator
{
public:

    FolderTagIterator(const QString &location, const QString &resourceType)
        : m_location(location)
        , m_resourceType(resourceType)
    {
        m_dirIterator.reset(new QDirIterator(location + '/' + resourceType,
                                             QStringList() << "*.tag",
                                             QDir::Files | QDir::Readable,
                                             QDirIterator::Subdirectories));
    }

    bool hasNext() const override
    {
        return m_dirIterator->hasNext();
    }

    void next() override
    {
        m_dirIterator->next();
        const_cast<FolderTagIterator*>(this)->m_tag.reset(new KisTag);
        load(m_tag);
    }

    QString url() const override { return m_tag ? m_tag->url() : QString(); }
    QString name() const override { return m_tag ? m_tag->name() : QString(); }
    QString comment() const override {return m_tag ? m_tag->comment() : QString(); }
    QString resourceType() const override { return  m_tag ? m_tag->resourceType() : resourceType(); }
    KisTagSP tag() const override
    {
        return m_tag;
    }

private:

    bool load(KisTagSP tag) const
    {
        QFile f(m_dirIterator->filePath());
        if (f.exists()) {
            f.open(QFile::ReadOnly);
            if (!tag->load(f)) {
                qWarning() << m_dirIterator << "is not a valid tag desktop file";
                return false;
            }
        }
        return true;
    }

    QScopedPointer<QDirIterator> m_dirIterator;
    QString m_location;
    QString m_resourceType;
    KisTagSP m_tag;
};


class FolderItem : public KisResourceStorage::ResourceItem
{
public:
    ~FolderItem() override {}
};

class FolderIterator : public KisResourceStorage::ResourceIterator
{
public:
    FolderIterator(const QString &location, const QString &resourceType)
        : m_location(location)
        , m_resourceType(resourceType)
    {
        m_dirIterator.reset(new QDirIterator(location + '/' + resourceType,
                                             KisResourceLoaderRegistry::instance()->filters(resourceType),
                                             QDir::Files | QDir::Readable,
                                             QDirIterator::Subdirectories));
    }

    ~FolderIterator() override {}

    bool hasNext() const override
    {
        return m_dirIterator->hasNext();
    }

    void next() override
    {
        m_dirIterator->next();
    }

    QString url() const override
    {
        return m_dirIterator->filePath();
    }

    QString type() const override
    {
        return m_resourceType;
    }

    QDateTime lastModified() const override
    {
        return m_dirIterator->fileInfo().lastModified();
    }

    KoResourceSP resource() const override
    {
        if (!loadResourceInternal()) {
            qWarning() << "Could not load resource" << m_dirIterator->filePath();
        }
        return m_resource;
    }

protected:

    bool loadResourceInternal() const {

        if (!m_resource || (m_resource && m_resource->filename() != m_dirIterator->filePath())) {
            QFile f(m_dirIterator->filePath());
            f.open(QFile::ReadOnly);
            QString mimeType = KisMimeDatabase::mimeTypeForFile(m_dirIterator->filePath());
            if (!m_resourceLoaders.contains(mimeType)) {
                KisResourceLoaderBase *resourceLoader = KisResourceLoaderRegistry::instance()->loader(m_resourceType, KisMimeDatabase::mimeTypeForFile(m_dirIterator->filePath()));
                if (!resourceLoader) { // cannot be an assert, because for an unknown file there won't be a loader, so let's not crash here
                    warnKrita << "Couldn't find a resource loader for " << m_dirIterator->filePath() << "mimetype = " << mimeType;
                    return false;
                }
                const_cast<FolderIterator*>(this)->m_resourceLoaders[mimeType] = resourceLoader;
            }
            const_cast<FolderIterator*>(this)->m_resource = m_resourceLoaders[mimeType]->load(m_dirIterator->fileName(), f, KisGlobalResourcesInterface::instance());
            f.close();
        }
        return !m_resource.isNull();
    }

    QMap<QString, KisResourceLoaderBase *> m_resourceLoaders;
    KoResourceSP m_resource;
    QScopedPointer<QDirIterator> m_dirIterator;
    const QString m_location;
    const QString m_resourceType;
};


KisFolderStorage::KisFolderStorage(const QString &location)
    : KisStoragePlugin(location)
{
}

KisFolderStorage::~KisFolderStorage()
{
}

bool KisFolderStorage::addTag(const QString &/*resourceType*/, KisTagSP /*tag*/)
{
    return false;
}

bool KisFolderStorage::addResource(const QString &resourceType, KoResourceSP _resource)
{
    QString fn = location() + "/" + resourceType + "/" + _resource->filename();
    bool update = QFileInfo(fn).exists();
    bool r = KisStorageVersioningHelper::addVersionedResource(fn, location() + "/" + resourceType, _resource);
    if (update) {
        _resource->setVersion(_resource->version() + 1);
    }
    return r;

}

KisResourceStorage::ResourceItem KisFolderStorage::resourceItem(const QString &url)
{
    QFileInfo fi(url);
    FolderItem item;
    item.url = url;
    item.folder = fi.path().split("/").last();
    item.lastModified = fi.lastModified();
    return item;
}

KoResourceSP KisFolderStorage::resource(const QString &url)
{
    QFileInfo fi(location() + '/' + url);
    const QString resourceType = fi.path().split("/").last();
    KisResourceLoaderBase *loader = KisResourceLoaderRegistry::instance()->loader(resourceType, KisMimeDatabase::mimeTypeForFile(fi.absoluteFilePath(), false));
    Q_ASSERT(loader);
    QFile f(fi.absoluteFilePath());
    if (!f.open(QFile::ReadOnly)) {
        qWarning() << "Could not open" << fi.absoluteFilePath() << "for reading";
        return 0;
    }

    KoResourceSP res = loader->load(fi.fileName(), f, KisGlobalResourcesInterface::instance());
    f.close();
    return res;
}

QSharedPointer<KisResourceStorage::ResourceIterator> KisFolderStorage::resources(const QString &resourceType)
{
    return QSharedPointer<KisResourceStorage::ResourceIterator>(new FolderIterator(location(), resourceType));
}

QSharedPointer<KisResourceStorage::TagIterator> KisFolderStorage::tags(const QString &resourceType)
{
    return QSharedPointer<KisResourceStorage::TagIterator>(new FolderTagIterator(location(), resourceType));
}

QStringList KisFolderStorage::metaDataKeys() const
{
    return QStringList() << KisResourceStorage::s_meta_name;
}

QVariant KisFolderStorage::metaData(const QString &key) const
{
    if (key == KisResourceStorage::s_meta_name) {
        return i18n("Local Resources");
    }
    return QVariant();

}
