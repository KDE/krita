/*
 *  SPDX-FileCopyrightText: 2015 Jouni Pentikäinen <joupent@gmail.com>
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

#include "kis_image_config.h"
#include <limits>

#include "KisViewManager.h"
#include "kis_icon_utils.h"

#include "KisPart.h"
#include "dialogs/KisAsyncAnimationCacheRenderDialog.h"
#include "KisRollingMeanAccumulatorWrapper.h"
#include "kis_onion_skin_compositor.h"

struct KisAnimationPlayer::Private
{
public:
    Private(KisAnimationPlayer *_q)
        : q(_q),
          realFpsAccumulator(24),
          droppedFpsAccumulator(24),
          droppedFramesPortion(24),
          dropFramesMode(true),
          nextFrameExpectedTime(0),
          expectedInterval(0),
          currentFrame(0),
          lastTimerInterval(0),
          lastPaintedFrame(0),
          playbackStatisticsCompressor(1000, KisSignalCompressor::FIRST_INACTIVE),
          stopAudioOnScrubbingCompressor(100, KisSignalCompressor::POSTPONE),
          audioOffsetTolerance(-1)
          {}

    KisAnimationPlayer *q;

    KisAnimationPlayer::PlaybackState playbackState;
    QTimer *timer;

    /// The frame user started playback from
    int playbackOriginFrame;
    int firstFrame;
    int lastFrame;
    qreal playbackSpeed;

    KisCanvas2 *canvas;

    KisSignalAutoConnectionsStore cancelStrokeConnections;

    QElapsedTimer realFpsTimer;
    KisRollingMeanAccumulatorWrapper realFpsAccumulator;
    KisRollingMeanAccumulatorWrapper droppedFpsAccumulator;
    KisRollingMeanAccumulatorWrapper droppedFramesPortion;

    bool dropFramesMode;

    bool useFastFrameUpload;

    /// Measures time since playback (re)started
    QElapsedTimer playbackTime;
    int nextFrameExpectedTime;
    int expectedInterval;
    /// The frame the current playback (re)started on
    int initialFrame;
    /// The frame currently displayed
    int currentFrame;
    int lastTimerInterval;
    int lastPaintedFrame;

    KisSignalCompressor playbackStatisticsCompressor;

    QScopedPointer<KisSyncedAudioPlayback> syncedAudio;
    QScopedPointer<KisSignalCompressorWithParam<int> > audioSyncScrubbingCompressor;
    KisSignalCompressor stopAudioOnScrubbingCompressor;

    int audioOffsetTolerance;
    QVector<KisNodeWSP> disabledDecoratedNodes;

    void haltImpl();

    int incFrame(int frame, int inc) {
        frame += inc;
        if (frame > lastFrame) {
            const int framesFromFirst = frame - firstFrame;
            const int rangeLength = lastFrame - firstFrame + 1;
            frame = firstFrame + framesFromFirst % rangeLength;
        }
        return frame;
    }

    qint64 framesToMSec(qreal value, int fps) const {
        return qRound(value / fps * 1000.0);
    }
    qreal msecToFrames(qint64 value, int fps) const {
        return qreal(value) * fps / 1000.0;
    }
    int framesToWalltime(qreal frame, int fps) const {
        return qRound(framesToMSec(frame, fps) / playbackSpeed);
    }
    qreal walltimeToFrames(qint64 time, int fps) const {
        return msecToFrames(time, fps) * playbackSpeed;
    }

    qreal playbackTimeInFrames(int fps) const {
        const qint64 cycleLength = lastFrame - firstFrame + 1;
        const qreal framesPlayed = walltimeToFrames(playbackTime.elapsed(), fps);
        const qreal framesSinceFirst = std::fmod(initialFrame + framesPlayed - firstFrame, cycleLength);
        return firstFrame + framesSinceFirst;
    }
};

KisAnimationPlayer::KisAnimationPlayer(KisCanvas2 *canvas)
    : QObject(canvas)
    , m_d(new Private(this))
{
    m_d->useFastFrameUpload = false;
    m_d->playbackState = STOPPED;
    m_d->canvas = canvas;
    m_d->playbackSpeed = 1.0;

    m_d->timer = new QTimer(this);
    connect(m_d->timer, SIGNAL(timeout()), this, SLOT(slotUpdate()));
    m_d->timer->setSingleShot(true);

    connect(KisConfigNotifier::instance(),
            SIGNAL(dropFramesModeChanged()),
            SLOT(slotUpdateDropFramesMode()));
    slotUpdateDropFramesMode();

    connect(&m_d->playbackStatisticsCompressor, SIGNAL(timeout()),
            this, SIGNAL(sigPlaybackStatisticsUpdated()));

    using namespace std::placeholders;
    std::function<void (int)> callback(
        std::bind(&KisAnimationPlayer::slotSyncScrubbingAudio, this, _1));

    const int defaultScrubbingUdpatesDelay = 40; /* 40 ms == 25 fps */

    m_d->audioSyncScrubbingCompressor.reset(
        new KisSignalCompressorWithParam<int>(defaultScrubbingUdpatesDelay, callback, KisSignalCompressor::FIRST_ACTIVE));

    m_d->stopAudioOnScrubbingCompressor.setDelay(defaultScrubbingUdpatesDelay);
    connect(&m_d->stopAudioOnScrubbingCompressor, SIGNAL(timeout()), SLOT(slotTryStopScrubbingAudio()));

    connect(m_d->canvas->image()->animationInterface(), SIGNAL(sigFramerateChanged()), SLOT(slotUpdateAudioChunkLength()));
    slotUpdateAudioChunkLength();

    connect(m_d->canvas->image()->animationInterface(), SIGNAL(sigAudioChannelChanged()), SLOT(slotAudioChannelChanged()));
    connect(m_d->canvas->image()->animationInterface(), SIGNAL(sigAudioVolumeChanged()), SLOT(slotAudioVolumeChanged()));

    // Grow to new playback range when new frames added (configurable)...
    connect(m_d->canvas->image()->animationInterface(), &KisImageAnimationInterface::sigKeyframeAdded, [this](const KisKeyframeChannel*, int time){
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

    slotAudioChannelChanged();
}

KisAnimationPlayer::~KisAnimationPlayer()
{}

void KisAnimationPlayer::slotUpdateDropFramesMode()
{
    KisConfig cfg(true);
    m_d->dropFramesMode = cfg.animationDropFrames();
}

void KisAnimationPlayer::slotSyncScrubbingAudio(int msecTime)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->syncedAudio);

    if (!m_d->syncedAudio->isPlaying()) {
        m_d->syncedAudio->play(msecTime);
    } else {
        m_d->syncedAudio->syncWithVideo(msecTime);
    }

    if (!isPlaying()) {
        m_d->stopAudioOnScrubbingCompressor.start();
    }
}

