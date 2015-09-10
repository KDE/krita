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

#include "kis_image.h"
#include "kis_canvas2.h"
#include "kis_animation_frame_cache.h"
#include "kis_signal_auto_connection.h"
#include "kis_image_animation_interface.h"
#include "kis_time_range.h"

struct KisAnimationPlayer::Private
{
public:
    bool playing;
    int currentFrame;

    QTimer *timer;
    QImage frame;

    int firstFrame;
    int lastFrame;
    float fps;

    KisCanvas2 *canvas;

    QVector<KisSignalAutoConnectionSP> cancelStrokeConnections;
};

KisAnimationPlayer::KisAnimationPlayer(KisCanvas2 *canvas)
    : m_d(new Private())
{
    m_d->playing = false;
    m_d->fps = 15;
    m_d->canvas = canvas;

    m_d->timer = new QTimer(this);

    connect(m_d->timer, SIGNAL(timeout()), this, SLOT(slotUpdate()));
}

KisAnimationPlayer::~KisAnimationPlayer()
{}

void KisAnimationPlayer::connectCancelSignals()
{
    m_d->cancelStrokeConnections.append(
        toQShared(new KisSignalAutoConnection(
                      m_d->canvas->image().data(), SIGNAL(sigUndoDuringStrokeRequested()),
                      this, SLOT(slotCancelPlayback()))));

    m_d->cancelStrokeConnections.append(
        toQShared(new KisSignalAutoConnection(
                      m_d->canvas->image().data(), SIGNAL(sigStrokeCancellationRequested()),
                      this, SLOT(slotCancelPlayback()))));

    m_d->cancelStrokeConnections.append(
        toQShared(new KisSignalAutoConnection(
                      m_d->canvas->image().data(), SIGNAL(sigStrokeEndRequested()),
                      this, SLOT(slotCancelPlayback()))));
}

void KisAnimationPlayer::disconnectCancelSignals()
{
    m_d->cancelStrokeConnections.clear();
}

void KisAnimationPlayer::play()
{
    const KisTimeRange &range = m_d->canvas->image()->animationInterface()->currentRange();
    if (!range.isValid()) return;

    m_d->fps = m_d->canvas->image()->animationInterface()->framerate();
    m_d->firstFrame = range.start();
    m_d->lastFrame = range.end();
    m_d->currentFrame = m_d->firstFrame;

    m_d->playing = true;
    m_d->timer->start(1000 / m_d->fps);

    connectCancelSignals();
}

void KisAnimationPlayer::stop()
{
    disconnectCancelSignals();

    m_d->timer->stop();
    m_d->playing = false;

    m_d->canvas->refetchDataFromImage();

    emit sigPlaybackStopped();
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

void KisAnimationPlayer::uploadFrame(int frame)
{
    if (m_d->canvas->frameCache()->uploadFrame(frame)) {
        m_d->canvas->updateCanvas();
        emit sigFrameChanged();
    }
}

void KisAnimationPlayer::slotCancelPlayback()
{
    stop();
}
