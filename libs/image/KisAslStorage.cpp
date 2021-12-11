/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisAslStorage.h"
#include <KisResourceStorage.h>
#include <kis_psd_layer_style.h>

#include <QFileInfo>

struct KisAslStorageStaticRegistrar {
    KisAslStorageStaticRegistrar() {
        KisStoragePluginRegistry::instance()->addStoragePluginFactory(KisResourceStorage::StorageType::AdobeStyleLibrary, new KisStoragePluginFactory<KisAslStorage>());
    }
};
static KisAslStorageStaticRegistrar s_registrar;


class AslTagIterator : public KisResourceStorage::TagIterator
{
public:

    AslTagIterator(const QString &location, const QString &resourceType)
        : m_location(location)
        , m_resourceType(resourceType)
    {}

    bool hasNext() const override {return false;}
    void next() override {}

    KisTagSP tag() const override { return nullptr; }

private:

    QString m_location;
    QString m_resourceType;

};

class AslIterator : public KisResourceStorage::ResourceIterator
{

private:

    QString m_filename;
    QSharedPointer<KisAslLayerStyleSerializer> m_aslSerializer;
    bool m_isLoaded;
    QHash<QString, KoPatternSP> m_patterns;
    QVector<KisPSDLayerStyleSP> m_styles;
    QScopedPointer<QHashIterator<QString, KoPatternSP>> m_patternsIterator;
    QScopedPointer<QVectorIterator<KisPSDLayerStyleSP>> m_stylesIterator;
    QString m_currentType;
    KoResourceSP m_currentResource;
    QString m_currentUuid;
    QString m_resourceType;

public:

    AslIterator(QSharedPointer<KisAslLayerStyleSerializer> aslSerializer, const QString& filename, const QString& resourceType)
        : m_filename(filename)
        , m_aslSerializer(aslSerializer)
        , m_isLoaded(false)
        , m_resourceType(resourceType)
    {
    }

    bool hasNext() const override
    {
        if (!m_isLoaded && (m_resourceType == ResourceType::Patterns || m_resourceType == ResourceType::LayerStyles)) {
            if (!m_aslSerializer->isInitialized()) {
                m_aslSerializer->readFromFile(m_filename);
            }

            const_cast<AslIterator*>(this)->m_isLoaded = true;
            const_cast<AslIterator*>(this)->m_patterns = m_aslSerializer->patterns();
            const_cast<AslIterator*>(this)->m_styles = m_aslSerializer->styles();

            const_cast<AslIterator*>(this)->m_patternsIterator.reset(new QHashIterator<QString, KoPatternSP>(m_patterns));
            const_cast<AslIterator*>(this)->m_stylesIterator.reset(new QVectorIterator<KisPSDLayerStyleSP>(m_styles));
        }
        if (!m_aslSerializer->isValid()) {
            return false;
        }

        if (m_resourceType == ResourceType::Patterns) {
            return m_patternsIterator->hasNext();
        } else if (m_resourceType == ResourceType::LayerStyles) {
            return m_stylesIterator->hasNext();
        }
        return false;
    }
    void next() override
    {
        if (m_resourceType == ResourceType::Patterns) {
            if (m_patternsIterator->hasNext()) {
                m_currentType = ResourceType::Patterns;
                m_patternsIterator->next();
                KoPatternSP currentPattern = m_patternsIterator->value();
                m_currentResource = currentPattern;
                KIS_ASSERT(currentPattern);
                m_currentUuid = currentPattern->filename();
            }
        } else if (m_resourceType == ResourceType::LayerStyles) {
            if (m_stylesIterator->hasNext()) {
                m_currentType = ResourceType::LayerStyles;
                KisPSDLayerStyleSP currentLayerStyle = m_stylesIterator->next();
                m_currentResource = currentLayerStyle;
                KIS_ASSERT(currentLayerStyle);
                m_currentUuid = currentLayerStyle->filename();
            }
        }
    }

    QString url() const override
    {
        if (m_currentResource.isNull()) {
            return QString();
        }
        return m_currentUuid;
    }

    QString type() const override
    {
        return m_currentResource.isNull() ? QString() : m_currentType;
    }

    QDateTime lastModified() const override {
        QFileInfo fi(m_filename);
        return fi.lastModified();
    }