void KisAnimationPlayer::slotTryStopScrubbingAudio()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->syncedAudio);
    if (m_d->syncedAudio && !isPlaying()) {
        m_d->syncedAudio->stop();
    }
}

void KisAnimationPlayer::slotAudioChannelChanged()
{

    KisImageAnimationInterface *interface = m_d->canvas->image()->animationInterface();
    QString fileName = interface->audioChannelFileName();
    QFileInfo info(fileName);
    if (info.exists() && !interface->isAudioMuted()) {
        m_d->syncedAudio.reset(new KisSyncedAudioPlayback(info.absoluteFilePath()));
        m_d->syncedAudio->setVolume(interface->audioVolume());
        m_d->syncedAudio->setSoundOffsetTolerance(m_d->audioOffsetTolerance);

        connect(m_d->syncedAudio.data(), SIGNAL(error(QString,QString)), SLOT(slotOnAudioError(QString,QString)));
    } else {
        m_d->syncedAudio.reset();
    }
}

void KisAnimationPlayer::slotAudioVolumeChanged()
{
    KisImageAnimationInterface *interface = m_d->canvas->image()->animationInterface();
    if (m_d->syncedAudio) {
        m_d->syncedAudio->setVolume(interface->audioVolume());
    }
}

void KisAnimationPlayer::slotOnAudioError(const QString &fileName, const QString &message)
{
    QString errorMessage(i18nc("floating on-canvas message", "Cannot open audio: \"%1\"\nError: %2", fileName, message));
    m_d->canvas->viewManager()->showFloatingMessage(errorMessage, KisIconUtils::loadIcon("warning"));
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
        this, SLOT(slotCancelPlaybackSafe()));

    m_d->cancelStrokeConnections.addConnection(
        m_d->canvas->image()->animationInterface(), SIGNAL(sigFramerateChanged()),
        this, SLOT(slotUpdatePlaybackTimer()));

    m_d->cancelStrokeConnections.addConnection(
        m_d->canvas->image()->animationInterface(), SIGNAL(sigFullClipRangeChanged()),
        this, SLOT(slotUpdatePlaybackTimer()));

    m_d->cancelStrokeConnections.addConnection(
        m_d->canvas->image()->animationInterface(), SIGNAL(sigPlaybackRangeChanged()),
        this, SLOT(slotUpdatePlaybackTimer()));
}

