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

#include "KisResourceStorage.h"

#include <QFileInfo>
#include <QDebug>

#include <quazip.h>

#include "KisFolderStorage.h"
#include "KisBundleStorage.h"
#include "KisAslStorage.h"
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

Q_GLOBAL_STATIC(KisStoragePluginRegistry, s_instance);

KisStoragePluginRegistry::KisStoragePluginRegistry()
{
    m_storageFactoryMap[KisResourceStorage::StorageType::Folder] = new KisStoragePluginFactory<KisFolderStorage>();
    m_storageFactoryMap[KisResourceStorage::StorageType::Memory] = new KisStoragePluginFactory<KisMemoryStorage>();
    m_storageFactoryMap[KisResourceStorage::StorageType::Bundle] = new KisStoragePluginFactory<KisBundleStorage>();
    m_storageFactoryMap[KisResourceStorage::StorageType::AdobeStyleLibrary] = new KisStoragePluginFactory<KisAslStorage>();
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
};

KisResourceStorage::KisResourceStorage(const QString &location)
    : d(new Private())
{
    Q_ASSERT(KisStoragePluginRegistry::instance()->m_storageFactoryMap.contains(StorageType::Folder));
    Q_ASSERT(KisStoragePluginRegistry::instance()->m_storageFactoryMap.contains(StorageType::Bundle));
    Q_ASSERT(KisStoragePluginRegistry::instance()->m_storageFactoryMap.contains(StorageType::Memory));
    Q_ASSERT(KisStoragePluginRegistry::instance()->m_storageFactoryMap.contains(StorageType::AdobeBrushLibrary));
    Q_ASSERT(KisStoragePluginRegistry::instance()->m_storageFactoryMap.contains(StorageType::AdobeStyleLibrary));

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
}

KisResourceStorage::~KisResourceStorage()
{
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

QDateTime KisResourceStorage::timestamp() const
{
    if (d->storageType == StorageType::Memory) return QDateTime();

    return QFileInfo(d->location).lastModified();
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

bool KisResourceStorage::addResource(const QString &resourceType, KoResourceSP resource)
{
    return d->storagePlugin->addResource(resourceType, resource);
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