    /// This only loads the resource when called (but not in case of asl...)
    KoResourceSP resourceImpl() const override
    {
        return m_currentResource;
    }
};

KisAslStorage::KisAslStorage(const QString &location)
    : KisStoragePlugin(location)
    , m_aslSerializer(new KisAslLayerStyleSerializer())
{
}

KisAslStorage::~KisAslStorage()
{

}

KisResourceStorage::ResourceItem KisAslStorage::resourceItem(const QString &url)
{
    KisResourceStorage::ResourceItem item;
    item.url = url;
    item.folder = location();
    item.resourceType = url.contains("pattern") ? ResourceType::Patterns : ResourceType::LayerStyles;
    item.lastModified = QFileInfo(location()).lastModified();
    return item;
}

KoResourceSP KisAslStorage::resource(const QString &url)
{
    if (!m_aslSerializer->isInitialized()) {
        m_aslSerializer->readFromFile(location());
    }
    int indexOfUnderscore = url.lastIndexOf("_");
    QString realUuid = url;
    if (indexOfUnderscore >= 0) {
        realUuid.remove(indexOfUnderscore, url.length() - indexOfUnderscore); // remove _pattern or _style added in iterator
    }
    // TODO: RESOURCES: Since we do get a resource type at the beginning of the path now
    //  maybe we could skip adding the _[resourcetype] at the end of the path as well?
    realUuid = QFileInfo(realUuid).baseName(); // remove patterns/ at the beginning, if there are any

    if (url.contains("pattern") || url.contains(".pat")) {
        QHash<QString, KoPatternSP> patterns = m_aslSerializer->patterns();

        if (patterns.contains(realUuid)) {
            return patterns[realUuid];
        }
    }
    else {
        QHash<QString, KisPSDLayerStyleSP> styles = m_aslSerializer->stylesHash();
        if (styles.contains(realUuid)) {
            return styles[realUuid];
        } else {
            // can be {realUuid} or {realUuid}
            if (realUuid.startsWith("{")) {
                realUuid = realUuid.right(realUuid.length() - 1);
            }
            if (realUuid.endsWith("}")) {
                realUuid = realUuid.left(realUuid.length() - 1);
            }

            if (styles.contains(realUuid)) {
                return styles[realUuid];
            } else {
                Q_FOREACH(QString ke, styles.keys()) {
                }
            }

        }
    }
    return 0;
}

bool KisAslStorage::loadVersionedResource(KoResourceSP /*resource*/)
{
    return false;
}

bool KisAslStorage::supportsVersioning() const
{
    return false;
}

QSharedPointer<KisResourceStorage::ResourceIterator> KisAslStorage::resources(const QString &resourceType)
{
    return QSharedPointer<KisResourceStorage::ResourceIterator>(new AslIterator(m_aslSerializer, location(), resourceType));
}

QSharedPointer<KisResourceStorage::TagIterator> KisAslStorage::tags(const QString &resourceType)
{
    return QSharedPointer<KisResourceStorage::TagIterator>(new AslTagIterator(location(), resourceType));
}

bool KisAslStorage::saveAsNewVersion(const QString &/*resourceType*/, KoResourceSP /*resource*/)
{
    // not implemented yet
    warnKrita << "KisAslStorage::saveAsNewVersion is not implemented yet";
    return false;
}

bool KisAslStorage::addResource(const QString &/*resourceType*/, KoResourceSP resource)
{
    if (!resource) {
        warnKrita << "Trying to add a null resource to KisAslStorage";
        return false;
    }
    KisPSDLayerStyleSP layerStyle = resource.dynamicCast<KisPSDLayerStyle>();
    if (!layerStyle) {
        warnKrita << "Trying to add a resource that is not a layer style to KisAslStorage";
        return false;
    }

    QVector<KisPSDLayerStyleSP> styles = m_aslSerializer->styles();
    styles << layerStyle;
    m_aslSerializer->setStyles(styles);
    return m_aslSerializer->saveToFile(location());
}

bool KisAslStorage::isValid() const
{
    if (!m_aslSerializer->isInitialized()) {
        m_aslSerializer->readFromFile(location());
    }
    return m_aslSerializer->isValid();
}
