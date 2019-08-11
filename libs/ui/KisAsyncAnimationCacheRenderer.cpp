/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KisAsyncAnimationCacheRenderer.h"

#include "kis_animation_frame_cache.h"
#include "kis_update_info.h"

struct KisAsyncAnimationCacheRenderer::Private
{
    KisAnimationFrameCacheSP requestedCache;
    KisOpenGLUpdateInfoSP requestInfo;
};


KisAsyncAnimationCacheRenderer::KisAsyncAnimationCacheRenderer()
    : m_d(new Private)
{
    connect(this, SIGNAL(sigCompleteRegenerationInternal(int)), SLOT(slotCompleteRegenerationInternal(int)));
}

KisAsyncAnimationCacheRenderer::~KisAsyncAnimationCacheRenderer()
{
}

void KisAsyncAnimationCacheRenderer::setFrameCache(KisAnimationFrameCacheSP cache)
{
    m_d->requestedCache = cache;
}

void KisAsyncAnimationCacheRenderer::frameCompletedCallback(int frame, const QRegion &requestedRegion)
{
    KisAnimationFrameCacheSP cache = m_d->requestedCache;
    KisImageSP image = requestedImage();
    if (!cache || !image) return;

    m_d->requestInfo = cache->fetchFrameData(frame, image, requestedRegion);
    emit sigCompleteRegenerationInternal(frame);
}

void KisAsyncAnimationCacheRenderer::slotCompleteRegenerationInternal(int frame)
{
    if (!isActive()) return;

    KIS_SAFE_ASSERT_RECOVER(m_d->requestInfo) {
        frameCancelledCallback(frame);
        return;
    }

    m_d->requestedCache->addConvertedFrameData(m_d->requestInfo, frame);

    notifyFrameCompleted(frame);
}


void KisAsyncAnimationCacheRenderer::frameCancelledCallback(int frame)
{
    notifyFrameCancelled(frame);
}

void KisAsyncAnimationCacheRenderer::clearFrameRegenerationState(bool isCancelled)
{
    m_d->requestInfo.clear();
    m_d->requestedCache.clear();

    KisAsyncAnimationRendererBase::clearFrameRegenerationState(isCancelled);
}


