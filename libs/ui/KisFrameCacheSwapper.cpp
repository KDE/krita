/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
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
#include "KisFrameCacheSwapper.h"

#include "KisFrameCacheStore.h"

#include "kis_update_info.h"
#include "opengl/KisOpenGLUpdateInfoBuilder.h"

struct KisFrameCacheSwapper::Private
{
    Private(const KisOpenGLUpdateInfoBuilder &_builder, const QString &frameCachePath)
        : frameStore(frameCachePath),
          builder(_builder)
    {
    }

    KisFrameCacheStore frameStore;
    const KisOpenGLUpdateInfoBuilder &builder;
};

KisFrameCacheSwapper::KisFrameCacheSwapper(const KisOpenGLUpdateInfoBuilder &builder)
    : KisFrameCacheSwapper(builder, "")
{
}

KisFrameCacheSwapper::KisFrameCacheSwapper(const KisOpenGLUpdateInfoBuilder &builder, const QString &frameCachePath)
    : m_d(new Private(builder, frameCachePath))
{
}

KisFrameCacheSwapper::~KisFrameCacheSwapper()
{
}

void KisFrameCacheSwapper::saveFrame(int frameId, KisOpenGLUpdateInfoSP info, const QRect &imageBounds)
{
    m_d->frameStore.saveFrame(frameId, info, imageBounds);
}

KisOpenGLUpdateInfoSP KisFrameCacheSwapper::loadFrame(int frameId)
{
    return m_d->frameStore.loadFrame(frameId, m_d->builder);
}

void KisFrameCacheSwapper::moveFrame(int srcFrameId, int dstFrameId)
{
    m_d->frameStore.moveFrame(srcFrameId, dstFrameId);
}

void KisFrameCacheSwapper::forgetFrame(int frameId)
{
    m_d->frameStore.forgetFrame(frameId);
}

bool KisFrameCacheSwapper::hasFrame(int frameId) const
{
    return m_d->frameStore.hasFrame(frameId);
}

int KisFrameCacheSwapper::frameLevelOfDetail(int frameId) const
{
    return m_d->frameStore.frameLevelOfDetail(frameId);
}

QRect KisFrameCacheSwapper::frameDirtyRect(int frameId) const
{
    return m_d->frameStore.frameDirtyRect(frameId);
}
