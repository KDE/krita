/*
 * Copyright (C) 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisMemoryStorage.h"

#include <QVector>
#include <QFileInfo>

#include <KisMimeDatabase.h>
#include <kis_debug.h>
#include <KisTag.h>
#include <KisResourceLoaderRegistry.h>
#include <KisResourceStorage.h>


class MemoryTagIterator : public KisResourceStorage::TagIterator
{
public:

    MemoryTagIterator(QVector<KisTagSP> tags, const QString &resourceType)
        : m_tags(tags)
        , m_resourceType(resourceType)
    {
    }

    bool hasNext() const override
    {
        return m_currentPosition < m_tags.size();
    }

    void next() override
    {
        const_cast<MemoryTagIterator*>(this)->m_currentPosition += 1;
    }

    QString url() const override { return tag() ? tag()->url() : QString(); }
    QString name() const override { return tag() ? tag()->name() : QString(); }
    QString comment() const override {return tag() ? tag()->comment() : QString(); }

    KisTagSP tag() const override
    {
        if (m_currentPosition >= m_tags.size()) return 0;
        return m_tags.at(m_currentPosition);

    }

private:

    int m_currentPosition {0};
    QVector<KisTagSP> m_tags;
    QString m_resourceType;
};


class MemoryItem : public KisResourceStorage::ResourceItem
{
public:
    ~MemoryItem() override {}
};

class MemoryIterator : public KisResourceStorage::ResourceIterator
{
public:
    MemoryIterator(QVector<KoResourceSP> resources, const QString &resourceType)
        : m_resources(resources)
        , m_resourceType(resourceType)
    {
    }

    ~MemoryIterator() override {}

    bool hasNext() const override
    {
        return m_currentPosition < m_resources.size();
    }

    void next() override
    {
        const_cast<MemoryIterator*>(this)->m_currentPosition++;
    }

    QString url() const override
    {
        if (resource()) {
            return resource()->filename();
        }
        return QString();
    }

    QString type() const override
    {
        return m_resourceType;
    }

    QDateTime lastModified() const override
    {
        return QDateTime::fromMSecsSinceEpoch(0);
    }

    KoResourceSP resource() const override
    {
        if (m_currentPosition > m_resources.size()) return 0;
        return m_resources.at(m_currentPosition - 1);
    }

private:

    int m_currentPosition {0};
    QVector<KoResourceSP> m_resources;
    QString m_resourceType;

};

class KisMemoryStorage::Private {
public:
    QHash<QString, QVector<KoResourceSP> > resources;
    QHash<QString, QVector<KisTagSP> > tags;
    QMap<QString, QVariant> metadata;
};


KisMemoryStorage::KisMemoryStorage(const QString &location)
    : KisStoragePlugin(location)
    , d(new Private)
{
}

KisMemoryStorage::~KisMemoryStorage()
{
}

KisMemoryStorage::KisMemoryStorage(const KisMemoryStorage &rhs)
    : KisStoragePlugin(rhs.location())
    , d(new Private)
{
    *this = rhs;
}

KisMemoryStorage &KisMemoryStorage::operator=(const KisMemoryStorage &rhs)
{
    if (this != &rhs) {
        Q_FOREACH(const QString &key, rhs.d->resources.keys()) {
            Q_FOREACH(const KoResourceSP resource, rhs.d->resources[key]) {
                if (!d->resources.contains(key)) {
                    d->resources[key] = QVector<KoResourceSP>();
                }
                d->resources[key] << resource->clone();
            }
        }
        Q_FOREACH(const QString &key, rhs.d->tags.keys()) {
            Q_FOREACH(const KisTagSP tag, rhs.d->tags[key]) {
                if (!d->tags.contains(key)) {
                    d->tags[key] = QVector<KisTagSP>();
                }
                d->tags[key] << tag->clone();
            }
        }
    }
    return *this;
}

bool KisMemoryStorage::addTag(const QString &resourceType, KisTagSP tag)
{
    if (!d->tags.contains(resourceType)) {
        d->tags[resourceType] = QVector<KisTagSP>();
    }
    if (!d->tags[resourceType].contains(tag)) {
        d->tags[resourceType].append(tag);
    }
    return true;
}

bool KisMemoryStorage::addResource(const QString &resourceType, KoResourceSP resource)
{
    if (!d->resources.contains(resourceType)) {
        d->resources[resourceType] = QVector<KoResourceSP>();
    }
    if (!d->resources[resourceType].contains(resource)) {
        d->resources[resourceType].append(resource);
    }
    else {
        resource->setVersion(resource->version() + 1);
    }
    return true;
}

KisResourceStorage::ResourceItem KisMemoryStorage::resourceItem(const QString &url)
{
    MemoryItem item;
    item.url = url;
    item.folder = QString();
    item.lastModified = QDateTime::fromMSecsSinceEpoch(0);
    return item;
}

KoResourceSP KisMemoryStorage::resource(const QString &url)
{
    KoResourceSP resource;
    QFileInfo fi(location() + '/' + url);
    const QString resourceType = fi.path().split("/").last();
    Q_FOREACH(resource, d->resources[resourceType]) {
        if (resource->filename() == url) {
            break;
        }
    }
    return resource;
}

QSharedPointer<KisResourceStorage::ResourceIterator> KisMemoryStorage::resources(const QString &resourceType)
{
    return QSharedPointer<KisResourceStorage::ResourceIterator>(new MemoryIterator(d->resources[resourceType], resourceType));
}

QSharedPointer<KisResourceStorage::TagIterator> KisMemoryStorage::tags(const QString &resourceType)
{
    return QSharedPointer<KisResourceStorage::TagIterator>(new MemoryTagIterator(d->tags[resourceType], resourceType));
}

void KisMemoryStorage::setMetaData(const QString &key, const QVariant &value)
{
    d->metadata[key] = value;
}

QStringList KisMemoryStorage::metaDataKeys() const
{
    return QStringList() << KisResourceStorage::s_meta_name;
}

QVariant KisMemoryStorage::metaData(const QString &key) const
{
    QVariant r;
    if (d->metadata.contains(key)) {
        r = d->metadata[key];
    }
    return r;
}
