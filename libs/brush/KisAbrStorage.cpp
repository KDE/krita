/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2019 Agata Cacko <cacko.azh@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
    AbrTagIterator(KisAbrBrushCollectionSP brushCollection, const QString &location, const QString &resourceType)
        : m_brushCollection(brushCollection)
        , m_location(location)
        , m_resourceType(resourceType)
    {}

    bool hasNext() const override {
        if (resourceType() != ResourceType::Brushes) return false;
        return !m_taggingDone;
    }
    void next() override { m_taggingDone = true; }

    QString url() const override { return m_location; }
    QString name() const override { return QFileInfo(m_location).fileName(); }
    QString resourceType() const override { return m_resourceType; }
    QString comment() const override {return QString(); }
    QString filename() const override {return QString(); }
    KisTagSP tag() const override
    {
        KisTagSP abrTag(new KisTag());
        abrTag->setUrl(url());
        abrTag->setComment(comment());
        abrTag->setName(name());
        abrTag->setFilename(filename());
        abrTag->setResourceType(resourceType());
        QStringList brushes;
        Q_FOREACH(const KisAbrBrushSP brush, m_brushCollection->brushes()) {
            brushes << brush->filename();
        }
        abrTag->setDefaultResources(brushes);

        return abrTag;
    }

private:

    bool m_taggingDone {false};
    KisAbrBrushCollectionSP m_brushCollection;
    QString m_location;
    QString m_resourceType;
};

class AbrIterator : public KisResourceStorage::ResourceIterator
{
public:
    KisAbrBrushCollectionSP m_brushCollection;
    QSharedPointer<QMap<QString, KisAbrBrushSP>> m_brushesMap;
    QMap<QString, KisAbrBrushSP>::const_iterator m_brushCollectionIterator;
    KisAbrBrushSP m_currentResource;
    bool isLoaded;
    QString m_currentUrl;
    QString m_resourceType;


    AbrIterator(KisAbrBrushCollectionSP brushCollection, const QString& resourceType)
        : m_brushCollection(brushCollection)
        , isLoaded(false)
        , m_resourceType(resourceType)
    {
    }

    bool hasNext() const override
    {
        if (m_resourceType != ResourceType::Brushes) {
            return false;
        }

        if (!isLoaded) {
            bool success = m_brushCollection->load();
            Q_UNUSED(success); // brush collection will be empty
            const_cast<AbrIterator*>(this)->m_brushesMap = m_brushCollection->brushesMap();
            const_cast<AbrIterator*>(this)->m_brushCollectionIterator = m_brushesMap->constBegin();
            const_cast<AbrIterator*>(this)->isLoaded = true;
        }

        if (m_brushCollectionIterator == m_brushesMap->constEnd()) {
            return false;
        }

        bool hasNext = (m_brushCollectionIterator != m_brushesMap->constEnd());
        return hasNext;
    }

    void next() override
    {
        if (m_resourceType != ResourceType::Brushes) {
            return;
        }
        KIS_SAFE_ASSERT_RECOVER_RETURN(m_brushCollectionIterator != m_brushesMap->constEnd());
        m_currentResource = m_brushCollectionIterator.value();
        m_currentUrl = m_brushCollectionIterator.key();
        m_brushCollectionIterator++;
    }

    QString url() const override { return m_currentUrl; }
    QString type() const override { return ResourceType::Brushes; }
    QDateTime lastModified() const override { return m_brushCollection->lastModified(); }

    KoResourceSP resourceImpl() const override
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
    return m_brushCollection->brushByName(QFileInfo(url).fileName());
}

bool KisAbrStorage::loadVersionedResource(KoResourceSP /*resource*/)
{
    return false;
}

bool KisAbrStorage::supportsVersioning() const
{
    return false;
}

QSharedPointer<KisResourceStorage::ResourceIterator> KisAbrStorage::resources(const QString &resourceType)
{
    return QSharedPointer<KisResourceStorage::ResourceIterator>(new AbrIterator(m_brushCollection, resourceType));
}

QSharedPointer<KisResourceStorage::TagIterator> KisAbrStorage::tags(const QString &resourceType)
{
    return QSharedPointer<KisResourceStorage::TagIterator>(new AbrTagIterator(m_brushCollection, location(), resourceType));
}

QImage KisAbrStorage::thumbnail() const
{
    return m_brushCollection->image();
}
