/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisBundleStorage.h"

#include <QDebug>
#include <QFileInfo>
#include <QDir>

#include <KisTag.h>
#include "KisResourceStorage.h"
#include "KoResourceBundle.h"
#include "KoResourceBundleManifest.h"
#include <KisGlobalResourcesInterface.h>
#include <kis_debug.h>

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
                    m_tags[tagname] = tag;
                }
                KoResourceSP resource = m_bundle->resource(resourceType, resourceReference.resourcePath);
                if (resource) {
                    m_tags[tagname]->setDefaultResources(m_tags[tagname]->defaultResources() << resource->filename());
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
    QString resourceType() const override { return m_resourceType; } // Tags in bundles are still lists, not KisTag files.

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
    BundleIterator(KisBundleStorage *_q, KoResourceBundle *bundle, const QString &resourceType)
        : q(_q),
          m_bundle(bundle)
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
        const QStringList parts = m_resourceReference.resourcePath.split('/', QString::SkipEmptyParts);
        Q_ASSERT(parts.size() == 2);
        return q->resource(m_resourceReference.resourcePath);
    }

private:
    KisBundleStorage *q {0};
    KoResourceBundle *m_bundle {0};
    QString m_resourceType;
    QScopedPointer<QListIterator<KoResourceBundleManifest::ResourceReference> > m_entriesIterator;
    KoResourceBundleManifest::ResourceReference m_resourceReference;

};


class KisBundleStorage::Private {
public:
    Private(KisBundleStorage *_q) : q(_q) {}

    KisBundleStorage *q;
    QScopedPointer<KoResourceBundle> bundle;
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
        if (QFileInfo(fn).exists()) {
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
            f.close();
        }
    }

    if (!foundVersionedFile) {
        d->bundle->loadResource(resource);
    }

    return true;
}

QSharedPointer<KisResourceStorage::ResourceIterator> KisBundleStorage::resources(const QString &resourceType)
{
    return QSharedPointer<KisResourceStorage::ResourceIterator>(new BundleIterator(this, d->bundle.data(), resourceType));
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

bool KisBundleStorage::addResource(const QString &resourceType, KoResourceSP resource)
{
    QString bundleSaveLocation = location() + "_modified" + "/" + resourceType;

    if (!QDir(bundleSaveLocation).exists()) {
        QDir().mkpath(bundleSaveLocation);
    }

    return KisStorageVersioningHelper::addVersionedResource(bundleSaveLocation, resource, 1);
}
