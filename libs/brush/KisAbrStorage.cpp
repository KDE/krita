/*
 * Copyright (C) 2018 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2019 Agata Cacko <cacko.azh@gmail.com>
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

#include "KisAbrStorage.h"
#include "KisResourceStorage.h"

#include <QFileInfo>

struct KisAbrStorageStaticRegistrar {
    KisAbrStorageStaticRegistrar() {
        KisStoragePluginRegistry::instance()->addStoragePluginFactory(KisResourceStorage::StorageType::AdobeBrushLibrary, new KisStoragePluginFactory<KisAbrStorage>());
    }
};
static KisAbrStorageStaticRegistrar s_registrar;


class AbrTagIterator : public KisResourceStorage::TagIterator
{
public:
    AbrTagIterator(const QString &location, const QString &resourceType)
        : m_location(location)
        , m_resourceType(resourceType)
    {}

    bool hasNext() const override {return false; }
    void next() override {}

    QString url() const override { return QString(); }
    QString name() const override { return QString(); }
    QString comment() const override {return QString(); }
    KisTagSP tag() const override { return 0; }
private:

    QString m_location;
    QString m_resourceType;
};

class AbrIterator : public KisResourceStorage::ResourceIterator
{
public:
    KisAbrBrushCollectionSP m_brushCollection;
    QSharedPointer<QMap<QString, KisAbrBrushSP>> m_brushesMap;
    QMap<QString, KisAbrBrushSP>::const_key_value_iterator m_brushCollectionIterator;
    KisAbrBrushSP m_currentResource;
    bool isLoaded;
    QString m_currentUrl;


    AbrIterator(KisAbrBrushCollectionSP brushCollection)
        : m_brushCollection(brushCollection)
        , isLoaded(false)
    {
    }

    bool hasNext() const override
    {
        if (!isLoaded) {
            bool success = m_brushCollection->load();
            Q_UNUSED(success); // brush collection will be empty
            const_cast<AbrIterator*>(this)->m_brushesMap = m_brushCollection->brushesMap();
            const_cast<AbrIterator*>(this)->m_brushCollectionIterator = m_brushesMap->constKeyValueBegin();
            const_cast<AbrIterator*>(this)->isLoaded = true;
        }

        const_cast<AbrIterator*>(this)->m_brushCollectionIterator++;
        bool hasNext = (m_brushCollectionIterator != m_brushesMap->constKeyValueEnd());
        const_cast<AbrIterator*>(this)->m_brushCollectionIterator--;

        return hasNext;
    }

    void next() override
    {
        m_brushCollectionIterator++;
        std::pair<QString, KisAbrBrushSP> resourcePair = *m_brushCollectionIterator;
        m_currentResource = resourcePair.second;
        m_currentUrl = resourcePair.first;
    }

    QString url() const override { return m_currentUrl; }
    QString type() const override { return ResourceType::Brushes; }
    QDateTime lastModified() const override { return m_brushCollection->lastModified(); }

    KoResourceSP resource() const override
    {
        return m_currentResource;
    }
};

KisAbrStorage::KisAbrStorage(const QString &location)
    : KisStoragePlugin(location)
    , m_brushCollection(new KisAbrBrushCollection(location))
{

}

KisAbrStorage::~KisAbrStorage()
{

}

KisResourceStorage::ResourceItem KisAbrStorage::resourceItem(const QString &url)
{
    KisResourceStorage::ResourceItem item;
    item.url = url;
    // last "_" with index is the suffix added by abr_collection
    int indexOfUnderscore = url.lastIndexOf("_");
    QString filenameUrl = url;
    // filenameUrl contains the name of the collection (filename without .abr, brush name without index)
    filenameUrl.remove(indexOfUnderscore, url.length() - indexOfUnderscore);
    item.folder = filenameUrl;
    item.resourceType = ResourceType::Brushes;
    item.lastModified = QFileInfo(m_brushCollection->filename()).lastModified();
    return item;
}


KoResourceSP KisAbrStorage::resource(const QString &url)
{
    if (!m_brushCollection->isLoaded()) {
        m_brushCollection->load();
    }
    return m_brushCollection->brushByName(url);
}

QSharedPointer<KisResourceStorage::ResourceIterator> KisAbrStorage::resources(const QString &/*resourceType*/)
{
    return QSharedPointer<KisResourceStorage::ResourceIterator>(new AbrIterator(m_brushCollection));
}

QSharedPointer<KisResourceStorage::TagIterator> KisAbrStorage::tags(const QString &resourceType)
{
    return QSharedPointer<KisResourceStorage::TagIterator>(new AbrTagIterator(location(), resourceType));
}
