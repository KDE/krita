/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_animation_frame_cache.h"

#include <QMap>
#include <QVector>

#include "kis_debug.h"

#include "kis_image.h"
#include "kis_image_animation_interface.h"
#include "kis_time_range.h"
#include "KisPart.h"
#include "kis_animation_cache_populator.h"

#include <KisAbstractFrameCacheSwapper.h>
#include "KisFrameCacheSwapper.h"
#include "KisInMemoryFrameCacheSwapper.h"

#include "kis_image_config.h"
#include "kis_config_notifier.h"

#include "opengl/kis_opengl_image_textures.h"

#include <kis_algebra_2d.h>
#include <cmath>


struct KisAnimationFrameCache::Private
{
    Private(KisOpenGLImageTexturesSP _textures)
        : textures(_textures)
    {
        image = textures->image();
    }

    ~Private()
    {
    }

    KisOpenGLImageTexturesSP textures;
    KisImageWSP image;

    QScopedPointer<KisAbstractFrameCacheSwapper> swapper;
    int frameSizeLimit = 777;

    KisOpenGLUpdateInfoSP fetchFrameDataImpl(KisImageSP image, const QRect &requestedRect, int lod);

    struct CacheEntry
    {
        int frameId;
        int length;

        CacheEntry(int id, int length)
            : frameId(id), length(length)
        {}

        bool isInfinite() const {
            return length < 0;
        }
    };

    int nextFreeId = 0;
    /// Map of cache entries by beginning time of the entry
    QMap<int, CacheEntry> cachedFrames;
    /// Maps a frame ID to a list of times, where entries with said frame ID begin
    QMap<int, QVector<int>> entriesById;

    void addFrame(int start, int length, int id) {
        cachedFrames.insert(start, CacheEntry(id, length));
        entriesById[id].append(start);
    }

    QMap<int, CacheEntry>::iterator iteratorFrom(int time) {
        if (cachedFrames.isEmpty()) return cachedFrames.end();

        auto it = cachedFrames.upperBound(time);
        if (it != cachedFrames.begin()) {
            auto previous = it - 1;
            if (previous.value().isInfinite() || time <= previous.key() + previous.value().length) {
                return previous;
            }

        }
        return it;
    }

    QMap<int, CacheEntry>::const_iterator constIteratorFrom(int time) const {
        if (cachedFrames.isEmpty()) return cachedFrames.constEnd();

        auto it = cachedFrames.upperBound(time);
        if (it != cachedFrames.constBegin()) {
            auto previous = it - 1;
            if (previous.value().isInfinite() || time <= previous.key() + previous.value().length) {
                return previous;
            }

        }
        return it;
    }

    int getFrameIdAtTime(int time) const
    {
        auto it = constIteratorFrom(time);
        if (it == cachedFrames.constEnd()) return -1;

        const int start = it.key();
        const CacheEntry &frame = it.value();

        bool foundFrameValid = false;
        if (frame.isInfinite()) {
            if (start <= time) {
                foundFrameValid = true;
            }
        } else {
            int end = start + frame.length - 1;
            if (start <= time && time <= end) {
                foundFrameValid = true;
            }
        }

        return foundFrameValid ? frame.frameId : -1;
    }

    bool hasFrame(int time) const {
        return getFrameIdAtTime(time) >= 0;
    }

    KisOpenGLUpdateInfoSP getFrame(int time)
    {
        const int frameId = getFrameIdAtTime(time);
        return frameId >= 0 ? swapper->loadFrame(frameId) : 0;
    }

    void addFrame(KisOpenGLUpdateInfoSP info, const KisFrameSet& targetFrames)
    {
        invalidate(targetFrames);

        const int id = nextFreeId++;
        swapper->saveFrame(id, info, image->bounds());

        for (const KisTimeSpan span : targetFrames.finiteSpans()) {
            addFrame(span.start(), span.duration(), id);
        }

        if (targetFrames.isInfinite()) {
            addFrame(targetFrames.firstFrameOfInfinity(), -1, id);
        }
    }

    /**
     * Invalidate any cached frames within the given time range.
     * @param range
     * @return true if frames were invalidated, false if nothing was changed
     */
    bool invalidate(const KisFrameSet& range)
    {
        if (cachedFrames.isEmpty()) return false;

        bool cacheChanged = false;

        for (const KisTimeSpan span : range.finiteSpans()) {
            cacheChanged |= invalidate(span.start(), span.end());
        }
        if (range.isInfinite()) {
            cacheChanged |= invalidate(range.firstFrameOfInfinity(), -1);
        }

        return cacheChanged;
    }

