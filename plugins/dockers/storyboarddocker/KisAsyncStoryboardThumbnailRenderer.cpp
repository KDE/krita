/*
 *  SPDX-FileCopyrightText: 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisAsyncStoryboardThumbnailRenderer.h"
#include "kis_image_animation_interface.h"
#include "kis_image.h"

KisAsyncStoryboardThumbnailRenderer::KisAsyncStoryboardThumbnailRenderer(QObject *parent)
{
    connect(this, SIGNAL(sigNotifyFrameCompleted(int)), SLOT(notifyFrameCompleted(int)), Qt::QueuedConnection);
    connect(this, SIGNAL(sigNotifyFrameCancelled(int)), SLOT(notifyFrameCancelled(int)), Qt::QueuedConnection);
}

KisAsyncStoryboardThumbnailRenderer::~KisAsyncStoryboardThumbnailRenderer()
{
}

void KisAsyncStoryboardThumbnailRenderer::frameCompletedCallback(int frameTime, const KisRegion &requestedRegion)
{
    KisImageSP image = requestedImage();
    KisPaintDeviceSP requestedFrame = image ? new KisPaintDevice(*image->projection(), KritaUtils::CopySnapshot) : nullptr;

    if (requestedFrame) {
        emit sigNotifyFrameCompleted(frameTime);
        emit sigNotifyFrameCompleted(frameTime, requestedFrame);
    } else {
        emit sigNotifyFrameCancelled(frameTime);
    }
}

void KisAsyncStoryboardThumbnailRenderer::frameCancelledCallback(int frame)
{
    emit sigNotifyFrameCancelled(frame);
}

void KisAsyncStoryboardThumbnailRenderer::clearFrameRegenerationState(bool isCancelled)
{
    KisAsyncAnimationRendererBase::clearFrameRegenerationState(isCancelled);
}