void KisAnimationPlayer::disconnectCancelSignals()
{
    m_d->cancelStrokeConnections.clear();
}

void KisAnimationPlayer::slotUpdateAudioChunkLength()
{
    const KisImageAnimationInterface *animation = m_d->canvas->image()->animationInterface();
    const int animationFramePeriod = qMax(1, 1000 / animation->framerate());

    KisConfig cfg(true);
    int scrubbingAudioUdpatesDelay = cfg.scrubbingAudioUpdatesDelay();

    if (scrubbingAudioUdpatesDelay < 0) {

        scrubbingAudioUdpatesDelay = qMax(1, animationFramePeriod);
    }

    m_d->audioSyncScrubbingCompressor->setDelay(scrubbingAudioUdpatesDelay);
    m_d->stopAudioOnScrubbingCompressor.setDelay(scrubbingAudioUdpatesDelay);

    m_d->audioOffsetTolerance = cfg.audioOffsetTolerance();
    if (m_d->audioOffsetTolerance < 0) {
        m_d->audioOffsetTolerance = animationFramePeriod;
    }

    if (m_d->syncedAudio) {
        m_d->syncedAudio->setSoundOffsetTolerance(m_d->audioOffsetTolerance);
    }

    if (isPlaying()) {
        slotUpdatePlaybackTimer();
    }
}

void KisAnimationPlayer::slotUpdatePlaybackTimer()
{
    m_d->timer->stop();

    const KisImageAnimationInterface *animation = m_d->canvas->image()->animationInterface();
    const KisTimeSpan &playBackRange = animation->playbackRange();
    if (!playBackRange.isValid()) return;

    const int fps = animation->framerate();

    m_d->initialFrame = isPlaying() ? m_d->currentFrame : animation->currentUITime();
    m_d->firstFrame = playBackRange.start();
    m_d->lastFrame = playBackRange.end();
    m_d->currentFrame = qBound(m_d->firstFrame, m_d->currentFrame, m_d->lastFrame);

    m_d->expectedInterval = m_d->framesToWalltime(1, fps);
    m_d->lastTimerInterval = m_d->expectedInterval;

    if (m_d->syncedAudio) {
        m_d->syncedAudio->setSpeed(m_d->playbackSpeed);
        const qint64 expectedAudioTime = m_d->framesToMSec(m_d->currentFrame, fps);
        if (qAbs(m_d->syncedAudio->position() - expectedAudioTime) > m_d->framesToMSec(1.5, fps)) {
            m_d->syncedAudio->syncWithVideo(expectedAudioTime);
        }
    }

    m_d->timer->start(m_d->expectedInterval);

    if (m_d->playbackTime.isValid()) {
        m_d->playbackTime.restart();
    } else {
        m_d->playbackTime.start();
    }

    m_d->nextFrameExpectedTime = m_d->playbackTime.elapsed() + m_d->expectedInterval;
}

