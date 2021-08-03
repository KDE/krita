/*
 *  SPDX-FileCopyrightText: 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#include "KisStoryboardThumbnailRenderScheduler.h"
#include "KisAsyncStoryboardThumbnailRenderer.h"
#include "kis_paint_device.h"

KisStoryboardThumbnailRenderScheduler::KisStoryboardThumbnailRenderScheduler(QObject *parent)
    : QObject(parent)
    , m_renderer(new KisAsyncStoryboardThumbnailRenderer(this))
{
    //connect signals to the renderer.
    connect(m_renderer, SIGNAL(sigNotifyFrameCompleted(int,KisPaintDeviceSP)), this, SLOT(slotFrameRegenerationCompleted(int, KisPaintDeviceSP)));
    connect(m_renderer, SIGNAL(sigFrameCancelled(int, KisAsyncAnimationRendererBase::CancelReason)), this, SLOT(slotFrameRegenerationCancelled(int)));
}

KisStoryboardThumbnailRenderScheduler::~KisStoryboardThumbnailRenderScheduler()
{
    delete m_renderer;
}

void KisStoryboardThumbnailRenderScheduler::setImage(KisImageSP image)
{
    if (m_image == image) {
        return;
    }
    cancelAllFrameRendering();
    m_image = image;
}

void KisStoryboardThumbnailRenderScheduler::scheduleFrameForRegeneration(int frame, bool affected)
{
    if (affected && m_affectedFramesQueue.contains(frame)) {
        return;
    }
    else if (affected && !m_changedFramesQueue.contains(frame)) {
        m_affectedFramesQueue.prepend(frame);
    }
    else {
        if (m_changedFramesQueue.contains(frame)) {
            int framePos = m_changedFramesQueue.indexOf(frame);
            if (framePos == 0) {
                return;
            }
            m_changedFramesQueue.move(framePos, 0);
        }
        else {
            m_changedFramesQueue.prepend(frame);
        }
        sortAffectedFrameQueue();
    }
}

void KisStoryboardThumbnailRenderScheduler::cancelAllFrameRendering()
{
    m_affectedFramesQueue.empty();
    m_changedFramesQueue.empty();
    if (m_renderer->isActive()) {
        m_renderer->cancelCurrentFrameRendering(KisAsyncAnimationRendererBase::UserCancelled);
    }
    m_currentFrame = -1;
}

void KisStoryboardThumbnailRenderScheduler::cancelFrameRendering(int frame)
{
    if (frame < 0) {
        return;
    }
    if (m_renderer->isActive() && frame == m_currentFrame) {
        m_renderer->cancelCurrentFrameRendering(KisAsyncAnimationRendererBase::UserCancelled);
        m_currentFrame = -1;
    }
    else if (m_changedFramesQueue.contains(frame)) {
        m_changedFramesQueue.removeAll(frame);
    }
    else if (m_affectedFramesQueue.contains(frame)) {
        m_affectedFramesQueue.removeAll(frame);
    }
}

void KisStoryboardThumbnailRenderScheduler::slotStartFrameRendering()
{
    //if the renderer is idle start rendering the frames in queues
    if (!m_renderer->isActive()) {
        renderNextFrame();
    }
}


void KisStoryboardThumbnailRenderScheduler::slotFrameRegenerationCompleted(int frame, KisPaintDeviceSP contents)
{
    emit sigFrameCompleted(frame, contents);
    renderNextFrame();
}

void KisStoryboardThumbnailRenderScheduler::slotFrameRegenerationCancelled(int frame)
{
    emit sigFrameCancelled(frame);
    renderNextFrame();
}

void KisStoryboardThumbnailRenderScheduler::sortAffectedFrameQueue()
{
    int lastChangedFrame = m_changedFramesQueue[0];

    //sort the affected queue based on proximity to the last changed frame.
    std::sort(m_affectedFramesQueue.begin(), m_affectedFramesQueue.end(),
                [lastChangedFrame](const int &arg1, const int &arg2)
                {
                    return std::abs(arg1 - lastChangedFrame) < std::abs(arg2 - lastChangedFrame);
                });
}

void KisStoryboardThumbnailRenderScheduler::renderNextFrame()
{
    if (!m_image || !m_image->isIdle()) {
        return;
    }

    if (m_changedFramesQueue.isEmpty() && m_affectedFramesQueue.isEmpty()) {
        return;
    }

    KisImageSP image = m_image->clone(false);
    KIS_SAFE_ASSERT_RECOVER_RETURN(image);

    int frame = !m_changedFramesQueue.isEmpty() ? m_changedFramesQueue.takeFirst() : m_affectedFramesQueue.takeFirst();;
    image->requestTimeSwitch(frame);

    ENTER_FUNCTION() << ppVar(frame);

    m_renderer->startFrameRegeneration(image, frame);
    m_currentFrame = frame;
}
