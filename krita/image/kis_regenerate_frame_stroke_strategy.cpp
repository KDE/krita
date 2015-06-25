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

#include "kis_regenerate_frame_stroke_strategy.h"

#include <QRegion>
#include "kis_image_interfaces.h"
#include "kis_image_animation_interface.h"
#include "kis_node.h"


struct KisRegenerateFrameStrokeStrategy::Private
{
    int frameId;
    int previousFrameId;
    QRegion dirtyRegion;
    KisImageAnimationInterface *interface;
};


KisRegenerateFrameStrokeStrategy::KisRegenerateFrameStrokeStrategy(int frameId,
                                                                   const QRegion &dirtyRegion,
                                                                   KisImageAnimationInterface *interface)
    : m_d(new Private)
{
    m_d->frameId = frameId;
    m_d->dirtyRegion = dirtyRegion;
    m_d->interface = interface;

    enableJob(JOB_INIT, true, KisStrokeJobData::BARRIER);
    enableJob(JOB_FINISH, true, KisStrokeJobData::BARRIER);
    enableJob(JOB_CANCEL, true, KisStrokeJobData::BARRIER);
}

KisRegenerateFrameStrokeStrategy::~KisRegenerateFrameStrokeStrategy()
{
}

void KisRegenerateFrameStrokeStrategy::initStrokeCallback()
{
    m_d->interface->saveAndResetCurrentTime(m_d->frameId, &m_d->previousFrameId);

    if (!m_d->dirtyRegion.isEmpty()) {
        m_d->interface->updatesFacade()->refreshGraphAsync();
    }
}

void KisRegenerateFrameStrokeStrategy::finishStrokeCallback()
{
    m_d->interface->notifyFrameReady();
    m_d->interface->restoreCurrentTime(&m_d->previousFrameId);
}

void KisRegenerateFrameStrokeStrategy::cancelStrokeCallback()
{
    m_d->interface->restoreCurrentTime(&m_d->previousFrameId);
}
