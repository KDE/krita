/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_image_animation_interface.h"

#include <QFileInfo>

#include "kis_global.h"
#include "kis_image.h"
#include "kis_regenerate_frame_stroke_strategy.h"
#include "kis_switch_time_stroke_strategy.h"
#include "kis_keyframe_channel.h"
#include "kis_raster_keyframe_channel.h"
#include "kis_time_span.h"

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
          exportInitialFrameNumber(-1),
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
          exportSequenceFilePath(rhs.exportSequenceFilePath),
          exportSequenceBaseName(rhs.exportSequenceBaseName),
          exportInitialFrameNumber(rhs.exportInitialFrameNumber),
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

    QSet<int> activeLayerSelectedTimes;

    QString exportSequenceFilePath;
    QString exportSequenceBaseName;
    int exportInitialFrameNumber;

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
    m_d->fullClipRange = KisTimeSpan::fromTimeToTime(0, 100);

    connect(this, SIGNAL(sigInternalRequestTimeSwitch(int,bool)), SLOT(switchCurrentTimeAsync(int,bool)));
}

KisImageAnimationInterface::KisImageAnimationInterface(const KisImageAnimationInterface &rhs, KisImage *newImage)
    : m_d(new Private(*rhs.m_d, newImage))
{
    connect(this, SIGNAL(sigInternalRequestTimeSwitch(int,bool)), SLOT(switchCurrentTimeAsync(int,bool)));
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
    KIS_SAFE_ASSERT_RECOVER_RETURN(!range.isInfinite());
    m_d->fullClipRange = range;
    emit sigFullClipRangeChanged();
}

void KisImageAnimationInterface::setFullClipRangeStartTime(int column)
{
    KisTimeSpan newRange = KisTimeSpan::fromTimeToTime(column,  m_d->fullClipRange.end());
    setFullClipRange(newRange);
}

void KisImageAnimationInterface::setFullClipRangeEndTime(int column)
{
    KisTimeSpan newRange = KisTimeSpan::fromTimeToTime(m_d->fullClipRange.start(), column);
    setFullClipRange(newRange);
}

const KisTimeSpan& KisImageAnimationInterface::playbackRange() const
{
    return m_d->playbackRange.isValid() ? m_d->playbackRange : m_d->fullClipRange;
}

void KisImageAnimationInterface::setPlaybackRange(const KisTimeSpan range)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(!range.isInfinite());
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

QString KisImageAnimationInterface::exportSequenceFilePath()
{
    return m_d->exportSequenceFilePath;
}

void KisImageAnimationInterface::setExportSequenceFilePath(const QString &filePath)
{
    m_d->exportSequenceFilePath = filePath;
}

QString KisImageAnimationInterface::exportSequenceBaseName()
{
    return m_d->exportSequenceBaseName;
}

void KisImageAnimationInterface::setExportSequenceBaseName(const QString &baseName)
{
    m_d->exportSequenceBaseName = baseName;
}

int KisImageAnimationInterface::exportInitialFrameNumber()
{
    return m_d->exportInitialFrameNumber;
}

void KisImageAnimationInterface::setExportInitialFrameNumber(const int frameNum)
{
    m_d->exportInitialFrameNumber = frameNum;
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

QSet<int> KisImageAnimationInterface::activeLayerSelectedTimes()
{
    return m_d->activeLayerSelectedTimes;
}

void KisImageAnimationInterface::setActiveLayerSelectedTimes(const QSet<int>& times)
{
    m_d->activeLayerSelectedTimes = times;
}

void KisImageAnimationInterface::setFramerate(int fps)
{
    if (fps > 0) {
        m_d->framerate = fps;
        emit sigFramerateChanged();
    }
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

    const KisTimeSpan range = KisTimeSpan::calculateIdenticalFramesRecursive(m_d->image->root(), currentUITime());
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

void KisImageAnimationInterface::requestFrameRegeneration(int frameId, const KisRegion &dirtyRegion)
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

    // even overlay selection masks are not rendered in the cache
    if (node->inherits("KisSelectionMask")) return;

    QSet<int> affectedTimes;
    affectedTimes << m_d->currentTime();

    // We need to also invalidate ranges that contain cloned keyframe data.
    if (recursive) {
        QSet<int> clonedTimes;
        const int time = m_d->currentTime();
        KisLayerUtils::recursiveApplyNodes(node, [&clonedTimes, time](const KisNode* node){
            clonedTimes += KisRasterKeyframeChannel::clonesOf(node, time);
        });

        affectedTimes += clonedTimes;
    } else {
        affectedTimes += KisRasterKeyframeChannel::clonesOf(node, m_d->currentTime());
    }

    foreach (const int& time, affectedTimes ){
        KisTimeSpan invalidateRange;

        if (recursive) {
            invalidateRange = KisTimeSpan::calculateAffectedFramesRecursive(node, time);
        } else {
            invalidateRange = KisTimeSpan::calculateNodeAffectedFrames(node, time);
        }

        // we compress the updated rect (atm, no one uses it anyway)
        QRect unitedRect;
        Q_FOREACH (const QRect &rc, rects) {
            unitedRect |= rc;
        }

        invalidateFrames(invalidateRange, unitedRect);
    }
}

void KisImageAnimationInterface::invalidateFrames(const KisTimeSpan &range, const QRect &rect)
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
        time = std::max(time, channel->lastKeyframeTime());
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
