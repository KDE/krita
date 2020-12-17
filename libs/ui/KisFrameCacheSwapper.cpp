/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
