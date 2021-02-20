/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    connect(this, SIGNAL(sigCompleteRegenerationInternal(int)), SLOT(slotCompleteRegenerationInternal(int)), Qt::QueuedConnection);
}

KisAsyncAnimationCacheRenderer::~KisAsyncAnimationCacheRenderer()
{
}

void KisAsyncAnimationCacheRenderer::setFrameCache(KisAnimationFrameCacheSP cache)
{
    m_d->requestedCache = cache;
}

void KisAsyncAnimationCacheRenderer::frameCompletedCallback(int frame, const KisRegion &requestedRegion)
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


