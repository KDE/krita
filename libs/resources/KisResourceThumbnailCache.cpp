/*
 * SPDX-FileCopyrightText: 2023 Sharaf Zaman <shzam@sdf.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisResourceThumbnailCache.h"

#include <QMap>
#include <QModelIndex>
#include <QSize>

#include <KisResourceLocator.h>
#include <KisResourceModel.h>

#include <kis_global.h>

Q_GLOBAL_STATIC(KisResourceThumbnailCache, s_instance);

struct ImageScalingParameters {
    QSize size;
    Qt::AspectRatioMode aspectRatioMode;
    Qt::TransformationMode transformationMode;

    bool operator<(const ImageScalingParameters &other) const
    {
        if (size != other.size) {
            if (size.width() != other.size.width()) {
                return size.width() < other.size.width();
            } else {
                return size.height() < other.size.height();
            }
        } else if (aspectRatioMode != other.aspectRatioMode) {
            return aspectRatioMode < other.aspectRatioMode;
        } else if (transformationMode != other.transformationMode) {
            return transformationMode < other.transformationMode;
        } else {
            // they're the same
            return false;
        }
    }
};

namespace
{
using ResourceKey = QPair<QString, QString>;
using ThumbnailCacheT = QMap<ImageScalingParameters, QImage>;
} // namespace

struct KisResourceThumbnailCache::Private {
    QMap<ResourceKey, ThumbnailCacheT> scaledThumbnailCache;
    QMap<ResourceKey, QImage> originalImageCache;

    QImage getExactMatch(const ResourceKey &key, ImageScalingParameters param) const;
    QImage getOriginal(const ResourceKey &key) const;
    void insertOriginal(const ResourceKey &key, const QImage &image);
    bool containsOriginal(const ResourceKey &key) const;

    ResourceKey
    key(const QString &storageLocation, const QString &resourceType, const QString &filename) const;
};

QImage KisResourceThumbnailCache::Private::getExactMatch(const ResourceKey &key,
                                                         ImageScalingParameters param) const
{
    const auto thumbnailEntries = scaledThumbnailCache.find(key);
    if (thumbnailEntries != scaledThumbnailCache.end()) {
        const auto scaledThumbnail = thumbnailEntries->find(param);
        if (scaledThumbnail != thumbnailEntries->end()) {
            return *scaledThumbnail;
        }
    }

    const auto originalImage = originalImageCache.find(key);
    if (originalImage != originalImageCache.end() && originalImage->size() == param.size) {
        return *originalImage;
    }

    return QImage();
}

QImage KisResourceThumbnailCache::Private::getOriginal(const ResourceKey &key) const
{
    return originalImageCache[key];
}

void KisResourceThumbnailCache::Private::insertOriginal(const ResourceKey &key, const QImage &image)
{
    // Someone else has added the image to this cache, when the only path to here is from a method which
    // checks whether this cache contains it or not.
    KIS_ASSERT(!originalImageCache.contains(key));
    originalImageCache.insert(key, image);
}

bool KisResourceThumbnailCache::Private::containsOriginal(const ResourceKey &key) const
{
    return originalImageCache.contains(key);
}

ResourceKey KisResourceThumbnailCache::Private::key(const QString &storageLocation,
                                                    const QString &resourceType,
                                                    const QString &filename) const
{
    return {storageLocation, resourceType + "/" + filename};
}

KisResourceThumbnailCache *KisResourceThumbnailCache::instance()
{
    return s_instance;
}

KisResourceThumbnailCache::KisResourceThumbnailCache()
    : m_d(new Private)
{
}

KisResourceThumbnailCache::~KisResourceThumbnailCache()
{
}

QImage KisResourceThumbnailCache::originalImage(const QString &storageLocation,
                                                const QString &resourceType,
                                                const QString &filename) const
{
    const ResourceKey key = m_d->key(storageLocation, resourceType, filename);
    return m_d->containsOriginal(key) ? m_d->getOriginal(key) : QImage();
}

void KisResourceThumbnailCache::insert(const QString &storageLocation,
                                       const QString &resourceType,
                                       const QString &filename,
                                       const QImage &image)
{
    if (image.isNull()) {
        return;
    }
    insert(m_d->key(storageLocation, resourceType, filename), image);
}

void KisResourceThumbnailCache::insert(const QPair<QString, QString> &key, const QImage &image)
{
    m_d->insertOriginal(key, image);
}

void KisResourceThumbnailCache::remove(const QString &storageLocation,
                                       const QString &resourceType,
                                       const QString &filename)
{
    remove(m_d->key(storageLocation, resourceType, filename));
}

void KisResourceThumbnailCache::remove(const QPair<QString, QString> &key)
{
    if (m_d->originalImageCache.contains(key)) {
        m_d->originalImageCache.remove(key);

        if (m_d->scaledThumbnailCache.contains(key)) {
            m_d->scaledThumbnailCache.remove(key);
        }
    } else {
        // Something must have gone wrong for thumbnail to exist in scaledThumbnailCache but not be in
        // original.
        KIS_ASSERT(!m_d->scaledThumbnailCache.contains(key));
    }
}

QImage KisResourceThumbnailCache::getImage(const QModelIndex &index,
                                           const QSize size,
                                           Qt::AspectRatioMode aspectMode,
                                           Qt::TransformationMode transformMode)
{
    const QString storageLocation = KisResourceLocator::instance()->makeStorageLocationAbsolute(
        index.data(Qt::UserRole + KisAbstractResourceModel::Location).value<QString>());
    const QString resourceType =
        index.data(Qt::UserRole + KisAbstractResourceModel::ResourceType).value<QString>();
    const QString filename = index.data(Qt::UserRole + KisAbstractResourceModel::Filename).value<QString>();

    const ImageScalingParameters param = {size, aspectMode, transformMode};

    ResourceKey key = m_d->key(storageLocation, resourceType, filename);

    QImage result = m_d->getExactMatch(key, param);
    if (!result.isNull()) {
        return result;
    } else if (m_d->containsOriginal(key)) {
        result = m_d->getOriginal(key);
    } else {
        result = index.data(Qt::UserRole + KisAbstractResourceModel::Thumbnail).value<QImage>();
        // KisResourceQueryMapper should have inserted the image, so we don't have to.
        // Why there? Because most of the API usage for Thumbnail is going to be from index.data(), so we just
        // remove the dependency that our user has to know this class for just accessing the cached original
        // thumbnail.
        KIS_SAFE_ASSERT_RECOVER_NOOP(result.isNull() || m_d->containsOriginal(key));
    }
    // if the size that the has been demanded, we will then cache the size and then pass it.
    if (!result.isNull() && param.size.isValid()) {
        const QImage scaledImage = result.scaled(param.size, param.aspectRatioMode, param.transformationMode);
        if (m_d->scaledThumbnailCache.contains(key)) {
            m_d->scaledThumbnailCache[key].insert(param, scaledImage);
        } else {
            ThumbnailCacheT scaledCacheMap;
            scaledCacheMap.insert(param, scaledImage);
            m_d->scaledThumbnailCache.insert(key, scaledCacheMap);
        }
        return scaledImage;
    } else {
        return result;
    }
}
