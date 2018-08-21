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

#include <QFileInfo>

#include "kis_global.h"
#include "kis_image.h"
#include "kis_regenerate_frame_stroke_strategy.h"
#include "kis_switch_time_stroke_strategy.h"
#include "kis_keyframe_channel.h"
#include "kis_time_range.h"

#include "kis_post_execution_undo_adapter.h"
#include "commands_new/kis_switch_current_time_command.h"
#include "kis_layer_utils.h"


struct KisImageAnimationInterface::Private
{
    Private()
        : image(0),
          externalFrameActive(false),
          frameInvalidationBlocked(false),
          cachedLastFrameValue(-1),
          audioChannelMuted(false),
          audioChannelVolume(0.5),
          m_currentTime(0),
          m_currentUITime(0)
    {
    }

    Private(const Private &rhs, KisImage *newImage)
        : image(newImage),
          externalFrameActive(false),
          frameInvalidationBlocked(false),
          fullClipRange(rhs.fullClipRange),
          playbackRange(rhs.playbackRange),
          framerate(rhs.framerate),
          cachedLastFrameValue(-1),
          audioChannelFileName(rhs.audioChannelFileName),
          audioChannelMuted(rhs.audioChannelMuted),
          audioChannelVolume(rhs.audioChannelVolume),
          m_currentTime(rhs.m_currentTime),
          m_currentUITime(rhs.m_currentUITime)
    {
    }

    KisImage *image;
    bool externalFrameActive;
    bool frameInvalidationBlocked;

    KisTimeSpan fullClipRange;
    KisTimeSpan playbackRange;
    int framerate;
    int cachedLastFrameValue;
    QString audioChannelFileName;
    bool audioChannelMuted;
    qreal audioChannelVolume;

    KisSwitchTimeStrokeStrategy::SharedTokenWSP switchToken;

    inline int currentTime() const {
        return m_currentTime;
    }

    inline int currentUITime() const {
        return m_currentUITime;
    }
    inline void setCurrentTime(int value) {
        m_currentTime = value;
    }

    inline void setCurrentUITime(int value) {
        m_currentUITime = value;
    }
private:
    int m_currentTime;
    int m_currentUITime;
};


KisImageAnimationInterface::KisImageAnimationInterface(KisImage *image)
    : QObject(image),
      m_d(new Private)
{
    m_d->image = image;

    m_d->framerate = 24;
    m_d->fullClipRange = KisTimeSpan(0, 100);

    connect(this, SIGNAL(sigInternalRequestTimeSwitch(int, bool)), SLOT(switchCurrentTimeAsync(int, bool)));
}

KisImageAnimationInterface::KisImageAnimationInterface(const KisImageAnimationInterface &rhs, KisImage *newImage)
    : m_d(new Private(*rhs.m_d, newImage))
{
    connect(this, SIGNAL(sigInternalRequestTimeSwitch(int, bool)), SLOT(switchCurrentTimeAsync(int, bool)));
}

KisImageAnimationInterface::~KisImageAnimationInterface()
{
}

bool KisImageAnimationInterface::hasAnimation() const
{
    bool hasAnimation = false;

    KisLayerUtils::recursiveApplyNodes(
        m_d->image->root(),
        [&hasAnimation](KisNodeSP node) {
            hasAnimation |= node->isAnimated();
        });

    return hasAnimation;
}

int KisImageAnimationInterface::currentTime() const
{
    return m_d->currentTime();
}

int KisImageAnimationInterface::currentUITime() const
{
    return m_d->currentUITime();
}

const KisTimeSpan& KisImageAnimationInterface::fullClipRange() const
{
    return m_d->fullClipRange;
}

void KisImageAnimationInterface::setFullClipRange(const KisTimeSpan range)
{
    m_d->fullClipRange = range;
    emit sigFullClipRangeChanged();
}

void KisImageAnimationInterface::setFullClipRangeStartTime(int column)
{
    KisTimeSpan newRange(column,  m_d->fullClipRange.end());
    setFullClipRange(newRange);
}

void KisImageAnimationInterface::setFullClipRangeEndTime(int column)
{
    KisTimeSpan newRange(m_d->fullClipRange.start(), column);
    setFullClipRange(newRange);
}

const KisTimeSpan& KisImageAnimationInterface::playbackRange() const
{
    return !m_d->playbackRange.isEmpty() ? m_d->playbackRange : m_d->fullClipRange;
}

void KisImageAnimationInterface::setPlaybackRange(const KisTimeSpan range)
{
    m_d->playbackRange = range;
    emit sigPlaybackRangeChanged();
}

int KisImageAnimationInterface::framerate() const
{
    return m_d->framerate;
}

QString KisImageAnimationInterface::audioChannelFileName() const
{
    return m_d->audioChannelFileName;
}

void KisImageAnimationInterface::setAudioChannelFileName(const QString &fileName)
{
    QFileInfo info(fileName);

    KIS_SAFE_ASSERT_RECOVER_NOOP(fileName.isEmpty() || info.isAbsolute());
    m_d->audioChannelFileName = fileName.isEmpty() ? fileName : info.absoluteFilePath();

    emit sigAudioChannelChanged();
}

bool KisImageAnimationInterface::isAudioMuted() const
{
    return m_d->audioChannelMuted;
}

