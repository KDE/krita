/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisBundleStorage.h"

#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>

#include <KisTag.h>
#include "KisResourceStorage.h"
#include <KoMD5Generator.h>
#include "KoResourceBundle.h"
#include "KoResourceBundleManifest.h"
#include <KisGlobalResourcesInterface.h>

#include <KisResourceLoaderRegistry.h>
#include <kis_pointer_utils.h>
#include <kis_debug.h>

class KisBundleStorage::Private {
public:
    Private(KisBundleStorage *_q) : q(_q) {}

    KisBundleStorage *q;
    QScopedPointer<KoResourceBundle> bundle;
};


class BundleTagIterator : public KisResourceStorage::TagIterator
{
public:

    BundleTagIterator(KoResourceBundle *bundle, const QString &resourceType)
        : m_bundle(bundle)
        , m_resourceType(resourceType)
    {
        QList<KoResourceBundleManifest::ResourceReference> resources = m_bundle->manifest().files(resourceType);
        Q_FOREACH(const KoResourceBundleManifest::ResourceReference &resourceReference, resources) {
            Q_FOREACH(const QString &tagname, resourceReference.tagList) {
                if (!m_tags.contains(tagname)){
                    KisTagSP tag = QSharedPointer<KisTag>(new KisTag());
                    tag->setName(tagname);
                    tag->setComment(tagname);
                    tag->setUrl(tagname);
                    tag->setResourceType(resourceType);
                    tag->setValid(true);
                    m_tags[tagname] = tag;
                }

                m_tags[tagname]->setDefaultResources(m_tags[tagname]->defaultResources()
                                                     << QFileInfo(resourceReference.resourcePath).fileName());
            }
        }
        m_tagIterator.reset(new QListIterator<KisTagSP>(m_tags.values()));
    }

    bool hasNext() const override
    {
        return m_tagIterator->hasNext();
    }

    void next() override
    {
        const_cast<BundleTagIterator*>(this)->m_tag = m_tagIterator->next();
    }
    KisTagSP tag() const override { return m_tag; }

private:
    QHash<QString, KisTagSP> m_tags;
    KoResourceBundle *m_bundle {0};
    QString m_resourceType;
    QScopedPointer<QListIterator<KisTagSP> > m_tagIterator;
    KisTagSP m_tag;
};


KisBundleStorage::KisBundleStorage(const QString &location)
    : KisStoragePlugin(location)
    , d(new Private(this))
{
    d->bundle.reset(new KoResourceBundle(location));
    if (!d->bundle->load()) {
        qWarning() << "Could not load bundle" << location;
    }
}

KisBundleStorage::~KisBundleStorage()
{
}

KisResourceStorage::ResourceItem KisBundleStorage::resourceItem(const QString &url)
{
    KisResourceStorage::ResourceItem item;
    item.url = url;
    QStringList parts = url.split('/', QString::SkipEmptyParts);
    Q_ASSERT(parts.size() == 2);
    item.folder = parts[0];
    item.resourceType = parts[0];
    item.lastModified = QFileInfo(d->bundle->filename()).lastModified();
    return item;
}

bool KisBundleStorage::loadVersionedResource(KoResourceSP resource)
{
    bool foundVersionedFile = false;

    const QString resourceType = resource->resourceType().first;
    const QString resourceUrl = resourceType + "/" + resource->filename();

    const QString bundleSaveLocation = location() + "_modified" + "/" + resourceType;

    if (QDir(bundleSaveLocation).exists()) {
        const QString fn = bundleSaveLocation  + "/" + resource->filename();
        const QFileInfo fi(fn);
        if (fi.exists()) {
            foundVersionedFile = true;

            QFile f(fn);
            if (!f.open(QFile::ReadOnly)) {
                qWarning() << "Could not open resource file for reading" << fn;
                return false;
            }
            if (!resource->loadFromDevice(&f, KisGlobalResourcesInterface::instance())) {
                qWarning() << "Could not reload resource file" << fn;
                return false;
            }

            sanitizeResourceFileNameCase(resource, fi.dir());

            // Check for the thumbnail
            if ((resource->image().isNull() || resource->thumbnail().isNull()) && !resource->thumbnailPath().isNull()) {
                QImage img(bundleSaveLocation  + "/" +  '/' + resource->thumbnailPath());
                resource->setImage(img);
                resource->updateThumbnail();
            }
            f.close();
        }
    }

    if (!foundVersionedFile) {
        d->bundle->loadResource(resource);
    }

    return true;
}

