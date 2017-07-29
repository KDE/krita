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

#include "opengl/kis_opengl_image_textures.h"


struct KisAnimationFrameCache::Private
{
    Private(KisOpenGLImageTexturesSP _textures)
        : textures(_textures)
    {
        image = textures->image();
    }

    ~Private()
    {
        qDeleteAll(frames);
    }

    KisOpenGLImageTexturesSP textures;
    KisImageWSP image;

    struct Frame
    {
        KisOpenGLUpdateInfoSP openGlFrame;
        int length;

        Frame(KisOpenGLUpdateInfoSP info, int length)
            : openGlFrame(info), length(length)
        {}
    };

    QMap<int, Frame*> frames;

    Frame *getFrame(int time)
    {
        if (frames.isEmpty()) return 0;

        QMap<int, Frame*>::iterator it = frames.upperBound(time);

        if (it != frames.begin()) it--;

        Q_ASSERT(it != frames.end());
        int start = it.key();
        int length = it.value()->length;

        if (length == -1) {
            if (start <= time) return it.value();
        } else {
            int end = start + length - 1;
            if (start <= time && time <= end) return it.value();
        }

        return 0;
    }

    void addFrame(KisOpenGLUpdateInfoSP info, const KisTimeRange& range)
    {
        invalidate(range);

        int length = range.isInfinite() ? -1 : range.end() - range.start() + 1;
        Frame *frame = new Frame(info, length);

        frames.insert(range.start(), frame);
    }

    /**
     * Invalidate any cached frames within the given time range.
     * @param range
     * @return true if frames were invalidated, false if nothing was changed
     */
    bool invalidate(const KisTimeRange& range)
    {
        if (frames.isEmpty()) return false;

        bool cacheChanged = false;

        QMap<int, Frame*>::iterator it = frames.lowerBound(range.start());
        if (it.key() != range.start() && it != frames.begin()) it--;

        while (it != frames.end()) {
            Frame *frame = it.value();
            int start = it.key();
            int length = it.value()->length;
            bool frameIsInfinite = (length == -1);
            int end = start + length - 1;

            if (start >= range.start()) {
                if (!range.isInfinite() && start > range.end()) {
                    break;
                }

                if (!range.isInfinite() && (frameIsInfinite || end > range.end())) {
                    // Reinsert with a later start
                    int newStart = range.end() + 1;
                    int newLength = frameIsInfinite ? -1 : (end - newStart + 1);
                    frames.insert(newStart, new Frame(frame->openGlFrame, newLength));
                }

                it = frames.erase(it);
                delete frame;

                cacheChanged = true;
                continue;

            } else if (frameIsInfinite || end >= range.start()) {
                int newEnd = range.start() - 1;
                frame->length = newEnd - start + 1;

                cacheChanged = true;
            }

            it++;
        }

        return cacheChanged;
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
    connect(m_d->image->animationInterface(), SIGNAL(sigFramesChanged(KisTimeRange,QRect)), this, SLOT(framesChanged(KisTimeRange,QRect)));
}

KisAnimationFrameCache::~KisAnimationFrameCache()
{
    Private::caches.remove(m_d->textures);
}

bool KisAnimationFrameCache::uploadFrame(int time)
{
    Private::Frame *frame = m_d->getFrame(time);

    if (!frame) {
        KisPart::instance()->cachePopulator()->regenerate(this, time);
    } else {
        m_d->textures->recalculateCache(frame->openGlFrame);
    }

    return frame != 0;
}

KisAnimationFrameCache::CacheStatus KisAnimationFrameCache::frameStatus(int time) const
{
    Private::Frame *frame = m_d->getFrame(time);
    return (frame) ? Cached : Uncached;
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

KisOpenGLUpdateInfoSP KisAnimationFrameCache::fetchFrameData(int time, KisImageSP image) const
{
    if (time != image->animationInterface()->currentTime()) {
        qWarning() << "WARNING: KisAnimationFrameCache::frameReady image's time doesn't coincide with the requested time!";
        qWarning() << "    "  << ppVar(image->animationInterface()->currentTime()) << ppVar(time);
    }

    return m_d->textures->updateCache(image->bounds(), image);
}

void KisAnimationFrameCache::addConvertedFrameData(KisOpenGLUpdateInfoSP info, int time)
{
    KisTimeRange identicalRange = KisTimeRange::infinite(0);
    KisTimeRange::calculateTimeRangeRecursive(m_d->image->root(), time, identicalRange, true);

    m_d->addFrame(info, identicalRange);

    emit changed();
}
