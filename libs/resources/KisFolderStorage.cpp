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
#include <kis_pointer_utils.h>
#include <KoMD5Generator.h>


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

    KisTagSP tag() const override
    {
        return m_tag;
    }

private:

    bool load(KisTagSP tag) const
    {
        QFile f(m_dirIterator->filePath());
        tag->setFilename(m_dirIterator->fileName());
        if (f.exists()) {
            f.open(QFile::ReadOnly);
            if (!tag->load(f)) {
                qWarning() << m_dirIterator->filePath() << "is not a valid tag desktop file";
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

bool KisFolderStorage::saveAsNewVersion(const QString &resourceType, KoResourceSP _resource)
{
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

    bool r = resource->loadFromDevice(&f, KisGlobalResourcesInterface::instance());

    // Check for the thumbnail
    if (r) {
        sanitizeResourceFileNameCase(resource, fi.dir());

        if ((resource->image().isNull() || resource->thumbnail().isNull()) && !resource->thumbnailPath().isNull()) {
            QImage img(location() + '/' + resource->resourceType().first + '/' + resource->thumbnailPath());
            resource->setImage(img);
            resource->updateThumbnail();
        }
    }

    return r;
}

QString KisFolderStorage::resourceMd5(const QString &url)
{
    QString result;

    QFile file(location() + "/" + url);
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        result = KoMD5Generator::generateHash(file.readAll());
    }

    return result;
}

QSharedPointer<KisResourceStorage::ResourceIterator> KisFolderStorage::resources(const QString &resourceType)
{
    QVector<VersionedResourceEntry> entries;

    const QString resourcesSaveLocation = location() + "/" + resourceType;

    QDirIterator it(resourcesSaveLocation,
                    KisResourceLoaderRegistry::instance()->filters(resourceType),
                    QDir::Files | QDir::Readable,
                    QDirIterator::Subdirectories);;

    while (it.hasNext()) {
        it.next();
        QFileInfo info(it.fileInfo());

        VersionedResourceEntry entry;
        entry.filename = it.filePath().mid(resourcesSaveLocation.size() + 1);

        // Don't load 4.x backup resources
        if (entry.filename.contains("backup")) {
            continue;
        }

        entry.lastModified = info.lastModified();
        entry.tagList = {}; // TODO
        entry.resourceType = resourceType;
        entries.append(entry);
    }

    KisStorageVersioningHelper::detectFileVersions(entries);

    return toQShared(new KisVersionedStorageIterator(entries, this));
}

QSharedPointer<KisResourceStorage::TagIterator> KisFolderStorage::tags(const QString &resourceType)
{
    return QSharedPointer<KisResourceStorage::TagIterator>(new FolderTagIterator(location(), resourceType));
}

bool KisFolderStorage::importResource(const QString &url, QIODevice *device)
{
    bool result = false;

    const QString resourcesLocation = location() + "/" + url;

    QFile f(resourcesLocation);

    if (f.exists()) return result;

    if (f.open(QFile::WriteOnly)) {
        f.write(device->readAll());
        f.close();
        result = true;
    } else {
        qWarning() << "Cannot open" << resourcesLocation << "for writing";
    }

    return result;
}

bool KisFolderStorage::exportResource(const QString &url, QIODevice *device)
{
    bool result = false;

    const QString resourcesLocation = location() + "/" + url;

    QFile f(resourcesLocation);

    if (!f.exists()) return result;

    if (f.open(QFile::ReadOnly)) {
        device->write(f.readAll());
        f.close();
        result = true;
    } else {
        qWarning() << "Cannot open" << resourcesLocation << "for reading";
    }

    return result;
}

bool KisFolderStorage::addResource(const QString &resourceType, KoResourceSP resource)
{
    if (!resource || !resource->valid()) return false;

    const QString resourcesSaveLocation = location() + "/" + resourceType;

    QFileInfo fi(resourcesSaveLocation + "/" + resource->filename());
    if (fi.exists()) {
        qWarning() << "Resource" << resourceType << resource->filename() << "already exists in" << resourcesSaveLocation;
        return false;
    }

    QFile f(fi.absoluteFilePath());
    if (!f.open(QFile::WriteOnly)) {
        qWarning() << "Could not open" << fi.absoluteFilePath() << "for writing.";
        return false;
    }

    if (!resource->saveToDevice(&f)) {
        qWarning() << "Could not save resource to" << fi.absoluteFilePath();
        f.close();
        return false;
    }
    f.close();




    return true;
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
