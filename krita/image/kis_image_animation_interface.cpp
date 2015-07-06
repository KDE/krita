/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_image_animation_interface.h"

#include "kis_image.h"
#include "kis_regenerate_frame_stroke_strategy.h"
#include "kis_animation_frame_cache.h"
#include "kis_keyframe_channel.h"

struct KisImageAnimationInterface::Private
{
    Private() : image(0), currentTime(0), externalFrameActive(false) {}

    KisImage *image;
    int currentTime;
    bool externalFrameActive;

    KisAnimationFrameCache *frameCache;
};


KisImageAnimationInterface::KisImageAnimationInterface(KisImage *image)
    : m_d(new Private)
{
    m_d->image = image;
    m_d->frameCache = new KisAnimationFrameCache();
}

KisImageAnimationInterface::~KisImageAnimationInterface()
{
    delete m_d->frameCache;
}

int KisImageAnimationInterface::currentTime() const
{
    return m_d->currentTime;
}

bool KisImageAnimationInterface::externalFrameActive() const
{
    return m_d->externalFrameActive;
}

void KisImageAnimationInterface::switchCurrentTimeAsync(int frameId)
{
    // TODO: remove once proper frame caching is implemented
    QImage frameProjection = m_d->image->convertToQImage(m_d->image->bounds(), m_d->image->profile());
    m_d->frameCache->cacheFrame(m_d->currentTime, frameProjection);
    // </TODO>

    m_d->image->barrierLock();
    m_d->currentTime = frameId;
    m_d->image->unlock();

    m_d->image->refreshGraphAsync();

    emit sigTimeChanged(frameId);
}

KisPaintDeviceSP KisImageAnimationInterface::frameProjection() const
{
    return m_d->image->projection();
}

void KisImageAnimationInterface::requestFrameRegeneration(int frameId, const QRegion &dirtyRegion)
{
    KisStrokeStrategy *strategy =
        new KisRegenerateFrameStrokeStrategy(frameId,
                                             dirtyRegion,
                                             this);

    KisStrokeId stroke = m_d->image->startStroke(strategy);
    m_d->image->endStroke(stroke);
}

void KisImageAnimationInterface::saveAndResetCurrentTime(int frameId, int *savedValue)
{
    m_d->externalFrameActive = true;
    *savedValue = m_d->currentTime;
    m_d->currentTime = frameId;
}

void KisImageAnimationInterface::restoreCurrentTime(int *savedValue)
{
    m_d->currentTime = *savedValue;
    m_d->externalFrameActive = false;
}

void KisImageAnimationInterface::notifyFrameReady()
{
    emit sigFrameReady();
}

KisUpdatesFacade* KisImageAnimationInterface::updatesFacade() const
{
    return m_d->image;
}

QImage KisImageAnimationInterface::getCachedFrame(int time)
{
    return m_d->frameCache->getFrame(time);
}

void KisImageAnimationInterface::notifyNodeChanged(KisNode *node,
                                                   const QRect &rect,
                                                   bool recursive)
{
    // TODO: handle 'recursive'

    KisKeyframeChannel *channel =
        node->getKeyframeChannel(KisKeyframeChannel::Content.id());

    if (!recursive && channel) {
        const int currentTime = m_d->currentTime;

        // TODO: check which frames are affected and emit corresponding
        //       range
        emit sigFramesChanged(KisTimeRange(currentTime, 1));
    } else {
        emit sigFramesChanged(KisTimeRange::infinity());
    }
}