void KisAnimationPlayer::play()
{
    const KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();

    {
        const KisTimeSpan &range = animInterface->playbackRange();
        if (!range.isValid()) return;

        // when openGL is disabled, there is no animation cache
        if (m_d->canvas->frameCache()) {
            KisImageConfig cfg(true);

            const int dimensionLimit =
                cfg.useAnimationCacheFrameSizeLimit() ?
                cfg.animationCacheFrameSizeLimit() :
                std::numeric_limits<int>::max();

            const int maxImageDimension = KisAlgebra2D::maxDimension(m_d->canvas->image()->bounds());

            const QRect regionOfInterest =
                cfg.useAnimationCacheRegionOfInterest() && maxImageDimension > dimensionLimit ?
                m_d->canvas->regionOfInterest() :
                m_d->canvas->coordinatesConverter()->imageRectInImagePixels();

            const QRect minimalNeedRect =
                m_d->canvas->coordinatesConverter()->widgetRectInImagePixels().toAlignedRect() &
                m_d->canvas->coordinatesConverter()->imageRectInImagePixels();

            m_d->canvas->frameCache()->dropLowQualityFrames(range, regionOfInterest, minimalNeedRect);

            KisAsyncAnimationCacheRenderDialog dlg(m_d->canvas->frameCache(),
                                                   range,
                                                   200);

            dlg.setRegionOfInterest(regionOfInterest);

            KisAsyncAnimationCacheRenderDialog::Result result =
                dlg.regenerateRange(m_d->canvas->viewManager());

            if (result != KisAsyncAnimationCacheRenderDialog::RenderComplete) {
                return;
            }

            m_d->canvas->setRenderingLimit(regionOfInterest);
        } else {
            KisImageBarrierLocker locker(m_d->canvas->image());
            KisLayerUtils::recursiveApplyNodes(m_d->canvas->image()->root(),
                [this] (KisNodeSP node) {
                    KisDecoratedNodeInterface *decoratedNode = dynamic_cast<KisDecoratedNodeInterface*>(node.data());
                    if (decoratedNode && decoratedNode->decorationsVisible()) {
                        decoratedNode->setDecorationsVisible(false, false);
                        m_d->disabledDecoratedNodes.append(node);
                    }
                });
        }
    }

    if (m_d->playbackState == STOPPED) {
        m_d->playbackOriginFrame = animInterface->currentUITime();
        m_d->currentFrame = m_d->playbackOriginFrame;
    }

    m_d->playbackState = PLAYING;

    slotUpdatePlaybackTimer();
    m_d->lastPaintedFrame = -1;

    connectCancelSignals();

    if (m_d->syncedAudio) {
        KisImageAnimationInterface *animationInterface = m_d->canvas->image()->animationInterface();
        m_d->syncedAudio->play(m_d->framesToMSec(m_d->currentFrame, animationInterface->framerate()));
    }

    emit sigPlaybackStateChanged(isPlaying());
    emit sigPlaybackStarted();
}

void KisAnimationPlayer::pause()
{
    m_d->haltImpl();

    m_d->playbackState = PAUSED;

    KisImageAnimationInterface *animationInterface = m_d->canvas->image()->animationInterface();
    if(animationInterface) {
        animationInterface->switchCurrentTimeAsync(m_d->currentFrame);
    }

    emit sigPlaybackStateChanged(isPlaying());
    emit sigPlaybackStopped();
}

void KisAnimationPlayer::playPause()
{
    if (isPlaying()) {
        pause();
    } else {
        play();
    }
}

/**
 * @brief Halts the playback.
 */
void KisAnimationPlayer::halt()
{
    m_d->haltImpl();

    m_d->playbackState = STOPPED;

    emit sigPlaybackStateChanged(isPlaying());
    emit sigPlaybackStopped();
}

