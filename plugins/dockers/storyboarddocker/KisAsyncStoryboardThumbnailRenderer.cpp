/*
 *  Copyright (c) 2020 Saurabh Kumar <saurabhk660@gmail.com>
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

void KisAsyncStoryboardThumbnailRenderer::frameCompletedCallback(int frame, const KisRegion &requestedRegion)
{
    KisImageSP image = requestedImage();
    if (image) {
        m_requestedFrameProjection = new KisPaintDevice(*image->projection(), KritaUtils::CopySnapshot);
    }
    emit sigNotifyFrameCompleted(frame);
}

void KisAsyncStoryboardThumbnailRenderer::frameCancelledCallback(int frame)
{
    emit sigNotifyFrameCancelled(frame);
}

void KisAsyncStoryboardThumbnailRenderer::clearFrameRegenerationState(bool isCancelled)
{
    KisAsyncAnimationRendererBase::clearFrameRegenerationState(isCancelled);
}