    bool invalidate(int invalidateFrom, int invalidateTo) {
        const bool infinite = invalidateTo < 0;

        bool cacheChanged = false;

        auto it = iteratorFrom(invalidateFrom);
        while (it != cachedFrames.end()) {
            const int start = it.key();
            const CacheEntry frame = it.value();
            const bool frameIsInfinite = (frame.length == -1);
            const int end = start + frame.length - 1;

            if (start >= invalidateFrom) {
                if (!infinite) {
                    if (start > invalidateTo) {
                        break;
                    }

                    if (frameIsInfinite || end > invalidateTo) {
                        // Shorten the entry from the beginning
                        int newStart = invalidateTo + 1;
                        int newLength = frameIsInfinite ? -1 : (end - newStart + 1);

                        cachedFrames.insert(newStart, CacheEntry(frame.frameId, newLength));
                        addFrame(newStart, newLength, frame.frameId);
                    }
                }

                QVector<int> &instances = entriesById[frame.frameId];
                instances.removeAll(it.key());
                if (instances.isEmpty()) swapper->forgetFrame(frame.frameId);
                it = cachedFrames.erase(it);

                cacheChanged = true;
                continue;

            } else if (frameIsInfinite || end >= invalidateFrom) {
                const int newEnd = invalidateFrom - 1;
                const int newLength = newEnd - start + 1;
                *it = CacheEntry(frame.frameId, newLength);

                cacheChanged = true;
            }

            it++;
        }

        return cacheChanged;
    }

    int effectiveLevelOfDetail(const QRect &rc) const {
        if (!frameSizeLimit) return 0;

        const int maxDimension = KisAlgebra2D::maxDimension(rc);

        const qreal minLod = -std::log2(qreal(frameSizeLimit) / maxDimension);
        const int lodLimit = qMax(0, qCeil(minLod));
        return lodLimit;
    }


    // TODO: verify that we don't have any leak here!
    typedef QMap<KisOpenGLImageTexturesSP, KisAnimationFrameCache*> CachesMap;
    static CachesMap caches;
};

KisAnimationFrameCache::Private::CachesMap KisAnimationFrameCache::Private::caches;

KisAnimationFrameCacheSP KisAnimationFrameCache::getFrameCache(KisOpenGLImageTexturesSP textures)
{
    KisAnimationFrameCache *cache;

    Private::CachesMap::iterator it = Private::caches.find(textures);
    if (it == Private::caches.end()) {
        cache = new KisAnimationFrameCache(textures);
        Private::caches.insert(textures, cache);
    } else {
        cache = it.value();
    }

    return cache;
}

const QList<KisAnimationFrameCache *> KisAnimationFrameCache::caches()
{
    return Private::caches.values();
}

KisAnimationFrameCache::KisAnimationFrameCache(KisOpenGLImageTexturesSP textures)
    : m_d(new Private(textures))
{
    // create swapping backend
    slotConfigChanged();

    connect(m_d->image->animationInterface(), SIGNAL(sigFramesChanged(KisFrameSet, QRect)), this, SLOT(framesChanged(KisFrameSet, QRect)));
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotConfigChanged()));
}

KisAnimationFrameCache::~KisAnimationFrameCache()
{
    Private::caches.remove(m_d->textures);
}

bool KisAnimationFrameCache::uploadFrame(int time)
{
    KisOpenGLUpdateInfoSP info = m_d->getFrame(time);

    if (!info) {
        // Do nothing!
        //
        // Previously we were trying to start cache regeneration in this point,
        // but it caused even bigger slowdowns when scrubbing
    } else {
        m_d->textures->recalculateCache(info, false);
    }

    return bool(info);
}

bool KisAnimationFrameCache::shouldUploadNewFrame(int newTime, int oldTime) const
{
    if (oldTime < 0) return true;

    const int oldFrameId = m_d->getFrameIdAtTime(oldTime);
    const int newFrameId = m_d->getFrameIdAtTime(newTime);
    return (oldFrameId < 0) || oldFrameId != newFrameId;
}

KisAnimationFrameCache::CacheStatus KisAnimationFrameCache::frameStatus(int time) const
{
    return m_d->hasFrame(time) ? Cached : Uncached;
}

KisImageWSP KisAnimationFrameCache::image()
{
    return m_d->image;
}

void KisAnimationFrameCache::framesChanged(const KisFrameSet &range, const QRect &rect)
{
    Q_UNUSED(rect);

    if (range.isEmpty()) return;

    bool cacheChanged = m_d->invalidate(range);

    if (cacheChanged) {
        emit changed();
    }
}

void KisAnimationFrameCache::slotConfigChanged()
{
    m_d->cachedFrames.clear();

    KisImageConfig cfg(true);

    if (cfg.useOnDiskAnimationCacheSwapping()) {
        m_d->swapper.reset(new KisFrameCacheSwapper(m_d->textures->updateInfoBuilder(), cfg.swapDir()));
    } else {
        m_d->swapper.reset(new KisInMemoryFrameCacheSwapper());
    }

    m_d->frameSizeLimit = cfg.useAnimationCacheFrameSizeLimit() ? cfg.animationCacheFrameSizeLimit() : 0;
    emit changed();
}

