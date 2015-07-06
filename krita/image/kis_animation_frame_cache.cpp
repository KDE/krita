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

struct KisAnimationFrameCache::Private
{
    QMap<int, QImage> frames;
};

KisAnimationFrameCache::KisAnimationFrameCache()
    : m_d(new Private())
{}

KisAnimationFrameCache::~KisAnimationFrameCache()
{}

QImage KisAnimationFrameCache::getFrame(int time)
{
    return m_d->frames.value(time);
}

void KisAnimationFrameCache::cacheFrame(int time, QImage frame)
{
    m_d->frames.insert(time, frame);
}
