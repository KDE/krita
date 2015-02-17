/*
 *  Copyright (c) 2015 Jouni Pentikäinen <mctyyppi42@gmail.com>
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
#include "kis_canvas2.h"

#include <QTimer>
#include <QImage>

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
};

KisAnimationPlayer::KisAnimationPlayer(KisCanvas2 *canvas, KisCoordinatesConverter *coordinatesConverter, QWidget *parent)
    : KisQPainterCanvas(canvas, coordinatesConverter, parent)
    , m_d(new Private())
{
    m_d->playing = false;
    m_d->fps = 15;

    m_d->timer = new QTimer(this);

    connect(m_d->timer, SIGNAL(timeout()), this, SLOT(slotUpdate()));
}

KisAnimationPlayer::~KisAnimationPlayer()
{
    delete m_d;
}

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
}

bool KisAnimationPlayer::isPlaying()
{
    return m_d->playing;
}

void KisAnimationPlayer::slotUpdate()
{
    m_d->frame = canvas()->image()->getRenderedFrame(m_d->currentFrame);
    m_d->currentFrame++;

    if (m_d->currentFrame > m_d->lastFrame) m_d->currentFrame = m_d->firstFrame;

    update();
}

void KisAnimationPlayer::drawImage(QPainter &gc, const QRect &updateWidgetRect) const
{
    KisCoordinatesConverter *converter = coordinatesConverter();

    QTransform imageTransform = converter->viewportToWidgetTransform();
    gc.setTransform(imageTransform);
    gc.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QRectF viewportRect = converter->widgetToViewport(updateWidgetRect);
    QRectF imageRect = converter->widgetToImage(updateWidgetRect);

    gc.setCompositionMode(QPainter::CompositionMode_SourceOver);
    gc.drawImage(viewportRect, m_d->frame, imageRect);
}

void KisAnimationPlayer::resizeEvent(QResizeEvent *e)
{
    QSize size(e->size());
    if (size.width() <= 0) {
        size.setWidth(1);
    }
    if (size.height() <= 0) {
        size.setHeight(1);
    }
}
