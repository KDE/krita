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

#include "kis_config.h"
#include "kis_config_notifier.h"
#include "kis_image.h"
#include "kis_canvas2.h"
#include "kis_animation_frame_cache.h"
#include "kis_signal_auto_connection.h"
#include "kis_image_animation_interface.h"
#include "kis_time_range.h"
#include "kis_signal_compressor.h"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/rolling_mean.hpp>

using namespace boost::accumulators;
typedef accumulator_set<qreal, stats<tag::rolling_mean> > FpsAccumulator;


struct KisAnimationPlayer::Private
{
public:
    Private(KisAnimationPlayer *_q)
        : q(_q),
          realFpsAccumulator(tag::rolling_window::window_size = 24),
          droppedFpsAccumulator(tag::rolling_window::window_size = 24),
          droppedFramesPortion(tag::rolling_window::window_size = 24),
          dropFramesMode(true),
          nextFrameExpectedTime(0),
          expectedInterval(0),
          expectedFrame(0),
          lastTimerInterval(0),
          lastPaintedFrame(0),
          playbackStatisticsCompressor(1000, KisSignalCompressor::FIRST_INACTIVE)
          {}

    KisAnimationPlayer *q;

    bool useFastFrameUpload;
    bool playing;

    QTimer *timer;

    int firstFrame;
    int lastFrame;
    int fps;
    qreal playbackSpeed;

    KisCanvas2 *canvas;

    KisSignalAutoConnectionsStore cancelStrokeConnections;

    QElapsedTimer realFpsTimer;
    FpsAccumulator realFpsAccumulator;
    FpsAccumulator droppedFpsAccumulator;
    FpsAccumulator droppedFramesPortion;


    bool dropFramesMode;

    QElapsedTimer playbackTime;
    int nextFrameExpectedTime;
    int expectedInterval;
    int expectedFrame;
    int lastTimerInterval;
    int lastPaintedFrame;

    KisSignalCompressor playbackStatisticsCompressor;

    void stopImpl(bool doUpdates);

    int incFrame(int frame, int inc) {
        frame += inc;
        if (frame > lastFrame) {
            frame = firstFrame + frame - lastFrame - 1;
        }
        return frame;
    }
};

KisAnimationPlayer::KisAnimationPlayer(KisCanvas2 *canvas)
    : m_d(new Private(this))
{
    m_d->useFastFrameUpload = false;
    m_d->playing = false;
    m_d->fps = 15;
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
}

KisAnimationPlayer::~KisAnimationPlayer()
{}

void KisAnimationPlayer::slotUpdateDropFramesMode()
{
    KisConfig cfg;
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

void KisAnimationPlayer::slotUpdatePlaybackTimer()
{
    m_d->timer->stop();

    const KisImageAnimationInterface *animation = m_d->canvas->image()->animationInterface();
    const KisTimeRange &range = animation->playbackRange();
    if (!range.isValid()) return;

    m_d->fps = animation->framerate();
    m_d->firstFrame = range.start();
    m_d->lastFrame = range.end();
    m_d->expectedFrame = qBound(m_d->firstFrame, m_d->expectedFrame, m_d->lastFrame);

    m_d->expectedInterval = qreal(1000) / m_d->fps / m_d->playbackSpeed;
    m_d->lastTimerInterval = m_d->expectedInterval;
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
    m_d->playing = true;

    slotUpdatePlaybackTimer();
    m_d->expectedFrame = m_d->firstFrame;
    m_d->lastPaintedFrame = m_d->firstFrame;

    connectCancelSignals();
}

void KisAnimationPlayer::Private::stopImpl(bool doUpdates)
{
    q->disconnectCancelSignals();

    timer->stop();
    playing = false;

    if (doUpdates) {
        canvas->refetchDataFromImage();
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

    if (m_d->canvas->frameCache()) {
        if (m_d->canvas->frameCache()->uploadFrame(frame)) {
            m_d->canvas->updateCanvas();

            m_d->useFastFrameUpload = true;
            emit sigFrameChanged();
        }
    } else {
        qWarning() << "WARNING: Animation playback can be very slow without openGL support!";
        m_d->canvas->image()->animationInterface()->switchCurrentTimeAsync(frame);

        m_d->useFastFrameUpload = false;
        emit sigFrameChanged();
    }

    if (!m_d->realFpsTimer.isValid()) {
        m_d->realFpsTimer.start();
    } else {
        const int elapsed = m_d->realFpsTimer.restart();
        m_d->realFpsAccumulator(elapsed);

        int numFrames = frame - m_d->lastPaintedFrame;
        if (numFrames < 0) {
            numFrames += m_d->lastFrame - m_d->firstFrame + 1;
        }

        m_d->droppedFramesPortion(qreal(int(numFrames != 1)));

        if (numFrames > 0) {
            m_d->droppedFpsAccumulator(qreal(elapsed) / numFrames);
        }

#ifdef PLAYER_DEBUG_FRAMERATE
        qDebug() << "    RFPS:" << 1000.0 / rolling_mean(m_d->realFpsAccumulator)
                 << "DFPS:" << 1000.0 / rolling_mean(m_d->droppedFpsAccumulator) << ppVar(numFrames);
#endif /* PLAYER_DEBUG_FRAMERATE */
    }

    m_d->lastPaintedFrame = frame;

}

qreal KisAnimationPlayer::effectiveFps() const
{
    return 1000.0 / rolling_mean(m_d->droppedFpsAccumulator);
}

qreal KisAnimationPlayer::realFps() const
{
    return 1000.0 / rolling_mean(m_d->realFpsAccumulator);
}

qreal KisAnimationPlayer::framesDroppedPortion() const
{
    return rolling_mean(m_d->droppedFramesPortion);
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
