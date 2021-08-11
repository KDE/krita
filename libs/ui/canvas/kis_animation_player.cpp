/*
 *  SPDX-FileCopyrightText: 2015 Jouni Pentikäinen <joupent@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Eoin O'Neill <eoinoneill1991@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_animation_player.h"

#include <QElapsedTimer>
#include <QTimer>
#include <QtMath>

//#define PLAYER_DEBUG_FRAMERATE

#include "kis_global.h"
#include "kis_algebra_2d.h"

#include "kis_config.h"
#include "kis_config_notifier.h"
#include "kis_image.h"
#include "kis_canvas2.h"
#include "kis_animation_frame_cache.h"
#include "kis_signal_auto_connection.h"
#include "kis_image_animation_interface.h"
#include "kis_time_span.h"
#include "kis_signal_compressor.h"
#include <KisDocument.h>
#include <QFileInfo>
#include "KisSyncedAudioPlayback.h"
#include "kis_signal_compressor_with_param.h"
#include "kis_image_barrier_locker.h"
#include "kis_layer_utils.h"
#include "KisDecoratedNodeInterface.h"
#include "kis_keyframe_channel.h"
#include "kis_algebra_2d.h"

#include "kis_image_config.h"
#include <limits>

#include "KisViewManager.h"
#include "kis_icon_utils.h"

#include "KisPart.h"
#include "dialogs/KisAsyncAnimationCacheRenderDialog.h"
#include "KisRollingMeanAccumulatorWrapper.h"
#include "kis_onion_skin_compositor.h"


qint64 framesToMSec(qreal value, int fps) {
    return qRound(value / fps * 1000.0);
}

qreal msecToFrames(qint64 value, int fps) {
    return qreal(value) * fps / 1000.0;
}

int framesToScaledTimeMS(qreal frame, int fps, qreal playbackSpeed) {
    return qRound(framesToMSec(frame, fps) / playbackSpeed);
}

qreal scaledTimeToFrames(qint64 time, int fps, qreal playbackSpeed) {
    return msecToFrames(time, fps) * playbackSpeed;
}


struct KisAnimationPlayer::Private
{
public:
    Private()
        : dropFramesMode(true),
          playbackFrame(0),
          visibleFrame(-1),
          playbackStatisticsCompressor(1000, KisSignalCompressor::FIRST_INACTIVE)
          {}

    KisCanvas2 *canvas;
    QVector<KisNodeWSP> disabledDecoratedNodes;

    KisAnimationPlayer::PlaybackState playbackState;

    KisSignalAutoConnectionsStore cancelStrokeConnections;

    int playbackOriginFrame; //!< The frame user started playback from.
    int initialFrame;     //TODO: Why need?
    qreal playbackSpeed; //TODO: Should go in timing specific class?

    bool dropFramesMode;    //!< Whether we should be dropping frames to preserve playback timing.
    bool useFastFrameUpload; //TODO: Figure this one out...

    //There are two frames we need to keep track of...
    int playbackFrame; //!< The **current** playback frame. May or may not be the same as the visible frame (Frame dropping, audio, etc...)
    int visibleFrame; //!< This the frame that is currently being displayed on the canvas. Can be different from the current frame.

    KisSignalCompressor playbackStatisticsCompressor;
};

KisAnimationPlayer::KisAnimationPlayer(KisCanvas2 *canvas)
    : QObject(canvas)
    , m_d(new Private())
{
    m_d->useFastFrameUpload = false;
    m_d->playbackState = STOPPED;
    m_d->canvas = canvas;
    m_d->playbackSpeed = 1.0;

    connect(KisConfigNotifier::instance(),
            SIGNAL(dropFramesModeChanged()),
            SLOT(updateDropFramesMode()));
    updateDropFramesMode();

    connect(&m_d->playbackStatisticsCompressor, SIGNAL(timeout()),
            this, SIGNAL(sigPlaybackStatisticsUpdated()));

    // Grow to new playback range when new frames added (configurable)...
    connect(m_d->canvas->image()->animationInterface(), &KisImageAnimationInterface::sigKeyframeAdded, m_d->canvas, [this](const KisKeyframeChannel*, int time){
        if (m_d->canvas && m_d->canvas->image()) {
            KisImageAnimationInterface* animInterface = m_d->canvas->image()->animationInterface();
            KisConfig cfg(true);
            if (animInterface && cfg.adaptivePlaybackRange()) {
                KisTimeSpan desiredPlaybackRange = animInterface->fullClipRange();
                desiredPlaybackRange.include(time);
                animInterface->setFullClipRange(desiredPlaybackRange);
            }
        }
    });
}

KisAnimationPlayer::~KisAnimationPlayer()
{}

KisAnimationPlayer::PlaybackState KisAnimationPlayer::playbackState()
{
    return m_d->playbackState;
}

void KisAnimationPlayer::updateDropFramesMode()
{
    KisConfig cfg(true);
    m_d->dropFramesMode = cfg.animationDropFrames();
}

void KisAnimationPlayer::connectCancelSignals()
{
    m_d->cancelStrokeConnections.addConnection(
        m_d->canvas->image().data(), SIGNAL(sigUndoDuringStrokeRequested()),
        this, SLOT(slotCancelPlayback()));

    m_d->cancelStrokeConnections.addConnection(
        m_d->canvas->image().data(), SIGNAL(sigStrokeCancellationRequested()),
        this, SLOT(slotCancelPlayback()));

    m_d->cancelStrokeConnections.addConnection(
        m_d->canvas->image().data(), SIGNAL(sigStrokeEndRequested()),
        this, SLOT(slotCancelPlaybackSafe())); // See master @slotCancelPlaybackSafe, but short is keep in mind that devices without caching shouldn't cancel playback at end of stroke!
}

void KisAnimationPlayer::disconnectCancelSignals()
{
    m_d->cancelStrokeConnections.clear();
}

void KisAnimationPlayer::play()
{
    const KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();
    const KisTimeSpan range = activePlaybackRange();

    setPlaybackState(PLAYING);
}

void KisAnimationPlayer::pause()
{
    seek(m_d->playbackFrame);
    setPlaybackState(PAUSED);
}

void KisAnimationPlayer::playPause()
{
    if (m_d->playbackState == PLAYING) {
        pause();
    } else {
        play();
    }
}

/**
 * @brief Higher level stop behavior.
 * When playing causes animation to halt and go to playback origin.
 * When stopped causes player to jump back to starting frame.
 */
