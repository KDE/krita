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

#include "KisFolderStorage.h"

#include <QDirIterator>
#include <KisMimeDatabase.h>
#include <kis_debug.h>
#include <KisTag.h>
#include <KisResourceLoaderRegistry.h>

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

    void next() const override
    {
        m_dirIterator->next();
        const_cast<FolderTagIterator*>(this)->m_tag.reset(new KisTag);
        load(m_tag);
    }

    QString url() const override { return m_tag ? m_tag->url() : QString(); }
    QString name() const override { return m_tag ? m_tag->name() : QString(); }
    QString comment() const override {return m_tag ? m_tag->comment() : QString(); }

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

    void next() const override
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
            if (!m_resourceLoader) {
                const_cast<FolderIterator*>(this)->m_resourceLoader = KisResourceLoaderRegistry::instance()->loader(m_resourceType, KisMimeDatabase::mimeTypeForFile(m_dirIterator->filePath()));
            }
            if (m_resourceLoader) {
                const_cast<FolderIterator*>(this)->m_resource = m_resourceLoader->load(m_dirIterator->filePath(), f);
            }
            f.close();
        }
        return !m_resource.isNull();
    }

    KisResourceLoaderBase *m_resourceLoader {0};
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

bool KisFolderStorage::addTag(const QString &resourceType, KisTagSP tag)
{
    return false;
}

bool KisFolderStorage::addResource(const QString &resourceType, KoResourceSP _resource)
{
    qDebug() << location() << _resource->filename() << _resource->shortFilename() << _resource->name();
    // Find a new filename for the resource if it already exists: we do not rename old resources, but rename updated resources

    return false;
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
    KoResourceSP res = loader->load(url, f);
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