QString KisBundleStorage::resourceMd5(const QString &url)
{
    QString result;

    QFile modifiedFile(location() + "_modified" + "/" + url);
    if (modifiedFile.exists() && modifiedFile.open(QIODevice::ReadOnly)) {
        result = KoMD5Generator::generateHash(modifiedFile.readAll());
    } else {
        result = d->bundle->resourceMd5(url);
    }

    return result;
}

QSharedPointer<KisResourceStorage::ResourceIterator> KisBundleStorage::resources(const QString &resourceType)
{
    QVector<VersionedResourceEntry> entries;

    QList<KoResourceBundleManifest::ResourceReference> references =
        d->bundle->manifest().files(resourceType);

    for (auto it = references.begin(); it != references.end(); ++it) {
        VersionedResourceEntry entry;
        // it->resourcePath() contains paths like "brushes/ink.png" or "brushes/subfolder/splash.png".
        // we need to cut off the first part and get "ink.png" in the first case,
        // but "subfolder/splash.png" in the second case in order for subfolders to work
        // so it cannot just use QFileInfo(verIt->url()).fileName() here.
        QString path = QDir::fromNativeSeparators(it->resourcePath); // make sure it uses Unix separators
        int folderEndIdx = path.indexOf("/");
        QString properFilenameWithSubfolders = path.right(path.length() - folderEndIdx - 1);

        entry.filename = properFilenameWithSubfolders;
        entry.lastModified = QFileInfo(location()).lastModified();
        entry.tagList = it->tagList;
        entry.resourceType = resourceType;
        entries.append(entry);
    }

    const QString bundleSaveLocation = location() + "_modified" + "/" + resourceType;

    QDirIterator it(bundleSaveLocation,
                    KisResourceLoaderRegistry::instance()->filters(resourceType),
                    QDir::Files | QDir::Readable,
                    QDirIterator::Subdirectories);;

    while (it.hasNext()) {
        it.next();
        QFileInfo info(it.fileInfo());

        VersionedResourceEntry entry;
        entry.filename = info.fileName();
        entry.lastModified = info.lastModified();
        entry.tagList = {}; // TODO
        entry.resourceType = resourceType;
        entries.append(entry);
    }

    KisStorageVersioningHelper::detectFileVersions(entries);

    return toQShared(new KisVersionedStorageIterator(entries, this));
}

QSharedPointer<KisResourceStorage::TagIterator> KisBundleStorage::tags(const QString &resourceType)
{
    return QSharedPointer<KisResourceStorage::TagIterator>(new BundleTagIterator(d->bundle.data(), resourceType));
}

QImage KisBundleStorage::thumbnail() const
{
    return d->bundle->image();
}

QStringList KisBundleStorage::metaDataKeys() const
{

    return QStringList() << KisResourceStorage::s_meta_generator
                         << KisResourceStorage::s_meta_author
                         << KisResourceStorage::s_meta_title
                         << KisResourceStorage::s_meta_description
                         << KisResourceStorage::s_meta_initial_creator
                         << KisResourceStorage::s_meta_creator
                         << KisResourceStorage::s_meta_creation_date
                         << KisResourceStorage::s_meta_dc_date
                         << KisResourceStorage::s_meta_user_defined
                         << KisResourceStorage::s_meta_name
                         << KisResourceStorage::s_meta_value
                         << KisResourceStorage::s_meta_version;

}

QVariant KisBundleStorage::metaData(const QString &key) const
{
    return d->bundle->metaData(key);
}

bool KisBundleStorage::saveAsNewVersion(const QString &resourceType, KoResourceSP resource)
{
    QString bundleSaveLocation = location() + "_modified" + "/" + resourceType;

    if (!QDir(bundleSaveLocation).exists()) {
        QDir().mkpath(bundleSaveLocation);
    }

    return KisStorageVersioningHelper::addVersionedResource(bundleSaveLocation, resource, 1);
}

bool KisBundleStorage::exportResource(const QString &url, QIODevice *device)
{
    QStringList parts = url.split('/', QString::SkipEmptyParts);
    Q_ASSERT(parts.size() == 2);

    const QString resourceType = parts[0];
    const QString resourceFileName = parts[1];

    bool foundVersionedFile = false;

    const QString bundleSaveLocation = location() + "_modified" + "/" + resourceType;

    if (QDir(bundleSaveLocation).exists()) {
        const QString fn = bundleSaveLocation  + "/" + resourceFileName;
        if (QFileInfo(fn).exists()) {
            foundVersionedFile = true;

            QFile f(fn);
            if (!f.open(QFile::ReadOnly)) {
                qWarning() << "Could not open resource file for reading" << fn;
                return false;
            }

            device->write(f.readAll());
        }
    }

    if (!foundVersionedFile) {
        d->bundle->exportResource(resourceType, resourceFileName, device);
    }

    return true;
}
