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
    FolderIterator(KisFolderStorage *_q, const QString &resourceType)
        : q(_q)
        , m_resourceType(resourceType)
    {
        m_dirIterator.reset(new QDirIterator(q->location() + '/' + resourceType,
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

    KoResourceSP resourceImpl() const override
    {
        return q->resource(m_resourceType + "/" + m_dirIterator->fileName());
    }

protected:

    KisFolderStorage *q;
    QScopedPointer<QDirIterator> m_dirIterator;
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
    return KisStorageVersioningHelper::addVersionedResource(location() + "/" + resourceType, _resource, 0);
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

bool KisFolderStorage::loadVersionedResource(KoResourceSP resource)
{
    QFileInfo fi(location() + '/' + resource->resourceType().first + '/' + resource->filename());

    QFile f(fi.absoluteFilePath());
    if (!f.open(QFile::ReadOnly)) {
        qWarning() << "Could not open" << fi.absoluteFilePath() << "for reading";
        return false;
    }

    return resource->loadFromDevice(&f, KisGlobalResourcesInterface::instance());
}

QSharedPointer<KisResourceStorage::ResourceIterator> KisFolderStorage::resources(const QString &resourceType)
{
    return QSharedPointer<KisResourceStorage::ResourceIterator>(new FolderIterator(this, resourceType));
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
