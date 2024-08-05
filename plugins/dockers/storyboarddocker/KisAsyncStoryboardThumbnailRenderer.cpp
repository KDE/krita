/*
 *  SPDX-FileCopyrightText: 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisAsyncStoryboardThumbnailRenderer.h"
#include "kis_image_animation_interface.h"
#include "kis_image.h"

KisAsyncStoryboardThumbnailRenderer::KisAsyncStoryboardThumbnailRenderer(QObject *parent)
    : KisAsyncAnimationRendererBase(parent)
{
    connect(this, SIGNAL(sigNotifyFrameCompleted(int)), SLOT(notifyFrameCompleted(int)), Qt::QueuedConnection);
    connect(this,
            SIGNAL(sigNotifyFrameCancelled(int, KisAsyncAnimationRendererBase::CancelReason)),
            SLOT(notifyFrameCancelled(int, KisAsyncAnimationRendererBase::CancelReason)), Qt::QueuedConnection);
}

KisAsyncStoryboardThumbnailRenderer::~KisAsyncStoryboardThumbnailRenderer()
{
}

void KisAsyncStoryboardThumbnailRenderer::frameCompletedCallback(int frameTime, const KisRegion &/*requestedRegion*/)
{
    KisImageSP image = requestedImage();
    KisPaintDeviceSP requestedFrame = image ? new KisPaintDevice(*image->projection(), KritaUtils::CopySnapshot) : nullptr;

    if (requestedFrame) {
        Q_EMIT sigNotifyFrameCompleted(frameTime);
        Q_EMIT sigNotifyFrameCompleted(frameTime, requestedFrame);
    } else {
        Q_EMIT sigNotifyFrameCancelled(frameTime, KisAsyncAnimationRendererBase::RenderingFailed);
    }
}

void KisAsyncStoryboardThumbnailRenderer::frameCancelledCallback(int frame, CancelReason cancelReason)
{
    Q_EMIT sigNotifyFrameCancelled(frame, cancelReason);
}

void KisAsyncStoryboardThumbnailRenderer::clearFrameRegenerationState(bool isCancelled)
{
    KisAsyncAnimationRendererBase::clearFrameRegenerationState(isCancelled);
}
