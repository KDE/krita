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


struct KisImageAnimationInterface::Private
{
    Private() : image(0), currentTime(0) {}

    KisImage *image;
    int currentTime;
};


KisImageAnimationInterface::KisImageAnimationInterface(KisImage *image)
    : m_d(new Private)
{
    m_d->image = image;
}

KisImageAnimationInterface::~KisImageAnimationInterface()
{
}

int KisImageAnimationInterface::currentTime() const
{
    return m_d->currentTime;
}

void KisImageAnimationInterface::switchCurrentTime(int frameId) const
{
    m_d->image->barrierLock();
    m_d->currentTime = frameId;
    m_d->image->unlock();
}

KisPaintDeviceSP KisImageAnimationInterface::frameProjection() const
{
    return m_d->image->projection();
}

void KisImageAnimationInterface::requestFrameRegeneration(int frameId, const QRegion &dirtyRegion, bool populateCache)
{
    KisStrokeStrategy *strategy =
        new KisRegenerateFrameStrokeStrategy(frameId,
                                             dirtyRegion,
                                             populateCache,
                                             this);

    KisStrokeId stroke = m_d->image->startStroke(strategy);
    m_d->image->endStroke(stroke);
}

void KisImageAnimationInterface::requestFrame(int frameId, bool populateCache)
{
    requestFrameRegeneration(frameId, QRegion(), populateCache);
}

void KisImageAnimationInterface::setCurrentTimeExplicitly(int frameId)
{
    m_d->currentTime = frameId;
}

void KisImageAnimationInterface::notifyFrameReady(bool populateCache)
{
    emit sigFrameReady(populateCache);
}

KisUpdatesFacade* KisImageAnimationInterface::updatesFacade() const
{
    return m_d->image;
}
