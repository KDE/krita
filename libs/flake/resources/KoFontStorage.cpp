/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoFontStorage.h"
#include "KoFontFamily.h"
#include "KoFontRegistry.h"
#include "KisStaticInitializer.h"
#include <KoMD5Generator.h>
#include <optional>

KIS_DECLARE_STATIC_INITIALIZER {
    KisStoragePluginRegistry::instance()->addStoragePluginFactory(KisResourceStorage::StorageType::FontStorage, new KisStoragePluginFactory<KoFontStorage>());
}

class FontTagIterator : public KisResourceStorage::TagIterator
{
public:

    FontTagIterator(QVector<KisTagSP> /*tags*/, const QString &resourceType)
        : m_resourceType(resourceType)
    {
    }

    bool hasNext() const override
    {
        return false;
    }

    void next() override
    {
    }

    KisTagSP tag() const override
    {
        return nullptr;
    }

private:
    QString m_resourceType;
};

class FontIterator : public KisResourceStorage::ResourceIterator
{
public:
    FontIterator(const QString resourceType): m_isLoaded(false), m_resourceType(resourceType) {

    }

    QString type() const override {
        return ResourceType::FontFamilies;
    }

    QDateTime lastModified() const override {
        return m_currentResource->lastModified();
    }

    bool hasNext() const override {
        if (m_resourceType != ResourceType::FontFamilies) return false;
        if (!m_isLoaded) {
            const_cast<FontIterator*>(this)->m_representationIterator.reset(new QListIterator<KoFontFamilyWWSRepresentation>(KoFontRegistry::instance()->collectRepresentations()));
            const_cast<FontIterator*>(this)->m_isLoaded = true;
        }

        return m_representationIterator->hasNext();
    }

    void next() override {
        KoFontFamilyWWSRepresentation rep = m_representationIterator->next();
        m_currentResource = KoFontFamilySP (new KoFontFamily(rep));
    }

    QString url() const override {
        if (m_currentResource.isNull()) {
            return QString();
        }
        return m_currentResource->filename();
    }

    KoResourceSP resourceImpl() const override {
        m_currentResource->updateThumbnail();
        return m_currentResource;
    }
private:
    bool m_isLoaded;
    QString m_resourceType;
    KoFontFamilySP m_currentResource;
    QScopedPointer<QListIterator<KoFontFamilyWWSRepresentation>> m_representationIterator;
};

KoFontStorage::KoFontStorage(const QString &location)
    : KisStoragePlugin(location)
{
}

KoFontStorage::~KoFontStorage()
{
}

KisResourceStorage::ResourceItem KoFontStorage::resourceItem(const QString &url)
{
    KisResourceStorage::ResourceItem item;
    item.resourceType = ResourceType::FontFamilies;
    item.url = url;
    item.folder = location();
    item.lastModified =  QDateTime::fromMSecsSinceEpoch(0);
    return item;
}

KoResourceSP KoFontStorage::resource(const QString &url)
{
    KoFontFamilySP fam;
    QString familyName = url;
    QString prefix (ResourceType::FontFamilies+"/");
    if (familyName.startsWith(prefix)) {
        familyName.remove(0, prefix.size());
    }

    std::optional<KoFontFamilyWWSRepresentation> rep = KoFontRegistry::instance()->representationByFamilyName(familyName);
    if (rep) {
        fam.reset(new KoFontFamily(rep.value()));
        fam->updateThumbnail();
    }

    return fam;
}

bool KoFontStorage::supportsVersioning() const
{
    // Even though it doesn't make sense, this needs to support versioning, otherwise the thumbnail is never updated...
    return true;
}

QSharedPointer<KisResourceStorage::ResourceIterator> KoFontStorage::resources(const QString &resourceType)
{
    return QSharedPointer<KisResourceStorage::ResourceIterator>(new FontIterator(resourceType));
}

QSharedPointer<KisResourceStorage::TagIterator> KoFontStorage::tags(const QString &resourceType)
{
    return QSharedPointer<KisResourceStorage::TagIterator>(new FontTagIterator(QVector<KisTagSP>(), resourceType));
}

bool KoFontStorage::isValid() const
{
    return true;
}

bool KoFontStorage::loadVersionedResource(KoResourceSP resource)
{
    //Q_UNUSED(resource);
    resource->updateThumbnail();
    return true;
}
