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

#include "KisAslStorage.h"
#include <KisResourceStorage.h>

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

public:

    AslIterator(QSharedPointer<KisAslLayerStyleSerializer> aslSerializer, const QString& filename)
        : m_filename(filename)
        , m_aslSerializer(aslSerializer)
        , m_isLoaded(false)
    {

    }

    bool hasNext() const override
    {
        if (!m_isLoaded) {
            if (!m_aslSerializer->isInitialized()) {
                m_aslSerializer->readFromFile(m_filename);
            }

            const_cast<AslIterator*>(this)->m_isLoaded = true;
            const_cast<AslIterator*>(this)->m_patterns = m_aslSerializer->patterns();
            const_cast<AslIterator*>(this)->m_styles = m_aslSerializer->styles();

            const_cast<AslIterator*>(this)->m_patternsIterator.reset(new QHashIterator<QString, KoPatternSP>(m_patterns));
            const_cast<AslIterator*>(this)->m_stylesIterator.reset(new QVectorIterator<KisPSDLayerStyleSP>(m_styles));

            QHash<QString, KisPSDLayerStyleSP> layerStyles = const_cast<AslIterator*>(this)->m_aslSerializer->stylesHash();
        }
        return m_patternsIterator->hasNext() ? true : m_stylesIterator->hasNext();
    }
    void next() override
    {
        if (m_patternsIterator->hasNext()) {
            m_currentType = ResourceType::Patterns;
            m_patternsIterator->next();
            KoPatternSP currentPattern = m_patternsIterator->value();
            m_currentResource = currentPattern;
            m_currentUuid = m_patternsIterator->value()->filename();
        }
        else if (m_stylesIterator->hasNext()) {
            m_currentType = ResourceType::LayerStyles;
            KisPSDLayerStyleSP currentLayerStyle = m_stylesIterator->next();
            m_currentResource = currentLayerStyle;
            m_currentUuid = currentLayerStyle->filename();
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

    QDateTime lastModified() const override { return QDateTime(); }


    /// This only loads the resource when called
    KoResourceSP resource() const override
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
    realUuid.remove(indexOfUnderscore, url.length() - indexOfUnderscore); // remove _pattern or _style added in iterator
    if (url.contains("pattern")) {
        QHash<QString, KoPatternSP> patterns = m_aslSerializer->patterns();

        if (patterns.contains(realUuid)) {
            return patterns[realUuid];
        }
    }
    else {
        QHash<QString, KisPSDLayerStyleSP> styles = m_aslSerializer->stylesHash();
        if (styles.contains(realUuid)) {
            return styles[realUuid];
        }
    }
    return 0;
}

QSharedPointer<KisResourceStorage::ResourceIterator> KisAslStorage::resources(const QString &/*resourceType*/)
{
    return QSharedPointer<KisResourceStorage::ResourceIterator>(new AslIterator(m_aslSerializer, location()));
}

QSharedPointer<KisResourceStorage::TagIterator> KisAslStorage::tags(const QString &resourceType)
{
    return QSharedPointer<KisResourceStorage::TagIterator>(new AslTagIterator(location(), resourceType));
}
