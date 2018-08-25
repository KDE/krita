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

#include "KisAsyncAnimationCacheRenderDialog.h"

#include "KisAsyncAnimationCacheRenderer.h"
#include "kis_animation_frame_cache.h"
#include <kis_time_range.h>
#include <kis_image.h>
#include <kis_image_animation_interface.h>

namespace {

QList<int> calcDirtyFramesList(KisAnimationFrameCacheSP cache, const KisTimeSpan &playbackRange)
{
    QList<int> framesToRegenerate;

    KisImageSP image = cache->image();
    if (!image) return framesToRegenerate;

    KisImageAnimationInterface *animation = image->animationInterface();
    if (!animation->hasAnimation()) return framesToRegenerate;

    KisFrameSet dirtyFrames = cache->dirtyFramesWithin(playbackRange);

    while (!dirtyFrames.isEmpty()) {
        int first = dirtyFrames.start();

        framesToRegenerate.append(first);

        KisFrameSet duplicates = KisTimeRange::calculateIdenticalFramesRecursive(image->root(), first).asFrameSet();
        dirtyFrames -= duplicates;
    }

    return framesToRegenerate;
}

}

struct KisAsyncAnimationCacheRenderDialog::Private
{
    Private(KisAnimationFrameCacheSP _cache, const KisTimeSpan &_range)
        : cache(_cache),
          range(_range)
    {
    }

    KisAnimationFrameCacheSP cache;
    KisTimeSpan range;
};

KisAsyncAnimationCacheRenderDialog::KisAsyncAnimationCacheRenderDialog(KisAnimationFrameCacheSP cache, const KisTimeSpan &range, int busyWait)
    : KisAsyncAnimationRenderDialogBase(i18n("Regenerating cache..."), cache->image(), busyWait),
      m_d(new Private(cache, range))
{
}

KisAsyncAnimationCacheRenderDialog::~KisAsyncAnimationCacheRenderDialog()
{

}

QList<int> KisAsyncAnimationCacheRenderDialog::calcDirtyFrames() const
{
    return calcDirtyFramesList(m_d->cache, m_d->range);
}

KisAsyncAnimationRendererBase *KisAsyncAnimationCacheRenderDialog::createRenderer(KisImageSP image)
{
    Q_UNUSED(image);
    return new KisAsyncAnimationCacheRenderer();
}

void KisAsyncAnimationCacheRenderDialog::initializeRendererForFrame(KisAsyncAnimationRendererBase *renderer, KisImageSP image, int frame)
{
    Q_UNUSED(image);
    Q_UNUSED(frame);

    KisAsyncAnimationCacheRenderer *cacheRenderer =
        dynamic_cast<KisAsyncAnimationCacheRenderer*>(renderer);

    KIS_SAFE_ASSERT_RECOVER_RETURN(cacheRenderer);

    cacheRenderer->setFrameCache(m_d->cache);
}