void KisImageAnimationInterface::setAudioMuted(bool value)
{
    m_d->audioChannelMuted = value;
    emit sigAudioChannelChanged();
}

qreal KisImageAnimationInterface::audioVolume() const
{
    return m_d->audioChannelVolume;
}

void KisImageAnimationInterface::setAudioVolume(qreal value)
{
    m_d->audioChannelVolume = value;
    emit sigAudioVolumeChanged();
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
    if (currentUITime() == time) return;
    requestTimeSwitchNonGUI(time, true);
}

void KisImageAnimationInterface::setDefaultProjectionColor(const KoColor &color)
{
    int savedTime = 0;
    saveAndResetCurrentTime(currentTime(), &savedTime);

    m_d->image->setDefaultProjectionColor(color);

    restoreCurrentTime(&savedTime);
}

void KisImageAnimationInterface::requestTimeSwitchNonGUI(int time, bool useUndo)
{
    emit sigInternalRequestTimeSwitch(time, useUndo);
}

void KisImageAnimationInterface::explicitlySetCurrentTime(int frameId)
{
    m_d->setCurrentTime(frameId);
}

void KisImageAnimationInterface::switchCurrentTimeAsync(int frameId, bool useUndo)
{
    if (currentUITime() == frameId) return;

    KisTimeRange range = KisTimeRange::infinite(0);
    KisTimeRange::calculateTimeRangeRecursive(m_d->image->root(), currentUITime(), range, true);

    const bool needsRegeneration = !range.contains(frameId);

    KisSwitchTimeStrokeStrategy::SharedTokenSP token =
        m_d->switchToken.toStrongRef();

    if (!token || !token->tryResetDestinationTime(frameId, needsRegeneration)) {

        {
            KisPostExecutionUndoAdapter *undoAdapter = useUndo ?
                m_d->image->postExecutionUndoAdapter() : 0;

            KisSwitchTimeStrokeStrategy *strategy =
                new KisSwitchTimeStrokeStrategy(frameId, needsRegeneration,
                                                this, undoAdapter);

            m_d->switchToken = strategy->token();

            KisStrokeId stroke = m_d->image->startStroke(strategy);
            m_d->image->endStroke(stroke);
        }

        if (needsRegeneration) {
            KisStrokeStrategy *strategy =
                new KisRegenerateFrameStrokeStrategy(this);

            KisStrokeId strokeId = m_d->image->startStroke(strategy);
            m_d->image->endStroke(strokeId);
        }
    }

    m_d->setCurrentUITime(frameId);
    emit sigUiTimeChanged(frameId);
}

void KisImageAnimationInterface::requestFrameRegeneration(int frameId, const QRegion &dirtyRegion)
{
    KisStrokeStrategy *strategy =
        new KisRegenerateFrameStrokeStrategy(frameId,
                                             dirtyRegion,
                                             this);

    QList<KisStrokeJobData*> jobs = KisRegenerateFrameStrokeStrategy::createJobsData(m_d->image);

    KisStrokeId stroke = m_d->image->startStroke(strategy);
    Q_FOREACH (KisStrokeJobData* job, jobs) {
        m_d->image->addJob(stroke, job);
    }
    m_d->image->endStroke(stroke);
}

void KisImageAnimationInterface::saveAndResetCurrentTime(int frameId, int *savedValue)
{
    m_d->externalFrameActive = true;
    *savedValue = m_d->currentTime();
    m_d->setCurrentTime(frameId);
}

void KisImageAnimationInterface::restoreCurrentTime(int *savedValue)
{
    m_d->setCurrentTime(*savedValue);
    m_d->externalFrameActive = false;
}

void KisImageAnimationInterface::notifyFrameReady()
{
    emit sigFrameReady(m_d->currentTime());
}

void KisImageAnimationInterface::notifyFrameCancelled()
{
    emit sigFrameCancelled();
}

KisUpdatesFacade* KisImageAnimationInterface::updatesFacade() const
{
    return m_d->image;
}

void KisImageAnimationInterface::notifyNodeChanged(const KisNode *node,
                                                   const QRect &rect,
                                                   bool recursive)
{
    notifyNodeChanged(node, QVector<QRect>({rect}), recursive);
}

void KisImageAnimationInterface::notifyNodeChanged(const KisNode *node,
                                                   const QVector<QRect> &rects,
                                                   bool recursive)
{
    if (externalFrameActive() || m_d->frameInvalidationBlocked) return;
    if (node->inherits("KisSelectionMask")) return;


    const int currentTime = m_d->currentTime();
    KisTimeRange invalidateRange;

    if (recursive) {
        KisTimeRange::calculateTimeRangeRecursive(node, currentTime, invalidateRange, false);
    } else {
        invalidateRange = KisTimeRange::calculateNodeAffectedFrames(node, currentTime);
    }

    // we compress the updated rect (atm, no one uses it anyway)
    QRect unitedRect;
    Q_FOREACH (const QRect &rc, rects) {
        unitedRect |= rc;
    }

    invalidateFrames(invalidateRange, unitedRect);
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

    lastKey  = std::max(lastKey, m_d->fullClipRange.end());
    lastKey  = std::max(lastKey, m_d->currentUITime());

    return lastKey + 1;
}
