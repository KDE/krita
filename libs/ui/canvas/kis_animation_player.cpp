/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
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
#include "kis_time_range.h"
#include "kis_signal_compressor.h"
#include <KisDocument.h>
#include <QFileInfo>
#include "KisSyncedAudioPlayback.h"
#include "kis_signal_compressor_with_param.h"

#include "kis_image_config.h"
#include <limits>

#include "KisViewManager.h"
#include "kis_icon_utils.h"

#include "KisPart.h"
#include "dialogs/KisAsyncAnimationCacheRenderDialog.h"
#include "KisRollingMeanAccumulatorWrapper.h"


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
          expectedFrame(0),
          lastTimerInterval(0),
          lastPaintedFrame(0),
          playbackStatisticsCompressor(1000, KisSignalCompressor::FIRST_INACTIVE),
          stopAudioOnScrubbingCompressor(100, KisSignalCompressor::POSTPONE),
          audioOffsetTolerance(-1)
          {}

    KisAnimationPlayer *q;

    bool useFastFrameUpload;
    bool playing;

    QTimer *timer;

    int initialFrame;
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

    QElapsedTimer playbackTime;
    int nextFrameExpectedTime;
    int expectedInterval;
    int expectedFrame;
    int lastTimerInterval;
    int lastPaintedFrame;

    KisSignalCompressor playbackStatisticsCompressor;

    QScopedPointer<KisSyncedAudioPlayback> syncedAudio;
    QScopedPointer<KisSignalCompressorWithParam<int> > audioSyncScrubbingCompressor;
    KisSignalCompressor stopAudioOnScrubbingCompressor;

    int audioOffsetTolerance;

    void stopImpl(bool doUpdates);

    int incFrame(int frame, int inc) {
        frame += inc;
        if (frame > lastFrame) {
            frame = firstFrame + frame - lastFrame - 1;
        }
        return frame;
    }

    qint64 frameToMSec(int value, int fps) {
        return qreal(value) / fps * 1000.0;
    }
    int msecToFrame(qint64 value, int fps) {
        return qreal(value) * fps / 1000.0;
    }
};

KisAnimationPlayer::KisAnimationPlayer(KisCanvas2 *canvas)
    : QObject(canvas)
    , m_d(new Private(this))
{
    m_d->useFastFrameUpload = false;
    m_d->playing = false;
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
}

void KisAnimationPlayer::slotUpdatePlaybackTimer()
{
    m_d->timer->stop();

    const KisImageAnimationInterface *animation = m_d->canvas->image()->animationInterface();
    const KisTimeSpan &playBackRange = animation->playbackRange();
    if (playBackRange.isEmpty()) return;

    const int fps = animation->framerate();

    m_d->initialFrame = animation->currentUITime();
    m_d->firstFrame = playBackRange.start();
    m_d->lastFrame = playBackRange.end();
    m_d->expectedFrame = qBound(m_d->firstFrame, m_d->expectedFrame, m_d->lastFrame);


    m_d->expectedInterval = qreal(1000) / fps / m_d->playbackSpeed;
    m_d->lastTimerInterval = m_d->expectedInterval;

    if (m_d->syncedAudio) {
        m_d->syncedAudio->setSpeed(m_d->playbackSpeed);
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
    {
        const KisImageAnimationInterface *animation = m_d->canvas->image()->animationInterface();
        const KisTimeSpan &range = animation->playbackRange();
        if (range.isEmpty()) return;

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
        }
    }

    m_d->playing = true;

    slotUpdatePlaybackTimer();
    m_d->expectedFrame = m_d->firstFrame;
    m_d->lastPaintedFrame = -1;

    connectCancelSignals();

    if (m_d->syncedAudio) {
        KisImageAnimationInterface *animationInterface = m_d->canvas->image()->animationInterface();
        m_d->syncedAudio->play(m_d->frameToMSec(m_d->firstFrame, animationInterface->framerate()));
    }

    emit sigPlaybackStarted();
}

void KisAnimationPlayer::Private::stopImpl(bool doUpdates)
{
    if (syncedAudio) {
        syncedAudio->stop();
    }

    q->disconnectCancelSignals();

    timer->stop();
    playing = false;
    canvas->setRenderingLimit(QRect());

    if (doUpdates) {
        KisImageAnimationInterface *animation = canvas->image()->animationInterface();
        if (animation->currentUITime() == initialFrame) {
            canvas->refetchDataFromImage();
        } else {
            animation->switchCurrentTimeAsync(initialFrame);
        }
    }

    emit q->sigPlaybackStopped();
}

void KisAnimationPlayer::stop()
{
    m_d->stopImpl(true);
}

void KisAnimationPlayer::forcedStopOnExit()
{
    m_d->stopImpl(false);
}

bool KisAnimationPlayer::isPlaying()
{
    return m_d->playing;
}

int KisAnimationPlayer::currentTime()
{
    return m_d->lastPaintedFrame;
}

void KisAnimationPlayer::displayFrame(int time)
{
    uploadFrame(time);
}

void KisAnimationPlayer::slotUpdate()
{
    uploadFrame(-1);
}

void KisAnimationPlayer::uploadFrame(int frame)
{
    KisImageAnimationInterface *animationInterface = m_d->canvas->image()->animationInterface();

    if (frame < 0) {
        const int currentTime = m_d->playbackTime.elapsed();
        const int framesDiff = currentTime - m_d->nextFrameExpectedTime;
        const qreal framesDiffNorm = qreal(framesDiff) / m_d->expectedInterval;

        // qDebug() << ppVar(framesDiff)
        //          << ppVar(m_d->expectedFrame)
        //          << ppVar(framesDiffNorm)
        //          << ppVar(m_d->lastTimerInterval);

        if (m_d->dropFramesMode) {
            const int numDroppedFrames = qMax(0, qRound(framesDiffNorm));
            frame = m_d->incFrame(m_d->expectedFrame, numDroppedFrames);
        } else {
            frame = m_d->expectedFrame;
        }

        m_d->nextFrameExpectedTime = currentTime + m_d->expectedInterval;

        m_d->lastTimerInterval = qMax(0.0, m_d->lastTimerInterval - 0.5 * framesDiff);
        m_d->expectedFrame = m_d->incFrame(frame,  1);

        m_d->timer->start(m_d->lastTimerInterval);

        m_d->playbackStatisticsCompressor.start();
    }

    if (m_d->syncedAudio) {
        const int msecTime = m_d->frameToMSec(frame, animationInterface->framerate());
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

    if (useFallbackUploadMethod) {
        m_d->useFastFrameUpload = false;

        m_d->canvas->image()->barrierLock(true);
        m_d->canvas->image()->unlock();

        // no OpenGL cache or the frame just not cached yet
        animationInterface->switchCurrentTimeAsync(frame);
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
    stop();
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
        stop();
    }
}

qreal KisAnimationPlayer::playbackSpeed()
{
    return m_d->playbackSpeed;
}

void KisAnimationPlayer::slotUpdatePlaybackSpeed(double value)
{
    m_d->playbackSpeed = value;
    if (m_d->playing) {
        slotUpdatePlaybackTimer();
    }
}