KisOpenGLUpdateInfoSP KisAnimationFrameCache::Private::fetchFrameDataImpl(KisImageSP image, const QRect &requestedRect, int lod)
{
    if (lod > 0) {
        KisPaintDeviceSP tempDevice = new KisPaintDevice(image->projection()->colorSpace());
        tempDevice->prepareClone(image->projection());
        image->projection()->generateLodCloneDevice(tempDevice, image->projection()->extent(), lod);

        const QRect fetchRect = KisLodTransform::alignedRect(requestedRect, lod);
        return textures->updateInfoBuilder().buildUpdateInfo(fetchRect, tempDevice, image->bounds(), lod, true);
    } else {
        return textures->updateCache(requestedRect, image);
    }
}

KisOpenGLUpdateInfoSP KisAnimationFrameCache::fetchFrameData(int time, KisImageSP image, const QRegion &requestedRegion) const
{
    if (time != image->animationInterface()->currentTime()) {
        qWarning() << "WARNING: KisAnimationFrameCache::frameReady image's time doesn't coincide with the requested time!";
        qWarning() << "    "  << ppVar(image->animationInterface()->currentTime()) << ppVar(time);
    }

    // the frames are always generated at full scale
    KIS_SAFE_ASSERT_RECOVER_NOOP(image->currentLevelOfDetail() == 0);

    const int lod = m_d->effectiveLevelOfDetail(requestedRegion.boundingRect());

    KisOpenGLUpdateInfoSP totalInfo;

    Q_FOREACH (const QRect &rc, requestedRegion.rects()) {
        KisOpenGLUpdateInfoSP info = m_d->fetchFrameDataImpl(image, rc, lod);
        if (!totalInfo) {
            totalInfo = info;
        } else {
            const bool result = totalInfo->tryMergeWith(*info);
            KIS_SAFE_ASSERT_RECOVER_NOOP(result);
        }
    }

    return totalInfo;
}

void KisAnimationFrameCache::addConvertedFrameData(KisOpenGLUpdateInfoSP info, int time)
{
    KisTimeSpan range = m_d->image->animationInterface()->fullClipRange();
    const KisFrameSet identicalFrames = calculateIdenticalFramesRecursive(m_d->image->root(), time, range);

    m_d->addFrame(info, identicalFrames);

    emit changed();
}

void KisAnimationFrameCache::dropLowQualityFrames(const KisTimeSpan &range, const QRect &regionOfInterest, const QRect &minimalRect)
{
    if (m_d->cachedFrames.isEmpty()) return;

    QVector<int> framesToDrop;

    for (auto it = m_d->constIteratorFrom(range.start()); it != m_d->cachedFrames.constEnd() && it.key() <= range.end(); it++) {
        const Private::CacheEntry &frame = it.value();

        const QRect frameRect = m_d->swapper->frameDirtyRect(frame.frameId);
        const int frameLod = m_d->swapper->frameLevelOfDetail(frame.frameId);

        if (frameLod > m_d->effectiveLevelOfDetail(regionOfInterest) || !frameRect.contains(minimalRect)) {
            if (!framesToDrop.contains(frame.frameId)) framesToDrop.append(frame.frameId);
        }
    }

    Q_FOREACH(int frameId, framesToDrop) {
        Q_FOREACH(int time, m_d->entriesById[frameId]) {
            m_d->cachedFrames.remove(time);
        }

        m_d->entriesById.remove(frameId);
        m_d->swapper->forgetFrame(frameId);
    }
}

KisFrameSet KisAnimationFrameCache::cachedFramesWithin(const KisTimeSpan range)
{
    auto it = m_d->constIteratorFrom(range.start());
    if (it == m_d->cachedFrames.constEnd()) return KisFrameSet(range);

    QVector<KisTimeSpan> cachedSpans;
    int firstOfInfinite = -1;

    for (; it != m_d->cachedFrames.constEnd() && it.key() <= range.end(); it++) {
        const int start = it.key();
        Private::CacheEntry entry = it.value();

        if (entry.isInfinite()) {
            firstOfInfinite = start;
        } else {
            cachedSpans.append(KisTimeSpan(start, start + entry.length - 1));
        }

    }

    return KisFrameSet(cachedSpans, firstOfInfinite);
}

int KisAnimationFrameCache::firstDirtyFrameWithin(const KisTimeSpan range, const KisFrameSet *ignoredFrames)
{
    int candidate = range.start();

    for (auto it = m_d->constIteratorFrom(range.start()); it != m_d->cachedFrames.constEnd(); it++) {
        const int start = it.key();
        const int end = start + it.value().length - 1;

        if (ignoredFrames) {
            candidate = ignoredFrames->firstExcludedSince(candidate);
        }

        if (candidate < start) {
            return candidate;
        } else if (candidate <= end) {
            candidate = end + 1;
        }
    }

    return -1;
}
