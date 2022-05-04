/*
 *  SPDX-FileCopyrightText: 2022 Eoin O'Neill <eoinoneill1991@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisFrameDisplayProxy.h"

#include "kis_canvas2.h"
#include "kis_animation_frame_cache.h"
#include "kis_image_animation_interface.h"

struct Private {
    Private(KisCanvas2* c)
        : displayedFrame(-1)
        , intendedFrame(0)
        , canvas(c) {}

    int intendedFrame;
    int displayedFrame;
    KisCanvas2 *canvas;
};

KisFrameDisplayProxy::KisFrameDisplayProxy(KisCanvas2* canvas, QObject *parent)
    : QObject(parent)
    , m_d(new Private(canvas))
{
    connect(m_d->canvas->image()->animationInterface(), &KisImageAnimationInterface::sigFrameRegenerated, this, [this](int frame){
        if (m_d->intendedFrame != frame ) {
            m_d->intendedFrame = frame;
            emit sigFrameChange();
        }

        if (m_d->displayedFrame != frame) {
            m_d->displayedFrame = frame;
            emit sigFrameDisplayRefreshed();
        }
    });
}

KisFrameDisplayProxy::~KisFrameDisplayProxy()
{
}

bool KisFrameDisplayProxy::displayFrame(int frame)
{
    KisAnimationFrameCacheSP cache = m_d->canvas->frameCache();
    KisImageAnimationInterface* ai = m_d->canvas->image()->animationInterface();

    if (frame != m_d->intendedFrame) {
        m_d->intendedFrame = frame;
        emit sigFrameChange();
    }

    if (cache && cache->shouldUploadNewFrame(frame, m_d->displayedFrame)
              && cache->uploadFrame(frame) ) {
        m_d->canvas->updateCanvas();
        m_d->displayedFrame = frame;
        emit sigFrameDisplayRefreshed();
        return true;
    } else if (!cache && ai->hasAnimation() && ai->currentUITime() != frame){
        if (m_d->canvas->image()->tryBarrierLock(true)) {
            m_d->canvas->image()->unlock();
            ai->switchCurrentTimeAsync(frame);
            return true;
        }
    }

    return false;
}

int KisFrameDisplayProxy::visibleFrame() const
{
    return m_d->displayedFrame;
}

int KisFrameDisplayProxy::frame() const
{
    return m_d->intendedFrame;
}
