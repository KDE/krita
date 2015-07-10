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

#include "kis_canvas2.h"
#include "kis_animation_frame_cache.h"


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

void KisAnimationPlayer::setFramerate(float fps) {
    m_d->fps = fps;
}

void KisAnimationPlayer::setRange(int firstFrame, int lastFrame)
{
    m_d->firstFrame = firstFrame;
    m_d->lastFrame = lastFrame;
}

void KisAnimationPlayer::play()
{
    m_d->playing = true;
    m_d->currentFrame = m_d->firstFrame;
    m_d->timer->start(1000 / m_d->fps);
}

void KisAnimationPlayer::stop()
{
    m_d->timer->stop();
    m_d->playing = false;

    m_d->canvas->refetchDataFromImage();
}

bool KisAnimationPlayer::isPlaying()
{
    return m_d->playing;
}

void KisAnimationPlayer::slotUpdate()
{
    if (m_d->canvas->frameCache()->uploadFrame(m_d->currentFrame)) {
        m_d->canvas->updateCanvas();
    }

    m_d->currentFrame++;
    if (m_d->currentFrame > m_d->lastFrame) m_d->currentFrame = m_d->firstFrame;
}
