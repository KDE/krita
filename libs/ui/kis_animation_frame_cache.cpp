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

    struct Frame
    {
        KisOpenGLUpdateInfoSP openGlFrame;
        int length;

        Frame(KisOpenGLUpdateInfoSP info, int length)
            : openGlFrame(info), length(length)
        {}
    };

    QMap<int, int> newFrames;

    int getFrameIdAtTime(int time) const
    {
        if (newFrames.isEmpty()) return -1;

        auto it = newFrames.upperBound(time);

        if (it != newFrames.constBegin()) it--;

        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(it != newFrames.constEnd(), 0);
        const int start = it.key();
        const int length = it.value();

        bool foundFrameValid = false;

        if (length == -1) {
            if (start <= time) {
                foundFrameValid = true;
            }
        } else {
            int end = start + length - 1;
            if (start <= time && time <= end) {
                foundFrameValid = true;
            }
        }

        return foundFrameValid ? start : -1;
    }

    bool hasFrame(int time) const {
        return getFrameIdAtTime(time) >= 0;
    }

    KisOpenGLUpdateInfoSP getFrame(int time)
    {
        const int frameId = getFrameIdAtTime(time);
        return frameId >= 0 ? swapper->loadFrame(frameId) : 0;
    }

    void addFrame(KisOpenGLUpdateInfoSP info, const KisTimeRange& range)
    {
        invalidate(range);

        const int length = range.isInfinite() ? -1 : range.end() - range.start() + 1;
        newFrames.insert(range.start(), length);
        swapper->saveFrame(range.start(), info, image->bounds());
    }

    /**
     * Invalidate any cached frames within the given time range.
     * @param range
     * @return true if frames were invalidated, false if nothing was changed
     */
    bool invalidate(const KisTimeRange& range)
    {
        if (newFrames.isEmpty()) return false;

        bool cacheChanged = false;

        auto it = newFrames.lowerBound(range.start());
        if (it.key() != range.start() && it != newFrames.begin()) it--;

        while (it != newFrames.end()) {
            const int start = it.key();
            const int length = it.value();
            const bool frameIsInfinite = (length == -1);
            const int end = start + length - 1;

            if (start >= range.start()) {
                if (!range.isInfinite() && start > range.end()) {
                    break;
                }

                if (!range.isInfinite() && (frameIsInfinite || end > range.end())) {
                    // Reinsert with a later start
                    int newStart = range.end() + 1;
                    int newLength = frameIsInfinite ? -1 : (end - newStart + 1);

                    newFrames.insert(newStart, newLength);
                    swapper->moveFrame(start, newStart);
                } else {
                    swapper->forgetFrame(start);
                }

                it = newFrames.erase(it);

                cacheChanged = true;
                continue;

            } else if (frameIsInfinite || end >= range.start()) {
                const int newEnd = range.start() - 1;
                *it = newEnd - start + 1;

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

    connect(m_d->image->animationInterface(), SIGNAL(sigFramesChanged(KisTimeRange,QRect)), this, SLOT(framesChanged(KisTimeRange,QRect)));
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
        m_d->textures->recalculateCache(info);
    }

    return bool(info);
}

bool KisAnimationFrameCache::shouldUploadNewFrame(int newTime, int oldTime) const
{
    if (oldTime < 0) return true;

    const int oldKeyframeStart = m_d->getFrameIdAtTime(oldTime);
    if (oldKeyframeStart < 0) return true;

    const int oldKeyFrameLength = m_d->newFrames[oldKeyframeStart];
    return !(newTime >= oldKeyframeStart && (newTime < oldKeyframeStart + oldKeyFrameLength || oldKeyFrameLength == -1));
}

KisAnimationFrameCache::CacheStatus KisAnimationFrameCache::frameStatus(int time) const
{
    return m_d->hasFrame(time) ? Cached : Uncached;
}

KisImageWSP KisAnimationFrameCache::image()
{
    return m_d->image;
}

void KisAnimationFrameCache::framesChanged(const KisTimeRange &range, const QRect &rect)
{
    Q_UNUSED(rect);

    if (!range.isValid()) return;

    bool cacheChanged = m_d->invalidate(range);

    if (cacheChanged) {
        emit changed();
    }
}

void KisAnimationFrameCache::slotConfigChanged()
{
    m_d->newFrames.clear();

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
    KisTimeRange identicalRange = KisTimeRange::infinite(0);
    KisTimeRange::calculateTimeRangeRecursive(m_d->image->root(), time, identicalRange, true);

    m_d->addFrame(info, identicalRange);

    emit changed();
}

void KisAnimationFrameCache::dropLowQualityFrames(const KisTimeSpan &range, const QRect &regionOfInterest, const QRect &minimalRect)
{
    if (m_d->newFrames.isEmpty()) return;

    auto it = m_d->newFrames.upperBound(range.start());

    // the vector is guaranteed to be non-empty,
    // so decrementing iterator is safe
    if (it != m_d->newFrames.begin()) it--;

    while (it != m_d->newFrames.end() && it.key() <= range.end()) {
        const int frameId = it.key();
        const int frameLength = it.value();

        if (frameId + frameLength - 1 < range.start()) {
            ++it;
            continue;
        }

        const QRect frameRect = m_d->swapper->frameDirtyRect(frameId);
        const int frameLod = m_d->swapper->frameLevelOfDetail(frameId);

        if (frameLod > m_d->effectiveLevelOfDetail(regionOfInterest) || !frameRect.contains(minimalRect)) {
            m_d->swapper->forgetFrame(frameId);
            it = m_d->newFrames.erase(it);
        } else {
            ++it;
        }
    }
}

bool KisAnimationFrameCache::framesHaveValidRoi(const KisTimeRange &range, const QRect &regionOfInterest)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!range.isInfinite(), false);
    if (m_d->newFrames.isEmpty()) return false;

    auto it = m_d->newFrames.upperBound(range.start());

    if (it != m_d->newFrames.begin()) it--;

    int expectedNextFrameStart = it.key();

    while (it.key() <= range.end()) {
        const int frameId = it.key();
        const int frameLength = it.value();

        if (frameId + frameLength - 1 < range.start()) {
            expectedNextFrameStart = frameId + frameLength;
            ++it;
            continue;
        }

        if (expectedNextFrameStart != frameId) {
            KIS_SAFE_ASSERT_RECOVER_NOOP(expectedNextFrameStart < frameId);
            return false;
        }

        if (!m_d->swapper->frameDirtyRect(frameId).contains(regionOfInterest)) {
            return false;
        }

        expectedNextFrameStart = frameId + frameLength;
        ++it;
    }

    return true;
}
