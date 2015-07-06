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

#include "kis_animation_frame_cache.h"

#include <QMap>

#include "kis_image.h"
#include "kis_image_animation_interface.h"

struct KisAnimationFrameCache::Private
{
    QMap<int, QImage> frames;
    KisImage *image;

    Private(KisImage *image)
        : image(image)
    {}
};

KisAnimationFrameCache::KisAnimationFrameCache(KisImage *image, KisImageAnimationInterface *interface)
    : m_d(new Private(image))
{
    // Note: we can't get the animation interface through image, since it's not fully initialized yet.

    connect(interface, SIGNAL(sigFramesChanged(KisTimeRange,QRect)), this, SLOT(framesChanged(KisTimeRange,QRect)));
    connect(interface, SIGNAL(sigFrameReady()), this, SLOT(frameReady()), Qt::DirectConnection);
}

KisAnimationFrameCache::~KisAnimationFrameCache()
{}

QImage KisAnimationFrameCache::getFrame(int time)
{
    if (!m_d->frames.contains(time)) {
        m_d->image->animationInterface()->requestFrameRegeneration(time, m_d->image->bounds());
    }

    return m_d->frames.value(time);
}

void KisAnimationFrameCache::framesChanged(const KisTimeRange &range, const QRect &rect)
{
    if (!range.isValid()) return;

    int end = range.isInfinite() ?
        (m_d->frames.constEnd().key()) : // TODO: better way to determine the "last" frame?
        (range.end());

    for (int t=range.start(); t <= end; t++) {
        // TODO: invalidate
        m_d->frames.remove(t);
    }
}

void KisAnimationFrameCache::frameReady()
{
    QImage projection = m_d->image->animationInterface()->frameProjection()->convertToQImage(m_d->image->profile(), m_d->image->bounds());
    m_d->frames.insert(m_d->image->animationInterface()->currentTime(), projection);
}
