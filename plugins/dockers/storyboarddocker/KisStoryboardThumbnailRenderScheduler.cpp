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


#include "KisStoryboardThumbnailRenderScheduler.h"
#include "KisAsyncStoryboardThumbnailRenderer.h"
#include "kis_paint_device.h"

KisStoryboardThumbnailRenderScheduler::KisStoryboardThumbnailRenderScheduler(QObject *parent)
    : m_renderer(new KisAsyncStoryboardThumbnailRenderer(this))
{
    //connect signals to the renderer.
    connect(m_renderer, SIGNAL(sigFrameCompleted(int)), this, SLOT(slotFrameRegenerationCompleted(int)));
    connect(m_renderer, SIGNAL(sigFrameCancelled(int)), this, SLOT(slotFrameRegenerationCancelled(int)));
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
    ENTER_FUNCTION() << ppVar(frame) << ppVar(affected);
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

    //if the renderer is idle start rendering the newly added frames
    if (!m_renderer->isActive()) {
        renderNextFrame();
    }
}

void KisStoryboardThumbnailRenderScheduler::cancelAllFrameRendering()
{
    m_affectedFramesQueue.empty();
    m_changedFramesQueue.empty();
    if (m_renderer->isActive()) {
        m_renderer->cancelCurrentFrameRendering();
    }
    m_currentFrame = -1;
}

void KisStoryboardThumbnailRenderScheduler::cancelFrameRendering(int frame)
{
    if (frame < 0) {
        return;
    }
    if (m_renderer->isActive() && frame == m_currentFrame) {
        m_renderer->cancelCurrentFrameRendering();
        m_currentFrame = -1;
    }
    else if (m_changedFramesQueue.contains(frame)) {
        m_changedFramesQueue.removeAll(frame);
    }
    else if (m_affectedFramesQueue.contains(frame)) {
        m_affectedFramesQueue.removeAll(frame);
    }
}

void KisStoryboardThumbnailRenderScheduler::slotFrameRegenerationCompleted(int frame)
{
    emit sigFrameCompleted(frame, m_renderer->frameProjection());
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
    if (!m_image) {
        return;
    }
    KisImageSP image = m_image->clone(false);
    if (!m_changedFramesQueue.isEmpty()) {
        int frame = m_changedFramesQueue.at(0);
        image->requestTimeSwitch(frame);
        m_renderer->startFrameRegeneration(image, frame);
        m_currentFrame = frame;
        m_changedFramesQueue.removeFirst();
    }
    else if (!m_affectedFramesQueue.isEmpty()) {
        int frame = m_affectedFramesQueue.at(0);
        image->requestTimeSwitch(frame);
        m_renderer->startFrameRegeneration(image, frame);
        m_currentFrame = frame;
        m_affectedFramesQueue.removeFirst();
    }
    else {
        m_currentFrame = -1;
    }
}