/**
 * @brief Higher level stop behavior.
 * When playing causes animation to halt and go to playback origin.
 * When stopped causes player to jump back to starting frame.
 */
void KisAnimationPlayer::stop()
{
    if (!m_d->canvas) return;

    if( m_d->canvas->animationPlayer()->isStopped()) {
        m_d->canvas->animationPlayer()->goToStartFrame();
    } else {
        m_d->canvas->animationPlayer()->halt();
        m_d->canvas->animationPlayer()->goToPlaybackOrigin();
    }
}

void KisAnimationPlayer::seek(int frameIndex)
{
    if (!m_d->canvas || !m_d->canvas->image()) return;

    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();

    if (m_d->canvas->animationPlayer()->isPlaying() ||
        frameIndex == animInterface->currentUITime()) {
        return;
    }

    animInterface->requestTimeSwitchWithUndo(frameIndex);
}

void KisAnimationPlayer::previousFrame()
{
    if (!m_d->canvas) return;
    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();

    const int startFrame = animInterface->playbackRange().start();
    const int endFrame = animInterface->playbackRange().end();

    int frame = animInterface->currentUITime() - 1;

    if (frame < startFrame || frame >  endFrame) {
        frame = endFrame;
    }

    if (frame >= 0) {
        animInterface->requestTimeSwitchWithUndo(frame);
    }
}

void KisAnimationPlayer::nextFrame()
{
    if (!m_d->canvas) return;
    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();

    const int startFrame = animInterface->playbackRange().start();
    const int endFrame = animInterface->playbackRange().end();

    int frame = animInterface->currentUITime() + 1;

    if (frame > endFrame || frame < startFrame ) {
        frame = startFrame;
    }

    animInterface->requestTimeSwitchWithUndo(frame);
}

