/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisResourceStorage.h"

#include <QFileInfo>
#include <QDebug>

#include <quazip.h>

#include "KisFolderStorage.h"
#include "KisBundleStorage.h"
#include "KisMemoryStorage.h"

const QString KisResourceStorage::s_meta_generator("meta:generator");
const QString KisResourceStorage::s_meta_author("dc:author");
const QString KisResourceStorage::s_meta_title("dc:title");
const QString KisResourceStorage::s_meta_description("dc:description");
const QString KisResourceStorage::s_meta_initial_creator("meta:initial-creator");
const QString KisResourceStorage::s_meta_creator("cd:creator");
const QString KisResourceStorage::s_meta_creation_date("meta:creation-data");
const QString KisResourceStorage::s_meta_dc_date("meta:dc-date");
const QString KisResourceStorage::s_meta_user_defined("meta:meta-userdefined");
const QString KisResourceStorage::s_meta_name("meta:name");
const QString KisResourceStorage::s_meta_value("meta:value");
const QString KisResourceStorage::s_meta_version("meta:bundle-version");
const QString KisResourceStorage::s_meta_email("meta:email");
const QString KisResourceStorage::s_meta_license("meta:license");

Q_GLOBAL_STATIC(KisStoragePluginRegistry, s_instance);

KisStoragePluginRegistry::KisStoragePluginRegistry()
{
    m_storageFactoryMap[KisResourceStorage::StorageType::Folder] = new KisStoragePluginFactory<KisFolderStorage>();
    m_storageFactoryMap[KisResourceStorage::StorageType::Memory] = new KisStoragePluginFactory<KisMemoryStorage>();
    m_storageFactoryMap[KisResourceStorage::StorageType::Bundle] = new KisStoragePluginFactory<KisBundleStorage>();
}

void KisStoragePluginRegistry::addStoragePluginFactory(KisResourceStorage::StorageType storageType, KisStoragePluginFactoryBase *factory)
{
    m_storageFactoryMap[storageType] = factory;
}

KisStoragePluginRegistry *KisStoragePluginRegistry::instance()
{
    return s_instance;
}

class KisResourceStorage::Private {
public:
    QString name;
    QString location;
    bool valid {false};
    KisResourceStorage::StorageType storageType {KisResourceStorage::StorageType::Unknown};
    QSharedPointer<KisStoragePlugin> storagePlugin;
    int storageId {-1};
};

KisResourceStorage::KisResourceStorage(const QString &location)
    : d(new Private())
{
    d->location = location;
    d->name = QFileInfo(d->location).fileName();
    QFileInfo fi(d->location);
    if (fi.isDir()) {
        d->storagePlugin.reset(KisStoragePluginRegistry::instance()->m_storageFactoryMap[StorageType::Folder]->create(location));
        d->storageType = StorageType::Folder;
        d->valid = fi.isWritable();
    }
    else if (d->location.endsWith(".bundle")) {
            d->storagePlugin.reset(KisStoragePluginRegistry::instance()->m_storageFactoryMap[StorageType::Bundle]->create(location));
            d->storageType = StorageType::Bundle;
            // XXX: should we also check whether there's a valid metadata entry? Or is this enough?
            d->valid = (fi.isReadable() && QuaZip(d->location).open(QuaZip::mdUnzip));
    } else if (d->location.endsWith(".abr")) {
            d->storagePlugin.reset(KisStoragePluginRegistry::instance()->m_storageFactoryMap[StorageType::AdobeBrushLibrary]->create(location));
            d->storageType = StorageType::AdobeBrushLibrary;
            d->valid = fi.isReadable();
    } else if (d->location.endsWith(".asl")) {
            d->storagePlugin.reset(KisStoragePluginRegistry::instance()->m_storageFactoryMap[StorageType::AdobeStyleLibrary]->create(location));
            d->storageType = StorageType::AdobeStyleLibrary;
            d->valid = fi.isReadable();
    }
    else if (!d->location.isEmpty()) {
        d->storagePlugin.reset(KisStoragePluginRegistry::instance()->m_storageFactoryMap[StorageType::Memory]->create(location));
        d->name = location;
        d->storageType = StorageType::Memory;
        d->valid = true;
    }
    else {
        d->valid = false;
    }
}

KisResourceStorage::~KisResourceStorage()
{
}

KisResourceStorage::KisResourceStorage(const KisResourceStorage &rhs)
    : d(new Private)
{
    *this = rhs;
}