void KisAnimationPlayer::stop()
{
    if(m_d->playbackSpeed == STOPPED) {
        goToStartFrame();
    } else {
        goToPlaybackOrigin();
        setPlaybackState(STOPPED);
    }
}

void KisAnimationPlayer::seek(int frameIndex, bool preferCachedFrames)
{
    if (!m_d->canvas || !m_d->canvas->image()) return;

    if (m_d->playbackState == PLAYING || preferCachedFrames) {
        if (m_d->playbackFrame != frameIndex) {
            m_d->playbackFrame = frameIndex >= 0 ? frameIndex : m_d->playbackFrame;
            displayFrame(m_d->playbackFrame);
        }
    } else {
        KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();

        if (frameIndex == animInterface->currentUITime()) {
            return;
        }

        animInterface->requestTimeSwitchWithUndo(frameIndex);
    }
}

void KisAnimationPlayer::previousFrame()
{
    if (!m_d->canvas) return;
    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();

    const int startFrame = animInterface->playbackRange().start();
    const int endFrame = animInterface->playbackRange().end();

    int frame = visibleFrame() - 1;

    if (frame < startFrame || frame >  endFrame) {
        frame = endFrame;
    }

    if (frame >= 0) {
        seek(frame);
    }
}

void KisAnimationPlayer::nextFrame()
{
    if (!m_d->canvas) return;
    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();

    const int startFrame = animInterface->playbackRange().start();
    const int endFrame = animInterface->playbackRange().end();

    int frame = visibleFrame() + 1;

    if (frame > endFrame || frame < startFrame ) {
        frame = startFrame;
    }

    if (frame >= 0) {
        seek(frame);
    }
}

