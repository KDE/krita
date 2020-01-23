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

#include "KisBundleStorage.h"

#include <QDebug>
#include <QFileInfo>

#include <KisTag.h>
#include "KisResourceStorage.h"
#include "KoResourceBundle.h"
#include "KoResourceBundleManifest.h"

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
                    m_tags[tagname] = tag;
                }
                KoResourceSP resource = m_bundle->resource(resourceType, resourceReference.resourcePath);
                if (resource) {
                    m_tags[tagname]->setDefaultResources(m_tags[tagname]->defaultResources() << resource->name());
                } else {
                    qWarning() << tagname << "The following resource could not be tagged:" << resourceReference.resourcePath << "from" << bundle->filename();
                }
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

    QString url() const override { return m_tag ? m_tag->url() : QString(); }
    QString name() const override { return m_tag ? m_tag->name() : QString(); }
    QString comment() const override {return m_tag ? m_tag->comment() : QString(); }
    KisTagSP tag() const override { return m_tag; }

private:
    QHash<QString, KisTagSP> m_tags;
    KoResourceBundle *m_bundle {0};
    QString m_resourceType;
    QScopedPointer<QListIterator<KisTagSP> > m_tagIterator;
    KisTagSP m_tag;
};


class BundleIterator : public KisResourceStorage::ResourceIterator
{
public:
    BundleIterator(KoResourceBundle *bundle, const QString &resourceType)
        : m_bundle(bundle)
        , m_resourceType(resourceType)
    {
        m_entriesIterator.reset(new QListIterator<KoResourceBundleManifest::ResourceReference>(m_bundle->manifest().files(resourceType)));
    }

    bool hasNext() const override
    {
        return m_entriesIterator->hasNext();
    }

    void next() override
    {
        KoResourceBundleManifest::ResourceReference ref = m_entriesIterator->next();
        const_cast<BundleIterator*>(this)->m_resourceReference = ref;
    }

    QString url() const override
    {
        return m_resourceReference.resourcePath;
    }

    QString type() const override
    {
        return m_resourceType;
    }

    QDateTime lastModified() const override
    {
        return QFileInfo(m_bundle->filename()).lastModified();
    }

    /// This only loads the resource when called
    KoResourceSP resource() const override
    {
        return m_bundle->resource(m_resourceType, m_resourceReference.resourcePath);
    }

private:

    KoResourceBundle *m_bundle {0};
    QString m_resourceType;
    QScopedPointer<QListIterator<KoResourceBundleManifest::ResourceReference> > m_entriesIterator;
    KoResourceBundleManifest::ResourceReference m_resourceReference;

};


class KisBundleStorage::Private {
public:
    QScopedPointer<KoResourceBundle> bundle;
};


KisBundleStorage::KisBundleStorage(const QString &location)
    : KisStoragePlugin(location)
    , d(new Private())
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

KoResourceSP KisBundleStorage::resource(const QString &url)
{
    QStringList parts = url.split('/', QString::SkipEmptyParts);
    Q_ASSERT(parts.size() == 2);
    return d->bundle->resource(parts[0], url);
}

QSharedPointer<KisResourceStorage::ResourceIterator> KisBundleStorage::resources(const QString &resourceType)
{
    return QSharedPointer<KisResourceStorage::ResourceIterator>(new BundleIterator(d->bundle.data(), resourceType));
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

QString KisBundleStorage::metaData(const QString &key) const
{
    return d->bundle->metaData(key);
}

