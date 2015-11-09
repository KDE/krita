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

#include <QTimer>

//#define DEBUG_FRAMERATE

#ifdef DEBUG_FRAMERATE
#include <QTime>
#endif /* DEBUG_FRAMERATE */

#include "kis_global.h"


#include "kis_image.h"
#include "kis_canvas2.h"
#include "kis_animation_frame_cache.h"
#include "kis_signal_auto_connection.h"
#include "kis_image_animation_interface.h"
#include "kis_time_range.h"

struct KisAnimationPlayer::Private
{
public:
    Private(KisAnimationPlayer *_q) : q(_q) {}

    KisAnimationPlayer *q;

    bool useFastFrameUpload;
    bool playing;
    int currentFrame;

    QTimer *timer;
    QImage frame;

    int firstFrame;
    int lastFrame;
    int fps;
    qreal playbackSpeed;

    KisCanvas2 *canvas;

    KisSignalAutoConnectionsStore cancelStrokeConnections;

#ifdef DEBUG_FRAMERATE
    QTime frameRateTimer;
#endif /* DEBUG_FRAMERATE */

    void stopImpl(bool doUpdates);
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
}

KisAnimationPlayer::~KisAnimationPlayer()
{}

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
        m_d->canvas->image()->animationInterface(), SIGNAL(sigRangeChanged()),
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
    const KisTimeRange &range = animation->currentRange();
    if (!range.isValid()) return;

    m_d->fps = animation->framerate();
    m_d->firstFrame = range.start();
    m_d->lastFrame = range.end();
    m_d->currentFrame = qBound(m_d->firstFrame, m_d->currentFrame, m_d->lastFrame);

    m_d->timer->start(qreal(1000) / m_d->fps / m_d->playbackSpeed);
}

void KisAnimationPlayer::play()
{
    m_d->playing = true;

    slotUpdatePlaybackTimer();
    m_d->currentFrame = m_d->firstFrame;

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
    return m_d->currentFrame;
}

void KisAnimationPlayer::displayFrame(int time)
{
    uploadFrame(time);
}

void KisAnimationPlayer::slotUpdate()
{
    m_d->currentFrame++;
    if (m_d->currentFrame > m_d->lastFrame) m_d->currentFrame = m_d->firstFrame;

    uploadFrame(m_d->currentFrame);
}

#ifdef DEBUG_FRAMERATE
#include "../../sdk/tests/testutil.h"
static TestUtil::MeasureAvgPortion C(25);
#endif /* DEBUG_FRAMERATE */

void KisAnimationPlayer::uploadFrame(int frame)
{
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

#ifdef DEBUG_FRAMERATE
    if (!m_d->frameRateTimer.isValid()) {
        m_d->frameRateTimer.start();
    } else {
        const int elapsed = m_d->frameRateTimer.restart();
        C.addTotal(1000 / elapsed);
    }
#endif /* DEBUG_FRAMERATE */
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