void KisAnimationPlayer::previousKeyframe()
{
    if (!m_d->canvas) return;

    KisNodeSP node = m_d->canvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    int currentFrame = visibleFrame();

    int destinationTime = -1;
    if (!keyframes->keyframeAt(currentFrame)) {
        destinationTime = keyframes->activeKeyframeTime(currentFrame);
    } else {
        destinationTime = keyframes->previousKeyframeTime(currentFrame);
    }

    if (keyframes->keyframeAt(destinationTime)) {
        seek(destinationTime);
    }
}

void KisAnimationPlayer::nextKeyframe()
{
    if (!m_d->canvas) return;
    KisNodeSP node = m_d->canvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    int currentTime = visibleFrame();

    int destinationTime = -1;
    if (keyframes->activeKeyframeAt(currentTime)) {
        destinationTime = keyframes->nextKeyframeTime(currentTime);
    }

    if (keyframes->keyframeAt(destinationTime)) {
        // Jump to next key...
        seek(destinationTime);
    } else {
        // Jump ahead by estimated timing...
        const int activeKeyTime = keyframes->activeKeyframeTime(currentTime);
        const int previousKeyTime = keyframes->previousKeyframeTime(activeKeyTime);

        if (previousKeyTime != -1) {
            const int timing = activeKeyTime - previousKeyTime;
            seek(currentTime + timing);
        }
    }
}

void KisAnimationPlayer::previousMatchingKeyframe()
{
    if (!m_d->canvas) return;

    KisNodeSP node = m_d->canvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    int time = visibleFrame();

    KisKeyframeSP currentKeyframe = keyframes->keyframeAt(time);
    int destinationTime = keyframes->activeKeyframeTime(time);
    const int desiredColor = currentKeyframe ? currentKeyframe->colorLabel() : keyframes->keyframeAt(destinationTime)->colorLabel();
    previousKeyframeWithColor(desiredColor);
}

void KisAnimationPlayer::nextMatchingKeyframe()
{
    if (!m_d->canvas) return;

    KisNodeSP node = m_d->canvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    int time = visibleFrame();

    if (!keyframes->activeKeyframeAt(time)) {
        return;
    }

    int destinationTime = keyframes->activeKeyframeTime(time);
    nextKeyframeWithColor(keyframes->keyframeAt(destinationTime)->colorLabel());
}

void KisAnimationPlayer::previousUnfilteredKeyframe()
{
    previousKeyframeWithColor(KisOnionSkinCompositor::instance()->colorLabelFilter());
}

void KisAnimationPlayer::nextUnfilteredKeyframe()
{
    nextKeyframeWithColor(KisOnionSkinCompositor::instance()->colorLabelFilter());
}

void KisAnimationPlayer::goToPlaybackOrigin()
{
    KisImageAnimationInterface *animation = m_d->canvas->image()->animationInterface();
    if (animation->currentUITime() == m_d->playbackOriginFrame) {
        m_d->canvas->refetchDataFromImage();
    } else {
        animation->switchCurrentTimeAsync(m_d->playbackOriginFrame);
    }
}

void KisAnimationPlayer::goToStartFrame()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->canvas);

    KisImageAnimationInterface *animation = m_d->canvas->image()->animationInterface();
    const int startFrame = animation->playbackRange().start();

    animation->switchCurrentTimeAsync(startFrame);
}


