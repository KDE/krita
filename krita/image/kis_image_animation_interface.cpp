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

#include "kis_global.h"
#include "kis_image.h"
#include "kis_regenerate_frame_stroke_strategy.h"
#include "kis_keyframe_channel.h"
#include "kis_time_range.h"

#include "kis_post_execution_undo_adapter.h"
#include "commands_new/kis_switch_current_time_command.h"



struct KisImageAnimationInterface::Private
{
    Private()
        : image(0),
          currentTime(0),
          currentUITime(0),
          externalFrameActive(false),
          frameInvalidationBlocked(false),
          cachedLastFrameValue(-1)
    {
    }

    KisImage *image;
    int currentTime;
    int currentUITime;
    bool externalFrameActive;
    bool frameInvalidationBlocked;

    KisTimeRange currentRange;
    KisTimeRange playbackRange;
    int framerate;
    int cachedLastFrameValue;
};


KisImageAnimationInterface::KisImageAnimationInterface(KisImage *image)
    : m_d(new Private)
{
    m_d->image = image;

    m_d->framerate = 24;
    m_d->currentRange = KisTimeRange::fromTime(0, 100);

    connect(this, SIGNAL(sigInternalRequestTimeSwitch(int)), SLOT(switchCurrentTimeAsync(int)));
}

KisImageAnimationInterface::~KisImageAnimationInterface()
{
}

int KisImageAnimationInterface::currentTime() const
{
    return m_d->currentTime;
}

int KisImageAnimationInterface::currentUITime() const
{
    return m_d->currentUITime;
}

const KisTimeRange& KisImageAnimationInterface::currentRange() const
{
    return m_d->currentRange;
}

void KisImageAnimationInterface::setRange(const KisTimeRange range) {
    m_d->currentRange = range;
    emit sigRangeChanged();
}

const KisTimeRange& KisImageAnimationInterface::playbackRange() const
{
    return m_d->playbackRange.isValid() ? m_d->playbackRange : m_d->currentRange;
}

void KisImageAnimationInterface::setPlaybackRange(const KisTimeRange range)
{
    m_d->playbackRange = range;
    emit sigPlaybackRangeChanged();
}

int KisImageAnimationInterface::framerate() const
{
    return m_d->framerate;
}

void KisImageAnimationInterface::setFramerate(int fps)
{
    m_d->framerate = fps;
    emit sigFramerateChanged();
}

KisImageWSP KisImageAnimationInterface::image() const
{
    return m_d->image;
}

bool KisImageAnimationInterface::externalFrameActive() const
{
    return m_d->externalFrameActive;
}

void KisImageAnimationInterface::requestTimeSwitchWithUndo(int time)
{
    if (m_d->currentUITime == time) return;

    KisSwitchCurrentTimeCommand *cmd =
        new KisSwitchCurrentTimeCommand(m_d->image, time);

    cmd->redo();
    m_d->image->postExecutionUndoAdapter()->addCommand(toQShared(cmd));
}

void KisImageAnimationInterface::requestTimeSwitchNonGUI(int time)
{
    emit sigInternalRequestTimeSwitch(time);
}

void KisImageAnimationInterface::switchCurrentTimeAsync(int frameId)
{
    if (m_d->currentUITime == frameId) return;

    m_d->image->barrierLock();
    m_d->currentTime = frameId;
    m_d->currentUITime = frameId;
    m_d->image->unlock();

    KisStrokeStrategy *strategy =
        new KisRegenerateFrameStrokeStrategy(this);

    KisStrokeId stroke = m_d->image->startStroke(strategy);
    m_d->image->endStroke(stroke);

    emit sigTimeChanged(frameId);
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
    emit sigFrameReady(m_d->currentTime);
}

KisUpdatesFacade* KisImageAnimationInterface::updatesFacade() const
{
    return m_d->image;
}

void KisImageAnimationInterface::notifyNodeChanged(const KisNode *node,
                                                   const QRect &rect,
                                                   bool recursive)
{
    if (externalFrameActive() || m_d->frameInvalidationBlocked) return;

    KisKeyframeChannel *channel =
        node->getKeyframeChannel(KisKeyframeChannel::Content.id());

    if (recursive) {
        KisTimeRange affectedRange;
        KisTimeRange::calculateTimeRangeRecursive(node, currentTime(), affectedRange, false);

        invalidateFrames(affectedRange, rect);
    } else if (channel) {
        const int currentTime = m_d->currentTime;

        invalidateFrames(channel->affectedFrames(currentTime), rect);
    } else {
        invalidateFrames(KisTimeRange::infinite(0), rect);
    }
}

void KisImageAnimationInterface::invalidateFrames(const KisTimeRange &range, const QRect &rect)
{
    m_d->cachedLastFrameValue = -1;
    emit sigFramesChanged(range, rect);
}

void KisImageAnimationInterface::blockFrameInvalidation(bool value)
{
    m_d->frameInvalidationBlocked = value;
}

int findLastKeyframeTimeRecursive(KisNodeSP node)
{
    int time = 0;

    KisKeyframeChannel *channel;
    Q_FOREACH (channel, node->keyframeChannels()) {
        KisKeyframeSP keyframe = channel->lastKeyframe();
        if (keyframe) {
            time = std::max(time, keyframe->time());
        }
    }

    KisNodeSP child = node->firstChild();
    while (child) {
        time = std::max(time, findLastKeyframeTimeRecursive(child));
        child = child->nextSibling();
    }

    return time;
}

int KisImageAnimationInterface::totalLength()
{
    if (m_d->cachedLastFrameValue < 0) {
        m_d->cachedLastFrameValue = findLastKeyframeTimeRecursive(m_d->image->root());
    }

    int lastKey = m_d->cachedLastFrameValue;

    lastKey  = std::max(lastKey, m_d->currentRange.end());
    lastKey  = std::max(lastKey, m_d->currentUITime);

    return lastKey + 1;
}
