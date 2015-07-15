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

#include "opengl/kis_opengl_image_textures.h"

struct KisAnimationFrameCache::Private
{
    Private(KisOpenGLImageTexturesSP _textures)
        : textures(_textures)
    {
        image = textures->image();
    }

    KisOpenGLImageTexturesSP textures;
    KisImageWSP image;

    QMap<int, KisOpenGLUpdateInfoSP> openGlFrames;


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

KisAnimationFrameCache::KisAnimationFrameCache(KisOpenGLImageTexturesSP textures)
    : m_d(new Private(textures))
{
    connect(m_d->image->animationInterface(), SIGNAL(sigFramesChanged(KisTimeRange,QRect)), this, SLOT(framesChanged(KisTimeRange,QRect)));
    connect(m_d->image->animationInterface(), SIGNAL(sigFrameReady()), this, SLOT(frameReady()), Qt::DirectConnection);
}

KisAnimationFrameCache::~KisAnimationFrameCache()
{
    Private::caches.remove(m_d->textures);
}

bool KisAnimationFrameCache::uploadFrame(int time)
{
    bool frameExists = m_d->openGlFrames.contains(time);

    if (!frameExists) {
        m_d->image->animationInterface()->requestFrameRegeneration(time, m_d->image->bounds());
    } else {
        m_d->textures->recalculateCache(m_d->openGlFrames[time]);
    }

    return frameExists;
}

KisAnimationFrameCache::CacheStatus KisAnimationFrameCache::frameStatus(int time) const
{
    return (m_d->openGlFrames.contains(time)) ? Cached : Uncached;
}

template <typename K, typename V>
K lastKeyValue(const QMap<K,V> &map, K defaultValue = K())
{
    return !map.isEmpty() ? (--map.constEnd()).key() : defaultValue;
}

void KisAnimationFrameCache::framesChanged(const KisTimeRange &range, const QRect &rect)
{
    Q_UNUSED(rect);

    if (!range.isValid()) return;
    if (m_d->openGlFrames.isEmpty()) return;

    int end = range.isInfinite() ?
        lastKeyValue(m_d->openGlFrames, range.start()) :
        range.end();

    for (int t=range.start(); t <= end; t++) {
        // TODO: invalidate
        m_d->openGlFrames.remove(t);
    }

    emit changed();
}

void KisAnimationFrameCache::frameReady()
{
    int currentTime = m_d->image->animationInterface()->currentTime();
    KisOpenGLUpdateInfoSP info =
        m_d->textures->updateCache(m_d->image->bounds());

    KisTimeRange identicalRange = KisTimeRange::infinite(0);
    KisTimeRange::calculateTimeRangeRecursive(m_d->image->root(), currentTime, identicalRange, true);

    int end;
    if (!identicalRange.isInfinite()) {
        end = identicalRange.end();
    } else {
        end = std::max(identicalRange.start(), lastKeyValue(m_d->openGlFrames, 0));
        end = std::max(end, m_d->image->animationInterface()->currentRange().end());
    }

    for (int t = identicalRange.start(); t <= end; t++) {
        m_d->openGlFrames.insert(t, info);
    }

    emit changed();
}