void KisAnimationPlayer::displayFrame(int frameToDisplay)
{
    KisAnimationFrameCacheSP frameCache = m_d->canvas->frameCache();
    if (frameCache
        && frameCache->shouldUploadNewFrame(frameToDisplay, m_d->visibleFrame)
        && frameCache->uploadFrame(frameToDisplay)) {

        m_d->canvas->updateCanvas();

    } else if (m_d->canvas->image()->animationInterface()->hasAnimation()) {

        if (m_d->canvas->image()->tryBarrierLock(true)) {
            m_d->canvas->image()->unlock();
            m_d->canvas->image()->animationInterface()->switchCurrentTimeAsync(frameToDisplay);
        }
    }

    m_d->visibleFrame = frameToDisplay;
    emit sigFrameChanged();
}

void KisAnimationPlayer::nextKeyframeWithColor(int color)
{
    QSet<int> validColors;
    validColors.insert(color);
    nextKeyframeWithColor(validColors);
}

void KisAnimationPlayer::nextKeyframeWithColor(const QSet<int> &validColors)
{
    if (!m_d->canvas) return;

    KisNodeSP node = m_d->canvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    int time = visibleFrame();

    if (!keyframes->activeKeyframeAt(time)) {
        return;
    }

    int destinationTime = keyframes->activeKeyframeTime(time);
    while ( keyframes->keyframeAt(destinationTime) &&
            ((destinationTime == time) ||
            !validColors.contains(keyframes->keyframeAt(destinationTime)->colorLabel()))) {

        destinationTime = keyframes->nextKeyframeTime(destinationTime);
    }

    if (keyframes->keyframeAt(destinationTime)) {
        seek(destinationTime);
    }
}

void KisAnimationPlayer::previousKeyframeWithColor(int color)
{
    QSet<int> validColors;
    validColors.insert(color);
    previousKeyframeWithColor(validColors);
}

void KisAnimationPlayer::previousKeyframeWithColor(const QSet<int> &validColors)
{
    if (!m_d->canvas) return;

    KisNodeSP node = m_d->canvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    int time = visibleFrame();

    int destinationTime = keyframes->activeKeyframeTime(time);
    while (keyframes->keyframeAt(destinationTime) &&
           ((destinationTime == time) ||
           !validColors.contains(keyframes->keyframeAt(destinationTime)->colorLabel()))) {

        destinationTime = keyframes->previousKeyframeTime(destinationTime);
    }

    if (keyframes->keyframeAt(destinationTime)) {
        seek(destinationTime);
    }
}

KisTimeSpan KisAnimationPlayer::activePlaybackRange()
{
    if (!m_d->canvas || !m_d->canvas->image()) {
        return KisTimeSpan::infinite(0);
    }

    const KisImageAnimationInterface *animation = m_d->canvas->image()->animationInterface();
    return animation->playbackRange();
}

qreal KisAnimationPlayer::playbackSpeed()
{
    return m_d->playbackSpeed;
}

int KisAnimationPlayer::visibleFrame()
{
    if (m_d->playbackState != PLAYING) {
        if (m_d->canvas && m_d->canvas->image()) {
            return m_d->canvas->image()->animationInterface()->currentUITime();
        } else {
            return -1;
        }
    } else {
        return m_d->visibleFrame;
    }
}

void KisAnimationPlayer::setPlaybackSpeedPercent(int value)
{
    const float normalizedSpeed = value / 100.0;
    setPlaybackSpeedNormalized(normalizedSpeed);
}

void KisAnimationPlayer::setPlaybackSpeedNormalized(double value)
{
    if (m_d->playbackSpeed != value) {
        m_d->playbackSpeed = value;
        emit sigPlaybackSpeedChanged(m_d->playbackSpeed);
    }
}

void KisAnimationPlayer::setPlaybackState(PlaybackState state)
{
    if (m_d->playbackState != state) {
        m_d->playbackState = state;
        emit sigPlaybackStateChanged(m_d->playbackState);
    }
}