KisResourceStorage &KisResourceStorage::operator=(const KisResourceStorage &rhs)
{
    if (this != &rhs) {
        d->name = rhs.d->name;
        d->location = rhs.d->location;
        d->storageType = rhs.d->storageType;
        if (d->storageType == StorageType::Memory) {
            d->storagePlugin = QSharedPointer<KisMemoryStorage>(new KisMemoryStorage(*dynamic_cast<KisMemoryStorage*>(rhs.d->storagePlugin.data())));
        }
        else {
            d->storagePlugin = rhs.d->storagePlugin;
        }
        d->valid = false;
    }
    return *this;
}

KisResourceStorageSP KisResourceStorage::clone() const
{
    return KisResourceStorageSP(new KisResourceStorage(*this));
}

QString KisResourceStorage::name() const
{
    return d->name;
}

QString KisResourceStorage::location() const
{
    return d->location;
}

KisResourceStorage::StorageType KisResourceStorage::type() const
{
    return d->storageType;
}

QImage KisResourceStorage::thumbnail() const
{
    return d->storagePlugin->thumbnail();
}

QDateTime KisResourceStorage::timestamp() const
{
    return d->storagePlugin->timestamp();
}

QDateTime KisResourceStorage::timeStampForResource(const QString &resourceType, const QString &filename) const
{
    QFileInfo li(d->location);
    if (li.suffix().toLower() == "bundle") {
        QFileInfo bf(d->location + "_modified/" + resourceType + "/" + filename);
        if (bf.exists()) {
            return bf.lastModified();
        }
    } else if (QFileInfo(d->location + "/" + resourceType + "/" + filename).exists()) {
        return QFileInfo(d->location + "/" + resourceType + "/" + filename).lastModified();
    }
    return this->timestamp();
}

KisResourceStorage::ResourceItem KisResourceStorage::resourceItem(const QString &url)
{
    return d->storagePlugin->resourceItem(url);
}

KoResourceSP KisResourceStorage::resource(const QString &url)
{
    return d->storagePlugin->resource(url);
}

QSharedPointer<KisResourceStorage::ResourceIterator> KisResourceStorage::resources(const QString &resourceType) const
{
    return d->storagePlugin->resources(resourceType);
}

QSharedPointer<KisResourceStorage::TagIterator> KisResourceStorage::tags(const QString &resourceType) const
{
    return d->storagePlugin->tags(resourceType);
}

bool KisResourceStorage::addTag(const QString &resourceType, KisTagSP tag)
{
    return d->storagePlugin->addTag(resourceType, tag);
}

bool KisResourceStorage::addResource(KoResourceSP resource)
{
    return d->storagePlugin->addResource(resource->resourceType().first, resource);
}

void KisResourceStorage::setMetaData(const QString &key, const QVariant &value)
{
    d->storagePlugin->setMetaData(key, value);
}

bool KisResourceStorage::valid() const
{
    return d->valid;
}

QStringList KisResourceStorage::metaDataKeys() const
{
    return d->storagePlugin->metaDataKeys();
}

QVariant KisResourceStorage::metaData(const QString &key) const
{
    return d->storagePlugin->metaData(key);
}

void KisResourceStorage::setStorageId(int storageId)
{
    d->storageId = storageId;
}

int KisResourceStorage::storageId()
{
    return d->storageId;
}

bool KisStorageVersioningHelper::addVersionedResource(const QString &filename, const QString &saveLocation, KoResourceSP resource)
{
    // Find a new filename for the resource if it already exists: we do not rename old resources, but rename updated resources
    if (!QFileInfo(filename).exists()) {
        // Simply save it
        QFile f(filename);
        if (!f.open(QFile::WriteOnly)) {
            qWarning() << "Could not open resource file for writing" << filename;
            return false;
        }
        if (!resource->saveToDevice(&f)) {
            qWarning() << "Could not save resource file" << filename;
            return false;
        }
        f.close();
    }
    else {
        QFileInfo fi(filename);

        // Save the new resource
        QString newFilename = fi.baseName() +
                "."
                + QString("%1").arg(resource->version() + 1, 4, 10, QChar('0'))
                + "."
                + fi.suffix();
        QFile f(saveLocation + "/" + newFilename);

        if (!f.open(QFile::WriteOnly)) {
            qWarning() << "Could not open resource file for writing" << newFilename;
            return false;
        }
        if (!resource->saveToDevice(&f)) {
            qWarning() << "Could not save resource file" << newFilename;
            return false;
        }
        resource->setFilename(newFilename);
        f.close();
    }
    return true;
};

