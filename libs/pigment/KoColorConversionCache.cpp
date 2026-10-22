/*
 * SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KoColorConversionCache.h"

#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include <QThreadStorage>

#include <KoColorSpace.h>

#include <boost/container/static_vector.hpp>

template <typename Key, typename Value, size_t CacheSize>
class RotatingCache
{
public:
    using CacheItem = std::pair<Key, Value>;
public:
    RotatingCache()
        : m_storage()
    {
    }

    void push(Key key, Value value) {
        if (m_storage.size() == m_storage.max_size()) {
            m_storage.erase(m_storage.begin());
        }
        m_storage.emplace_back(std::move(key), std::move(value));
    }

    inline std::optional<Value> find(const Key &key) const {
        for (auto it = m_storage.begin(); it != m_storage.end(); ++it) {
            if (it->first == key) {
                return it->second;
            }
        }
        return std::nullopt;
    }

    void clear() {
        m_storage.clear();
    }

private:
    boost::container::static_vector<CacheItem, CacheSize> m_storage;
};

struct KoColorConversionCacheKey {

    KoColorConversionCacheKey(const KoColorSpace* _src,
                              const KoColorSpace* _dst,
                              KoColorConversionTransformation::Intent _renderingIntent,
                              KoColorConversionTransformation::ConversionFlags _conversionFlags)
        : src(_src)
        , dst(_dst)
        , renderingIntent(_renderingIntent)
        , conversionFlags(_conversionFlags)
    {
    }

    bool operator==(const KoColorConversionCacheKey& rhs) const {
        return (*src == *(rhs.src)) && (*dst == *(rhs.dst))
                && (renderingIntent == rhs.renderingIntent)
                && (conversionFlags == rhs.conversionFlags);
    }

    const KoColorSpace* src;
    const KoColorSpace* dst;
    KoColorConversionTransformation::Intent renderingIntent;
    KoColorConversionTransformation::ConversionFlags conversionFlags;
};

uint qHash(const KoColorConversionCacheKey& key)
{
    return qHash(key.src) + qHash(key.dst) + qHash(key.renderingIntent) + qHash(key.conversionFlags);
}

struct KoColorConversionCache::Private {
    QMultiHash< KoColorConversionCacheKey, CachedTransformation*> cache;
    QMutex cacheMutex;

    using PerThreadRotatingCache = RotatingCache<KoColorConversionCacheKey, KoCachedColorConversionTransformation, 4>;
    QThreadStorage<PerThreadRotatingCache> fastStorage;
};


KoColorConversionCache::KoColorConversionCache() : d(new Private)
{
}

KoColorConversionCache::~KoColorConversionCache()
{
    Q_FOREACH (CachedTransformation* transfo, d->cache) {
        delete transfo;
    }
    delete d;
}

KoCachedColorConversionTransformation KoColorConversionCache::cachedConverter(const KoColorSpace* src,
                                                                              const KoColorSpace* dst,
                                                                              KoColorConversionTransformation::Intent _renderingIntent,
                                                                              KoColorConversionTransformation::ConversionFlags _conversionFlags)
{
    KoColorConversionCacheKey key(src, dst, _renderingIntent, _conversionFlags);

    std::optional<KoCachedColorConversionTransformation> cachedTransformation;

    if (!d->fastStorage.hasLocalData()) {
        d->fastStorage.setLocalData(Private::PerThreadRotatingCache());
    } else {
        cachedTransformation = d->fastStorage.localData().find(key);
    }

    if (cachedTransformation) {
        return *cachedTransformation;
    }

    QMutexLocker lock(&d->cacheMutex);

    auto it = d->cache.constFind(key);
    if (it != d->cache.constEnd()) {
        CachedTransformation* ct = it.value();
        ct->transfo->setSrcColorSpace(src);
        ct->transfo->setDstColorSpace(dst);
        cachedTransformation = KoCachedColorConversionTransformation(ct);
    } else {
        KoColorConversionTransformation* transfo = src->createColorConverter(dst, _renderingIntent, _conversionFlags);
        CachedTransformation* ct = new CachedTransformation(transfo);
        d->cache.insert(key, ct);
        cachedTransformation = KoCachedColorConversionTransformation(ct);
    }
    d->fastStorage.localData().push(key, *cachedTransformation);

    return *cachedTransformation;
}

void KoColorConversionCache::colorSpaceIsDestroyed(const KoColorSpace* cs)
{
    if (d->fastStorage.hasLocalData()) {
        d->fastStorage.localData().clear();
    }

    QMutexLocker lock(&d->cacheMutex);
    QMultiHash< KoColorConversionCacheKey, CachedTransformation*>::iterator endIt = d->cache.end();
    for (QMultiHash< KoColorConversionCacheKey, CachedTransformation*>::iterator it = d->cache.begin(); it != endIt;) {
        if (it.key().src == cs || it.key().dst == cs) {
            Q_ASSERT(it.value()->isNotInUse()); // That's terribly evil, if that assert fails, that means that someone is using a color transformation with a color space which is currently being deleted
            delete it.value();
            it = d->cache.erase(it);
        } else {
            ++it;
        }
    }
}

//--------- KoColorConversionCache::CachedTransformation ----------//

KoColorConversionCache::CachedTransformation::CachedTransformation(KoColorConversionTransformation *_transfo)
    : transfo(_transfo)
    , use(0)
{
}

KoColorConversionCache::CachedTransformation::~CachedTransformation()
{
    delete transfo;
}

bool KoColorConversionCache::CachedTransformation::isNotInUse()
{
    return !use;
}


//--------- KoCachedColorConversionTransformation ----------//

KoCachedColorConversionTransformation::KoCachedColorConversionTransformation(KoColorConversionCache::CachedTransformation* transfo)
    : m_transfo(transfo)
{
    m_transfo = transfo;
    m_transfo->use.ref();
}

KoCachedColorConversionTransformation::KoCachedColorConversionTransformation(const KoCachedColorConversionTransformation& rhs)
    : m_transfo(rhs.m_transfo)
{
    m_transfo->use.ref();
}

KoCachedColorConversionTransformation::~KoCachedColorConversionTransformation()
{
    Q_ASSERT(m_transfo->use > 0);
    m_transfo->use.deref();
}