void KisAnimationPlayer::previousKeyframe()
{
    if (!m_d->canvas) return;

    KisNodeSP node = m_d->canvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();
    int currentFrame = animInterface->currentUITime();

    int destinationTime = -1;
    if (!keyframes->keyframeAt(currentFrame)) {
        destinationTime = keyframes->activeKeyframeTime(currentFrame);
    } else {
        destinationTime = keyframes->previousKeyframeTime(currentFrame);
    }

    if (keyframes->keyframeAt(destinationTime)) {
        animInterface->requestTimeSwitchWithUndo(destinationTime);
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

    KisImageAnimationInterface *animation = m_d->canvas->image()->animationInterface();
    int currentTime = animation->currentUITime();

    int destinationTime = -1;
    if (keyframes->activeKeyframeAt(currentTime)) {
        destinationTime = keyframes->nextKeyframeTime(currentTime);
    }

    if (keyframes->keyframeAt(destinationTime)) {
        // Jump to next key...
        animation->requestTimeSwitchWithUndo(destinationTime);
    } else {
        // Jump ahead by estimated timing...
        const int activeKeyTime = keyframes->activeKeyframeTime(currentTime);
        const int previousKeyTime = keyframes->previousKeyframeTime(activeKeyTime);

        if (previousKeyTime != -1) {
            const int timing = activeKeyTime - previousKeyTime;
            animation->requestTimeSwitchWithUndo(currentTime + timing);
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

    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();
    int time = animInterface->currentUITime();

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

    KisImageAnimationInterface *animation = m_d->canvas->image()->animationInterface();
    int time = animation->currentUITime();

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

void KisAnimationPlayer::forcedStopOnExit()
{
    m_d->haltImpl();
}

void KisAnimationPlayer::Private::haltImpl()
{
    if (syncedAudio) {
        syncedAudio->stop();
    }

    q->disconnectCancelSignals();
    timer->stop();
    canvas->setRenderingLimit(QRect());

    if (!canvas->frameCache()) {
        KisImageBarrierLocker locker(canvas->image());

        Q_FOREACH (KisNodeSP node, disabledDecoratedNodes) {
            // we have just upgraded from a weak shared pointer
            KIS_SAFE_ASSERT_RECOVER(node) { continue; }

            KisDecoratedNodeInterface *decoratedNode = dynamic_cast<KisDecoratedNodeInterface*>(node.data());
            KIS_SAFE_ASSERT_RECOVER(decoratedNode) { continue; }

            decoratedNode->setDecorationsVisible(true);
        }
    }
}

bool KisAnimationPlayer::isPlaying()
{
    return m_d->playbackState == PLAYING;
}

bool KisAnimationPlayer::isPaused()
{
    return m_d->playbackState == PAUSED;
}

bool KisAnimationPlayer::isStopped()
{
    return m_d->playbackState == STOPPED;
}

int KisAnimationPlayer::visibleFrame()
{
    return m_d->lastPaintedFrame;
}

void KisAnimationPlayer::displayFrame(int time)
{
    uploadFrame(time, true);
}

void KisAnimationPlayer::slotUpdate()
{
    uploadFrame(-1, false);
}

void KisAnimationPlayer::uploadFrame(int frame, bool forceSyncAudio)
{
    KisImageAnimationInterface *animationInterface = m_d->canvas->image()->animationInterface();
    const int fps = animationInterface->framerate();
    const bool syncToAudio = !forceSyncAudio && m_d->dropFramesMode && m_d->syncedAudio && m_d->syncedAudio->isPlaying();

    if (frame < 0) {
        if (m_d->dropFramesMode) {
            const qreal currentTimeInFrames = syncToAudio ?
                m_d->msecToFrames(m_d->syncedAudio->position(), fps) :
                m_d->playbackTimeInFrames(fps);
            frame = qFloor(currentTimeInFrames);

            const int timeToNextFrame = m_d->framesToWalltime(frame + 1 - currentTimeInFrames, fps);
            m_d->lastTimerInterval = qMax(0, timeToNextFrame);

            if (frame < m_d->currentFrame) {
                // Returned to beginning of animation. Restart audio playback.
                forceSyncAudio = true;
            }
        } else {
            const qint64 currentTime = m_d->playbackTime.elapsed();
            const qint64 framesDiff = currentTime - m_d->nextFrameExpectedTime;

            frame = m_d->incFrame(m_d->currentFrame, 1);
            m_d->nextFrameExpectedTime = currentTime + m_d->expectedInterval;
            m_d->lastTimerInterval = qMax(0.0, m_d->lastTimerInterval - 0.5 * framesDiff);
        }

        m_d->currentFrame = frame;
        m_d->timer->start(m_d->lastTimerInterval);
        m_d->playbackStatisticsCompressor.start();
    }

    if (m_d->syncedAudio && (!syncToAudio || forceSyncAudio)) {
        const int msecTime = m_d->framesToMSec(frame, fps);
        if (isPlaying()) {
            slotSyncScrubbingAudio(msecTime);
        } else {
            m_d->audioSyncScrubbingCompressor->start(msecTime);
        }
    }


    bool useFallbackUploadMethod = !m_d->canvas->frameCache();

    if (m_d->canvas->frameCache() &&
        m_d->canvas->frameCache()->shouldUploadNewFrame(frame, m_d->lastPaintedFrame)) {

        if (m_d->canvas->frameCache()->uploadFrame(frame)) {
            m_d->canvas->updateCanvas();

            m_d->useFastFrameUpload = true;
        } else {
            useFallbackUploadMethod = true;
        }
    }

    if (useFallbackUploadMethod &&
        m_d->canvas->image()->animationInterface()->hasAnimation()) {

        m_d->useFastFrameUpload = false;

        if (m_d->canvas->image()->tryBarrierLock(true)) {
            m_d->canvas->image()->unlock();

            // no OpenGL cache or the frame just not cached yet
            animationInterface->switchCurrentTimeAsync(frame);
        }
    }

    if (!m_d->realFpsTimer.isValid()) {
        m_d->realFpsTimer.start();
    } else {
        const int elapsed = m_d->realFpsTimer.restart();
        m_d->realFpsAccumulator(elapsed);

        if (m_d->lastPaintedFrame >= 0) {
            int numFrames = frame - m_d->lastPaintedFrame;
            if (numFrames < 0) {
                numFrames += m_d->lastFrame - m_d->firstFrame + 1;
            }

            m_d->droppedFramesPortion(qreal(int(numFrames != 1)));

            if (numFrames > 0) {
                m_d->droppedFpsAccumulator(qreal(elapsed) / numFrames);
            }

#ifdef PLAYER_DEBUG_FRAMERATE
            qDebug() << "    RFPS:" << 1000.0 / m_d->realFpsAccumulator.rollingMean()
                     << "DFPS:" << 1000.0 / m_d->droppedFpsAccumulator.rollingMean() << ppVar(numFrames);
#endif /* PLAYER_DEBUG_FRAMERATE */
        }
    }

    m_d->lastPaintedFrame = frame;
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

    KisImageAnimationInterface *animation = m_d->canvas->image()->animationInterface();
    int time = animation->currentUITime();

    if (!keyframes->activeKeyframeAt(time)) {
        return;
    }

    int destinationTime = keyframes->activeKeyframeTime(time);
    while (keyframes->keyframeAt(destinationTime) &&
                (keyframes->keyframeAt(destinationTime) == keyframes->keyframeAt(time) ||
                 !validColors.contains(keyframes->keyframeAt(destinationTime)->colorLabel()))) {
        destinationTime = keyframes->nextKeyframeTime(destinationTime);
    }

    if (keyframes->keyframeAt(destinationTime)) {
        animation->requestTimeSwitchWithUndo(destinationTime);
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

    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();
    int time = animInterface->currentUITime();

    KisKeyframeSP currentKeyframe = keyframes->keyframeAt(time);
    int destinationTime = keyframes->activeKeyframeTime(time);
    while (keyframes->keyframeAt(destinationTime) &&
           (currentKeyframe == keyframes->keyframeAt(destinationTime) || !validColors.contains(keyframes->keyframeAt(destinationTime)->colorLabel()))) {
        destinationTime = keyframes->previousKeyframeTime(destinationTime);
    }

    if (keyframes->keyframeAt(destinationTime)) {
        animInterface->requestTimeSwitchWithUndo(destinationTime);
    }
}

qreal KisAnimationPlayer::effectiveFps() const
{
    return 1000.0 / m_d->droppedFpsAccumulator.rollingMean();
}

qreal KisAnimationPlayer::realFps() const
{
    return 1000.0 / m_d->realFpsAccumulator.rollingMean();
}

qreal KisAnimationPlayer::framesDroppedPortion() const
{
    return m_d->droppedFramesPortion.rollingMean();
}

void KisAnimationPlayer::slotCancelPlayback()
{
    halt();
}

void KisAnimationPlayer::slotCancelPlaybackSafe()
{
    /**
     * If there is no openGL support, then we have no (!) cache at
     * all. Therefore we should regenerate frame on every time switch,
     * which, yeah, can be very slow.  What is more important, when
     * regenerating a frame animation interface will emit a
     * sigStrokeEndRequested() signal and we should ignore it. That is
     * not an ideal solution, because the user will be able to paint
     * on random frames while playing, but it lets users with faulty
     * GPUs see at least some preview of their animation.
     */

    if (m_d->useFastFrameUpload) {
        halt();
    }
}

void KisAnimationPlayer::setPlaybackSpeedPercent(int value) {
    const float normalizedSpeed = value / 100.0;
    setPlaybackSpeedNormalized(normalizedSpeed);
}

qreal KisAnimationPlayer::playbackSpeed()
{
    return m_d->playbackSpeed;
}

void KisAnimationPlayer::setPlaybackSpeedNormalized(double value)
{
    if (m_d->playbackSpeed != value) {
        m_d->playbackSpeed = value;
        if (isPlaying()) {
            slotUpdatePlaybackTimer();
        }
        emit sigPlaybackSpeedChanged(m_d->playbackSpeed);
    }
}
