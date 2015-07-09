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

#include "kis_image.h"
#include "kis_image_animation_interface.h"

QMap<KisImageWSP, KisAnimationFrameCache*> KisAnimationFrameCache::caches;

struct KisAnimationFrameCache::Private
{
    QMap<int, QImage> frames;
    KisImageWSP image;

    Private(KisImageWSP image)
        : image(image)
    {}
};

KisAnimationFrameCacheSP KisAnimationFrameCache::getFrameCache(KisImageWSP image)
{
    KisAnimationFrameCache *cache;

    QMap<KisImageWSP, KisAnimationFrameCache*>::iterator it = caches.find(image);
    if (it == caches.end()) {
        cache = new KisAnimationFrameCache(image);
        caches.insert(image, cache);
    } else {
        cache = it.value();
    }

    return cache;
}

KisAnimationFrameCache::KisAnimationFrameCache(KisImageWSP image)
    : m_d(new Private(image))
{
    connect(image->animationInterface(), SIGNAL(sigFramesChanged(KisTimeRange,QRect)), this, SLOT(framesChanged(KisTimeRange,QRect)));
    connect(image->animationInterface(), SIGNAL(sigFrameReady()), this, SLOT(frameReady()), Qt::DirectConnection);
}

KisAnimationFrameCache::~KisAnimationFrameCache()
{
    caches.remove(m_d->image);
}

QImage KisAnimationFrameCache::getFrame(int time)
{
    if (!m_d->frames.contains(time)) {
        m_d->image->animationInterface()->requestFrameRegeneration(time, m_d->image->bounds());
    }

    return m_d->frames.value(time);
}

KisAnimationFrameCache::CacheStatus KisAnimationFrameCache::frameStatus(int time) const
{
    return (m_d->frames.contains(time)) ? Cached : Uncached;
}

void KisAnimationFrameCache::framesChanged(const KisTimeRange &range, const QRect &rect)
{
    if (!range.isValid()) return;
    if (m_d->frames.isEmpty()) return;

    int end = range.isInfinite() ?
        ((m_d->frames.constEnd() - 1).key()) : // TODO: better way to determine the "last" frame?
        (range.end());

    for (int t=range.start(); t <= end; t++) {
        // TODO: invalidate
        m_d->frames.remove(t);
    }

    emit changed();
}

void KisAnimationFrameCache::frameReady()
{
    QImage projection = m_d->image->animationInterface()->frameProjection()->convertToQImage(m_d->image->profile(), m_d->image->bounds());
    m_d->frames.insert(m_d->image->animationInterface()->currentTime(), projection);

    emit changed();
}
